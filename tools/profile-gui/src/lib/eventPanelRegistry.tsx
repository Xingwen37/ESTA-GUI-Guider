import type React from "react";
import type { ProfileSet } from "./types";
import type { MenuProfile } from "./menu.registry";
import type { TableProfile } from "./table.registry";
import type { WaveProfile } from "./wave.registry";
import EventEditor from "../components/EventEditor";
import StringTableEditor from "../components/StringTableEditor";
import SequenceEditor from "../components/SequenceEditor";
import SoftTimerEditor from "../components/SoftTimerEditor";

export interface EventPanelProps {
  data: ProfileSet;
  instCounts: Record<string, number>;
  setData: (data: ProfileSet) => void;
}

export interface EventPanelEntry {
  key: string;
  label: string;
  Component: React.ComponentType<EventPanelProps>;
}

export const EVENT_PANELS: EventPanelEntry[] = [
  {
    key: "bindings",
    label: "Event Bindings",
    Component: ({ data, instCounts, setData }) => (
      <EventEditor
        bindings={data.bindings ?? []}
        buttonCount={data.button_count}
        instCounts={instCounts}
        menuProfiles={(data["menu_profiles"] as MenuProfile[]) ?? []}
        strings={data.strings ?? []}
        sequences={data.sequences ?? []}
        onChange={(bindings) => setData({ ...data, bindings, binding_count: bindings.length })}
      />
    ),
  },
  {
    key: "strings",
    label: "String Table",
    Component: ({ data, instCounts: _instCounts, setData }) => (
      <StringTableEditor
        strings={data.strings ?? []}
        bindings={data.bindings ?? []}
        sequences={data.sequences ?? []}
        tableProfiles={(data["table_profiles"] as TableProfile[]) ?? []}
        menuProfiles={(data["menu_profiles"] as MenuProfile[]) ?? []}
        waveProfiles={(data["wave_profiles"] as WaveProfile[]) ?? []}
        onChange={(strings) => setData({ ...data, strings, string_count: strings.length })}
      />
    ),
  },
  {
    key: "sequences",
    label: "Action Sequences",
    Component: ({ data, instCounts, setData }) => (
      <SequenceEditor
        sequences={data.sequences ?? []}
        instCounts={instCounts}
        strings={data.strings ?? []}
        onChange={(sequences) => setData({ ...data, sequences, sequence_count: sequences.length })}
      />
    ),
  },
  {
    key: "timers",
    label: "Soft Timers",
    Component: ({ data, setData }) => (
      <SoftTimerEditor data={data} setData={setData} />
    ),
  },
];
