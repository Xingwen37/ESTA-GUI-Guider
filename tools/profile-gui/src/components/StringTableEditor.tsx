import type { StringEntry } from "../lib/types";
import { MAX_STRING_ENTRIES, MAX_STRING_LEN, MAX_TABLE_COLS } from "../lib/types";

interface Props {
  strings: StringEntry[];
  onChange: (strings: StringEntry[]) => void;
}

const EMPTY_ENTRY: StringEntry = {
  sub_addr: 0,
  text: "",
};

export default function StringTableEditor(props: Props) {
  const { strings, onChange } = props;

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

  return (
    <div style={{ padding: 12 }}>
      <fieldset className="group-box">
        <legend>字符串表 ({strings.length}/{MAX_STRING_ENTRIES})</legend>
        <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 13 }}>
          <thead>
            <tr style={{ borderBottom: "1px solid #444" }}>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>#</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Sub Addr</th>
              <th style={{ textAlign: "left", padding: "4px 6px" }}>Text</th>
              <th style={{ width: 40 }}></th>
            </tr>
          </thead>
          <tbody>
            {strings.map((s, i) => (
              <tr key={i} style={{ borderBottom: "1px solid #333" }}>
                <td style={{ padding: "4px 6px", color: "#888" }}>{i}</td>
                <td style={{ padding: "4px 6px" }}>
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
                    <span style={{ color: "#888", marginLeft: 4, fontSize: 11 }}>= {s.sub_addr}</span>
                  </span>
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
            ))}
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
