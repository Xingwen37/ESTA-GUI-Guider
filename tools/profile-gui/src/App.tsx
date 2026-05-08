import { useState, useEffect, useCallback } from "react";
import ProfileEditor from "./components/ProfileEditor";
import BarChartEditor from "./components/BarChartEditor";
import * as api from "./lib/tauri-api";
import type { ProfileSet, EstaProfile } from "./lib/types";
import {
  MAX_WAVE_INST,
  MAX_BAR_INST,
  MAX_BUTTON_COUNT,
  MAX_RULER_X_NUM,
  MAX_RULER_Y_NUM,
} from "./lib/types";

const EMPTY_PROFILE: EstaProfile = {
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
  bar_x_origin: 10, bar_y_origin: 125, bar_x_width: 300, bar_y_width: 110,
  bar_display_num_min: 0, bar_display_num_max: 100,
  bar_count: 6, bar_width: 0, bar_spacing: 0,
  bar_is_display_value: true, bar_is_display_axis: true,
  bar_theme_type: "BARCHART_THEME_DEFAULT",
};

function validateWave(profiles: EstaProfile[], count: number): string | null {
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

function validateBar(profiles: EstaProfile[], count: number): string | null {
  for (let i = 0; i < count; i++) {
    const p = profiles[i];
    if (p.bar_display_num_min >= p.bar_display_num_max)
      return `BARCHART${i}: bar_display_num_min 必须小于 bar_display_num_max`;
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

  const waveCount = data.inst_count;
  const barCount = data.bar_inst_count;
  const totalTabs = waveCount + barCount;
  const isWaveTab = activeTab < waveCount;
  const profileIndex = isWaveTab ? activeTab : activeTab - waveCount;
  const currentProfile = data.profiles[profileIndex] ?? EMPTY_PROFILE;

  const updateProfile = (p: EstaProfile) => {
    const profiles = [...data.profiles];
    profiles[profileIndex] = p;
    setData({ ...data, profiles });
  };

  const handleGenerate = async () => {
    const waveErr = validateWave(data.profiles, waveCount);
    if (waveErr) { showStatus({ type: "error", msg: waveErr }); return; }
    const barErr = validateBar(data.profiles, barCount);
    if (barErr) { showStatus({ type: "error", msg: barErr }); return; }
    try {
      await api.saveProfile({
        inst_count: waveCount,
        bar_inst_count: barCount,
        button_count: data.button_count,
        profiles: data.profiles.slice(0, Math.max(waveCount, barCount)),
      });
      showStatus({ type: "success", msg: "已生成 core/ESTA_Profile.c" });
    } catch (e) {
      showStatus({ type: "error", msg: `保存失败: ${e}` });
    }
  };

  const handleBuildRun = async () => {
    const waveErr = validateWave(data.profiles, waveCount);
    if (waveErr) { showStatus({ type: "error", msg: waveErr }); return; }
    const barErr = validateBar(data.profiles, barCount);
    if (barErr) { showStatus({ type: "error", msg: barErr }); return; }
    try {
      await api.saveProfile({
        inst_count: waveCount,
        bar_inst_count: barCount,
        button_count: data.button_count,
        profiles: data.profiles.slice(0, Math.max(waveCount, barCount)),
      });
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
      {/* Toolbar */}
      <div className="toolbar">
        <label>WAVE</label>
        <input
          type="number"
          value={data.inst_count}
          min={0}
          max={MAX_WAVE_INST}
          onChange={(e) =>
            setData({
              ...data,
              inst_count: Math.max(0, Math.min(MAX_WAVE_INST, Number(e.target.value) || 0)),
            })
          }
        />
        <label style={{ marginLeft: 12 }}>BARCHART</label>
        <input
          type="number"
          value={data.bar_inst_count}
          min={0}
          max={MAX_BAR_INST}
          onChange={(e) =>
            setData({
              ...data,
              bar_inst_count: Math.max(0, Math.min(MAX_BAR_INST, Number(e.target.value) || 0)),
            })
          }
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

      {/* Tabs */}
      <div className="tabs">
        {Array.from({ length: waveCount }, (_, i) => (
          <button
            key={`w${i}`}
            className={`tab ${activeTab === i ? "active" : ""}`}
            onClick={() => setActiveTab(i)}
          >
            WAVE{i}
          </button>
        ))}
        {waveCount > 0 && barCount > 0 && <span className="tab-sep" />}
        {Array.from({ length: barCount }, (_, i) => (
          <button
            key={`b${i}`}
            className={`tab ${activeTab === waveCount + i ? "active" : ""}`}
            onClick={() => setActiveTab(waveCount + i)}
          >
            BARCHART{i}
          </button>
        ))}
      </div>

      {/* Editor */}
      <div className="editor-scroll">
        {totalTabs === 0 ? (
          <div style={{ color: "#999", padding: 24 }}>请设置 WAVE 或 BARCHART 数量</div>
        ) : isWaveTab ? (
          <ProfileEditor profile={currentProfile} onChange={updateProfile} />
        ) : (
          <BarChartEditor profile={currentProfile} onChange={updateProfile} />
        )}
      </div>
    </>
  );
}
