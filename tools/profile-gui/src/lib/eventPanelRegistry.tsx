import type React from "react";
import type { ProfileSet } from "./types";
import type { MenuProfile } from "./menu.registry";
import type { TableProfile } from "./table.registry";
import EventEditor from "../components/EventEditor";
import StringTableEditor from "../components/StringTableEditor";
import SequenceEditor from "../components/SequenceEditor";

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
        onChange={(sequences) => setData({ ...data, sequences, sequence_count: sequences.length })}
      />
    ),
  },
];
