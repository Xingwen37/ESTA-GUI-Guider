import type React from "react";
import WaveEditor from "../components/WaveEditor";
import type { ComponentEntry } from "./componentRegistry";

export interface WaveRulerLabel {
  value: number;
}

export interface WaveProfile {
  x_origin: number;
  y_origin: number;
  x_width: number;
  y_width: number;
  display_num_min: number;
  display_num_max: number;
  x_scale: number;
  channel_num: number;
  channel_mask: number;
  is_display_ruler_y: boolean;
  ruler_y: number[];
  ruler_label_y: WaveRulerLabel[];
  ruler_unit_y: string;
  ruler_precision_y: number;
  ruler_count_y: number;
  is_display_ruler_x: boolean;
  ruler_x: number[];
  ruler_label_x: WaveRulerLabel[];
  ruler_unit_x: string;
  ruler_precision_x: number;
  ruler_count_x: number;
  ruler_zero_value_x: number;
  ruler_full_value_x: number;
  ruler_font_size: string;
  theme_type: string;
  is_auto_clear: boolean;
  is_use_batch_draw: boolean;
  page: number;
}

export const MAX_WAVE_INST = 4;
export const MAX_WAVE_CHANNEL = 4;
export const MAX_RULER_X_NUM = 11;
export const MAX_RULER_Y_NUM = 11;

export const WAVE_THEME_OPTIONS = [
  ["WAVE_THEME_DEFAULT", "Default"],
  ["WAVE_THEME_LIGHT", "Light"],
] as const;

export const WAVE_RULER_LABEL_TYPE_OPTIONS = [
  ["WAVE_RULER_LABEL_INT", "Integer"],
  ["WAVE_RULER_LABEL_FLOAT", "Float"],
] as const;

const DEFAULT_RULER_Y = [1000, 2000, 3000, 4000, 0, 0, 0, 0, 0, 0, 0];
const DEFAULT_RULER_X = [30, 50, 90, 0, 0, 0, 0, 0, 0, 0, 0];
const labelFromPosition = (value: number) => ({ value });

const EMPTY_WAVE: WaveProfile = {
  x_origin: 0, y_origin: 0, x_width: 200, y_width: 120,
  display_num_min: 0, display_num_max: 4095,
  x_scale: 1,
  channel_num: 4, channel_mask: 0b00001111,
  is_display_ruler_y: true,
  ruler_y: DEFAULT_RULER_Y,
  ruler_label_y: DEFAULT_RULER_Y.map(labelFromPosition),
  ruler_unit_y: "",
  ruler_precision_y: 0,
  ruler_count_y: 4,
  is_display_ruler_x: true,
  ruler_x: DEFAULT_RULER_X,
  ruler_label_x: DEFAULT_RULER_X.map(labelFromPosition),
  ruler_unit_x: "",
  ruler_precision_x: 0,
  ruler_count_x: 3, ruler_zero_value_x: 0, ruler_full_value_x: 100,
  ruler_font_size: "ESTA_FONT_1608",
  theme_type: "WAVE_THEME_DEFAULT",
  is_auto_clear: true,
  is_use_batch_draw: false,
  page: 0,
};

export function makeDefaultWave(): WaveProfile {
  return {
    ...EMPTY_WAVE,
    ruler_y: [...EMPTY_WAVE.ruler_y],
    ruler_x: [...EMPTY_WAVE.ruler_x],
    ruler_label_y: EMPTY_WAVE.ruler_label_y.map((l) => ({ ...l })),
    ruler_label_x: EMPTY_WAVE.ruler_label_x.map((l) => ({ ...l })),
  };
}

function validateWave(profiles: WaveProfile[], count: number): string | null {
  for (let i = 0; i < count; i++) {
    const p = profiles[i];
    if (p.display_num_min >= p.display_num_max)
      return `WAVE${i}: display_num_min 必须小于 display_num_max`;
    if (p.x_scale < 1)
      return `WAVE${i}: x_scale 必须大于等于 1`;
    if (p.ruler_count_x > MAX_RULER_X_NUM)
      return `WAVE${i}: ruler_count_x 超过上限`;
    if (p.ruler_count_y > MAX_RULER_Y_NUM)
      return `WAVE${i}: ruler_count_y 超过上限`;
  }
  return null;
}

export const waveEntry: ComponentEntry<WaveProfile> = {
  key: "wave",
  label: "WAVE",
  countField: "wave_inst_count" as string,
  profilesField: "wave_profiles" as string,
  maxCount: MAX_WAVE_INST,
  makeDefault: makeDefaultWave,
  validate: validateWave as ComponentEntry["validate"],
  Editor: WaveEditor as unknown as React.ComponentType<{ profile: WaveProfile; onChange: (p: WaveProfile) => void }>,
};
