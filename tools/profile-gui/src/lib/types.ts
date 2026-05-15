export interface WaveRulerLabel {
  value_type: string;
  int_value: number;
  float_value: number;
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
  ruler_num_digits_y: number;
  ruler_font_size_y: string;
  is_display_ruler_x: boolean;
  ruler_x: number[];
  ruler_label_x: WaveRulerLabel[];
  ruler_unit_x: string;
  ruler_precision_x: number;
  ruler_count_x: number;
  ruler_zero_value_x: number;
  ruler_full_value_x: number;
  ruler_num_digits_x: number;
  ruler_font_size_x: string;
  theme_type: string;
  is_auto_clear: boolean;
  is_use_batch_draw: boolean;
}

export interface BarChartProfile {
  x_origin: number;
  y_origin: number;
  x_width: number;
  y_width: number;
  display_num_min: number;
  display_num_max: number;
  bar_count: number;
  bar_width: number;
  bar_spacing: number;
  is_display_value: boolean;
  is_display_axis: boolean;
  font_size: string;
  theme_type: string;
}

export interface TableColProfile {
  header: string;
  cell_type: string;
  width: number;
  precision: number;
}

export interface TableCellProfile {
  text: string;
  u32: number;
  f32: number;
}

export interface TableProfile {
  x_origin: number;
  y_origin: number;
  x_width: number;
  y_width: number;
  row_count: number;
  col_count: number;
  row_height: number;
  is_show_header: boolean;
  is_show_frame: boolean;
  is_show_row_line: boolean;
  is_show_col_line: boolean;
  is_fill_background: boolean;
  font_size: string;
  theme_type: string;
  cols: TableColProfile[];
  cells: TableCellProfile[][];
}

export interface MenuItemProfile {
  label: string;
  parent_idx: number;
  is_submenu: boolean;
  event_id: number;
}

export interface MenuProfile {
  x_origin: number;
  y_origin: number;
  x_width: number;
  y_width: number;
  item_count: number;
  item_height: number;
  breadcrumb_height: number;
  is_show_frame: boolean;
  is_show_breadcrumb: boolean;
  is_fill_background: boolean;
  font_size: string;
  theme_type: string;
  items: MenuItemProfile[];
}

export interface ProfileSet {
  wave_inst_count: number;
  bar_inst_count: number;
  table_inst_count: number;
  menu_inst_count: number;
  button_count: number;
  wave_profiles: WaveProfile[];
  bar_profiles: BarChartProfile[];
  table_profiles: TableProfile[];
  menu_profiles: MenuProfile[];
}

export const WAVE_THEME_OPTIONS = [
  ["WAVE_THEME_DEFAULT", "Default"],
  ["WAVE_THEME_LIGHT", "Light"],
] as const;

export const BAR_THEME_OPTIONS = [
  ["BARCHART_THEME_DEFAULT", "Default"],
  ["BARCHART_THEME_LIGHT", "Light"],
] as const;

export const TABLE_THEME_OPTIONS = [
  ["TABLE_THEME_DEFAULT", "Default"],
  ["TABLE_THEME_LIGHT", "Light"],
] as const;

export const UI_FONT_SIZE_OPTIONS = [
  ["ESTA_FONT_1206", "6x12"],
  ["ESTA_FONT_1608", "8x16"],
  ["ESTA_FONT_2412", "12x24"],
] as const;

export const WAVE_RULER_LABEL_TYPE_OPTIONS = [
  ["WAVE_RULER_LABEL_INT", "Integer"],
  ["WAVE_RULER_LABEL_FLOAT", "Float"],
] as const;

export const TABLE_VALUE_KIND_OPTIONS = [
  ["TABLE_VALUE_NUMBER", "Number"],
  ["TABLE_VALUE_TEXT", "Text"],
] as const;

export const TABLE_NUMBER_TYPE_OPTIONS = [
  ["TABLE_NUMBER_UINT32", "UInt32"],
  ["TABLE_NUMBER_FLOAT", "Float"],
] as const;

export const TABLE_CELL_TYPE_OPTIONS = [
  ["TABLE_CELL_TEXT", "Text"],
  ["TABLE_CELL_UINT32", "UInt32"],
  ["TABLE_CELL_FLOAT", "Float"],
] as const;

export const MENU_THEME_OPTIONS = [
  ["MENU_THEME_DEFAULT", "Default"],
  ["MENU_THEME_LIGHT", "Light"],
] as const;

export const MAX_WAVE_INST = 4;
export const MAX_BAR_INST = 4;
export const MAX_TABLE_INST = 4;
export const MAX_MENU_INST = 4;
export const MAX_BUTTON_COUNT = 8;
export const MAX_WAVE_CHANNEL = 4;
export const MAX_RULER_X_NUM = 10;
export const MAX_RULER_Y_NUM = 10;
export const MAX_TABLE_ROWS = 8;
export const MAX_TABLE_COLS = 6;
export const MAX_TABLE_STRING_LEN = 16;
export const MAX_MENU_ITEMS = 32;
export const MAX_MENU_STRING_LEN = 16;
export const MAX_MENU_DEPTH = 8;
