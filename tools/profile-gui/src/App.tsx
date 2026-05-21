import { useState } from "react";
import { COMPONENT_REGISTRY, type ComponentEntry } from "./lib/componentRegistry";
import { EVENT_PANELS } from "./lib/eventPanelRegistry";
import { useProfileState } from "./hooks/useProfileState";
import { MAX_BUTTON_COUNT } from "./lib/types";

export default function App() {
  const state = useProfileState();
  const {
    data, activeTab, setActiveTab, status,
    screenW, setScreenW, screenH, setScreenH,
    previewPages, setPreviewPages, previewPage, setPreviewPage,
    handleGenerate, handleBuildRun, handlePreview, handleAutoLayout,
    setCount, deleteItem, updateProfile, setButtonCount,
  } = state;

  const [confirmDelete, setConfirmDelete] = useState<{ entry: ComponentEntry; idx: number } | null>(null);

  if (!data) {
    return (
      <div className="editor-scroll">
        <div>Loading...</div>
        {status.type === "error" && (
          <div className="status" style={{ color: "#f44747", marginTop: 12 }}>{status.msg}</div>
        )}
      </div>
    );
  }

  // ---- Tab index calculation ----
  const totalComponentTabs = COMPONENT_REGISTRY.reduce(
    (sum, e) => sum + ((data[e.countField] as number) ?? 0), 0
  );
  const activeSafeTab = Math.min(activeTab, totalComponentTabs);
  const isEventTab = activeSafeTab === totalComponentTabs;

  const instCounts = Object.fromEntries(
    COMPONENT_REGISTRY.map((e) => [e.key, (data[e.countField] as number) ?? 0])
  );

  let activeEntry = COMPONENT_REGISTRY[0];
  let profileIndex = 0;
  let tabOffset = 0;
  for (const entry of COMPONENT_REGISTRY) {
    const count = (data[entry.countField] as number) ?? 0;
    if (!isEventTab && activeSafeTab < tabOffset + count) {
      activeEntry = entry;
      profileIndex = activeSafeTab - tabOffset;
      break;
    }
    tabOffset += count;
  }

  const currentProfile = isEventTab
    ? null
    : ((data[activeEntry.profilesField] as unknown[]) ?? [])[profileIndex] ?? activeEntry.makeDefault();

  return (
    <>
      <div className="toolbar">
        {COMPONENT_REGISTRY.map((entry) => (
          <span key={entry.key} style={{ display: "contents" }}>
            <label style={entry !== COMPONENT_REGISTRY[0] ? { marginLeft: 12 } : undefined}>
              {entry.label}
            </label>
            <input
              type="number"
              value={(data[entry.countField] as number) ?? 0}
              min={0}
              max={entry.maxCount}
              onChange={(e) => setCount(entry, Number(e.target.value) || 0)}
            />
          </span>
        ))}
        <label style={{ marginLeft: 12 }}>BUTTON</label>
        <input
          type="number"
          value={data.button_count}
          min={0}
          max={MAX_BUTTON_COUNT}
          onChange={(e) => setButtonCount(Number(e.target.value) || 0)}
        />
        <div className="toolbar-spacer" />
        <label>Screen</label>
        <input type="number" value={screenW} min={100} max={800}
          onChange={(e) => { const v = parseInt(e.target.value, 10); if (!isNaN(v)) setScreenW(v); }} />
        <span style={{ color: "#888" }}>×</span>
        <input type="number" value={screenH} min={100} max={800}
          onChange={(e) => { const v = parseInt(e.target.value, 10); if (!isNaN(v)) setScreenH(v); }} />
        <button className="btn-auto-calc" onClick={() => handleAutoLayout(screenW, screenH)}>
          自动布局
        </button>
        <button className="btn-generate" onClick={handleGenerate}>生成 ESTA_Profile.c</button>
        <button className="btn-run" onClick={handleBuildRun}>Build &amp; Run</button>
        <button className="btn-auto-calc" onClick={handlePreview}>预览</button>
        {status.type !== "idle" && <span className="status">{status.msg}</span>}
      </div>

      <div className="tabs">
        {COMPONENT_REGISTRY.flatMap((entry, ei) => {
          const count = (data[entry.countField] as number) ?? 0;
          const offset = COMPONENT_REGISTRY.slice(0, ei).reduce(
            (s, e) => s + ((data[e.countField] as number) ?? 0), 0
          );
          const prevCount = COMPONENT_REGISTRY.slice(0, ei).reduce(
            (s, e) => s + ((data[e.countField] as number) ?? 0), 0
          );
          const tabs = Array.from({ length: count }, (_, i) => (
            <button
              key={`${entry.key}${i}`}
              className={`tab ${activeSafeTab === offset + i ? "active" : ""}`}
              onClick={() => setActiveTab(offset + i)}
            >
              {entry.label}{i}
              <span className="tab-close" onClick={(e) => { e.stopPropagation(); setConfirmDelete({ entry, idx: i }); }}>×</span>
            </button>
          ));
          const sep = ei > 0 && prevCount > 0 && count > 0
            ? [<span key={`sep${ei}`} className="tab-sep" />]
            : [];
          return [...sep, ...tabs];
        })}
        <span className="tab-sep" />
        <button
          className={`tab ${isEventTab ? "active" : ""}`}
          onClick={() => setActiveTab(totalComponentTabs)}
        >
          EVENT
        </button>
      </div>

      {previewPages.length > 0 && (
        <div style={{ padding: "8px 16px", background: "#1a1a1a", borderBottom: "1px solid #3c3c3c", position: "relative" }}>
          <img src={previewPages[previewPage]} alt={`Preview page ${previewPage}`}
            style={{ maxWidth: "100%", height: "auto", borderRadius: 4, border: "1px solid #555" }} />
          {previewPages.length > 1 && (
            <div style={{ marginTop: 6, display: "flex", alignItems: "center", gap: 8 }}>
              <button className="btn-auto-calc" disabled={previewPage === 0}
                onClick={() => setPreviewPage(previewPage - 1)}>&lt;</button>
              <span style={{ color: "#ccc", fontSize: 12 }}>{previewPage + 1} / {previewPages.length}</span>
              <button className="btn-auto-calc" disabled={previewPage === previewPages.length - 1}
                onClick={() => setPreviewPage(previewPage + 1)}>&gt;</button>
            </div>
          )}
          <button onClick={() => setPreviewPages([])}
            style={{ position: "absolute", top: 12, right: 20, background: "#333", border: "1px solid #555", color: "#ccc", borderRadius: 4, cursor: "pointer", padding: "2px 8px", fontSize: 12 }}>
            ×
          </button>
        </div>
      )}

      <div className="editor-scroll">
        {isEventTab ? (
          <>
            {EVENT_PANELS.map((panel) => (
              <panel.Component
                key={panel.key}
                data={data}
                instCounts={instCounts}
                setData={state.setData}
              />
            ))}
          </>
        ) : totalComponentTabs === 0 ? (
          <div style={{ color: "#999", padding: 24 }}>请设置组件数量</div>
        ) : (
          <activeEntry.Editor
            profile={currentProfile}
            onChange={(p: unknown) => updateProfile(activeEntry, profileIndex, p)}
            instIndex={profileIndex}
            onAutoAssign={state.remapMenuEventIds}
          />
        )}
      </div>

      {confirmDelete && (
        <div
          style={{
            position: "fixed", inset: 0, background: "rgba(0,0,0,0.6)",
            display: "flex", alignItems: "center", justifyContent: "center", zIndex: 1000,
          }}
          onClick={() => setConfirmDelete(null)}
        >
          <div
            style={{
              background: "#252526", border: "1px solid #555", borderRadius: 6,
              padding: "20px 28px", minWidth: 300, textAlign: "center",
            }}
            onClick={(e) => e.stopPropagation()}
          >
            <div style={{ fontSize: 14, color: "#ccc", marginBottom: 20 }}>
              确定删除此组件吗？
            </div>
            <div style={{ display: "flex", gap: 12, justifyContent: "center" }}>
              <button
                onClick={() => setConfirmDelete(null)}
                style={{
                  background: "#3c3c3c", border: "1px solid #555", color: "#ccc",
                  borderRadius: 4, padding: "6px 24px", cursor: "pointer", fontSize: 13,
                }}
              >
                取消
              </button>
              <button
                onClick={() => {
                  deleteItem(confirmDelete.entry, confirmDelete.idx);
                  setConfirmDelete(null);
                }}
                style={{
                  background: "#a33", border: "1px solid #c44", color: "#fff",
                  borderRadius: 4, padding: "6px 24px", cursor: "pointer", fontSize: 13,
                }}
              >
                确认删除
              </button>
            </div>
          </div>
        </div>
      )}
    </>
  );
}
