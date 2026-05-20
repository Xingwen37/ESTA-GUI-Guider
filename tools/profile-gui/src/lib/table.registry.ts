import type React from "react";
import type { ProfileSet } from "./types";
import TableEditor from "../components/TableEditor";
import type { ComponentEntry } from "./componentRegistry";

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

export const MAX_TABLE_INST = 4;
export const MAX_TABLE_ROWS = 8;
export const MAX_TABLE_COLS = 6;
export const MAX_TABLE_STRING_LEN = 16;

export const TABLE_THEME_OPTIONS = [
  ["TABLE_THEME_DEFAULT", "Default"],
  ["TABLE_THEME_LIGHT", "Light"],
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

export function makeDefaultTable(): TableProfile {
  return {
    ...EMPTY_TABLE,
    cols: EMPTY_TABLE.cols.map((c) => ({ ...c })),
    cells: EMPTY_TABLE.cells.map((row) => row.map((cell) => ({ ...cell }))),
  };
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

export const tableEntry: ComponentEntry<TableProfile> = {
  key: "table",
  label: "TABLE",
  countField: "table_inst_count" as keyof ProfileSet,
  profilesField: "table_profiles" as keyof ProfileSet,
  maxCount: MAX_TABLE_INST,
  makeDefault: makeDefaultTable,
  validate: validateTable as ComponentEntry["validate"],
  Editor: TableEditor as unknown as React.ComponentType<{ profile: TableProfile; onChange: (p: TableProfile) => void }>,
};
