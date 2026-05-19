import type { ActionSequence, EventBinding, MenuProfile, StringEntry, TableProfile } from "../lib/types";
import {
  MAX_STRING_ENTRIES,
  MAX_STRING_LEN,
  MAX_TABLE_COLS,
  MAX_TABLE_ROWS,
  MAX_MENU_ITEMS,
} from "../lib/types";

interface Props {
  strings: StringEntry[];
  bindings: EventBinding[];
  sequences: ActionSequence[];
  tableProfiles: TableProfile[];
  menuProfiles: MenuProfile[];
  onChange: (strings: StringEntry[]) => void;
}

const EMPTY_ENTRY: StringEntry = {
  sub_addr: 0,
  text: "",
};

function inferTarget(
  strIdx: number,
  bindings: EventBinding[],
  sequences: ActionSequence[]
): { type: number; inst: number } | null {
  const hits: { type: number; inst: number }[] = [];
  for (const b of bindings) {
    if (b.action === 10 && b.param === strIdx)
      hits.push({ type: b.target_type, inst: b.target_inst });
  }
  for (const seq of sequences) {
    for (const step of seq.steps) {
      if (step.action === 10 && step.param === strIdx)
        hits.push({ type: step.target_type, inst: step.target_inst });
    }
  }
  if (hits.length === 0) return null;
  const first = hits[0];
  if (hits.every(h => h.type === first.type)) return first;
  return null;
}

function getInitialText(
  sub_addr: number,
  target: { type: number; inst: number },
  tableProfiles: TableProfile[],
  menuProfiles: MenuProfile[]
): string | null {
  if (target.type === 2) {
    const tp = tableProfiles[target.inst];
    if (!tp) return null;
    const row = Math.floor(sub_addr / MAX_TABLE_COLS);
    const col = sub_addr % MAX_TABLE_COLS;
    const cell = tp.cells?.[row]?.[col];
    if (!cell) return null;
    const colDef = tp.cols?.[col];
    if (colDef?.cell_type === "TABLE_CELL_TEXT") return cell.text || "(empty)";
    if (colDef?.cell_type === "TABLE_CELL_UINT32") return String(cell.u32);
    if (colDef?.cell_type === "TABLE_CELL_FLOAT") return String(cell.f32);
    return cell.text || null;
  }
  if (target.type === 3) {
    const mp = menuProfiles[target.inst];
    if (!mp) return null;
    return mp.items?.[sub_addr]?.label ?? null;
  }
  return null;
}

const TYPE_LABEL: Record<number, string> = { 2: "TABLE", 3: "MENU" };

export default function StringTableEditor(props: Props) {
  const { strings, bindings, sequences, tableProfiles, menuProfiles, onChange } = props;

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
            {strings.map((s, i) => {
              const target = inferTarget(i, bindings, sequences);
              const typeLabel = target !== null ? (TYPE_LABEL[target.type] ?? "—") : "—";
              const initialText = target !== null
                ? getInitialText(s.sub_addr, target, tableProfiles, menuProfiles)
                : null;
              return (
                <tr key={i} style={{ borderBottom: "1px solid #333" }}>
                  <td style={{ padding: "4px 6px", color: "#888" }}>{i}</td>
                  <td style={{ padding: "4px 6px" }}>
                    {target?.type === 2 ? (
                      <span>
                        R<input type="number" value={Math.floor(s.sub_addr / MAX_TABLE_COLS)}
                          min={0} max={MAX_TABLE_ROWS - 1} style={{ width: 36 }}
                          onChange={(e) => {
                            const row = Number(e.target.value) || 0;
                            const col = s.sub_addr % MAX_TABLE_COLS;
                            updateEntry(i, "sub_addr", row * MAX_TABLE_COLS + col);
                          }} />
                        {" "}C<input type="number" value={s.sub_addr % MAX_TABLE_COLS}
                          min={0} max={MAX_TABLE_COLS - 1} style={{ width: 36 }}
                          onChange={(e) => {
                            const row = Math.floor(s.sub_addr / MAX_TABLE_COLS);
                            const col = Number(e.target.value) || 0;
                            updateEntry(i, "sub_addr", row * MAX_TABLE_COLS + col);
                          }} />
                        <span style={{ color: "#888", marginLeft: 4, fontSize: 11 }}>= {s.sub_addr}</span>
                      </span>
                    ) : target?.type === 3 ? (
                      <span>
                        Item#<input type="number" value={s.sub_addr}
                          min={0} max={MAX_MENU_ITEMS - 1} style={{ width: 44 }}
                          onChange={(e) => updateEntry(i, "sub_addr", Math.min(MAX_MENU_ITEMS - 1, Number(e.target.value) || 0))} />
                      </span>
                    ) : (
                      <input type="number" value={s.sub_addr}
                        min={0} max={255} style={{ width: 52 }}
                        onChange={(e) => updateEntry(i, "sub_addr", Math.min(255, Number(e.target.value) || 0))} />
                    )}
                    <span style={{ color: "#666", marginLeft: 6, fontSize: 11 }}>{typeLabel}</span>
                    {initialText !== null && (
                      <span style={{ color: "#7a9", marginLeft: 4, fontSize: 11, fontStyle: "italic" }}>
                        → "{initialText}"
                      </span>
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
