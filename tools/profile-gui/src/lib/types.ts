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
  wave_profiles: import("./wave.registry").WaveProfile[];
  bar_profiles: import("./bar.registry").BarChartProfile[];
  table_profiles: import("./table.registry").TableProfile[];
  menu_profiles: import("./menu.registry").MenuProfile[];
  bindings: EventBinding[];
  string_count: number;
  strings: StringEntry[];
  sequence_count: number;
  sequences: ActionSequence[];
}

export const UI_FONT_SIZE_OPTIONS = [
  ["ESTA_FONT_1206", "6x12"],
  ["ESTA_FONT_1608", "8x16"],
  ["ESTA_FONT_2412", "12x24"],
] as const;

export const MAX_BUTTON_COUNT = 8;
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
  0: [3, 4, 11, 255],
  1: [3, 11, 255],
  2: [10, 11, 255],
  3: [5, 6, 7, 8, 10, 11, 255],
  4: [1, 2, 11, 255],
  5: [3, 11, 255],
  6: [9, 11, 255],
};

export const SOURCE_ANY = 0xFF;
export const TRIGGER_ID_ANY = 0xFFFF;
