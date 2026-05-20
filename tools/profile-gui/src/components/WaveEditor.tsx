import { useState, useRef } from "react";
import type { WaveProfile, WaveRulerLabel } from "../lib/wave.registry";
import {
  MAX_WAVE_CHANNEL,
  MAX_RULER_X_NUM,
  MAX_RULER_Y_NUM,
  WAVE_THEME_OPTIONS,
} from "../lib/wave.registry";
import { UI_FONT_SIZE_OPTIONS } from "../lib/types";

interface Props {
  profile: WaveProfile;
  onChange: (p: WaveProfile) => void;
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

function FloatInput({ value, onChange, ...props }: {
  value: number;
  onChange: (v: number) => void;
} & Omit<React.InputHTMLAttributes<HTMLInputElement>, "value" | "onChange" | "type">) {
  const [text, setText] = useState(String(value));
  const prev = useRef(value);

  if (prev.current !== value) {
    prev.current = value;
    setText(String(value));
  }

  return (
    <input
      {...props}
      type="text"
      inputMode="decimal"
      value={text}
      onChange={(e) => {
        setText(e.target.value);
        const n = parseFloat(e.target.value);
        if (!isNaN(n)) {
          prev.current = n;
          onChange(n);
        }
      }}
      onBlur={() => setText(String(value))}
    />
  );
}

interface RulerGenState { min: number; max: number; count: number; }

function RulerEditor({ axis, profile, onChange }: {
  axis: "y" | "x"; profile: WaveProfile; onChange: (p: WaveProfile) => void;
}) {
  const isY = axis === "y";
  const labels: WaveRulerLabel[] = isY ? profile.ruler_label_y : profile.ruler_label_x;
  const positions: number[] = isY ? profile.ruler_y : profile.ruler_x;
  const count = isY ? profile.ruler_count_y : profile.ruler_count_x;
  const unit = isY ? profile.ruler_unit_y : profile.ruler_unit_x;
  const precision = isY ? profile.ruler_precision_y : profile.ruler_precision_x;
  const isDisplay = isY ? profile.is_display_ruler_y : profile.is_display_ruler_x;
  const maxCount = isY ? MAX_RULER_Y_NUM : MAX_RULER_X_NUM;

  const [gen, setGen] = useState<RulerGenState>({ min: 0, max: isY ? 4000 : 100, count: Math.max(count, 2) });

  const set = (key: string, value: unknown) => onChange({ ...profile, [key]: value });

  const handleGenerate = () => {
    const n = Math.max(2, Math.min(maxCount, gen.count));
    const newPos = [...positions];
    const newLabels = [...labels];

    const dataMin = isY ? profile.display_num_min : profile.ruler_zero_value_x;
    const dataMax = isY ? profile.display_num_max : profile.ruler_full_value_x;

    for (let i = 0; i < n; i++) {
      const t = i / (n - 1);
      const labelVal = gen.min + (gen.max - gen.min) * t;
      const posVal = dataMin + (dataMax - dataMin) * t;
      newPos[i] = Math.round(posVal);
      newLabels[i] = { value: labelVal };
    }
    for (let i = n; i < maxCount; i++) { newPos[i] = 0; newLabels[i] = { value: 0 }; }
    const step = Math.abs((gen.max - gen.min) / (n - 1));
    const autoPrecision = step > 0 && step < 1
      ? Math.min(4, Math.ceil(-Math.log10(step)))
      : 0;
    if (isY) onChange({ ...profile, ruler_y: newPos, ruler_label_y: newLabels, ruler_count_y: n, ruler_precision_y: autoPrecision });
    else onChange({ ...profile, ruler_x: newPos, ruler_label_x: newLabels, ruler_count_x: n, ruler_precision_x: autoPrecision });
  };

  const updateTick = (idx: number, labelVal: number) => {
    const newPos = [...positions];
    const newLabels = [...labels];
    newLabels[idx] = { value: labelVal };

    const dataMin = isY ? profile.display_num_min : profile.ruler_zero_value_x;
    const dataMax = isY ? profile.display_num_max : profile.ruler_full_value_x;
    const allLabels = newLabels.slice(0, count);
    const labelMin = Math.min(...allLabels.map(l => l.value));
    const labelMax = Math.max(...allLabels.map(l => l.value));
    if (labelMax > labelMin) {
      const t = (labelVal - labelMin) / (labelMax - labelMin);
      newPos[idx] = Math.round(dataMin + t * (dataMax - dataMin));
    }

    if (isY) onChange({ ...profile, ruler_y: newPos, ruler_label_y: newLabels });
    else onChange({ ...profile, ruler_x: newPos, ruler_label_x: newLabels });
  };

  return (
    <fieldset className="group-box">
      <legend>{isY ? "Y Axis" : "X Axis"}</legend>
      <div className="form-row">
        <label>show_ruler</label>
        <input type="checkbox" checked={isDisplay}
          onChange={(e) => set(isY ? "is_display_ruler_y" : "is_display_ruler_x", e.target.checked)} />
      </div>
      <div className="form-row">
        <label>unit</label>
        <input type="text" value={unit} maxLength={16} placeholder="e.g. 0.1V"
          onChange={(e) => set(isY ? "ruler_unit_y" : "ruler_unit_x", e.target.value)} />
      </div>
      <div className="form-row">
        <label>precision</label>
        {spin(precision, 0, 4, (v) => set(isY ? "ruler_precision_y" : "ruler_precision_x", v))}
      </div>
      {!isY && (
        <>
          <div className="form-row"><label>zero_value</label>{spin(profile.ruler_zero_value_x, 0, 65535, (v) => set("ruler_zero_value_x", v))}</div>
          <div className="form-row"><label>full_value</label>{spin(profile.ruler_full_value_x, 0, 65535, (v) => set("ruler_full_value_x", v))}</div>
        </>
      )}
      <div className="ruler-gen-row">
        <label>min</label><FloatInput value={gen.min} onChange={(v) => setGen({ ...gen, min: v })} />
        <label>max</label><FloatInput value={gen.max} onChange={(v) => setGen({ ...gen, max: v })} />
        <label>count</label><input type="number" value={gen.count} min={2} max={maxCount} onChange={(e) => setGen({ ...gen, count: Number(e.target.value) || 2 })} />
        <button className="btn-auto-calc" onClick={handleGenerate} type="button">生成</button>
      </div>
      <div className="ruler-ticks">
        {Array.from({ length: count }, (_, i) => (
          <FloatInput key={i} value={labels[i]?.value ?? 0}
            onChange={(v) => updateTick(i, v)} />
        ))}
      </div>
    </fieldset>
  );
}

export default function WaveEditor({ profile, onChange }: Props) {
  const set = (key: keyof WaveProfile, value: unknown) =>
    onChange({ ...profile, [key]: value });

  const toggleChannel = (bit: number) => set("channel_mask", profile.channel_mask ^ (1 << bit));

  const handleAutoCalc = () => {
    const metrics = FONT_METRICS[profile.ruler_font_size] ?? FONT_METRICS["ESTA_FONT_1608"];
    let x_width = 200;
    if (profile.is_display_ruler_y) {
      const maxLabel = Math.max(...profile.ruler_label_y.slice(0, profile.ruler_count_y).map((l) => {
        const s = profile.ruler_precision_y > 0 ? l.value.toFixed(profile.ruler_precision_y) : String(Math.round(l.value));
        return s.length;
      }), 1);
      x_width += maxLabel * metrics.w;
    }
    let y_width = 100;
    if (profile.is_display_ruler_x) y_width += metrics.h;
    onChange({ ...profile, x_width, y_width });
  };

  return (
    <div>
      <fieldset className="group-box">
        <legend>Position & Layout</legend>
        <div className="form-row"><label>x_origin</label>{spin(profile.x_origin, 0, 65535, (v) => set("x_origin", v))}</div>
        <div className="form-row"><label>y_origin</label>{spin(profile.y_origin, 0, 65535, (v) => set("y_origin", v))}</div>
        <div className="form-row"><label>x_width</label>{spin(profile.x_width, 1, 65535, (v) => set("x_width", v))}</div>
        <div className="form-row"><label>y_width</label>{spin(profile.y_width, 1, 65535, (v) => set("y_width", v))}</div>
        <button className="btn-auto-calc" onClick={handleAutoCalc} type="button">自动计算尺寸</button>
      </fieldset>

      <fieldset className="group-box">
        <legend>Waveform</legend>
        <div className="form-row"><label>display_num_min</label>{spin(profile.display_num_min, 0, 65535, (v) => set("display_num_min", v))}</div>
        <div className="form-row"><label>display_num_max</label>{spin(profile.display_num_max, 0, 65535, (v) => set("display_num_max", v))}</div>
        <div className="form-row"><label>x_scale</label>{spin(profile.x_scale, 1, 16, (v) => set("x_scale", v))}</div>
        <div className="form-row"><label>channel_num</label>{spin(profile.channel_num, 1, MAX_WAVE_CHANNEL, (v) => set("channel_num", v))}</div>
        <div className="form-row">
          <label>channels</label>
          <div className="ch-row">
            {Array.from({ length: MAX_WAVE_CHANNEL }, (_, i) => (
              <label key={i}><input type="checkbox" checked={((profile.channel_mask >> i) & 1) !== 0} onChange={() => toggleChannel(i)} />CH{i}</label>
            ))}
          </div>
        </div>
      </fieldset>

      <RulerEditor axis="y" profile={profile} onChange={onChange} />
      <RulerEditor axis="x" profile={profile} onChange={onChange} />

      <fieldset className="group-box">
        <legend>Display</legend>
        <div className="form-row"><label>theme_type</label>
          <select value={profile.theme_type} onChange={(e) => set("theme_type", e.target.value)}>
            {WAVE_THEME_OPTIONS.map(([v, t]) => <option key={v} value={v}>{t}</option>)}
          </select>
        </div>
        <div className="form-row"><label>ruler_font_size</label>
          <select value={profile.ruler_font_size} onChange={(e) => set("ruler_font_size", e.target.value)}>
            {UI_FONT_SIZE_OPTIONS.map(([v, t]) => <option key={v} value={v}>{t}</option>)}
          </select>
        </div>
        <div className="form-row"><label>auto_clear</label><input type="checkbox" checked={profile.is_auto_clear} onChange={(e) => set("is_auto_clear", e.target.checked)} /></div>
        <div className="form-row"><label>batch_draw</label><input type="checkbox" checked={profile.is_use_batch_draw} onChange={(e) => set("is_use_batch_draw", e.target.checked)} /></div>
      </fieldset>
    </div>
  );
}
