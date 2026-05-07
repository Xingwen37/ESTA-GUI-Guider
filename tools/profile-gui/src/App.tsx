import { useState, useEffect, useCallback } from "react";
import ProfileEditor from "./components/ProfileEditor";
import * as api from "./lib/tauri-api";
import type { ProfileSet, OscProfile } from "./lib/types";
import {
  MAX_OSC_INST,
  MAX_RULER_X_NUM,
  MAX_RULER_Y_NUM,
} from "./lib/types";

const EMPTY_PROFILE: OscProfile = {
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
  theme_type: "OSC_THEME_DEFAULT",
  is_auto_clear: true,
};

function validate(profiles: OscProfile[]): string | null {
  for (let i = 0; i < profiles.length; i++) {
    const p = profiles[i];
    if (p.display_num_min >= p.display_num_max)
      return `OSC${i}: display_num_min 必须小于 display_num_max`;
    if (p.ruler_count_x > MAX_RULER_X_NUM)
      return `OSC${i}: ruler_count_x 超过上限`;
    if (p.ruler_count_y > MAX_RULER_Y_NUM)
      return `OSC${i}: ruler_count_y 超过上限`;
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

  if (!data) return <div className="editor-scroll">Loading...</div>;

  const currentProfile = data.profiles[activeTab] ?? EMPTY_PROFILE;

  const updateProfile = (p: OscProfile) => {
    const profiles = [...data.profiles];
    profiles[activeTab] = p;
    setData({ ...data, profiles });
  };

  const handleGenerate = async () => {
    const err = validate(data.profiles.slice(0, data.osc_count));
    if (err) {
      showStatus({ type: "error", msg: err });
      return;
    }
    try {
      await api.saveProfile({
        osc_count: data.osc_count,
        profiles: data.profiles.slice(0, data.osc_count),
      });
      showStatus({ type: "success", msg: "已生成 core/OSC_Profile.c" });
    } catch (e) {
      showStatus({ type: "error", msg: `保存失败: ${e}` });
    }
  };

  const handleBuildRun = async () => {
    const err = validate(data.profiles.slice(0, data.osc_count));
    if (err) {
      showStatus({ type: "error", msg: err });
      return;
    }
    try {
      // Save first
      await api.saveProfile({
        osc_count: data.osc_count,
        profiles: data.profiles.slice(0, data.osc_count),
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
        <label>osc_count</label>
        <input
          type="number"
          value={data.osc_count}
          min={1}
          max={MAX_OSC_INST}
          onChange={(e) =>
            setData({
              ...data,
              osc_count: Math.max(1, Math.min(MAX_OSC_INST, Number(e.target.value) || 1)),
            })
          }
        />
        <div className="toolbar-spacer" />
        <button className="btn-generate" onClick={handleGenerate}>
          生成 OSC_Profile.c
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
        {Array.from({ length: data.osc_count }, (_, i) => (
          <button
            key={i}
            className={`tab ${activeTab === i ? "active" : ""}`}
            onClick={() => setActiveTab(i)}
          >
            OSC{i}
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
