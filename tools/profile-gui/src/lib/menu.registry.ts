import type React from "react";
import MenuEditor from "../components/MenuEditor";
import type { ComponentEntry } from "./componentRegistry";

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

export const MAX_MENU_INST = 4;
export const MAX_MENU_ITEMS = 32;
export const MAX_MENU_STRING_LEN = 16;
export const MAX_MENU_DEPTH = 8;

export const MENU_THEME_OPTIONS = [
  ["MENU_THEME_DEFAULT", "Default"],
  ["MENU_THEME_LIGHT", "Light"],
] as const;

const DEFAULT_MENU_ITEMS: MenuItemProfile[] = [
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

export function makeDefaultMenu(): MenuProfile {
  return {
    ...EMPTY_MENU,
    items: EMPTY_MENU.items.map((item) => ({ ...item })),
  };
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

export const menuEntry: ComponentEntry<MenuProfile> = {
  key: "menu",
  label: "MENU",
  countField: "menu_inst_count" as string,
  profilesField: "menu_profiles" as string,
  maxCount: MAX_MENU_INST,
  makeDefault: makeDefaultMenu,
  validate: validateMenu as ComponentEntry["validate"],
  Editor: MenuEditor as unknown as React.ComponentType<{ profile: MenuProfile; onChange: (p: MenuProfile) => void }>,
};
