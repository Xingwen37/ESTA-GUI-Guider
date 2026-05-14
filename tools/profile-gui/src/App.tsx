import { useState, useEffect, useCallback } from "react";
import ProfileEditor from "./components/ProfileEditor";
import BarChartEditor from "./components/BarChartEditor";
import * as api from "./lib/tauri-api";
import type { ProfileSet, WaveProfile, BarChartProfile } from "./lib/types";
import {
  MAX_WAVE_INST,
  MAX_BAR_INST,
  MAX_BUTTON_COUNT,
  MAX_RULER_X_NUM,
  MAX_RULER_Y_NUM,
} from "./lib/types";

const EMPTY_WAVE_PROFILE: WaveProfile = {
  x_origin: 0, y_origin: 0, x_width: 200, y_width: 120,
  display_num_min: 0, display_num_max: 4095,
  channel_num: 4, channel_mask: 0b00001111,
  is_display_ruler_y: true,
  ruler_y: [1000, 2000, 3000, 4000, 0],
  ruler_count_y: 4, ruler_num_digits_y: 4,
  is_display_ruler_x: true,
  ruler_x: [30, 50, 90, 0, 0],
  ruler_count_x: 3, ruler_zero_value_x: 0, ruler_full_value_x: 100,
  ruler_num_digits_x: 8,
  theme_type: "WAVE_THEME_DEFAULT",
  is_auto_clear: true,
  is_use_batch_draw: false,
};

const EMPTY_BAR_PROFILE: BarChartProfile = {
  x_origin: 10, y_origin: 125, x_width: 300, y_width: 110,
  display_num_min: 0, display_num_max: 100,
  bar_count: 6, bar_width: 0, bar_spacing: 0,
  is_display_value: true, is_display_axis: true,
  theme_type: "BARCHART_THEME_DEFAULT",
};

function cloneWaveProfile(): WaveProfile {
  return {
    ...EMPTY_WAVE_PROFILE,
    ruler_y: [...EMPTY_WAVE_PROFILE.ruler_y],
    ruler_x: [...EMPTY_WAVE_PROFILE.ruler_x],
  };
}

function cloneBarProfile(): BarChartProfile {
  return { ...EMPTY_BAR_PROFILE };
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
  const totalTabs = waveCount + barCount;
  const activeSafeTab = totalTabs > 0 ? Math.min(activeTab, totalTabs - 1) : 0;
  const isWaveTab = activeSafeTab < waveCount;
  const profileIndex = isWaveTab ? activeSafeTab : activeSafeTab - waveCount;
  const currentWaveProfile = data.wave_profiles[profileIndex] ?? cloneWaveProfile();
  const currentBarProfile = data.bar_profiles[profileIndex] ?? cloneBarProfile();

  const setWaveCount = (count: number) => {
    const wave_inst_count = Math.max(0, Math.min(MAX_WAVE_INST, count));
    const nextTotal = wave_inst_count + data.bar_inst_count;
    setActiveTab((tab) => (nextTotal > 0 ? Math.min(tab, nextTotal - 1) : 0));
    setData({
      ...data,
      wave_inst_count,
      wave_profiles: ensureCount(data.wave_profiles, wave_inst_count, cloneWaveProfile),
    });
  };

  const setBarCount = (count: number) => {
    const bar_inst_count = Math.max(0, Math.min(MAX_BAR_INST, count));
    const nextTotal = data.wave_inst_count + bar_inst_count;
    setActiveTab((tab) => (nextTotal > 0 ? Math.min(tab, nextTotal - 1) : 0));
    setData({
      ...data,
      bar_inst_count,
      bar_profiles: ensureCount(data.bar_profiles, bar_inst_count, cloneBarProfile),
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

  const buildSavePayload = (): ProfileSet => ({
    wave_inst_count: waveCount,
    bar_inst_count: barCount,
    button_count: data.button_count,
    wave_profiles: ensureCount(data.wave_profiles, waveCount, cloneWaveProfile),
    bar_profiles: ensureCount(data.bar_profiles, barCount, cloneBarProfile),
  });

  const validateAll = (): string | null => {
    const payload = buildSavePayload();
    return validateWave(payload.wave_profiles, waveCount) ||
      validateBar(payload.bar_profiles, barCount);
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
      </div>

      <div className="editor-scroll">
        {totalTabs === 0 ? (
          <div style={{ color: "#999", padding: 24 }}>请设置 WAVE 或 BARCHART 数量</div>
        ) : isWaveTab ? (
          <ProfileEditor profile={currentWaveProfile} onChange={updateWaveProfile} />
        ) : (
          <BarChartEditor profile={currentBarProfile} onChange={updateBarProfile} />
        )}
      </div>
    </>
  );
}
