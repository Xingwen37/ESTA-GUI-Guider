import type React from "react";
import { waveEntry } from "./wave.registry";
import { barEntry } from "./bar.registry";
import { tableEntry } from "./table.registry";
import { menuEntry } from "./menu.registry";

export interface ComponentEntry<P = unknown> {
  key: string;
  label: string;
  countField: string;
  profilesField: string;
  maxCount: number;
  makeDefault: () => P;
  validate: (profiles: P[], count: number) => string | null;
  Editor: React.ComponentType<{
    profile: P;
    onChange: (p: P) => void;
    instIndex?: number;
    onAutoAssign?: (instIndex: number, idMap: Record<number, number>) => void;
  }>;
}

export const COMPONENT_REGISTRY: ComponentEntry[] = [
  waveEntry as unknown as ComponentEntry,
  barEntry as unknown as ComponentEntry,
  tableEntry as unknown as ComponentEntry,
  menuEntry as unknown as ComponentEntry,
];
