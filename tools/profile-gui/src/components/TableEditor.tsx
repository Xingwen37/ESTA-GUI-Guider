import type { TableProfile, TableRowProfile } from "../lib/types";
import {
  MAX_TABLE_ROWS,
  MAX_TABLE_STRING_LEN,
  TABLE_NUMBER_TYPE_OPTIONS,
  TABLE_THEME_OPTIONS,
  TABLE_VALUE_KIND_OPTIONS,
} from "../lib/types";

interface Props {
  profile: TableProfile;
  onChange: (p: TableProfile) => void;
}

function spin(value: number, min: number, max: number, onChange: (v: number) => void) {
  return (
    <input
      type="number"
      value={value}
      min={min}
      max={max}
      onChange={(e) => onChange(Number(e.target.value) || 0)}
    />
  );
}

function defaultRow(): TableRowProfile {
  return {
    label: "Item",
    value_kind: "TABLE_VALUE_NUMBER",
    number_type: "TABLE_NUMBER_UINT32",
    unit: "",
    precision: 0,
    default_u32: 0,
    default_float: 0,
    default_text: "",
  };
}

function clampText(value: string) {
  return value.slice(0, MAX_TABLE_STRING_LEN);
}

export default function TableEditor({ profile, onChange }: Props) {
  const set = (key: keyof TableProfile, value: unknown) =>
    onChange({ ...profile, [key]: value });

  const setRowCount = (count: number) => {
    const row_count = Math.max(0, Math.min(MAX_TABLE_ROWS, count));
    const rows = profile.rows.slice(0, row_count);
    while (rows.length < row_count) rows.push(defaultRow());
    onChange({ ...profile, row_count, rows });
  };

  const updateRow = (idx: number, patch: Partial<TableRowProfile>) => {
    const rows = [...profile.rows];
    rows[idx] = { ...(rows[idx] ?? defaultRow()), ...patch };
    onChange({ ...profile, rows });
  };

  return (
    <div>
      <fieldset className="group-box">
        <legend>Position</legend>
        <div className="form-row"><label>x_origin</label>{spin(profile.x_origin, 0, 65535, (v) => set("x_origin", v))}</div>
        <div className="form-row"><label>y_origin</label>{spin(profile.y_origin, 0, 65535, (v) => set("y_origin", v))}</div>
        <div className="form-row"><label>x_width</label>{spin(profile.x_width, 1, 65535, (v) => set("x_width", v))}</div>
        <div className="form-row"><label>y_width</label>{spin(profile.y_width, 1, 65535, (v) => set("y_width", v))}</div>
      </fieldset>

      <fieldset className="group-box">
        <legend>Layout</legend>
        <div className="form-row"><label>row_count</label>{spin(profile.row_count, 0, MAX_TABLE_ROWS, setRowCount)}</div>
        <div className="form-row"><label>row_height</label>{spin(profile.row_height, 16, 65535, (v) => set("row_height", v))}</div>
        <div className="form-row"><label>auto_col_width</label><input type="checkbox" checked={profile.is_auto_col_width} onChange={(e) => set("is_auto_col_width", e.target.checked)} /></div>
        <div className="form-row"><label>label_col_width</label>{spin(profile.label_col_width, 0, 65535, (v) => set("label_col_width", v))}</div>
        <div className="form-row"><label>value_col_width</label>{spin(profile.value_col_width, 0, 65535, (v) => set("value_col_width", v))}</div>
        <div className="form-row"><label>unit_col_width</label>{spin(profile.unit_col_width, 0, 65535, (v) => set("unit_col_width", v))}</div>
      </fieldset>

      <fieldset className="group-box">
        <legend>Display</legend>
        <div className="form-row"><label>show_frame</label><input type="checkbox" checked={profile.is_show_frame} onChange={(e) => set("is_show_frame", e.target.checked)} /></div>
        <div className="form-row"><label>show_row_line</label><input type="checkbox" checked={profile.is_show_row_line} onChange={(e) => set("is_show_row_line", e.target.checked)} /></div>
        <div className="form-row"><label>fill_background</label><input type="checkbox" checked={profile.is_fill_background} onChange={(e) => set("is_fill_background", e.target.checked)} /></div>
        <div className="form-row">
          <label>theme_type</label>
          <select value={profile.theme_type} onChange={(e) => set("theme_type", e.target.value)}>
            {TABLE_THEME_OPTIONS.map(([value, text]) => <option key={value} value={value}>{text}</option>)}
          </select>
        </div>
      </fieldset>

      <fieldset className="group-box">
        <legend>Rows</legend>
        {Array.from({ length: profile.row_count }, (_, i) => {
          const row = profile.rows[i] ?? defaultRow();
          return (
            <div key={i} className="group-box" style={{ margin: "8px 0" }}>
              <div className="form-row"><label>label[{i}]</label><input value={row.label} maxLength={MAX_TABLE_STRING_LEN} onChange={(e) => updateRow(i, { label: clampText(e.target.value) })} /></div>
              <div className="form-row">
                <label>value_kind</label>
                <select value={row.value_kind} onChange={(e) => updateRow(i, { value_kind: e.target.value })}>
                  {TABLE_VALUE_KIND_OPTIONS.map(([value, text]) => <option key={value} value={value}>{text}</option>)}
                </select>
              </div>
              <div className="form-row">
                <label>number_type</label>
                <select value={row.number_type} onChange={(e) => updateRow(i, { number_type: e.target.value })}>
                  {TABLE_NUMBER_TYPE_OPTIONS.map(([value, text]) => <option key={value} value={value}>{text}</option>)}
                </select>
              </div>
              <div className="form-row"><label>unit</label><input value={row.unit} maxLength={MAX_TABLE_STRING_LEN} onChange={(e) => updateRow(i, { unit: clampText(e.target.value) })} /></div>
              <div className="form-row"><label>precision</label>{spin(row.precision, 0, 4, (v) => updateRow(i, { precision: v }))}</div>
              <div className="form-row"><label>default_u32</label>{spin(row.default_u32, 0, 4294967295, (v) => updateRow(i, { default_u32: v }))}</div>
              <div className="form-row"><label>default_float</label><input type="number" value={row.default_float} step="0.1" onChange={(e) => updateRow(i, { default_float: Number(e.target.value) || 0 })} /></div>
              <div className="form-row"><label>default_text</label><input value={row.default_text} maxLength={MAX_TABLE_STRING_LEN} onChange={(e) => updateRow(i, { default_text: clampText(e.target.value) })} /></div>
            </div>
          );
        })}
      </fieldset>
    </div>
  );
}
