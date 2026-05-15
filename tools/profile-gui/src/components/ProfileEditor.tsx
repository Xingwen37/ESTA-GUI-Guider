import type { WaveProfile, WaveRulerLabel } from "../lib/types";
import {
  MAX_RULER_X_NUM,
  MAX_RULER_Y_NUM,
  MAX_WAVE_CHANNEL,
  UI_FONT_SIZE_OPTIONS,
  WAVE_RULER_LABEL_TYPE_OPTIONS,
  WAVE_THEME_OPTIONS,
} from "../lib/types";

interface Props {
  profile: WaveProfile;
  onChange: (p: WaveProfile) => void;
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

const emptyLabel = (): WaveRulerLabel => ({
  value_type: "WAVE_RULER_LABEL_INT",
  int_value: 0,
  float_value: 0,
});

export default function ProfileEditor({ profile, onChange }: Props) {
  const set = (key: keyof WaveProfile, value: unknown) =>
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

  const setRulerLabelY = (idx: number, patch: Partial<WaveRulerLabel>) => {
    const arr = [...profile.ruler_label_y];
    arr[idx] = { ...(arr[idx] ?? emptyLabel()), ...patch };
    set("ruler_label_y", arr);
  };

  const setRulerLabelX = (idx: number, patch: Partial<WaveRulerLabel>) => {
    const arr = [...profile.ruler_label_x];
    arr[idx] = { ...(arr[idx] ?? emptyLabel()), ...patch };
    set("ruler_label_x", arr);
  };

  const rulerLabelEditor = (
    axis: "x" | "y",
    max: number,
    positions: number[],
    labels: WaveRulerLabel[],
    onPos: (idx: number, value: number) => void,
    onLabel: (idx: number, patch: Partial<WaveRulerLabel>) => void,
  ) => (
    <div className="ruler-label-grid">
      <div className="ruler-label-row ruler-label-head">
        <span>{axis.toUpperCase()}</span>
        <span>pos</span>
        <span>type</span>
        <span>int</span>
        <span>float</span>
      </div>
      {Array.from({ length: max }, (_, i) => {
        const label = labels[i] ?? emptyLabel();
        return (
          <div className="ruler-label-row" key={`${axis}-${i}`}>
            <span>#{i}</span>
            <input
              type="number"
              title="tick position"
              value={positions[i] ?? 0}
              min={0}
              max={65535}
              onChange={(e) => onPos(i, Number(e.target.value) || 0)}
            />
            <select
              value={label.value_type}
              onChange={(e) => onLabel(i, { value_type: e.target.value })}
            >
              {WAVE_RULER_LABEL_TYPE_OPTIONS.map(([value, text]) => (
                <option key={value} value={value}>{text}</option>
              ))}
            </select>
            <input
              type="number"
              title="integer label"
              value={label.int_value}
              onChange={(e) => onLabel(i, { int_value: Number(e.target.value) || 0 })}
            />
            <input
              type="number"
              title="float label"
              step="0.01"
              value={label.float_value}
              onChange={(e) => onLabel(i, { float_value: Number(e.target.value) || 0 })}
            />
          </div>
        );
      })}
    </div>
  );

  return (
    <div>
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
          <label>x_scale</label>
          {spin(profile.x_scale, 1, 16, (v) => set("x_scale", v < 1 ? 1 : v))}
        </div>
        <div className="form-row">
          <label>channel_num</label>
          {spin(profile.channel_num, 1, MAX_WAVE_CHANNEL, (v) => set("channel_num", v))}
        </div>
        <div className="form-row">
          <label>theme_type</label>
          <select value={profile.theme_type} onChange={(e) => set("theme_type", e.target.value)}>
            {WAVE_THEME_OPTIONS.map(([value, text]) => (
              <option key={value} value={value}>{text}</option>
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
        <div className="form-row">
          <label>is_use_batch_draw</label>
          <input
            type="checkbox"
            checked={profile.is_use_batch_draw}
            onChange={(e) => set("is_use_batch_draw", e.target.checked)}
          />
        </div>
      </fieldset>

      <fieldset className="group-box">
        <legend>通道使能</legend>
        <div className="ch-row">
          {Array.from({ length: MAX_WAVE_CHANNEL }, (_, i) => (
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
          <label>unit / precision</label>
          <input
            type="text"
            value={profile.ruler_unit_y}
            maxLength={16}
            onChange={(e) => set("ruler_unit_y", e.target.value)}
          />
          {spin(profile.ruler_precision_y, 0, 4, (v) => set("ruler_precision_y", v))}
        </div>
        <div className="form-row">
          <label>Y ticks / labels</label>
          {rulerLabelEditor(
            "y",
            MAX_RULER_Y_NUM,
            profile.ruler_y,
            profile.ruler_label_y,
            setRulerY,
            setRulerLabelY,
          )}
        </div>
        <div className="form-row">
          <label>ruler_count_y</label>
          {spin(profile.ruler_count_y, 0, MAX_RULER_Y_NUM, (v) => set("ruler_count_y", v))}
        </div>
        <div className="form-row">
          <label>ruler_num_digits_y</label>
          {spin(profile.ruler_num_digits_y, 0, 16, (v) => set("ruler_num_digits_y", v))}
        </div>
        <div className="form-row">
          <label>ruler_font_size_y</label>
          <select value={profile.ruler_font_size_y} onChange={(e) => set("ruler_font_size_y", e.target.value)}>
            {UI_FONT_SIZE_OPTIONS.map(([value, text]) => (
              <option key={value} value={value}>{text}</option>
            ))}
          </select>
        </div>
      </fieldset>

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
          <label>unit / precision</label>
          <input
            type="text"
            value={profile.ruler_unit_x}
            maxLength={16}
            onChange={(e) => set("ruler_unit_x", e.target.value)}
          />
          {spin(profile.ruler_precision_x, 0, 4, (v) => set("ruler_precision_x", v))}
        </div>
        <div className="form-row">
          <label>X ticks / labels</label>
          {rulerLabelEditor(
            "x",
            MAX_RULER_X_NUM,
            profile.ruler_x,
            profile.ruler_label_x,
            setRulerX,
            setRulerLabelX,
          )}
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
        <div className="form-row">
          <label>ruler_font_size_x</label>
          <select value={profile.ruler_font_size_x} onChange={(e) => set("ruler_font_size_x", e.target.value)}>
            {UI_FONT_SIZE_OPTIONS.map(([value, text]) => (
              <option key={value} value={value}>{text}</option>
            ))}
          </select>
        </div>
      </fieldset>
    </div>
  );
}
