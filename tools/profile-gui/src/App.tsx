import EventEditor from "./components/EventEditor";
import StringTableEditor from "./components/StringTableEditor";
import SequenceEditor from "./components/SequenceEditor";
import { COMPONENT_REGISTRY } from "./lib/componentRegistry";
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
              <span className="tab-close" onClick={(e) => { e.stopPropagation(); deleteItem(entry, i); }}>×</span>
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
            <EventEditor
              bindings={data.bindings ?? []}
              buttonCount={data.button_count}
              waveInstCount={data.wave_inst_count}
              barInstCount={data.bar_inst_count}
              tableInstCount={data.table_inst_count}
              menuInstCount={data.menu_inst_count}
              menuProfiles={data.menu_profiles ?? []}
              strings={data.strings ?? []}
              sequences={data.sequences ?? []}
              onChange={(bindings) => state.setData({ ...data, bindings, binding_count: bindings.length })}
            />
            <StringTableEditor
              strings={data.strings ?? []}
              bindings={data.bindings ?? []}
              sequences={data.sequences ?? []}
              tableProfiles={data.table_profiles ?? []}
              menuProfiles={data.menu_profiles ?? []}
              onChange={(strings) => state.setData({ ...data, strings, string_count: strings.length })}
            />
            <SequenceEditor
              sequences={data.sequences ?? []}
              waveInstCount={data.wave_inst_count}
              barInstCount={data.bar_inst_count}
              tableInstCount={data.table_inst_count}
              menuInstCount={data.menu_inst_count}
              onChange={(sequences) => state.setData({ ...data, sequences, sequence_count: sequences.length })}
            />
          </>
        ) : totalComponentTabs === 0 ? (
          <div style={{ color: "#999", padding: 24 }}>请设置组件数量</div>
        ) : (
          <activeEntry.Editor
            profile={currentProfile}
            onChange={(p: unknown) => updateProfile(activeEntry, profileIndex, p)}
          />
        )}
      </div>
    </>
  );
}
