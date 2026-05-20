import type React from "react";
import type { ProfileSet } from "./types";
import { waveEntry } from "./wave.registry";
import { barEntry } from "./bar.registry";
import { tableEntry } from "./table.registry";
import { menuEntry } from "./menu.registry";

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

export const COMPONENT_REGISTRY: ComponentEntry[] = [
  waveEntry as unknown as ComponentEntry,
  barEntry as unknown as ComponentEntry,
  tableEntry as unknown as ComponentEntry,
  menuEntry as unknown as ComponentEntry,
];
