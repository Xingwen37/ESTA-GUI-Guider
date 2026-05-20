import type { ProfileSet, FlagConfig } from "../lib/types";

interface Props {
  data: ProfileSet;
  setData: (data: ProfileSet) => void;
}

const DISABLED_FLAG: FlagConfig = { mode: 0 };

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
    update(id, { mode: 1 });
  };

  const disable = (id: number) => {
    update(id, { ...DISABLED_FLAG });
  };

  return (
    <div style={{ padding: 12 }}>
      <fieldset className="group-box">
        <legend>Flag 配置 ({enabledCount} / {FLAG_MAX} 已启用)</legend>
        <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 13 }}>
          <thead>
            <tr style={{ borderBottom: "1px solid #444" }}>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Flag ID</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }} title="MANUAL：状态位，可用于 guard 门控；FlagSignal 置位时同时推送 FLAG 事件供 binding 订阅">说明</th>
              <th style={{ width: 40 }}></th>
            </tr>
          </thead>
          <tbody>
            {flags.map((f, i) => {
              if (f.mode === 0) return null;
              return (
                <tr key={i} style={{ borderBottom: "1px solid #333" }}>
                  <td style={{ padding: "4px 6px", color: "#888" }}>#{i}</td>
                  <td style={{ padding: "4px 6px", color: "#aaa", fontSize: 12 }}>
                    MANUAL — 状态位 + FLAG 事件触发源
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
