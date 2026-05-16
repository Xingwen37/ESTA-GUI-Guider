import { useState, useEffect, useRef, useCallback } from "react";
import type { MenuProfile, MenuItemProfile } from "../lib/types";
import {
  MAX_MENU_ITEMS,
  MAX_MENU_STRING_LEN,
  MAX_MENU_DEPTH,
  MENU_THEME_OPTIONS,
  UI_FONT_SIZE_OPTIONS,
} from "../lib/types";

// --- Tree data model ---

interface TreeNode {
  id: string;
  label: string;
  is_submenu: boolean;
  event_id: number;
  children: TreeNode[];
  expanded: boolean;
}

function spin(value: number, min: number, max: number, onChange: (v: number) => void) {
  const [text, setText] = useState(String(value));
  const prev = useRef(value);
  if (prev.current !== value) { prev.current = value; setText(String(value)); }
  return (
    <input type="number" value={text} min={min} max={max}
      onChange={(e) => {
        setText(e.target.value);
        const v = parseInt(e.target.value, 10);
        if (!isNaN(v)) { prev.current = v; onChange(v); }
      }}
      onBlur={() => setText(String(value))} />
  );
}

let _nextId = 1;
function genId(): string {
  return "n" + _nextId++;
}

function flatToTree(items: MenuItemProfile[]): TreeNode[] {
  const nodes: TreeNode[] = items.map((item) => ({
    id: genId(),
    label: item.label,
    is_submenu: item.is_submenu,
    event_id: item.event_id,
    children: [],
    expanded: true,
  }));

  const roots: TreeNode[] = [];
  for (let i = 0; i < items.length; i++) {
    const parentIdx = items[i].parent_idx;
    if (parentIdx === 255 || parentIdx >= items.length) {
      roots.push(nodes[i]);
    } else {
      nodes[parentIdx].children.push(nodes[i]);
    }
  }
  return roots;
}

function treeToFlat(roots: TreeNode[]): MenuItemProfile[] {
  const result: MenuItemProfile[] = [];

  function walk(node: TreeNode, parentIdx: number) {
    const myIdx = result.length;
    result.push({
      label: node.label,
      parent_idx: parentIdx,
      is_submenu: node.is_submenu,
      event_id: node.event_id,
    });
    for (const child of node.children) {
      walk(child, myIdx);
    }
  }

  for (const root of roots) {
    walk(root, 255);
  }
  return result;
}

function countNodes(roots: TreeNode[]): number {
  let count = 0;
  function walk(node: TreeNode) {
    count++;
    for (const child of node.children) walk(child);
  }
  for (const root of roots) walk(root);
  return count;
}

function getDepth(roots: TreeNode[], targetId: string): number {
  function walk(nodes: TreeNode[], depth: number): number {
    for (const node of nodes) {
      if (node.id === targetId) return depth;
      const found = walk(node.children, depth + 1);
      if (found >= 0) return found;
    }
    return -1;
  }
  return walk(roots, 0);
}

function findNode(roots: TreeNode[], id: string): TreeNode | null {
  for (const root of roots) {
    if (root.id === id) return root;
    const found = findNode(root.children, id);
    if (found) return found;
  }
  return null;
}

function cloneTree(roots: TreeNode[]): TreeNode[] {
  return roots.map((n) => ({ ...n, children: cloneTree(n.children) }));
}

// --- Font metrics (mirrors core/infra/ui_base.c) ---

const FONT_METRICS: Record<string, { w: number; h: number }> = {
  "ESTA_FONT_1206": { w: 6, h: 12 },
  "ESTA_FONT_1608": { w: 8, h: 16 },
  "ESTA_FONT_2412": { w: 12, h: 24 },
};

// --- Tree analysis for auto-calc ---

function analyzeTree(roots: TreeNode[]): { maxLevelItems: number; maxLabelLen: number } {
  let maxLevelItems = roots.length;
  let maxLabelLen = 0;

  function walk(nodes: TreeNode[]) {
    for (const node of nodes) {
      maxLabelLen = Math.max(maxLabelLen, node.label.length);
      if (node.is_submenu && node.children.length > 0) {
        maxLevelItems = Math.max(maxLevelItems, node.children.length);
        walk(node.children);
      }
    }
  }
  walk(roots);
  return { maxLevelItems: Math.max(maxLevelItems, 1), maxLabelLen: Math.max(maxLabelLen, 1) };
}

// --- Context menu ---

interface ContextMenuState {
  x: number;
  y: number;
  nodeId: string;
}

function ContextMenu({
  state,
  roots,
  onAction,
  onClose,
}: {
  state: ContextMenuState;
  roots: TreeNode[];
  onAction: (action: string) => void;
  onClose: () => void;
}) {
  const ref = useRef<HTMLDivElement>(null);
  const node = findNode(roots, state.nodeId);

  useEffect(() => {
    const handler = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) onClose();
    };
    document.addEventListener("mousedown", handler);
    return () => document.removeEventListener("mousedown", handler);
  }, [onClose]);

  if (!node) return null;

  const depth = getDepth(roots, state.nodeId);
  const canAddChild = node.is_submenu && depth < MAX_MENU_DEPTH - 1;
  const totalCount = countNodes(roots);
  const canAdd = totalCount < MAX_MENU_ITEMS;

  return (
    <div ref={ref} className="menu-ctx" style={{ left: state.x, top: state.y }}>
      {canAddChild && canAdd && (
        <div className="menu-ctx-item" onClick={() => onAction("add-child")}>
          添加子节点
        </div>
      )}
      {canAdd && (
        <div className="menu-ctx-item" onClick={() => onAction("add-sibling")}>
          添加同级节点
        </div>
      )}
      <div className="menu-ctx-sep" />
      {node.is_submenu ? (
        <div
          className={`menu-ctx-item${node.children.length > 0 ? " disabled" : ""}`}
          onClick={() => node.children.length === 0 && onAction("to-leaf")}
        >
          转为叶子节点{node.children.length > 0 ? "（有子节点）" : ""}
        </div>
      ) : (
        <div className="menu-ctx-item" onClick={() => onAction("to-submenu")}>
          转为子菜单
        </div>
      )}
      <div className="menu-ctx-sep" />
      <div className="menu-ctx-item danger" onClick={() => onAction("delete")}>
        删除
      </div>
    </div>
  );
}

// --- Tree node rendering ---

function TreeNodeRow({
  node,
  depth,
  selectedId,
  onSelect,
  onToggle,
  onContextMenu,
}: {
  node: TreeNode;
  depth: number;
  selectedId: string | null;
  onSelect: (id: string) => void;
  onToggle: (id: string) => void;
  onContextMenu: (e: React.MouseEvent, id: string) => void;
}) {
  const isSelected = node.id === selectedId;

  return (
    <>
      <div
        className={`tree-row${isSelected ? " selected" : ""}`}
        style={{ paddingLeft: 8 + depth * 20 }}
        onClick={() => onSelect(node.id)}
        onContextMenu={(e) => {
          e.preventDefault();
          onSelect(node.id);
          onContextMenu(e, node.id);
        }}
      >
        {node.is_submenu ? (
          <span
            className="tree-arrow"
            onClick={(e) => { e.stopPropagation(); onToggle(node.id); }}
          >
            {node.expanded ? "▼" : "▶"}
          </span>
        ) : (
          <span className="tree-arrow-placeholder" />
        )}
        <span className="tree-label">{node.label || "(empty)"}</span>
        {node.is_submenu && (
          <span className="tree-badge">submenu</span>
        )}
      </div>
      {node.is_submenu && node.expanded && node.children.map((child) => (
        <TreeNodeRow
          key={child.id}
          node={child}
          depth={depth + 1}
          selectedId={selectedId}
          onSelect={onSelect}
          onToggle={onToggle}
          onContextMenu={onContextMenu}
        />
      ))}
    </>
  );
}

// --- Properties panel ---

function PropertiesPanel({
  node,
  onUpdate,
}: {
  node: TreeNode;
  onUpdate: (patch: Partial<TreeNode>) => void;
}) {
  return (
    <div className="menu-props">
      <div className="menu-props-title">节点属性</div>
      <div className="form-row">
        <label>Label</label>
        <input
          type="text"
          value={node.label}
          maxLength={MAX_MENU_STRING_LEN}
          onChange={(e) => onUpdate({ label: e.target.value.slice(0, MAX_MENU_STRING_LEN) })}
        />
      </div>
      <div className="form-row">
        <label>类型</label>
        <div className="radio-group">
          <label className="radio-label">
            <input
              type="radio"
              checked={node.is_submenu}
              onChange={() => onUpdate({ is_submenu: true })}
              disabled={!node.is_submenu && false}
            />
            子菜单
          </label>
          <label className="radio-label">
            <input
              type="radio"
              checked={!node.is_submenu}
              onChange={() => {
                if (node.children.length === 0) onUpdate({ is_submenu: false });
              }}
              disabled={node.is_submenu && node.children.length > 0}
            />
            叶子节点
          </label>
        </div>
      </div>
      {!node.is_submenu && (
        <div className="form-row">
          <label>Event ID</label>
          <input
            type="number"
            value={node.event_id}
            min={0}
            max={255}
            onChange={(e) => { const v = parseInt(e.target.value, 10); if (!isNaN(v)) onUpdate({ event_id: Math.max(0, Math.min(255, v)) }); }}
          />
        </div>
      )}
    </div>
  );
}

// --- Main component ---

interface Props {
  profile: MenuProfile;
  onChange: (p: MenuProfile) => void;
}

export default function MenuEditor({ profile, onChange }: Props) {
  const [roots, setRoots] = useState<TreeNode[]>(() => flatToTree(profile.items.slice(0, profile.item_count)));
  const [selectedId, setSelectedId] = useState<string | null>(null);
  const [ctxMenu, setCtxMenu] = useState<ContextMenuState | null>(null);

  const internalUpdate = useRef(false);
  useEffect(() => {
    if (internalUpdate.current) {
      internalUpdate.current = false;
      return;
    }
    setRoots(flatToTree(profile.items.slice(0, profile.item_count)));
    setSelectedId(null);
  }, [profile]);

  const syncToParent = useCallback((newRoots: TreeNode[]) => {
    const flat = treeToFlat(newRoots);
    internalUpdate.current = true;
    onChange({
      ...profile,
      item_count: flat.length,
      items: flat,
    });
  }, [profile, onChange]);

  const updateRoots = useCallback((newRoots: TreeNode[]) => {
    setRoots(newRoots);
    syncToParent(newRoots);
  }, [syncToParent]);

  const handleToggle = (id: string) => {
    const next = cloneTree(roots);
    const node = findNode(next, id);
    if (node) node.expanded = !node.expanded;
    setRoots(next);
  };

  const handleContextMenu = (e: React.MouseEvent, id: string) => {
    setCtxMenu({ x: e.clientX, y: e.clientY, nodeId: id });
  };

  const handleCtxAction = (action: string) => {
    if (!ctxMenu) return;
    const next = cloneTree(roots);
    const node = findNode(next, ctxMenu.nodeId);
    if (!node) { setCtxMenu(null); return; }

    switch (action) {
      case "add-child": {
        const child: TreeNode = {
          id: genId(), label: "New Item", is_submenu: false,
          event_id: 0, children: [], expanded: true,
        };
        node.children.push(child);
        node.expanded = true;
        updateRoots(next);
        setSelectedId(child.id);
        break;
      }
      case "add-sibling": {
        const sibling: TreeNode = {
          id: genId(), label: "New Item", is_submenu: false,
          event_id: 0, children: [], expanded: true,
        };
        const parent = findParent(next, ctxMenu.nodeId);
        const list = parent ? parent.children : next;
        const idx = list.findIndex((n) => n.id === ctxMenu.nodeId);
        list.splice(idx + 1, 0, sibling);
        updateRoots(parent ? next : [...list]);
        setSelectedId(sibling.id);
        break;
      }
      case "delete": {
        const parent = findParent(next, ctxMenu.nodeId);
        const list = parent ? parent.children : next;
        const idx = list.findIndex((n) => n.id === ctxMenu.nodeId);
        if (idx >= 0) list.splice(idx, 1);
        if (selectedId === ctxMenu.nodeId) setSelectedId(null);
        updateRoots(parent ? next : [...list]);
        break;
      }
      case "to-leaf":
        node.is_submenu = false;
        updateRoots(next);
        break;
      case "to-submenu":
        node.is_submenu = true;
        updateRoots(next);
        break;
    }
    setCtxMenu(null);
  };

  const handleNodeUpdate = (patch: Partial<TreeNode>) => {
    if (!selectedId) return;
    const next = cloneTree(roots);
    const node = findNode(next, selectedId);
    if (node) Object.assign(node, patch);
    updateRoots(next);
  };

  const handleAddRoot = () => {
    if (countNodes(roots) >= MAX_MENU_ITEMS) return;
    const node: TreeNode = {
      id: genId(), label: "New Item", is_submenu: false,
      event_id: 0, children: [], expanded: true,
    };
    const next = [...cloneTree(roots), node];
    updateRoots(next);
    setSelectedId(node.id);
  };

  const handleAutoCalc = () => {
    const metrics = FONT_METRICS[profile.font_size] ?? FONT_METRICS["ESTA_FONT_1608"];
    const { maxLevelItems, maxLabelLen } = analyzeTree(roots);

    const item_height = metrics.h + 4;
    const breadcrumb_height = profile.is_show_breadcrumb ? (metrics.h + 4) : 0;
    const x_width = maxLabelLen * metrics.w + 2 + 12 + 4;
    const y_width = breadcrumb_height + maxLevelItems * item_height;

    onChange({
      ...profile,
      item_height,
      breadcrumb_height,
      x_width,
      y_width,
    });
  };

  const selectedNode = selectedId ? findNode(roots, selectedId) : null;

  const set = (key: keyof MenuProfile, value: unknown) =>
    onChange({ ...profile, [key]: value });

  return (
    <div>
      {/* Position / Layout / Display settings */}
      <fieldset className="group-box">
        <legend>Position & Layout</legend>
        <div className="form-row"><label>x_origin</label>{spin(profile.x_origin, 0, 65535, (v) => set("x_origin", v))}</div>
        <div className="form-row"><label>y_origin</label>{spin(profile.y_origin, 0, 65535, (v) => set("y_origin", v))}</div>
        <div className="form-row"><label>x_width</label>{spin(profile.x_width, 1, 65535, (v) => set("x_width", v))}</div>
        <div className="form-row"><label>y_width</label>{spin(profile.y_width, 1, 65535, (v) => set("y_width", v))}</div>
        <div className="form-row"><label>item_height</label>{spin(profile.item_height, 16, 65535, (v) => set("item_height", v))}</div>
        <div className="form-row"><label>breadcrumb_height</label>{spin(profile.breadcrumb_height, 0, 65535, (v) => set("breadcrumb_height", v))}</div>
        <button className="btn-auto-calc" onClick={handleAutoCalc} type="button">
          自动计算尺寸
        </button>
      </fieldset>

      <fieldset className="group-box">
        <legend>Display</legend>
        <div className="form-row"><label>show_frame</label><input type="checkbox" checked={profile.is_show_frame} onChange={(e) => set("is_show_frame", e.target.checked)} /></div>
        <div className="form-row"><label>show_breadcrumb</label><input type="checkbox" checked={profile.is_show_breadcrumb} onChange={(e) => set("is_show_breadcrumb", e.target.checked)} /></div>
        <div className="form-row"><label>fill_background</label><input type="checkbox" checked={profile.is_fill_background} onChange={(e) => set("is_fill_background", e.target.checked)} /></div>
        <div className="form-row">
          <label>font_size</label>
          <select value={profile.font_size} onChange={(e) => set("font_size", e.target.value)}>
            {UI_FONT_SIZE_OPTIONS.map(([value, text]) => <option key={value} value={value}>{text}</option>)}
          </select>
        </div>
        <div className="form-row">
          <label>theme_type</label>
          <select value={profile.theme_type} onChange={(e) => set("theme_type", e.target.value)}>
            {MENU_THEME_OPTIONS.map(([value, text]) => <option key={value} value={value}>{text}</option>)}
          </select>
        </div>
      </fieldset>

      {/* Tree editor */}
      <fieldset className="group-box">
        <legend>Menu Items ({countNodes(roots)}/{MAX_MENU_ITEMS})</legend>
        <div className="menu-tree-layout">
          <div className="menu-tree-panel">
            {roots.map((node) => (
              <TreeNodeRow
                key={node.id}
                node={node}
                depth={0}
                selectedId={selectedId}
                onSelect={setSelectedId}
                onToggle={handleToggle}
                onContextMenu={handleContextMenu}
              />
            ))}
            {roots.length === 0 && (
              <div className="tree-empty">暂无菜单项，点击下方按钮添加</div>
            )}
            <button
              className="tree-add-root"
              onClick={handleAddRoot}
              disabled={countNodes(roots) >= MAX_MENU_ITEMS}
            >
              + 添加根节点
            </button>
          </div>
          <div className="menu-props-panel">
            {selectedNode ? (
              <PropertiesPanel node={selectedNode} onUpdate={handleNodeUpdate} />
            ) : (
              <div className="menu-props-empty">选择节点以编辑属性</div>
            )}
          </div>
        </div>
      </fieldset>

      {/* Context menu */}
      {ctxMenu && (
        <ContextMenu
          state={ctxMenu}
          roots={roots}
          onAction={handleCtxAction}
          onClose={() => setCtxMenu(null)}
        />
      )}
    </div>
  );
}

// Helper: find parent of a node by id
function findParent(roots: TreeNode[], id: string): TreeNode | null {
  for (const root of roots) {
    if (root.children.some((c) => c.id === id)) return root;
    const found = findParent(root.children, id);
    if (found) return found;
  }
  return null;
}
