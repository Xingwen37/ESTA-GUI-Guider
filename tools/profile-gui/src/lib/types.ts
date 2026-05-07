export interface OscProfile {
  x_origin: number;
  y_origin: number;
  x_width: number;
  y_width: number;
  display_num_min: number;
  display_num_max: number;
  channel_num: number;
  channel_mask: number;
  is_display_ruler_y: boolean;
  ruler_y: number[];
  ruler_count_y: number;
  ruler_num_digits_y: number;
  is_display_ruler_x: boolean;
  ruler_x: number[];
  ruler_count_x: number;
  ruler_zero_value_x: number;
  ruler_full_value_x: number;
  ruler_num_digits_x: number;
  theme_type: string;
  is_auto_clear: boolean;
}

export interface ProfileSet {
  osc_count: number;
  profiles: OscProfile[];
}

export const THEME_OPTIONS = [
  ["OSC_THEME_DEFAULT", "Default"],
  ["OSC_THEME_LIGHT", "Light"],
] as const;

export const MAX_OSC_INST = 2;
export const MAX_OSC_CHANNEL = 4;
export const MAX_RULER_X_NUM = 5;
export const MAX_RULER_Y_NUM = 5;
