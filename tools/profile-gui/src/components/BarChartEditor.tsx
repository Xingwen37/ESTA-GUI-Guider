import { useState, useRef } from "react";
import type { BarChartProfile } from "../lib/types";
import { BAR_THEME_OPTIONS, UI_FONT_SIZE_OPTIONS } from "../lib/types";

interface Props {
  profile: BarChartProfile;
  onChange: (p: BarChartProfile) => void;
}

const FONT_METRICS: Record<string, { w: number; h: number }> = {
  "ESTA_FONT_1206": { w: 6, h: 12 },
  "ESTA_FONT_1608": { w: 8, h: 16 },
  "ESTA_FONT_2412": { w: 12, h: 24 },
};

function spin(value: number, min: number, max: number, onChange: (v: number) => void) {
  const [text, setText] = useState(String(value));
  const prev = useRef(value);
  if (prev.current !== value) { prev.current = value; setText(String(value)); }
  return (
    <input type="number" value={text} min={min} max={max}
      onChange={(e) => {
        setText(e.target.value);
        const v = parseInt(e.target.value, 10);
        if (!isNaN(v)) { prev.current = v; onChange(v); }
      }}
      onBlur={() => setText(String(value))} />
  );
}

export default function BarChartEditor({ profile, onChange }: Props) {
  const set = (key: keyof BarChartProfile, value: unknown) =>
    onChange({ ...profile, [key]: value });

  const handleAutoCalc = () => {
    const metrics = FONT_METRICS[profile.font_size] ?? FONT_METRICS["ESTA_FONT_1608"];
    const barCount = Math.max(1, profile.bar_count);

    let bw = profile.bar_width;
    let bs = profile.bar_spacing;
    if (bw === 0 && bs === 0) {
      bw = 10;
      bs = 5;
    }

    const x_width = barCount * bw + (barCount - 1) * bs + 8;

    let y_height = 60;
    if (profile.is_display_value) {
      y_height += metrics.h + 2;
    }
    if (profile.is_display_axis) {
      y_height += 2;
    }

    onChange({
      ...profile,
      x_width,
      y_width: y_height,
      bar_width: bw,
      bar_spacing: bs,
    });
  };

  return (
    <div>
      <fieldset className="group-box">
        <legend>Position & Layout</legend>
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
          <label>bar_width</label>
          {spin(profile.bar_width, 0, 65535, (v) => set("bar_width", v))}
          <span className="hint">(0=auto)</span>
        </div>
        <div className="form-row">
          <label>bar_spacing</label>
          {spin(profile.bar_spacing, 0, 65535, (v) => set("bar_spacing", v))}
          <span className="hint">(0=auto)</span>
        </div>
        <button className="btn-auto-calc" onClick={handleAutoCalc} type="button">自动计算尺寸</button>
      </fieldset>

      <fieldset className="group-box">
        <legend>Data</legend>
        <div className="form-row">
          <label>display_num_min</label>
          {spin(profile.display_num_min, 0, 65535, (v) => set("display_num_min", v))}
        </div>
        <div className="form-row">
          <label>display_num_max</label>
          {spin(profile.display_num_max, 0, 65535, (v) => set("display_num_max", v))}
        </div>
        <div className="form-row">
          <label>bar_count</label>
          {spin(profile.bar_count, 1, 32, (v) => set("bar_count", v))}
        </div>
      </fieldset>

      <fieldset className="group-box">
        <legend>Display</legend>
        <div className="form-row">
          <label>is_display_value</label>
          <input
            type="checkbox"
            checked={profile.is_display_value}
            onChange={(e) => set("is_display_value", e.target.checked)}
          />
        </div>
        <div className="form-row">
          <label>is_display_axis</label>
          <input
            type="checkbox"
            checked={profile.is_display_axis}
            onChange={(e) => set("is_display_axis", e.target.checked)}
          />
        </div>
        <div className="form-row">
          <label>font_size</label>
          <select value={profile.font_size} onChange={(e) => set("font_size", e.target.value)}>
            {UI_FONT_SIZE_OPTIONS.map(([value, text]) => (
              <option key={value} value={value}>{text}</option>
            ))}
          </select>
        </div>
        <div className="form-row">
          <label>theme_type</label>
          <select
            value={profile.theme_type}
            onChange={(e) => set("theme_type", e.target.value)}
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
