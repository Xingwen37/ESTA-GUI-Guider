import { useState, useEffect, useCallback } from "react";
import type { ProfileSet } from "../lib/types";
import { MAX_BUTTON_COUNT } from "../lib/types";
import { COMPONENT_REGISTRY } from "../lib/componentRegistry";
import type { ComponentEntry } from "../lib/componentRegistry";
import { guillotineLayout } from "../lib/autoLayout";
import * as api from "../lib/tauri-api";

type Status =
  | { type: "idle" }
  | { type: "success"; msg: string }
  | { type: "error"; msg: string };

function ensureCount<T>(items: T[], count: number, makeDefault: () => T): T[] {
  const next = items.slice(0, count);
  while (next.length < count) next.push(makeDefault());
  return next;
}

export function useProfileState() {
  const [data, setData] = useState<ProfileSet | null>(null);
  const [activeTab, setActiveTab] = useState(0);
  const [status, setStatus] = useState<Status>({ type: "idle" });
  const [screenW, setScreenW] = useState(800);
  const [screenH, setScreenH] = useState(480);
  const [previewPages, setPreviewPages] = useState<string[]>([]);
  const [previewPage, setPreviewPage] = useState(0);

  useEffect(() => {
    api.loadProfile().then(setData).catch((e) =>
      setStatus({ type: "error", msg: `加载失败: ${e}` })
    );
  }, []);

  const showStatus = useCallback((s: Status) => {
    setStatus(s);
    setTimeout(() => setStatus({ type: "idle" }), 3000);
  }, []);

  const buildSavePayload = useCallback((): ProfileSet => {
    if (!data) throw new Error("no data");
    const payload: ProfileSet = {
      ...data,
      button_count: data.button_count,
      page_count: data.page_count ?? 1,
      binding_count: data.binding_count ?? 0,
      bindings: data.bindings ?? [],
      string_count: data.string_count ?? 0,
      strings: data.strings ?? [],
      sequence_count: data.sequence_count ?? 0,
      sequences: data.sequences ?? [],
    };
    for (const entry of COMPONENT_REGISTRY) {
      const count = (data[entry.countField] as number) ?? 0;
      const profiles = (data[entry.profilesField] as unknown[]) ?? [];
      (payload as unknown as Record<string, unknown>)[entry.profilesField as string] =
        ensureCount(profiles, count, entry.makeDefault);
    }
    return payload;
  }, [data]);

  const validateAll = useCallback((): string | null => {
    if (!data) return null;
    for (const entry of COMPONENT_REGISTRY) {
      const count = (data[entry.countField] as number) ?? 0;
      const profiles = (data[entry.profilesField] as unknown[]) ?? [];
      const err = entry.validate(profiles, count);
      if (err) return err;
    }
    return null;
  }, [data]);

  const handleGenerate = useCallback(async () => {
    const err = validateAll();
    if (err) { showStatus({ type: "error", msg: err }); return; }
    try {
      await api.saveProfile(buildSavePayload());
      showStatus({ type: "success", msg: "已生成 core/profile/ESTA_Profile.c" });
    } catch (e) {
      showStatus({ type: "error", msg: `保存失败: ${e}` });
    }
  }, [validateAll, buildSavePayload, showStatus]);

  const handleBuildRun = useCallback(async () => {
    const err = validateAll();
    if (err) { showStatus({ type: "error", msg: err }); return; }
    try {
      await api.saveProfile(buildSavePayload());
      showStatus({ type: "success", msg: "已保存，开始编译..." });
      const msg = await api.buildSimulator();
      showStatus({ type: "success", msg });
      await api.runSimulator();
      showStatus({ type: "success", msg: "模拟器已启动" });
    } catch (e) {
      showStatus({ type: "error", msg: `${e}` });
    }
  }, [validateAll, buildSavePayload, showStatus]);

  const handlePreview = useCallback(async () => {
    const err = validateAll();
    if (err) { showStatus({ type: "error", msg: err }); return; }
    try {
      showStatus({ type: "success", msg: "正在生成预览..." });
      const pages = await api.previewSimulator(buildSavePayload());
      setPreviewPages(pages);
      setPreviewPage(0);
      showStatus({ type: "success", msg: `预览已生成（${pages.length} 页）` });
    } catch (e) {
      showStatus({ type: "error", msg: `预览失败: ${e}` });
    }
  }, [validateAll, buildSavePayload, showStatus]);

  const handleAutoLayout = useCallback((sw: number, sh: number) => {
    if (!data) return;
    const items = COMPONENT_REGISTRY.flatMap((entry) => {
      const count = (data[entry.countField] as number) ?? 0;
      const profiles = (data[entry.profilesField] as Array<{ x_width: number; y_width: number }>) ?? [];
      return profiles.slice(0, count).map((p, i) => ({
        key: `${entry.key}_${i}`,
        w: p.x_width,
        h: p.y_width,
      }));
    });

    const { positions, pageCount } = guillotineLayout(items, sw, sh);

    const next: Partial<ProfileSet> = {};
    for (const entry of COMPONENT_REGISTRY) {
      const profiles = [...((data[entry.profilesField] as unknown[]) ?? [])];
      const updated = profiles.map((p, i) => {
        const pos = positions.get(`${entry.key}_${i}`);
        return pos ? { ...(p as object), x_origin: pos.x, y_origin: pos.y, page: pos.page } : p;
      });
      (next as Record<string, unknown>)[entry.profilesField as string] = updated;
    }
    setData({ ...data, ...next, page_count: pageCount });
    if (pageCount > 1) showStatus({ type: "success", msg: `已自动布局，分配到 ${pageCount} 页` });
    else showStatus({ type: "success", msg: "已自动布局" });
  }, [data, showStatus]);

  // ---- Generic count / delete / update ----

  const setCount = useCallback((entry: ComponentEntry, count: number) => {
    if (!data) return;
    const clamped = Math.max(0, Math.min(entry.maxCount, count));
    const totalOther = COMPONENT_REGISTRY
      .filter((e) => e.key !== entry.key)
      .reduce((sum, e) => sum + ((data[e.countField] as number) ?? 0), 0);
    const nextTotal = clamped + totalOther;
    setActiveTab((tab) => (nextTotal > 0 ? Math.min(tab, nextTotal - 1) : 0));
    const profiles = (data[entry.profilesField] as unknown[]) ?? [];
    setData({
      ...data,
      [entry.countField]: clamped,
      [entry.profilesField]: ensureCount(profiles, clamped, entry.makeDefault),
    });
  }, [data]);

  const deleteItem = useCallback((entry: ComponentEntry, idx: number) => {
    if (!data) return;
    const profiles = [...((data[entry.profilesField] as unknown[]) ?? [])];
    profiles.splice(idx, 1);
    const totalOther = COMPONENT_REGISTRY
      .filter((e) => e.key !== entry.key)
      .reduce((sum, e) => sum + ((data[e.countField] as number) ?? 0), 0);
    const nextTotal = profiles.length + totalOther;
    setActiveTab((tab) => Math.min(tab, Math.max(0, nextTotal - 1)));
    setData({ ...data, [entry.countField]: profiles.length, [entry.profilesField]: profiles });
  }, [data]);

  const updateProfile = useCallback((entry: ComponentEntry, idx: number, profile: unknown) => {
    if (!data) return;
    const count = (data[entry.countField] as number) ?? 0;
    const profiles = ensureCount(
      (data[entry.profilesField] as unknown[]) ?? [],
      count,
      entry.makeDefault
    );
    profiles[idx] = profile;
    setData({ ...data, [entry.profilesField]: profiles });
  }, [data]);

  const setButtonCount = useCallback((count: number) => {
    if (!data) return;
    setData({ ...data, button_count: Math.max(0, Math.min(MAX_BUTTON_COUNT, count)) });
  }, [data]);

  const remapMenuEventIds = useCallback((menuInstIdx: number, newItems: unknown[], idMap: Record<number, number>) => {
    if (!data) return;
    const profiles = [...((data["menu_profiles"] as unknown[]) ?? [])];
    const existing = (profiles[menuInstIdx] ?? {}) as Record<string, unknown>;
    profiles[menuInstIdx] = { ...existing, items: newItems, item_count: newItems.length };

    const updatedBindings = (data.bindings ?? []).map((b) => {
      if (b.trigger !== 3) return b;
      if (b.source_id !== menuInstIdx) return b;
      if (b.trigger_id === 0xFFFF) return b;
      const newId = idMap[b.trigger_id];
      if (newId == null) return b;
      return { ...b, trigger_id: newId };
    });

    setData({ ...data, menu_profiles: profiles, bindings: updatedBindings });
  }, [data]);

  return {
    data, setData,
    activeTab, setActiveTab,
    status, showStatus,
    screenW, setScreenW,
    screenH, setScreenH,
    previewPages, setPreviewPages,
    previewPage, setPreviewPage,
    buildSavePayload,
    validateAll,
    handleGenerate,
    handleBuildRun,
    handlePreview,
    handleAutoLayout,
    setCount,
    deleteItem,
    updateProfile,
    setButtonCount,
    remapMenuEventIds,
  };
}
