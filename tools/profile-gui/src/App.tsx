import { useState, useEffect, useCallback } from "react";
import ProfileEditor from "./components/ProfileEditor";
import * as api from "./lib/tauri-api";
import type { ProfileSet, EstaProfile } from "./lib/types";
import {
  MAX_WAVE_INST,
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
};

function validate(profiles: EstaProfile[]): string | null {
  for (let i = 0; i < profiles.length; i++) {
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

  const currentProfile = data.profiles[activeTab] ?? EMPTY_PROFILE;

  const updateProfile = (p: EstaProfile) => {
    const profiles = [...data.profiles];
    profiles[activeTab] = p;
    setData({ ...data, profiles });
  };

  const handleGenerate = async () => {
    const err = validate(data.profiles.slice(0, data.inst_count));
    if (err) {
      showStatus({ type: "error", msg: err });
      return;
    }
    try {
      await api.saveProfile({
        inst_count: data.inst_count,
        profiles: data.profiles.slice(0, data.inst_count),
      });
      showStatus({ type: "success", msg: "已生成 core/ESTA_Profile.c" });
    } catch (e) {
      showStatus({ type: "error", msg: `保存失败: ${e}` });
    }
  };

  const handleBuildRun = async () => {
    const err = validate(data.profiles.slice(0, data.inst_count));
    if (err) {
      showStatus({ type: "error", msg: err });
      return;
    }
    try {
      // Save first
      await api.saveProfile({
        inst_count: data.inst_count,
        profiles: data.profiles.slice(0, data.inst_count),
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
        <label>inst_count</label>
        <input
          type="number"
          value={data.inst_count}
          min={1}
          max={MAX_WAVE_INST}
          onChange={(e) =>
            setData({
              ...data,
              inst_count: Math.max(1, Math.min(MAX_WAVE_INST, Number(e.target.value) || 1)),
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
        {Array.from({ length: data.inst_count }, (_, i) => (
          <button
            key={i}
            className={`tab ${activeTab === i ? "active" : ""}`}
            onClick={() => setActiveTab(i)}
          >
            WAVE{i}
          </button>
        ))}
      </div>

      {/* Editor */}
      <div className="editor-scroll">
        <ProfileEditor profile={currentProfile} onChange={updateProfile} />
      </div>
    </>
  );
}
