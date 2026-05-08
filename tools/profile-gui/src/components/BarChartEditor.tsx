import type { EstaProfile } from "../lib/types";
import { BAR_THEME_OPTIONS } from "../lib/types";

interface Props {
  profile: EstaProfile;
  onChange: (p: EstaProfile) => void;
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

export default function BarChartEditor({ profile, onChange }: Props) {
  const set = (key: keyof EstaProfile, value: unknown) =>
    onChange({ ...profile, [key]: value });

  return (
    <div>
      {/* 位置与尺寸 */}
      <fieldset className="group-box">
        <legend>位置与尺寸</legend>
        <div className="form-row">
          <label>bar_x_origin</label>
          {spin(profile.bar_x_origin, 0, 65535, (v) => set("bar_x_origin", v))}
        </div>
        <div className="form-row">
          <label>bar_y_origin</label>
          {spin(profile.bar_y_origin, 0, 65535, (v) => set("bar_y_origin", v))}
        </div>
        <div className="form-row">
          <label>bar_x_width</label>
          {spin(profile.bar_x_width, 1, 65535, (v) => set("bar_x_width", v))}
        </div>
        <div className="form-row">
          <label>bar_y_width</label>
          {spin(profile.bar_y_width, 1, 65535, (v) => set("bar_y_width", v))}
        </div>
      </fieldset>

      {/* 数据与柱体 */}
      <fieldset className="group-box">
        <legend>数据与柱体</legend>
        <div className="form-row">
          <label>bar_display_num_min</label>
          {spin(profile.bar_display_num_min, 0, 65535, (v) => set("bar_display_num_min", v))}
        </div>
        <div className="form-row">
          <label>bar_display_num_max</label>
          {spin(profile.bar_display_num_max, 0, 65535, (v) => set("bar_display_num_max", v))}
        </div>
        <div className="form-row">
          <label>bar_count</label>
          {spin(profile.bar_count, 1, 32, (v) => set("bar_count", v))}
        </div>
      </fieldset>

      {/* 柱体布局 */}
      <fieldset className="group-box">
        <legend>柱体布局</legend>
        <div className="form-row">
          <label>bar_width</label>
          {spin(profile.bar_width, 0, 65535, (v) => set("bar_width", v))}
          <span className="hint">(0=自动)</span>
        </div>
        <div className="form-row">
          <label>bar_spacing</label>
          {spin(profile.bar_spacing, 0, 65535, (v) => set("bar_spacing", v))}
          <span className="hint">(0=自动)</span>
        </div>
      </fieldset>

      {/* 显示选项 */}
      <fieldset className="group-box">
        <legend>显示选项</legend>
        <div className="form-row">
          <label>bar_is_display_value</label>
          <input
            type="checkbox"
            checked={profile.bar_is_display_value}
            onChange={(e) => set("bar_is_display_value", e.target.checked)}
          />
        </div>
        <div className="form-row">
          <label>bar_is_display_axis</label>
          <input
            type="checkbox"
            checked={profile.bar_is_display_axis}
            onChange={(e) => set("bar_is_display_axis", e.target.checked)}
          />
        </div>
        <div className="form-row">
          <label>bar_theme_type</label>
          <select
            value={profile.bar_theme_type}
            onChange={(e) => set("bar_theme_type", e.target.value)}
          >
            {BAR_THEME_OPTIONS.map(([value, text]) => (
              <option key={value} value={value}>
                {text}
              </option>
            ))}
          </select>
        </div>
      </fieldset>
    </div>
  );
}
