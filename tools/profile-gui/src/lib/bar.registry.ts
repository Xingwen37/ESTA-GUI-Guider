import type React from "react";
import type { ProfileSet } from "./types";
import BarChartEditor from "../components/BarChartEditor";
import type { ComponentEntry } from "./componentRegistry";

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

export const MAX_BAR_INST = 4;

export const BAR_THEME_OPTIONS = [
  ["BARCHART_THEME_DEFAULT", "Default"],
  ["BARCHART_THEME_LIGHT", "Light"],
] as const;

const EMPTY_BAR: BarChartProfile = {
  x_origin: 10, y_origin: 125, x_width: 300, y_width: 110,
  display_num_min: 0, display_num_max: 100,
  bar_count: 6, bar_width: 0, bar_spacing: 0,
  is_display_value: true, is_display_axis: true,
  font_size: "ESTA_FONT_1608",
  theme_type: "BARCHART_THEME_DEFAULT",
  page: 0,
};

export function makeDefaultBar(): BarChartProfile {
  return { ...EMPTY_BAR };
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

export const barEntry: ComponentEntry<BarChartProfile> = {
  key: "bar",
  label: "BARCHART",
  countField: "bar_inst_count" as keyof ProfileSet,
  profilesField: "bar_profiles" as keyof ProfileSet,
  maxCount: MAX_BAR_INST,
  makeDefault: makeDefaultBar,
  validate: validateBar as ComponentEntry["validate"],
  Editor: BarChartEditor as unknown as React.ComponentType<{ profile: BarChartProfile; onChange: (p: BarChartProfile) => void }>,
};
