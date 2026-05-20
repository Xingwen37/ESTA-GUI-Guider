import type { ProfileSet, FlagConfig } from "../lib/types";

interface Props {
  data: ProfileSet;
  setData: (data: ProfileSet) => void;
}

const FLAG_MODE_OPTIONS = [
  [1, "AUTO_EVENT"],
] as const;

const EVENT_TYPE_OPTIONS = [
  [1, "BUTTON_PRESS"],
  [2, "BUTTON_RELEASE"],
  [3, "MENU_SELECT"],
  [4, "ENCODER_ROTATE"],
  [5, "TIMER"],
  [6, "FLAG"],
] as const;

const DISABLED_FLAG: FlagConfig = {
  mode: 0,
  event_type: 1,
  event_source: 0,
  event_id: 0,
};

const FLAG_MAX = 8;

function ensureLength(profiles: FlagConfig[]): FlagConfig[] {
  const arr = [...profiles];
  while (arr.length < FLAG_MAX) arr.push({ ...DISABLED_FLAG });
  return arr.slice(0, FLAG_MAX);
}

export default function FlagConfigEditor({ data, setData }: Props) {
  const flags = ensureLength(
    data.flag_profiles ?? Array.from({ length: FLAG_MAX }, () => ({ ...DISABLED_FLAG }))
  );

  const enabledCount = flags.filter((f) => f.mode !== 0).length;
  const availableIds = flags
    .map((f, i) => (f.mode === 0 ? i : -1))
    .filter((i) => i !== -1);

  const update = (i: number, patch: Partial<FlagConfig>) => {
    const next = flags.map((f, j) => (j === i ? { ...f, ...patch } : f));
    setData({ ...data, flag_profiles: next, flag_inst_count: FLAG_MAX });
  };

  const enable = (id: number) => {
    update(id, { mode: 1, event_type: 1, event_source: 0, event_id: 0 });
  };

  const disable = (id: number) => {
    update(id, { ...DISABLED_FLAG });
  };

  return (
    <div style={{ padding: 12 }}>
      <fieldset className="group-box">
        <legend>Flag 配置 ({enabledCount}/{FLAG_MAX} 已启用)</legend>
        <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 13 }}>
          <thead>
            <tr style={{ borderBottom: "1px solid #444" }}>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Flag</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Mode</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Event Type</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Source</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>ID</th>
              <th style={{ width: 40 }}></th>
            </tr>
          </thead>
          <tbody>
            {flags.map((f, i) => {
              if (f.mode === 0) return null;
              return (
                <tr key={i} style={{ borderBottom: "1px solid #333" }}>
                  <td style={{ padding: "4px 6px", color: "#888" }}>#{i}</td>
                  <td style={{ padding: "4px 6px" }}>
                    <select
                      value={f.mode}
                      onChange={(e) => update(i, { mode: Number(e.target.value) })}
                    >
                      {FLAG_MODE_OPTIONS.map(([v, label]) => (
                        <option key={v} value={v}>{label}</option>
                      ))}
                    </select>
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    <select
                      value={f.event_type}
                      onChange={(e) => update(i, { event_type: Number(e.target.value) })}
                    >
                      {EVENT_TYPE_OPTIONS.map(([v, label]) => (
                        <option key={v} value={v}>{label}</option>
                      ))}
                    </select>
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    <input
                      type="number"
                      value={f.event_source}
                      min={0}
                      max={255}
                      style={{ width: 50 }}
                      onChange={(e) => update(i, { event_source: Number(e.target.value) || 0 })}
                    />
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    <input
                      type="number"
                      value={f.event_id}
                      min={0}
                      max={65535}
                      style={{ width: 60 }}
                      onChange={(e) => update(i, { event_id: Number(e.target.value) || 0 })}
                    />
                  </td>
                  <td style={{ padding: "4px 6px", textAlign: "center" }}>
                    <button
                      onClick={() => disable(i)}
                      title="禁用"
                      style={{ background: "none", border: "none", color: "#e55", cursor: "pointer", fontSize: 16 }}
                    >
                      &times;
                    </button>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>

        {enabledCount === 0 && (
          <div style={{ color: "#999", padding: 12 }}>暂无已启用的 Flag，点击下方按钮添加</div>
        )}

        {availableIds.length > 0 && (
          <div style={{ marginTop: 8, display: "flex", alignItems: "center", gap: 8 }}>
            <span style={{ color: "#888", fontSize: 12 }}>启用 Flag:</span>
            {availableIds.map((id) => (
              <button
                key={id}
                onClick={() => enable(id)}
                style={{
                  padding: "2px 10px",
                  background: "#37373d",
                  border: "1px solid #555",
                  color: "#ccc",
                  borderRadius: 3,
                  cursor: "pointer",
                  fontSize: 12,
                }}
              >
                #{id}
              </button>
            ))}
          </div>
        )}
      </fieldset>
    </div>
  );
}
