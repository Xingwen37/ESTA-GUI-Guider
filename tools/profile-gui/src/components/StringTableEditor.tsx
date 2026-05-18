import type { StringEntry } from "../lib/types";
import {
  TARGET_TYPE_OPTIONS,
  MAX_STRING_ENTRIES,
  MAX_STRING_LEN,
  MAX_TABLE_COLS,
} from "../lib/types";

interface Props {
  strings: StringEntry[];
  tableInstCount: number;
  menuInstCount: number;
  onChange: (strings: StringEntry[]) => void;
}

const EMPTY_ENTRY: StringEntry = {
  target_type: 2,
  target_inst: 0,
  sub_addr: 0,
  text: "",
};

const STRING_TARGET_TYPES = TARGET_TYPE_OPTIONS.filter(
  ([v]) => v === 2 || v === 3
);

export default function StringTableEditor(props: Props) {
  const { strings, tableInstCount, menuInstCount, onChange } = props;

  const addEntry = () => {
    if (strings.length >= MAX_STRING_ENTRIES) return;
    onChange([...strings, { ...EMPTY_ENTRY }]);
  };

  const removeEntry = (idx: number) => {
    onChange(strings.filter((_, i) => i !== idx));
  };

  const updateEntry = (idx: number, field: keyof StringEntry, value: string | number) => {
    const next = [...strings];
    next[idx] = { ...next[idx], [field]: value };
    onChange(next);
  };

  const getInstCount = (targetType: number): number => {
    switch (targetType) {
      case 2: return tableInstCount;
      case 3: return menuInstCount;
      default: return 0;
    }
  };

  return (
    <div style={{ padding: 12 }}>
      <fieldset className="group-box">
        <legend>字符串表 ({strings.length}/{MAX_STRING_ENTRIES})</legend>
        <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 13 }}>
          <thead>
            <tr style={{ borderBottom: "1px solid #444" }}>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Target</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Inst</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Sub Addr</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Text</th>
              <th style={{ width: 40 }}></th>
            </tr>
          </thead>
          <tbody>
            {strings.map((s, i) => {
              const instCount = getInstCount(s.target_type);
              const isTable = s.target_type === 2;
              return (
                <tr key={i} style={{ borderBottom: "1px solid #333" }}>
                  <td style={{ padding: "4px 6px" }}>
                    <select value={s.target_type}
                      onChange={(e) => updateEntry(i, "target_type", Number(e.target.value))}>
                      {STRING_TARGET_TYPES.map(([val, label]) => (
                        <option key={val} value={val}>{label}</option>
                      ))}
                    </select>
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    <select value={s.target_inst}
                      onChange={(e) => updateEntry(i, "target_inst", Number(e.target.value))}>
                      {Array.from({ length: Math.max(instCount, 1) }, (_, k) => (
                        <option key={k} value={k}>#{k}</option>
                      ))}
                    </select>
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    {isTable ? (
                      <span>
                        R<input type="number" value={Math.floor(s.sub_addr / MAX_TABLE_COLS)}
                          min={0} max={7} style={{ width: 36 }}
                          onChange={(e) => {
                            const row = Number(e.target.value) || 0;
                            const col = s.sub_addr % MAX_TABLE_COLS;
                            updateEntry(i, "sub_addr", row * MAX_TABLE_COLS + col);
                          }} />
                        C<input type="number" value={s.sub_addr % MAX_TABLE_COLS}
                          min={0} max={5} style={{ width: 36 }}
                          onChange={(e) => {
                            const row = Math.floor(s.sub_addr / MAX_TABLE_COLS);
                            const col = Number(e.target.value) || 0;
                            updateEntry(i, "sub_addr", row * MAX_TABLE_COLS + col);
                          }} />
                      </span>
                    ) : (
                      <input type="number" value={s.sub_addr} min={0} max={31}
                        style={{ width: 50 }}
                        onChange={(e) => updateEntry(i, "sub_addr", Number(e.target.value) || 0)} />
                    )}
                  </td>
                  <td style={{ padding: "4px 6px" }}>
                    <input type="text" value={s.text} maxLength={MAX_STRING_LEN}
                      style={{ width: "100%" }}
                      onChange={(e) => updateEntry(i, "text", e.target.value)} />
                  </td>
                  <td style={{ padding: "4px 6px", textAlign: "center" }}>
                    <button onClick={() => removeEntry(i)} title="删除"
                      style={{ background: "none", border: "none", color: "#e55", cursor: "pointer", fontSize: 16 }}>
                      &times;
                    </button>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
        {strings.length < MAX_STRING_ENTRIES && (
          <button onClick={addEntry} style={{ marginTop: 8 }}>+ 添加字符串</button>
        )}
        {strings.length === 0 && (
          <div style={{ color: "#999", padding: 12 }}>暂无字符串条目，TEXT_SET action 需要引用此表</div>
        )}
      </fieldset>
    </div>
  );
}