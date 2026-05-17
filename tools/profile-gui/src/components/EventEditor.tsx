import type { EventBinding, MenuProfile } from "../lib/types";
import {
  TRIGGER_OPTIONS,
  TARGET_TYPE_OPTIONS,
  ACTION_TYPE_OPTIONS,
  VALID_ACTIONS,
  SOURCE_ANY,
  TRIGGER_ID_ANY,
  MAX_BINDINGS,
} from "../lib/types";

interface Props {
  bindings: EventBinding[];
  buttonCount: number;
  waveInstCount: number;
  barInstCount: number;
  tableInstCount: number;
  menuInstCount: number;
  menuProfiles: MenuProfile[];
  onChange: (bindings: EventBinding[]) => void;
}

const EMPTY_BINDING: EventBinding = {
  trigger: 1,
  source_id: 0,
  trigger_id: TRIGGER_ID_ANY,
  target_type: 4,
  target_inst: 0,
  action: 1,
};

function getInstCount(targetType: number, props: Props): number {
  switch (targetType) {
    case 0: return props.waveInstCount;
    case 1: return props.barInstCount;
    case 2: return props.tableInstCount;
    case 3: return props.menuInstCount;
    case 6: return 8; // FLAG_MAX
    default: return 0;
  }
}

function isSingletonTarget(targetType: number): boolean {
  return targetType === 4 || targetType === 5;
}

function isButtonTrigger(trigger: number): boolean {
  return trigger === 1 || trigger === 2;
}

export default function EventEditor(props: Props) {
  const { bindings, buttonCount, menuProfiles, onChange } = props;

  const addBinding = () => {
    if (bindings.length >= MAX_BINDINGS) return;
    onChange([...bindings, { ...EMPTY_BINDING }]);
  };

  const removeBinding = (idx: number) => {
    onChange(bindings.filter((_, i) => i !== idx));
  };

  const updateBinding = (idx: number, field: keyof EventBinding, value: number) => {
    const next = [...bindings];
    const updated = { ...next[idx], [field]: value };

    if (field === "target_type") {
      const validActions = VALID_ACTIONS[value] || [];
      if (!validActions.includes(updated.action)) {
        updated.action = validActions[0] || 0;
      }
      if (isSingletonTarget(value)) {
        updated.target_inst = 0;
      }
    }
    if (field === "trigger" && value !== 3) {
      updated.trigger_id = TRIGGER_ID_ANY;
    }

    next[idx] = updated;
    onChange(next);
  };

  const getMenuItemOptions = (menuInst: number) => {
    const profile = menuProfiles[menuInst];
    if (!profile) return [];
    return profile.items
      .filter(item => !item.is_submenu)
      .map(item => ({ label: item.label, eventId: item.event_id }));
  };

  return (
    <div style={{ padding: 12 }}>
      <fieldset className="group-box">
        <legend>事件绑定 ({bindings.length}/{MAX_BINDINGS})</legend>
        <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 13 }}>
          <thead>
            <tr style={{ borderBottom: "1px solid #444" }}>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Trigger</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Source</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Event ID</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Target</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Inst</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Action</th>
              <th style={{ width: 40 }}></th>
            </tr>
          </thead>
          <tbody>
            {bindings.map((b, i) => {
              const validActions = VALID_ACTIONS[b.target_type] || [];
              const instCount = getInstCount(b.target_type, props);
              const singleton = isSingletonTarget(b.target_type);
              const isButton = isButtonTrigger(b.trigger);
              const isMenuSelect = b.trigger === 3;
              const isFlagTrigger = b.trigger === 6;
              const menuItems = isMenuSelect ? getMenuItemOptions(b.source_id) : [];

              return (
                <tr key={i} style={{ borderBottom: "1px solid #333" }}>
                  <td style={{ padding: "4px 6px" }}>
                    <select value={b.trigger}
                      onChange={(e) => updateBinding(i, "trigger", Number(e.target.value))}>
                      {TRIGGER_OPTIONS.map(([val, label]) => (
                        <option key={val} value={val}>{label}</option>
                      ))}
                    </select>
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    {isButton ? (
                      <select value={b.source_id}
                        onChange={(e) => updateBinding(i, "source_id", Number(e.target.value))}>
                        <option value={SOURCE_ANY}>ANY</option>
                        {Array.from({ length: buttonCount }, (_, k) => (
                          <option key={k} value={k}>Button {k}</option>
                        ))}
                      </select>
                    ) : isMenuSelect ? (
                      <select value={b.source_id}
                        onChange={(e) => updateBinding(i, "source_id", Number(e.target.value))}>
                        {Array.from({ length: props.menuInstCount }, (_, k) => (
                          <option key={k} value={k}>MENU #{k}</option>
                        ))}
                      </select>
                    ) : isFlagTrigger ? (
                      <select value={b.source_id}
                        onChange={(e) => updateBinding(i, "source_id", Number(e.target.value))}>
                        <option value={SOURCE_ANY}>ANY</option>
                        {Array.from({ length: 8 }, (_, k) => (
                          <option key={k} value={k}>Flag #{k}</option>
                        ))}
                      </select>
                    ) : (
                      <input type="number" value={b.source_id} min={0} max={255}
                        style={{ width: 50 }}
                        onChange={(e) => updateBinding(i, "source_id", Number(e.target.value) || 0)} />
                    )}
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    {isMenuSelect ? (
                      <select value={b.trigger_id}
                        onChange={(e) => updateBinding(i, "trigger_id", Number(e.target.value))}>
                        <option value={TRIGGER_ID_ANY}>ANY</option>
                        {menuItems.map((item, mi) => (
                          <option key={mi} value={item.eventId}>
                            {item.label} ({item.eventId})
                          </option>
                        ))}
                      </select>
                    ) : (
                      <span style={{ color: "#888" }}>—</span>
                    )}
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    <select value={b.target_type}
                      onChange={(e) => updateBinding(i, "target_type", Number(e.target.value))}>
                      {TARGET_TYPE_OPTIONS.map(([val, label]) => (
                        <option key={val} value={val}>{label}</option>
                      ))}
                    </select>
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    {singleton ? (
                      <span style={{ color: "#888" }}>—</span>
                    ) : (
                      <select value={b.target_inst}
                        onChange={(e) => updateBinding(i, "target_inst", Number(e.target.value))}>
                        {Array.from({ length: Math.max(instCount, 1) }, (_, k) => (
                          <option key={k} value={k}>#{k}</option>
                        ))}
                      </select>
                    )}
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    <select value={b.action}
                      onChange={(e) => updateBinding(i, "action", Number(e.target.value))}>
                      {validActions.length === 0 ? (
                        <option value={0}>(none)</option>
                      ) : (
                        validActions.map((actionVal) => {
                          const opt = ACTION_TYPE_OPTIONS.find(([v]) => v === actionVal);
                          return (
                            <option key={actionVal} value={actionVal}>
                              {opt ? opt[1] : `Action ${actionVal}`}
                            </option>
                          );
                        })
                      )}
                    </select>
                  </td>
                  <td style={{ padding: "4px 6px", textAlign: "center" }}>
                    <button onClick={() => removeBinding(i)} title="删除"
                      style={{ background: "none", border: "none", color: "#e55", cursor: "pointer", fontSize: 16 }}>
                      &times;
                    </button>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
        {bindings.length < MAX_BINDINGS && (
          <button onClick={addBinding} style={{ marginTop: 8 }}>+ 添加绑定</button>
        )}
        {bindings.length === 0 && (
          <div style={{ color: "#999", padding: 12 }}>暂无事件绑定，点击上方按钮添加</div>
        )}
      </fieldset>
    </div>
  );
}
