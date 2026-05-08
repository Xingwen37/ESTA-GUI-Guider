export interface EstaProfile {
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
  bar_x_origin: number;
  bar_y_origin: number;
  bar_x_width: number;
  bar_y_width: number;
  bar_display_num_min: number;
  bar_display_num_max: number;
  bar_count: number;
  bar_width: number;
  bar_spacing: number;
  bar_is_display_value: boolean;
  bar_is_display_axis: boolean;
  bar_theme_type: string;
}

export interface ProfileSet {
  inst_count: number;
  profiles: EstaProfile[];
}

export const THEME_OPTIONS = [
  ["WAVE_THEME_DEFAULT", "Default"],
  ["WAVE_THEME_LIGHT", "Light"],
] as const;

export const MAX_WAVE_INST = 2;
export const MAX_WAVE_CHANNEL = 4;
export const MAX_RULER_X_NUM = 5;
export const MAX_RULER_Y_NUM = 5;
