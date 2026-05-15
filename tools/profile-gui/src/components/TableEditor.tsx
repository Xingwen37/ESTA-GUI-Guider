import { useState, useEffect, useRef } from "react";
import type { TableProfile, TableColProfile, TableCellProfile } from "../lib/types";
import {
  MAX_TABLE_ROWS,
  MAX_TABLE_COLS,
  MAX_TABLE_STRING_LEN,
  TABLE_CELL_TYPE_OPTIONS,
  TABLE_THEME_OPTIONS,
  UI_FONT_SIZE_OPTIONS,
} from "../lib/types";

interface Props {
  profile: TableProfile;
  onChange: (p: TableProfile) => void;
}

const FONT_METRICS: Record<string, { w: number; h: number }> = {
  "ESTA_FONT_1206": { w: 6, h: 12 },
  "ESTA_FONT_1608": { w: 8, h: 16 },
  "ESTA_FONT_2412": { w: 12, h: 24 },
};

const THEME_COLORS: Record<string, { bg: string; header: string; text: string; frame: string; line: string }> = {
  "TABLE_THEME_DEFAULT": { bg: "#000", header: "#4fc3f7", text: "#fff", frame: "#fff", line: "#888" },
  "TABLE_THEME_LIGHT": { bg: "#fff", header: "#1565c0", text: "#000", frame: "#000", line: "#888" },
};

function defaultCol(): TableColProfile {
  return { header: "Col", cell_type: "TABLE_CELL_TEXT", width: 0, precision: 0 };
}

function defaultCell(): TableCellProfile {
  return { text: "", u32: 0, f32: 0 };
}

function clampText(v: string) { return v.slice(0, MAX_TABLE_STRING_LEN); }

interface CtxMenuState {
  x: number;
  y: number;
  target: { type: "row"; idx: number } | { type: "col"; idx: number };
}

// --- PLACEHOLDER_COMPONENT ---

export default function TableEditor({ profile, onChange }: Props) {
  const [ctxMenu, setCtxMenu] = useState<CtxMenuState | null>(null);
  const ctxRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!ctxMenu) return;
    const handler = (e: MouseEvent) => {
      if (ctxRef.current && !ctxRef.current.contains(e.target as Node)) setCtxMenu(null);
    };
    document.addEventListener("mousedown", handler);
    return () => document.removeEventListener("mousedown", handler);
  }, [ctxMenu]);

  const set = (key: keyof TableProfile, value: unknown) =>
    onChange({ ...profile, [key]: value });

  const updateCol = (idx: number, patch: Partial<TableColProfile>) => {
    const cols = [...profile.cols];
    cols[idx] = { ...(cols[idx] ?? defaultCol()), ...patch };
    onChange({ ...profile, cols });
  };

  const updateCell = (row: number, col: number, patch: Partial<TableCellProfile>) => {
    const cells = profile.cells.map((r) => [...r]);
    if (!cells[row]) cells[row] = Array.from({ length: profile.col_count }, () => defaultCell());
    cells[row][col] = { ...(cells[row][col] ?? defaultCell()), ...patch };
    onChange({ ...profile, cells });
  };

  const insertRow = (at: number) => {
    if (profile.row_count >= MAX_TABLE_ROWS) return;
    const cells = [...profile.cells];
    cells.splice(at, 0, Array.from({ length: profile.col_count }, () => defaultCell()));
    onChange({ ...profile, row_count: profile.row_count + 1, cells });
  };

  const deleteRow = (at: number) => {
    if (profile.row_count <= 0) return;
    const cells = [...profile.cells];
    cells.splice(at, 1);
    onChange({ ...profile, row_count: profile.row_count - 1, cells });
  };

  const insertCol = (at: number) => {
    if (profile.col_count >= MAX_TABLE_COLS) return;
    const cols = [...profile.cols];
    cols.splice(at, 0, defaultCol());
    const cells = profile.cells.map((row) => {
      const r = [...row];
      r.splice(at, 0, defaultCell());
      return r;
    });
    onChange({ ...profile, col_count: profile.col_count + 1, cols, cells });
  };

  const deleteCol = (at: number) => {
    if (profile.col_count <= 1) return;
    const cols = [...profile.cols];
    cols.splice(at, 1);
    const cells = profile.cells.map((row) => {
      const r = [...row];
      r.splice(at, 1);
      return r;
    });
    onChange({ ...profile, col_count: profile.col_count - 1, cols, cells });
  };

  const handleCtxAction = (action: string) => {
    if (!ctxMenu) return;
    const { target } = ctxMenu;
    if (target.type === "row") {
      switch (action) {
        case "insert-above": insertRow(target.idx); break;
        case "insert-below": insertRow(target.idx + 1); break;
        case "delete": deleteRow(target.idx); break;
      }
    } else {
      switch (action) {
        case "insert-left": insertCol(target.idx); break;
        case "insert-right": insertCol(target.idx + 1); break;
        case "delete": deleteCol(target.idx); break;
      }
    }
    setCtxMenu(null);
  };

  const handleAutoCalc = () => {
    const metrics = FONT_METRICS[profile.font_size] ?? FONT_METRICS["ESTA_FONT_1608"];
    const row_height = metrics.h + 4;
    const headerRows = profile.is_show_header ? 1 : 0;
    const y_width = (profile.row_count + headerRows) * row_height;
    let x_width = 0;
    for (let c = 0; c < profile.col_count; c++) {
      const col = profile.cols[c];
      if (col && col.width > 0) { x_width += col.width; continue; }
      let maxLen = (col?.header ?? "").length;
      for (let r = 0; r < profile.row_count; r++) {
        const cell = profile.cells[r]?.[c];
        if (!cell) continue;
        const cellType = col?.cell_type ?? "TABLE_CELL_TEXT";
        let len = 0;
        if (cellType === "TABLE_CELL_TEXT") len = cell.text.length;
        else if (cellType === "TABLE_CELL_UINT32") len = String(cell.u32).length;
        else len = cell.f32.toFixed(col?.precision ?? 0).length;
        if (len > maxLen) maxLen = len;
      }
      x_width += (maxLen + 1) * metrics.w;
    }
    onChange({ ...profile, row_height, x_width, y_width });
  };

  const addRow = () => insertRow(profile.row_count);
  const addCol = () => insertCol(profile.col_count);

  const colors = THEME_COLORS[profile.theme_type] ?? THEME_COLORS["TABLE_THEME_DEFAULT"];
  const metrics = FONT_METRICS[profile.font_size] ?? FONT_METRICS["ESTA_FONT_1608"];

  return (
    <div>
      <fieldset className="group-box">
        <legend>Position & Layout</legend>
        <div className="form-row"><label>x_origin</label><input type="number" value={profile.x_origin} min={0} max={65535} onChange={(e) => set("x_origin", Number(e.target.value) || 0)} /></div>
        <div className="form-row"><label>y_origin</label><input type="number" value={profile.y_origin} min={0} max={65535} onChange={(e) => set("y_origin", Number(e.target.value) || 0)} /></div>
        <div className="form-row"><label>x_width</label><input type="number" value={profile.x_width} min={1} max={65535} onChange={(e) => set("x_width", Number(e.target.value) || 0)} /></div>
        <div className="form-row"><label>y_width</label><input type="number" value={profile.y_width} min={1} max={65535} onChange={(e) => set("y_width", Number(e.target.value) || 0)} /></div>
        <div className="form-row"><label>row_height</label><input type="number" value={profile.row_height} min={12} max={65535} onChange={(e) => set("row_height", Number(e.target.value) || 0)} /></div>
        <button className="btn-auto-calc" onClick={handleAutoCalc} type="button">自动计算尺寸</button>
      </fieldset>

      <fieldset className="group-box">
        <legend>Display</legend>
        <div className="form-row"><label>show_header</label><input type="checkbox" checked={profile.is_show_header} onChange={(e) => set("is_show_header", e.target.checked)} /></div>
        <div className="form-row"><label>show_frame</label><input type="checkbox" checked={profile.is_show_frame} onChange={(e) => set("is_show_frame", e.target.checked)} /></div>
        <div className="form-row"><label>show_row_line</label><input type="checkbox" checked={profile.is_show_row_line} onChange={(e) => set("is_show_row_line", e.target.checked)} /></div>
        <div className="form-row"><label>show_col_line</label><input type="checkbox" checked={profile.is_show_col_line} onChange={(e) => set("is_show_col_line", e.target.checked)} /></div>
        <div className="form-row"><label>fill_background</label><input type="checkbox" checked={profile.is_fill_background} onChange={(e) => set("is_fill_background", e.target.checked)} /></div>
        <div className="form-row"><label>font_size</label>
          <select value={profile.font_size} onChange={(e) => set("font_size", e.target.value)}>
            {UI_FONT_SIZE_OPTIONS.map(([v, t]) => <option key={v} value={v}>{t}</option>)}
          </select>
        </div>
        <div className="form-row"><label>theme_type</label>
          <select value={profile.theme_type} onChange={(e) => set("theme_type", e.target.value)}>
            {TABLE_THEME_OPTIONS.map(([v, t]) => <option key={v} value={v}>{t}</option>)}
          </select>
        </div>
      </fieldset>

      <fieldset className="group-box">
        <legend>Data ({profile.row_count}×{profile.col_count})</legend>
        <div className="table-grid" style={{ overflowX: "auto" }}>
          <table className="table-data-grid">
            <thead>
              <tr>
                <th></th>
                {Array.from({ length: profile.col_count }, (_, c) => {
                  const col = profile.cols[c] ?? defaultCol();
                  return (
                    <th key={c} onContextMenu={(e) => { e.preventDefault(); setCtxMenu({ x: e.clientX, y: e.clientY, target: { type: "col", idx: c } }); }}>
                      <div className="table-col-header">
                        <input className="table-col-header-input" value={col.header} maxLength={MAX_TABLE_STRING_LEN} onChange={(e) => updateCol(c, { header: clampText(e.target.value) })} />
                        <select className="table-col-type-select" value={col.cell_type} onChange={(e) => updateCol(c, { cell_type: e.target.value })}>
                          {TABLE_CELL_TYPE_OPTIONS.map(([v, t]) => <option key={v} value={v}>{t}</option>)}
                        </select>
                      </div>
                    </th>
                  );
                })}
                <th>
                  <button className="table-add-btn" onClick={addCol} disabled={profile.col_count >= MAX_TABLE_COLS} title="添加列">+</button>
                </th>
              </tr>
            </thead>
            <tbody>
              {Array.from({ length: profile.row_count }, (_, r) => (
                <tr key={r}>
                  <td className="table-row-idx" onContextMenu={(e) => { e.preventDefault(); setCtxMenu({ x: e.clientX, y: e.clientY, target: { type: "row", idx: r } }); }}>{r}</td>
                  {Array.from({ length: profile.col_count }, (_, c) => {
                    const col = profile.cols[c] ?? defaultCol();
                    const cell = profile.cells[r]?.[c] ?? defaultCell();
                    return (
                      <td key={c}>
                        {col.cell_type === "TABLE_CELL_TEXT" ? (
                          <input value={cell.text} maxLength={MAX_TABLE_STRING_LEN} onChange={(e) => updateCell(r, c, { text: clampText(e.target.value) })} />
                        ) : col.cell_type === "TABLE_CELL_UINT32" ? (
                          <input type="number" value={cell.u32} min={0} onChange={(e) => updateCell(r, c, { u32: Number(e.target.value) || 0 })} />
                        ) : (
                          <input type="number" value={cell.f32} step={0.1} onChange={(e) => updateCell(r, c, { f32: Number(e.target.value) || 0 })} />
                        )}
                      </td>
                    );
                  })}
                </tr>
              ))}
              <tr>
                <td colSpan={profile.col_count + 1}>
                  <button className="table-add-btn wide" onClick={addRow} disabled={profile.row_count >= MAX_TABLE_ROWS}>+ 添加行</button>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </fieldset>

      <fieldset className="group-box">
        <legend>Preview</legend>
        <div className="table-preview" style={{
          width: profile.x_width,
          background: profile.is_fill_background ? colors.bg : "transparent",
          border: profile.is_show_frame ? `1px solid ${colors.frame}` : "none",
          fontFamily: "monospace",
          fontSize: metrics.h,
          lineHeight: `${profile.row_height}px`,
          color: colors.text,
        }}>
          {profile.is_show_header && (
            <div className="table-preview-row" style={{ color: colors.header, borderBottom: profile.is_show_row_line ? `1px solid ${colors.line}` : "none" }}>
              {Array.from({ length: profile.col_count }, (_, c) => (
                <span key={c} className="table-preview-cell" style={{ borderRight: profile.is_show_col_line && c + 1 < profile.col_count ? `1px solid ${colors.line}` : "none" }}>
                  {profile.cols[c]?.header ?? ""}
                </span>
              ))}
            </div>
          )}
          {Array.from({ length: profile.row_count }, (_, r) => (
            <div key={r} className="table-preview-row" style={{ borderBottom: profile.is_show_row_line && r + 1 < profile.row_count ? `1px solid ${colors.line}` : "none" }}>
              {Array.from({ length: profile.col_count }, (_, c) => {
                const col = profile.cols[c] ?? defaultCol();
                const cell = profile.cells[r]?.[c] ?? defaultCell();
                let text = "";
                if (col.cell_type === "TABLE_CELL_TEXT") text = cell.text;
                else if (col.cell_type === "TABLE_CELL_UINT32") text = String(cell.u32);
                else text = cell.f32.toFixed(col.precision);
                return (
                  <span key={c} className="table-preview-cell" style={{ borderRight: profile.is_show_col_line && c + 1 < profile.col_count ? `1px solid ${colors.line}` : "none" }}>
                    {text}
                  </span>
                );
              })}
            </div>
          ))}
        </div>
      </fieldset>

      {ctxMenu && (
        <div ref={ctxRef} className="menu-ctx" style={{ left: ctxMenu.x, top: ctxMenu.y }}>
          {ctxMenu.target.type === "row" ? (
            <>
              <div className="menu-ctx-item" onClick={() => handleCtxAction("insert-above")}>在上方插入行</div>
              <div className="menu-ctx-item" onClick={() => handleCtxAction("insert-below")}>在下方插入行</div>
              <div className="menu-ctx-sep" />
              <div className="menu-ctx-item danger" onClick={() => handleCtxAction("delete")}>删除行</div>
            </>
          ) : (
            <>
              <div className="menu-ctx-item" onClick={() => handleCtxAction("insert-left")}>在左侧插入列</div>
              <div className="menu-ctx-item" onClick={() => handleCtxAction("insert-right")}>在右侧插入列</div>
              <div className="menu-ctx-sep" />
              <div className="menu-ctx-item danger" onClick={() => handleCtxAction("delete")}>删除列</div>
            </>
          )}
        </div>
      )}
    </div>
  );
}
