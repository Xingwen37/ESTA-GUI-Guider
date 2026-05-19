import type React from "react";
import type {
  ProfileSet,
  WaveProfile,
  BarChartProfile,
  TableProfile,
  MenuProfile,
} from "./types";
import WaveEditor from "../components/WaveEditor";
import BarChartEditor from "../components/BarChartEditor";
import TableEditor from "../components/TableEditor";
import MenuEditor from "../components/MenuEditor";
import {
  MAX_WAVE_INST,
  MAX_BAR_INST,
  MAX_TABLE_INST,
  MAX_MENU_INST,
  MAX_RULER_X_NUM,
  MAX_RULER_Y_NUM,
  MAX_TABLE_ROWS,
} from "./types";

// ---- ComponentEntry interface ----

export interface ComponentEntry<P = unknown> {
  key: string;
  label: string;
  countField: keyof ProfileSet;
  profilesField: keyof ProfileSet;
  maxCount: number;
  makeDefault: () => P;
  validate: (profiles: P[], count: number) => string | null;
  Editor: React.ComponentType<{ profile: P; onChange: (p: P) => void }>;
}

// ---- Default values ----

const labelFromPosition = (value: number) => ({ value });

const DEFAULT_RULER_Y = [1000, 2000, 3000, 4000, 0, 0, 0, 0, 0, 0, 0];
const DEFAULT_RULER_X = [30, 50, 90, 0, 0, 0, 0, 0, 0, 0, 0];

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

const EMPTY_BAR: BarChartProfile = {
  x_origin: 10, y_origin: 125, x_width: 300, y_width: 110,
  display_num_min: 0, display_num_max: 100,
  bar_count: 6, bar_width: 0, bar_spacing: 0,
  is_display_value: true, is_display_axis: true,
  font_size: "ESTA_FONT_1608",
  theme_type: "BARCHART_THEME_DEFAULT",
  page: 0,
};

const EMPTY_TABLE: TableProfile = {
  x_origin: 210, y_origin: 0, x_width: 110, y_width: 72,
  row_count: 2, col_count: 3, row_height: 20,
  is_show_header: false,
  is_show_frame: true,
  is_show_row_line: false,
  is_show_col_line: false,
  is_fill_background: true,
  font_size: "ESTA_FONT_1608",
  theme_type: "TABLE_THEME_LIGHT",
  cols: [
    { header: "Name", cell_type: "TABLE_CELL_TEXT", width: 0, precision: 0 },
    { header: "Value", cell_type: "TABLE_CELL_UINT32", width: 0, precision: 0 },
    { header: "Unit", cell_type: "TABLE_CELL_TEXT", width: 0, precision: 0 },
  ],
  cells: [
    [{ text: "Vpp", u32: 0, f32: 0 }, { text: "", u32: 1000, f32: 0 }, { text: "mV", u32: 0, f32: 0 }],
    [{ text: "Fre", u32: 0, f32: 0 }, { text: "", u32: 1230, f32: 0 }, { text: "Hz", u32: 0, f32: 0 }],
  ],
  page: 0,
};

const DEFAULT_MENU_ITEMS = [
  { label: "Settings", parent_idx: 255, is_submenu: true, event_id: 0 },
  { label: "Display", parent_idx: 0, is_submenu: true, event_id: 0 },
  { label: "Brightness", parent_idx: 1, is_submenu: false, event_id: 10 },
  { label: "Backlight", parent_idx: 1, is_submenu: false, event_id: 11 },
  { label: "Calibrate", parent_idx: 255, is_submenu: false, event_id: 20 },
  { label: "About", parent_idx: 255, is_submenu: false, event_id: 30 },
];

const EMPTY_MENU: MenuProfile = {
  x_origin: 10, y_origin: 10, x_width: 160, y_width: 200,
  item_count: 6, item_height: 30, breadcrumb_height: 20,
  is_show_frame: true,
  is_show_breadcrumb: true,
  is_fill_background: true,
  font_size: "ESTA_FONT_1608",
  theme_type: "MENU_THEME_DEFAULT",
  items: DEFAULT_MENU_ITEMS.map((item) => ({ ...item })),
  page: 0,
};

// ---- Clone factories ----

export function makeDefaultWave(): WaveProfile {
  return {
    ...EMPTY_WAVE,
    ruler_y: [...EMPTY_WAVE.ruler_y],
    ruler_x: [...EMPTY_WAVE.ruler_x],
    ruler_label_y: EMPTY_WAVE.ruler_label_y.map((l) => ({ ...l })),
    ruler_label_x: EMPTY_WAVE.ruler_label_x.map((l) => ({ ...l })),
  };
}

export function makeDefaultBar(): BarChartProfile {
  return { ...EMPTY_BAR };
}

export function makeDefaultTable(): TableProfile {
  return {
    ...EMPTY_TABLE,
    cols: EMPTY_TABLE.cols.map((c) => ({ ...c })),
    cells: EMPTY_TABLE.cells.map((row) => row.map((cell) => ({ ...cell }))),
  };
}

export function makeDefaultMenu(): MenuProfile {
  return {
    ...EMPTY_MENU,
    items: EMPTY_MENU.items.map((item) => ({ ...item })),
  };
}

// ---- Validators ----

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

function validateBar(profiles: BarChartProfile[], count: number): string | null {
  for (let i = 0; i < count; i++) {
    const p = profiles[i];
    if (p.display_num_min >= p.display_num_max)
      return `BARCHART${i}: display_num_min 必须小于 display_num_max`;
    if (p.bar_count < 1 || p.bar_count > 32)
      return `BARCHART${i}: bar_count 必须在 1-32 之间`;
  }
  return null;
}

function validateTable(profiles: TableProfile[], count: number): string | null {
  for (let i = 0; i < count; i++) {
    const p = profiles[i];
    if (p.row_count > MAX_TABLE_ROWS)
      return `TABLE${i}: row_count exceeds limit`;
    if (p.col_count > 6)
      return `TABLE${i}: col_count exceeds limit`;
    if (p.cols.length < p.col_count)
      return `TABLE${i}: cols length is smaller than col_count`;
  }
  return null;
}

function validateMenu(profiles: MenuProfile[], count: number): string | null {
  for (let i = 0; i < count; i++) {
    const p = profiles[i];
    if (p.item_count > 32)
      return `MENU${i}: item_count exceeds limit`;
    if (p.items.length < p.item_count)
      return `MENU${i}: items length is smaller than item_count`;
  }
  return null;
}

// ---- Registry ----

export const COMPONENT_REGISTRY: ComponentEntry[] = [
  {
    key: "wave",
    label: "WAVE",
    countField: "wave_inst_count",
    profilesField: "wave_profiles",
    maxCount: MAX_WAVE_INST,
    makeDefault: makeDefaultWave,
    validate: validateWave as ComponentEntry["validate"],
    Editor: WaveEditor as ComponentEntry["Editor"],
  },
  {
    key: "bar",
    label: "BARCHART",
    countField: "bar_inst_count",
    profilesField: "bar_profiles",
    maxCount: MAX_BAR_INST,
    makeDefault: makeDefaultBar,
    validate: validateBar as ComponentEntry["validate"],
    Editor: BarChartEditor as ComponentEntry["Editor"],
  },
  {
    key: "table",
    label: "TABLE",
    countField: "table_inst_count",
    profilesField: "table_profiles",
    maxCount: MAX_TABLE_INST,
    makeDefault: makeDefaultTable,
    validate: validateTable as ComponentEntry["validate"],
    Editor: TableEditor as ComponentEntry["Editor"],
  },
  {
    key: "menu",
    label: "MENU",
    countField: "menu_inst_count",
    profilesField: "menu_profiles",
    maxCount: MAX_MENU_INST,
    makeDefault: makeDefaultMenu,
    validate: validateMenu as ComponentEntry["validate"],
    Editor: MenuEditor as ComponentEntry["Editor"],
  },
];
