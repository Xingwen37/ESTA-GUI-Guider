import { useState, useEffect, useCallback } from "react";
import ProfileEditor from "./components/ProfileEditor";
import BarChartEditor from "./components/BarChartEditor";
import TableEditor from "./components/TableEditor";
import MenuEditor from "./components/MenuEditor";
import * as api from "./lib/tauri-api";
import type { ProfileSet, WaveProfile, BarChartProfile, TableProfile, MenuProfile } from "./lib/types";
import {
  MAX_WAVE_INST,
  MAX_BAR_INST,
  MAX_TABLE_INST,
  MAX_MENU_INST,
  MAX_BUTTON_COUNT,
  MAX_RULER_X_NUM,
  MAX_RULER_Y_NUM,
  MAX_TABLE_ROWS,
} from "./lib/types";

const labelFromPosition = (value: number) => ({
  value_type: "WAVE_RULER_LABEL_INT",
  int_value: value,
  float_value: value,
});

const DEFAULT_MENU_ITEMS = [
  { label: "Settings", parent_idx: 255, is_submenu: true, event_id: 0 },
  { label: "Display", parent_idx: 0, is_submenu: true, event_id: 0 },
  { label: "Brightness", parent_idx: 1, is_submenu: false, event_id: 10 },
  { label: "Backlight", parent_idx: 1, is_submenu: false, event_id: 11 },
  { label: "Calibrate", parent_idx: 255, is_submenu: false, event_id: 20 },
  { label: "About", parent_idx: 255, is_submenu: false, event_id: 30 },
];

const DEFAULT_RULER_Y = [1000, 2000, 3000, 4000, 0, 0, 0, 0, 0, 0];
const DEFAULT_RULER_X = [30, 50, 90, 0, 0, 0, 0, 0, 0, 0];

const EMPTY_WAVE_PROFILE: WaveProfile = {
  x_origin: 0, y_origin: 0, x_width: 200, y_width: 120,
  display_num_min: 0, display_num_max: 4095,
  x_scale: 1,
  channel_num: 4, channel_mask: 0b00001111,
  is_display_ruler_y: true,
  ruler_y: DEFAULT_RULER_Y,
  ruler_label_y: DEFAULT_RULER_Y.map(labelFromPosition),
  ruler_unit_y: "",
  ruler_precision_y: 0,
  ruler_count_y: 4, ruler_num_digits_y: 4,
  ruler_font_size_y: "ESTA_FONT_1608",
  is_display_ruler_x: true,
  ruler_x: DEFAULT_RULER_X,
  ruler_label_x: DEFAULT_RULER_X.map(labelFromPosition),
  ruler_unit_x: "",
  ruler_precision_x: 0,
  ruler_count_x: 3, ruler_zero_value_x: 0, ruler_full_value_x: 100,
  ruler_num_digits_x: 8,
  ruler_font_size_x: "ESTA_FONT_1608",
  theme_type: "WAVE_THEME_DEFAULT",
  is_auto_clear: true,
  is_use_batch_draw: false,
};

const EMPTY_BAR_PROFILE: BarChartProfile = {
  x_origin: 10, y_origin: 125, x_width: 300, y_width: 110,
  display_num_min: 0, display_num_max: 100,
  bar_count: 6, bar_width: 0, bar_spacing: 0,
  is_display_value: true, is_display_axis: true,
  font_size: "ESTA_FONT_1608",
  theme_type: "BARCHART_THEME_DEFAULT",
};

const EMPTY_TABLE_PROFILE: TableProfile = {
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
};

function cloneWaveProfile(): WaveProfile {
  return {
    ...EMPTY_WAVE_PROFILE,
    ruler_y: [...EMPTY_WAVE_PROFILE.ruler_y],
    ruler_x: [...EMPTY_WAVE_PROFILE.ruler_x],
    ruler_label_y: EMPTY_WAVE_PROFILE.ruler_label_y.map((label) => ({ ...label })),
    ruler_label_x: EMPTY_WAVE_PROFILE.ruler_label_x.map((label) => ({ ...label })),
  };
}

function cloneBarProfile(): BarChartProfile {
  return { ...EMPTY_BAR_PROFILE };
}

const EMPTY_MENU_PROFILE: MenuProfile = {
  x_origin: 10, y_origin: 10, x_width: 160, y_width: 200,
  item_count: 6, item_height: 30, breadcrumb_height: 20,
  is_show_frame: true,
  is_show_breadcrumb: true,
  is_fill_background: true,
  font_size: "ESTA_FONT_1608",
  theme_type: "MENU_THEME_DEFAULT",
  items: DEFAULT_MENU_ITEMS.map((item) => ({ ...item })),
};

function cloneTableProfile(): TableProfile {
  return {
    ...EMPTY_TABLE_PROFILE,
    cols: EMPTY_TABLE_PROFILE.cols.map((col) => ({ ...col })),
    cells: EMPTY_TABLE_PROFILE.cells.map((row) => row.map((cell) => ({ ...cell }))),
  };
}

function cloneMenuProfile(): MenuProfile {
  return {
    ...EMPTY_MENU_PROFILE,
    items: EMPTY_MENU_PROFILE.items.map((item) => ({ ...item })),
  };
}

function ensureCount<T>(items: T[], count: number, makeDefault: () => T): T[] {
  const next = items.slice(0, count);
  while (next.length < count) {
    next.push(makeDefault());
  }
  return next;
}

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

type Status =
  | { type: "idle" }
  | { type: "success"; msg: string }
  | { type: "error"; msg: string };

export default function App() {
  const [data, setData] = useState<ProfileSet | null>(null);
  const [activeTab, setActiveTab] = useState(0);
  const [status, setStatus] = useState<Status>({ type: "idle" });

  useEffect(() => {
    api.loadProfile().then(setData).catch((e) =>
      setStatus({ type: "error", msg: `加载失败: ${e}` })
    );
  }, []);

  const showStatus = useCallback((s: Status) => {
    setStatus(s);
    setTimeout(() => setStatus({ type: "idle" }), 3000);
  }, []);

  if (!data) {
    return (
      <div className="editor-scroll">
        <div>Loading...</div>
        {status.type === "error" && (
          <div className="status" style={{ color: "#f44747", marginTop: 12 }}>
            {status.msg}
          </div>
        )}
      </div>
    );
  }

  const waveCount = data.wave_inst_count;
  const barCount = data.bar_inst_count;
  const tableCount = data.table_inst_count ?? 0;
  const menuCount = data.menu_inst_count ?? 0;
  const totalTabs = waveCount + barCount + tableCount + menuCount;
  const activeSafeTab = totalTabs > 0 ? Math.min(activeTab, totalTabs - 1) : 0;
  const isWaveTab = activeSafeTab < waveCount;
  const isBarTab = !isWaveTab && activeSafeTab < waveCount + barCount;
  const isTableTab = !isWaveTab && !isBarTab && activeSafeTab < waveCount + barCount + tableCount;
  const profileIndex = isWaveTab ? activeSafeTab :
    isBarTab ? activeSafeTab - waveCount :
    isTableTab ? activeSafeTab - waveCount - barCount :
    activeSafeTab - waveCount - barCount - tableCount;
  const currentWaveProfile = data.wave_profiles[profileIndex] ?? cloneWaveProfile();
  const currentBarProfile = data.bar_profiles[profileIndex] ?? cloneBarProfile();
  const currentTableProfile = (data.table_profiles ?? [])[profileIndex] ?? cloneTableProfile();
  const currentMenuProfile = (data.menu_profiles ?? [])[profileIndex] ?? cloneMenuProfile();

  const setWaveCount = (count: number) => {
    const wave_inst_count = Math.max(0, Math.min(MAX_WAVE_INST, count));
    const nextTotal = wave_inst_count + data.bar_inst_count + (data.table_inst_count ?? 0) + (data.menu_inst_count ?? 0);
    setActiveTab((tab) => (nextTotal > 0 ? Math.min(tab, nextTotal - 1) : 0));
    setData({
      ...data,
      wave_inst_count,
      wave_profiles: ensureCount(data.wave_profiles, wave_inst_count, cloneWaveProfile),
    });
  };

  const setBarCount = (count: number) => {
    const bar_inst_count = Math.max(0, Math.min(MAX_BAR_INST, count));
    const nextTotal = data.wave_inst_count + bar_inst_count + (data.table_inst_count ?? 0) + (data.menu_inst_count ?? 0);
    setActiveTab((tab) => (nextTotal > 0 ? Math.min(tab, nextTotal - 1) : 0));
    setData({
      ...data,
      bar_inst_count,
      bar_profiles: ensureCount(data.bar_profiles, bar_inst_count, cloneBarProfile),
    });
  };

  const setTableCount = (count: number) => {
    const table_inst_count = Math.max(0, Math.min(MAX_TABLE_INST, count));
    const nextTotal = data.wave_inst_count + data.bar_inst_count + table_inst_count + (data.menu_inst_count ?? 0);
    setActiveTab((tab) => (nextTotal > 0 ? Math.min(tab, nextTotal - 1) : 0));
    setData({
      ...data,
      table_inst_count,
      table_profiles: ensureCount(data.table_profiles ?? [], table_inst_count, cloneTableProfile),
    });
  };

  const setMenuCount = (count: number) => {
    const menu_inst_count = Math.max(0, Math.min(MAX_MENU_INST, count));
    const nextTotal = data.wave_inst_count + data.bar_inst_count + (data.table_inst_count ?? 0) + menu_inst_count;
    setActiveTab((tab) => (nextTotal > 0 ? Math.min(tab, nextTotal - 1) : 0));
    setData({
      ...data,
      menu_inst_count,
      menu_profiles: ensureCount(data.menu_profiles ?? [], menu_inst_count, cloneMenuProfile),
    });
  };

  const updateWaveProfile = (p: WaveProfile) => {
    const wave_profiles = ensureCount(data.wave_profiles, waveCount, cloneWaveProfile);
    wave_profiles[profileIndex] = p;
    setData({ ...data, wave_profiles });
  };

  const updateBarProfile = (p: BarChartProfile) => {
    const bar_profiles = ensureCount(data.bar_profiles, barCount, cloneBarProfile);
    bar_profiles[profileIndex] = p;
    setData({ ...data, bar_profiles });
  };

  const updateTableProfile = (p: TableProfile) => {
    const table_profiles = ensureCount(data.table_profiles ?? [], tableCount, cloneTableProfile);
    table_profiles[profileIndex] = p;
    setData({ ...data, table_profiles });
  };

  const updateMenuProfile = (p: MenuProfile) => {
    const menu_profiles = ensureCount(data.menu_profiles ?? [], menuCount, cloneMenuProfile);
    menu_profiles[profileIndex] = p;
    setData({ ...data, menu_profiles });
  };

  const buildSavePayload = (): ProfileSet => ({
    wave_inst_count: waveCount,
    bar_inst_count: barCount,
    table_inst_count: tableCount,
    menu_inst_count: menuCount,
    button_count: data.button_count,
    wave_profiles: ensureCount(data.wave_profiles, waveCount, cloneWaveProfile),
    bar_profiles: ensureCount(data.bar_profiles, barCount, cloneBarProfile),
    table_profiles: ensureCount(data.table_profiles ?? [], tableCount, cloneTableProfile),
    menu_profiles: ensureCount(data.menu_profiles ?? [], menuCount, cloneMenuProfile),
  });

  const validateAll = (): string | null => {
    const payload = buildSavePayload();
    return validateWave(payload.wave_profiles, waveCount) ||
      validateBar(payload.bar_profiles, barCount) ||
      validateTable(payload.table_profiles, tableCount) ||
      validateMenu(payload.menu_profiles, menuCount);
  };

  const handleGenerate = async () => {
    const err = validateAll();
    if (err) { showStatus({ type: "error", msg: err }); return; }
    try {
      await api.saveProfile(buildSavePayload());
      showStatus({ type: "success", msg: "已生成 core/profile/ESTA_Profile.c" });
    } catch (e) {
      showStatus({ type: "error", msg: `保存失败: ${e}` });
    }
  };

  const handleBuildRun = async () => {
    const err = validateAll();
    if (err) { showStatus({ type: "error", msg: err }); return; }
    try {
      await api.saveProfile(buildSavePayload());
      showStatus({ type: "success", msg: "已保存，开始编译..." });
      const msg = await api.buildSimulator();
      showStatus({ type: "success", msg: msg });
      await api.runSimulator();
      showStatus({ type: "success", msg: "模拟器已启动" });
    } catch (e) {
      showStatus({ type: "error", msg: `${e}` });
    }
  };

  return (
    <>
      <div className="toolbar">
        <label>WAVE</label>
        <input
          type="number"
          value={data.wave_inst_count}
          min={0}
          max={MAX_WAVE_INST}
          onChange={(e) => setWaveCount(Number(e.target.value) || 0)}
        />
        <label style={{ marginLeft: 12 }}>BARCHART</label>
        <input
          type="number"
          value={data.bar_inst_count}
          min={0}
          max={MAX_BAR_INST}
          onChange={(e) => setBarCount(Number(e.target.value) || 0)}
        />
        <label style={{ marginLeft: 12 }}>TABLE</label>
        <input
          type="number"
          value={data.table_inst_count ?? 0}
          min={0}
          max={MAX_TABLE_INST}
          onChange={(e) => setTableCount(Number(e.target.value) || 0)}
        />
        <label style={{ marginLeft: 12 }}>MENU</label>
        <input
          type="number"
          value={data.menu_inst_count ?? 0}
          min={0}
          max={MAX_MENU_INST}
          onChange={(e) => setMenuCount(Number(e.target.value) || 0)}
        />
        <label style={{ marginLeft: 12 }}>BUTTON</label>
        <input
          type="number"
          value={data.button_count}
          min={0}
          max={MAX_BUTTON_COUNT}
          onChange={(e) =>
            setData({
              ...data,
              button_count: Math.max(0, Math.min(MAX_BUTTON_COUNT, Number(e.target.value) || 0)),
            })
          }
        />
        <div className="toolbar-spacer" />
        <button className="btn-generate" onClick={handleGenerate}>
          生成 ESTA_Profile.c
        </button>
        <button className="btn-run" onClick={handleBuildRun}>
          Build & Run
        </button>
        {status.type !== "idle" && (
          <span className="status">{status.msg}</span>
        )}
      </div>

      <div className="tabs">
        {Array.from({ length: waveCount }, (_, i) => (
          <button
            key={`w${i}`}
            className={`tab ${activeSafeTab === i ? "active" : ""}`}
            onClick={() => setActiveTab(i)}
          >
            WAVE{i}
          </button>
        ))}
        {waveCount > 0 && barCount > 0 && <span className="tab-sep" />}
        {Array.from({ length: barCount }, (_, i) => (
          <button
            key={`b${i}`}
            className={`tab ${activeSafeTab === waveCount + i ? "active" : ""}`}
            onClick={() => setActiveTab(waveCount + i)}
          >
            BARCHART{i}
          </button>
        ))}
        {(waveCount + barCount) > 0 && tableCount > 0 && <span className="tab-sep" />}
        {Array.from({ length: tableCount }, (_, i) => (
          <button
            key={`t${i}`}
            className={`tab ${activeSafeTab === waveCount + barCount + i ? "active" : ""}`}
            onClick={() => setActiveTab(waveCount + barCount + i)}
          >
            TABLE{i}
          </button>
        ))}
        {(waveCount + barCount + tableCount) > 0 && menuCount > 0 && <span className="tab-sep" />}
        {Array.from({ length: menuCount }, (_, i) => (
          <button
            key={`m${i}`}
            className={`tab ${activeSafeTab === waveCount + barCount + tableCount + i ? "active" : ""}`}
            onClick={() => setActiveTab(waveCount + barCount + tableCount + i)}
          >
            MENU{i}
          </button>
        ))}
      </div>

      <div className="editor-scroll">
        {totalTabs === 0 ? (
          <div style={{ color: "#999", padding: 24 }}>请设置组件数量</div>
        ) : isWaveTab ? (
          <ProfileEditor profile={currentWaveProfile} onChange={updateWaveProfile} />
        ) : isBarTab ? (
          <BarChartEditor profile={currentBarProfile} onChange={updateBarProfile} />
        ) : isTableTab ? (
          <TableEditor profile={currentTableProfile} onChange={updateTableProfile} />
        ) : (
          <MenuEditor profile={currentMenuProfile} onChange={updateMenuProfile} />
        )}
      </div>
    </>
  );
}
