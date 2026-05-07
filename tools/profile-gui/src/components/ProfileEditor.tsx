import type { EstaProfile } from "../lib/types";
import { THEME_OPTIONS, MAX_RULER_X_NUM, MAX_RULER_Y_NUM, MAX_ESTA_CHANNEL } from "../lib/types";

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

export default function ProfileEditor({ profile, onChange }: Props) {
  const set = (key: keyof EstaProfile, value: unknown) =>
    onChange({ ...profile, [key]: value });

  const toggleChannel = (bit: number) => {
    const newMask = profile.channel_mask ^ (1 << bit);
    set("channel_mask", newMask);
  };

  const setRulerY = (idx: number, val: number) => {
    const arr = [...profile.ruler_y];
    arr[idx] = val;
    set("ruler_y", arr);
  };

  const setRulerX = (idx: number, val: number) => {
    const arr = [...profile.ruler_x];
    arr[idx] = val;
    set("ruler_x", arr);
  };

  return (
    <div>
      {/* 基础参数 */}
      <fieldset className="group-box">
        <legend>基础参数</legend>
        <div className="form-row">
          <label>x_origin</label>
          {spin(profile.x_origin, 0, 65535, (v) => set("x_origin", v))}
        </div>
        <div className="form-row">
          <label>y_origin</label>
          {spin(profile.y_origin, 0, 65535, (v) => set("y_origin", v))}
        </div>
        <div className="form-row">
          <label>x_width</label>
          {spin(profile.x_width, 1, 65535, (v) => set("x_width", v))}
        </div>
        <div className="form-row">
          <label>y_width</label>
          {spin(profile.y_width, 1, 65535, (v) => set("y_width", v))}
        </div>
        <div className="form-row">
          <label>display_num_min</label>
          {spin(profile.display_num_min, 0, 65535, (v) => set("display_num_min", v))}
        </div>
        <div className="form-row">
          <label>display_num_max</label>
          {spin(profile.display_num_max, 0, 65535, (v) => set("display_num_max", v))}
        </div>
        <div className="form-row">
          <label>channel_num</label>
          {spin(profile.channel_num, 1, MAX_ESTA_CHANNEL, (v) => set("channel_num", v))}
        </div>
        <div className="form-row">
          <label>theme_type</label>
          <select
            value={profile.theme_type}
            onChange={(e) => set("theme_type", e.target.value)}
          >
            {THEME_OPTIONS.map(([value, text]) => (
              <option key={value} value={value}>
                {text}
              </option>
            ))}
          </select>
        </div>
        <div className="form-row">
          <label>is_auto_clear</label>
          <input
            type="checkbox"
            checked={profile.is_auto_clear}
            onChange={(e) => set("is_auto_clear", e.target.checked)}
          />
        </div>
      </fieldset>

      {/* 通道使能 */}
      <fieldset className="group-box">
        <legend>通道使能</legend>
        <div className="ch-row">
          {Array.from({ length: MAX_ESTA_CHANNEL }, (_, i) => (
            <label key={i}>
              <input
                type="checkbox"
                checked={((profile.channel_mask >> i) & 1) !== 0}
                onChange={() => toggleChannel(i)}
              />
              CH{i}
            </label>
          ))}
        </div>
      </fieldset>

      {/* Y 标尺 */}
      <fieldset className="group-box">
        <legend>Y 标尺</legend>
        <div className="form-row">
          <label>is_display_ruler_y</label>
          <input
            type="checkbox"
            checked={profile.is_display_ruler_y}
            onChange={(e) => set("is_display_ruler_y", e.target.checked)}
          />
        </div>
        <div className="form-row">
          <label>ruler_y[]</label>
          <div className="ruler-values">
            {Array.from({ length: MAX_RULER_Y_NUM }, (_, i) => (
              <input
                key={i}
                type="number"
                value={profile.ruler_y[i] ?? 0}
                min={0}
                max={65535}
                onChange={(e) => setRulerY(i, Number(e.target.value) || 0)}
              />
            ))}
          </div>
        </div>
        <div className="form-row">
          <label>ruler_count_y</label>
          {spin(profile.ruler_count_y, 0, MAX_RULER_Y_NUM, (v) => set("ruler_count_y", v))}
        </div>
        <div className="form-row">
          <label>ruler_num_digits_y</label>
          {spin(profile.ruler_num_digits_y, 0, 16, (v) => set("ruler_num_digits_y", v))}
        </div>
      </fieldset>

      {/* X 标尺 */}
      <fieldset className="group-box">
        <legend>X 标尺</legend>
        <div className="form-row">
          <label>is_display_ruler_x</label>
          <input
            type="checkbox"
            checked={profile.is_display_ruler_x}
            onChange={(e) => set("is_display_ruler_x", e.target.checked)}
          />
        </div>
        <div className="form-row">
          <label>ruler_x[]</label>
          <div className="ruler-values">
            {Array.from({ length: MAX_RULER_X_NUM }, (_, i) => (
              <input
                key={i}
                type="number"
                value={profile.ruler_x[i] ?? 0}
                min={0}
                max={65535}
                onChange={(e) => setRulerX(i, Number(e.target.value) || 0)}
              />
            ))}
          </div>
        </div>
        <div className="form-row">
          <label>ruler_count_x</label>
          {spin(profile.ruler_count_x, 0, MAX_RULER_X_NUM, (v) => set("ruler_count_x", v))}
        </div>
        <div className="form-row">
          <label>ruler_zero_value_x</label>
          {spin(profile.ruler_zero_value_x, 0, 65535, (v) => set("ruler_zero_value_x", v))}
        </div>
        <div className="form-row">
          <label>ruler_full_value_x</label>
          {spin(profile.ruler_full_value_x, 0, 65535, (v) => set("ruler_full_value_x", v))}
        </div>
        <div className="form-row">
          <label>ruler_num_digits_x</label>
          {spin(profile.ruler_num_digits_x, 0, 16, (v) => set("ruler_num_digits_x", v))}
        </div>
      </fieldset>
    </div>
  );
}
