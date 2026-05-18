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
  page: number;
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
  page: number;
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
  page: number;
}

export interface EventBinding {
  trigger: number;
  source_id: number;
  trigger_id: number;
  target_type: number;
  target_inst: number;
  action: number;
  param: number;
}

export interface StringEntry {
  sub_addr: number;
  text: string;
}

export interface ActionStep {
  action: number;
  target_type: number;
  target_inst: number;
  param: number;
}

export interface ActionSequence {
  step_count: number;
  steps: ActionStep[];
}

export interface ProfileSet {
  wave_inst_count: number;
  bar_inst_count: number;
  table_inst_count: number;
  menu_inst_count: number;
  button_count: number;
  page_count: number;
  binding_count: number;
  wave_profiles: WaveProfile[];
  bar_profiles: BarChartProfile[];
  table_profiles: TableProfile[];
  menu_profiles: MenuProfile[];
  bindings: EventBinding[];
  string_count: number;
  strings: StringEntry[];
  sequence_count: number;
  sequences: ActionSequence[];
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
export const MAX_RULER_X_NUM = 11;
export const MAX_RULER_Y_NUM = 11;
export const MAX_TABLE_ROWS = 8;
export const MAX_TABLE_COLS = 6;
export const MAX_TABLE_STRING_LEN = 16;
export const MAX_MENU_ITEMS = 32;
export const MAX_MENU_STRING_LEN = 16;
export const MAX_MENU_DEPTH = 8;
export const MAX_BINDINGS = 8;
export const MAX_STRING_ENTRIES = 16;
export const MAX_STRING_LEN = 16;
export const MAX_SEQUENCES = 4;
export const MAX_SEQUENCE_STEPS = 4;

export const TRIGGER_OPTIONS = [
  [1, "BUTTON_PRESS"],
  [2, "BUTTON_RELEASE"],
  [3, "MENU_SELECT"],
  [4, "ENCODER_ROTATE"],
  [5, "TIMER"],
  [6, "FLAG"],
] as const;

export const TARGET_TYPE_OPTIONS = [
  [0, "WAVE"],
  [1, "BARCHART"],
  [2, "TABLE"],
  [3, "MENU"],
  [4, "PAGE"],
  [5, "GLOBAL"],
  [6, "FLAG"],
] as const;

export const ACTION_TYPE_OPTIONS = [
  [1, "PAGE_NEXT"],
  [2, "PAGE_PREV"],
  [3, "THEME_TOGGLE"],
  [4, "WAVE_REDRAW"],
  [5, "MENU_UP"],
  [6, "MENU_DOWN"],
  [7, "MENU_ENTER"],
  [8, "MENU_BACK"],
  [9, "FLAG_SET"],
  [10, "TEXT_SET"],
  [11, "SEQUENCE"],
  [255, "CUSTOM"],
] as const;

export const VALID_ACTIONS: Record<number, number[]> = {
  0: [3, 4, 11, 255],             // WAVE: THEME_TOGGLE, WAVE_REDRAW, SEQUENCE, CUSTOM
  1: [3, 11, 255],                // BARCHART: THEME_TOGGLE, SEQUENCE, CUSTOM
  2: [10, 11, 255],               // TABLE: TEXT_SET, SEQUENCE, CUSTOM
  3: [5, 6, 7, 8, 10, 11, 255],  // MENU: UP, DOWN, ENTER, BACK, TEXT_SET, SEQUENCE, CUSTOM
  4: [1, 2, 11, 255],             // PAGE: PAGE_NEXT, PAGE_PREV, SEQUENCE, CUSTOM
  5: [3, 11, 255],                // GLOBAL: THEME_TOGGLE, SEQUENCE, CUSTOM
  6: [9, 11, 255],                // FLAG: FLAG_SET, SEQUENCE, CUSTOM
};

export const SOURCE_ANY = 0xFF;
export const TRIGGER_ID_ANY = 0xFFFF;
