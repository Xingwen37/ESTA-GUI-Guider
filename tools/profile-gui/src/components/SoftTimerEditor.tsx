import type { ProfileSet, SoftTimerConfig } from "../lib/types";
import { MAX_SOFT_TIMERS, TRIGGER_OPTIONS } from "../lib/types";

interface Props {
  data: ProfileSet;
  setData: (data: ProfileSet) => void;
}

const EMPTY_TIMER: SoftTimerConfig = {
  period_ms: 20,
  event_type: 5,
  event_source: 0,
  event_id: 0,
};

export default function SoftTimerEditor({ data, setData }: Props) {
  const timers: SoftTimerConfig[] = (data.timer_configs as SoftTimerConfig[]) ?? [];

  const addTimer = () => {
    if (timers.length >= MAX_SOFT_TIMERS) return;
    const next = [...timers, { ...EMPTY_TIMER }];
    setData({ ...data, timer_configs: next, timer_count: next.length });
  };

  const removeTimer = (idx: number) => {
    const next = timers.filter((_, i) => i !== idx);
    setData({ ...data, timer_configs: next, timer_count: next.length });
  };

  const update = (idx: number, patch: Partial<SoftTimerConfig>) => {
    const next = timers.map((t, i) => (i === idx ? { ...t, ...patch } : t));
    setData({ ...data, timer_configs: next, timer_count: next.length });
  };

  return (
    <div style={{ padding: 12 }}>
      <fieldset className="group-box">
        <legend>软件定时器 ({timers.length}/{MAX_SOFT_TIMERS})</legend>
        <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 13 }}>
          <thead>
            <tr style={{ borderBottom: "1px solid #444" }}>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Timer</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Period (ms)</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Event Type</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Source</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>ID</th>
              <th style={{ width: 40 }}></th>
            </tr>
          </thead>
          <tbody>
            {timers.map((t, i) => (
              <tr key={i} style={{ borderBottom: "1px solid #333" }}>
                <td style={{ padding: "4px 6px", color: "#888" }}>#{i}</td>
                <td style={{ padding: "4px 6px" }}>
                  <input
                    type="number"
                    value={t.period_ms}
                    min={1}
                    max={65535}
                    style={{ width: 70 }}
                    onChange={(e) =>
                      update(i, { period_ms: Math.max(1, Number(e.target.value) || 1) })
                    }
                  />
                </td>
                <td style={{ padding: "4px 6px" }}>
                  <select
                    value={t.event_type}
                    onChange={(e) => update(i, { event_type: Number(e.target.value) })}
                  >
                    {TRIGGER_OPTIONS.map(([v, label]) => (
                      <option key={v} value={v}>
                        {label}
                      </option>
                    ))}
                  </select>
                </td>
                <td style={{ padding: "4px 6px" }}>
                  <input
                    type="number"
                    value={t.event_source}
                    min={0}
                    max={255}
                    style={{ width: 50 }}
                    onChange={(e) => update(i, { event_source: Number(e.target.value) || 0 })}
                  />
                </td>
                <td style={{ padding: "4px 6px" }}>
                  <input
                    type="number"
                    value={t.event_id}
                    min={0}
                    max={65535}
                    style={{ width: 60 }}
                    onChange={(e) => update(i, { event_id: Number(e.target.value) || 0 })}
                  />
                </td>
                <td style={{ padding: "4px 6px", textAlign: "center" }}>
                  <button
                    onClick={() => removeTimer(i)}
                    title="删除"
                    style={{
                      background: "none",
                      border: "none",
                      color: "#e55",
                      cursor: "pointer",
                      fontSize: 16,
                    }}
                  >
                    &times;
                  </button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
        {timers.length === 0 && (
          <div style={{ color: "#999", padding: 12 }}>暂无定时器，点击下方按钮添加</div>
        )}
        {timers.length < MAX_SOFT_TIMERS && (
          <button onClick={addTimer} style={{ marginTop: 8 }}>
            + 添加定时器
          </button>
        )}
      </fieldset>
    </div>
  );
}
