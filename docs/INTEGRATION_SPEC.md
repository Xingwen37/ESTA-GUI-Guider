# TL-ESTA 新 UI 组件集成规范

版本：v2.0
适用范围：为代码配置器、生成器与仿真器添加新 UI 组件支持的全部步骤
参考实现：WAVE（第一个组件）、BARCHART（第二个组件，验证了集成模式）

---

## 第一章：架构总览

### 1.1 三层数据流

```
┌─────────────────────────────────────────────────────────────┐
│ TypeScript 前端 (tools/profile-gui/src/)                     │
│   types.ts → App.tsx → XxxEditor.tsx                        │
│   用户编辑 ProfileSet → 调用 saveProfile()                   │
└──────────────────────┬──────────────────────────────────────┘
                       │ invoke("save_profile", { data })
                       ▼
┌─────────────────────────────────────────────────────────────┐
│ Rust 后端 (tools/profile-gui/src-tauri/src/)                 │
│   Plugin 架构 (v2.0):                                       │
│   plugins/xxx.rs (模型+impl) → registry.rs →                 │
│   commands/profile_io.rs (注册表驱动) → Tera 模板渲染         │
│   ProfileSet(HashMap) → fill_template_context() → 模板       │
└──────────────────────┬──────────────────────────────────────┘
                       │ 写入文件
                       ▼
┌─────────────────────────────────────────────────────────────┐
│ C 代码 (core/) + 仿真器 (simulator/)                         │
│   ESTA_Profile.c (生成) + XXX.c (手写)                       │
│   ESTA_Profile_ApplyXXX() → XXX_Init() → XXX_ReDraw()       │
│   main.c 主循环: scenario.GetData() → XXX_Update()           │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 涉及文件总览（以 BARCHART 为参考）

| 层 | 文件 | 操作 | 变更量 |
|----|------|:---:|:---:|
| C-组件 | `core/BARCHART.h` | **新建** | ~120 行 |
| C-组件 | `core/BARCHART.c` | **新建** | ~300 行 |
| C-Profile | `core/ESTA_Profile.h` | 修改 | +15 行 |
| C-Profile | `core/ESTA_Profile.c` | 修改 | +30 行 |
| C-Profile | `core/ESTA_Profile.json` | 修改 | +13 行 |
| C-仿真 | `simulator/sim_scenario.h` | 修改 | +2 行 |
| C-仿真 | `simulator/sim_scenario.c` | 修改 | +15 行 |
| C-仿真 | `simulator/main.c` | 修改 | +25 行 |
| Rust | `src-tauri/src/plugins/bar.rs` | **新建**（自包含） | ~100 行 |
| Rust | `src-tauri/src/plugins/mod.rs` | 修改 | +1 行 |
| Rust | `src-tauri/src/lib.rs` | 修改（注册） | +1 行 |
| Rust | `src-tauri/templates/ESTA_Profile.c.j2` | 修改 | +28 行 |
| TS | `src/lib/types.ts` | 修改 | +10 行 |
| TS | `src/components/XxxEditor.tsx` | **新建** | ~100 行 |
| TS | `src/App.tsx` | 修改 | ~40 行 |
| 可选 | `src/styles/app.css` | 修改 | 按需 |

> **v2.0 插件化**：Rust 后端不再需要修改 `models.rs` 或 `commands.rs`。每个组件自包含在一个插件文件（`plugins/xxx.rs`）中，只需在 `plugins/mod.rs` + `lib.rs` 各加 1 行注册。

### 1.3 约定

- 本文以虚构组件 `NEWCOMP` 为例（前缀 `newcomp_`）
- 字段命名：C 层 `snake_case`，Rust 层 `snake_case`，TS 层 `snake_case`
- 实例数最大值默认 2（`MAX_NEWCOMP_INST = 2`）
- 主题颜色：组件自建本地色表，颜色索引从 0 开始（见 `docs/COMPONENT_SPEC.md`）

---

## 第二章：C 核心层集成

### 步骤 1：创建组件代码

按 `docs/COMPONENT_SPEC.md` 规范创建 `core/NEWCOMP.h` 和 `core/NEWCOMP.c`，以 `core/BARCHART.h/.c` 为模板。

必须包含：
- `ESTA_BaseConfig` 兼容的 Config 结构体（前 4 字段 `x_origin, y_origin, x_width, y_width`）
- 主题枚举（`NEWCOMP_theme_type`、`NEWCOMP_theme_color_index_type`）
- 7 个标准访问宏 + 实例宏 + 验证宏
- Config Setter 函数（返回 `ESTA_StatusTypeDef`）
- `Init` / `DeInit` / `ReDraw` 生命周期函数
- 组件专属显示/更新函数

**颜色索引枚举值从 0 开始**（组件本地，无需全局槽位协调）：

```c
typedef enum {
    NEWCOMP_THEME_FRAME_INDEX      = 0,
    NEWCOMP_THEME_ITEM_INDEX       = 1,
    NEWCOMP_THEME_BACKGROUND_INDEX = 2,
    NEWCOMP_THEME_INDEX_COUNT
} NEWCOMP_theme_color_index_type;
```

在组件 `.c` 文件中定义本地静态色表（详见 `docs/COMPONENT_SPEC.md` 第九章），无需修改 `ui_theme.h` 或 `ui_theme.c`。

### 步骤 2：扩展 Profile 类型

**文件**：`core/ESTA_Profile.h`

(1) 添加 `#include "NEWCOMP.h"`

(2) 在 `ESTA_Profile_TypeDef` 末尾追加字段块（在 BARCHART 字段之后、右花括号之前）：

```c
    /* ---- NEWCOMP 组件字段 ---- */
    uint16_t newcomp_x_origin;
    uint16_t newcomp_y_origin;
    uint16_t newcomp_x_width;
    uint16_t newcomp_y_width;
    // ... 其他组件专属字段 ...
    NEWCOMP_theme_type newcomp_theme_type;
} ESTA_Profile_TypeDef;
```

(3) 在 `ESTA_ProfileSet_TypeDef` 中追加实例计数：

```c
typedef struct {
    uint16_t inst_count;
    uint16_t bar_inst_count;
    uint16_t newcomp_inst_count;   // 新增
    ESTA_Profile_TypeDef profiles[ESTA_PROFILE_MAX_INST];
} ESTA_ProfileSet_TypeDef;
```

(4) 追加两个函数声明：

```c
bool ESTA_Profile_ToNEWCOMP_Config(const ESTA_Profile_TypeDef *profile,
    NEWCOMP_Config_TypeDef *out_config);
ESTA_StatusTypeDef ESTA_Profile_ApplyNEWCOMP(int inst_idx,
    const ESTA_Profile_TypeDef *profile);
```

### 步骤 3：实现 Profile 函数

**文件**：`core/ESTA_Profile.c`

> **注意**：此文件由 Tera 模板自动生成。以下为代码模式说明，实际内容在 GUI 保存时覆盖。需同步修改模板（见第三章步骤 3）。

(1) 在 `g_default_profiles` 中追加 `.newcomp_inst_count`：

```c
static const ESTA_ProfileSet_TypeDef g_default_profiles = {
    .inst_count = 1,
    .bar_inst_count = 1,
    .newcomp_inst_count = 1,   // 新增
    .profiles = { ... }
};
```

(2) 在 profile 条目中追加 `newcomp_` 字段默认值：

```c
.newcomp_x_origin = 10,
.newcomp_y_origin = 0,
// ...
.newcomp_theme_type = NEWCOMP_THEME_DEFAULT,
```

(3) 实现 `ESTA_Profile_ToNEWCOMP_Config`（以 `ToBARCHART_Config` 为模板）：

```c
bool ESTA_Profile_ToNEWCOMP_Config(const ESTA_Profile_TypeDef *profile,
    NEWCOMP_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));

    ESTA_ConfigSetPositionAndSize((ESTA_BaseConfig *)out_config,
        profile->newcomp_x_origin, profile->newcomp_y_origin,
        profile->newcomp_x_width, profile->newcomp_y_width);
    NEWCOMP_ConfigSetXxx(out_config, profile->newcomp_xxx);
    // ... 其他 Setter 调用 ...

    return true;
}
```

(4) 实现 `ESTA_Profile_ApplyNEWCOMP`：

```c
ESTA_StatusTypeDef ESTA_Profile_ApplyNEWCOMP(int inst_idx,
    const ESTA_Profile_TypeDef *profile) {
    NEWCOMP_Config_TypeDef config;
    if (!ESTA_Profile_ToNEWCOMP_Config(profile, &config)) {
        return ESTA_ERROR;
    }
    return NEWCOMP_Init(inst_idx, &config);
}
```

### 步骤 4：更新 JSON 数据源

**文件**：`core/ESTA_Profile.json`

(1) 根对象追加 `"newcomp_inst_count": 1`

(2) 每个 profile 对象追加 `newcomp_` 字段：

```json
{
  "inst_count": 1,
  "bar_inst_count": 1,
  "newcomp_inst_count": 1,
  "profiles": [
    {
      "x_origin": 10,
      ...
      "bar_theme_type": "BARCHART_THEME_LIGHT",
      "newcomp_x_origin": 10,
      "newcomp_y_origin": 0,
      "newcomp_x_width": 200,
      "newcomp_y_width": 100,
      "newcomp_theme_type": "NEWCOMP_THEME_DEFAULT"
    }
  ]
}
```

### 步骤 5：添加模拟器场景数据

**文件**：`simulator/sim_scenario.h`

```c
#define SIM_SCENARIO_NEWCOMP_COUNT 1

bool SimScenario_NEWCOMP_GetData(const SimScenarioRuntime *runtime,
    uint16_t *out_data, uint16_t data_count);
```

**文件**：`simulator/sim_scenario.c`

以 `SimScenario_BARCHART_GetData` 为模板，使用 `runtime->signal_lut` 和 `runtime->tick` 产生动态测试数据：

```c
bool SimScenario_NEWCOMP_GetData(const SimScenarioRuntime *runtime,
    uint16_t *out_data, uint16_t data_count) {
    if (runtime == NULL || out_data == NULL) return false;
    size_t t = runtime->tick;
    size_t len = runtime->signal_len;
    for (int i = 0; i < data_count; i++) {
        size_t phase = (t + i * 10U) % len;
        out_data[i] = runtime->signal_lut[phase];
    }
    return true;
}
```

### 步骤 6：修改模拟器主循环

**文件**：`simulator/main.c`

(1) 添加 `#include "NEWCOMP.h"`

(2) 在 BARCHART 初始化代码块之后添加 NEWCOMP 初始化（模式相同）：

```c
if (ESTA_Profile_ApplyNEWCOMP(NEWCOMP_INST(0), &profiles->profiles[0]) != ESTA_OK) {
    printf("ESTA_Profile_ApplyNEWCOMP failed.\n");
    ESTA_SDL2_Quit();
    return 1;
}
if (NEWCOMP_ReDraw(NEWCOMP_INST(0)) != ESTA_OK) {
    printf("NEWCOMP_ReDraw failed.\n");
    ESTA_SDL2_Quit();
    return 1;
}
```

(3) 在主循环中添加数据获取和更新：

```c
uint16_t data_NEWCOMP[BUF_SIZE] = {0};
// ... 循环内 ...
if (SimScenario_NEWCOMP_GetData(&scenario, data_NEWCOMP, count)) {
    NEWCOMP_Update(NEWCOMP_INST(0), data_NEWCOMP, count);
}
```

---

## 第三章：Rust 后端集成（插件化架构 v2.0）

Rust 后端已重构为插件架构。核心变化：
- `ProfileSet.components` 使用 `#[serde(flatten)] HashMap<String, serde_json::Value>`（JSON 平铺格式不变）
- 每个组件是一个**自包含插件文件**，实现 `ComponentPlugin` trait
- `save_profile` 遍历注册表驱动模板上下文构建，不感知具体组件
- 新增组件**无需修改** `models.rs`、`commands/`、`defaults.rs`

### 步骤 1：创建 Rust 插件文件

**文件**：`src-tauri/src/plugins/newcomp.rs` — **新建**

每个组件插件自包含：模型结构体 + `ComponentPlugin` trait 实现。

```rust
use std::collections::HashMap;
use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use tera::Context;
use crate::plugin::ComponentPlugin;

// ── 数据模型 ──
fn default_font_size() -> String { "ESTA_FONT_1608".into() }

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct NewCompProfile {
    pub x_origin: u16,
    pub y_origin: u16,
    pub x_width: u16,
    pub y_width: u16,
    pub theme_type: String,
    #[serde(default)]
    pub page: u8,
}

// ── 插件实现 ──
pub struct NewCompPlugin;

impl ComponentPlugin for NewCompPlugin {
    fn type_name(&self) -> &'static str { "newcomp" }

    fn set_defaults(&self, components: &mut HashMap<String, Value>) {
        let profile = NewCompProfile {
            x_origin: 10, y_origin: 10, x_width: 200, y_width: 100,
            theme_type: "NEWCOMP_THEME_DEFAULT".into(),
            page: 0,
        };
        components.insert("newcomp_inst_count".into(), json!(1));
        components.insert("newcomp_profiles".into(), json!([profile]));
    }

    fn fill_template_context(
        &self,
        components: &HashMap<String, Value>,
        ctx: &mut Context,
    ) {
        if let Some(count) = components.get("newcomp_inst_count") {
            ctx.insert("newcomp_inst_count", count);
        }
        if let Some(raw) = components.get("newcomp_profiles") {
            let values: Vec<Value> = raw
                .as_array().into_iter().flatten()
                .map(|v| match serde_json::from_value::<NewCompProfile>(v.clone()) {
                    Ok(p) => json!({
                        "x_origin": p.x_origin,
                        "y_origin": p.y_origin,
                        "x_width": p.x_width,
                        "y_width": p.y_width,
                        "theme_type": p.theme_type,
                        "page": p.page,
                    }),
                    Err(_) => v.clone(),
                })
                .collect();
            ctx.insert("newcomp_profiles", &values);
        }
    }

    // normalize() 可选覆盖，用于向后兼容（默认 no-op）
}
```

**关键约定**：
- `type_name()` 返回值 = JSON/Tera 中字段前缀（`"newcomp"` → `newcomp_inst_count` / `newcomp_profiles`）
- `set_defaults()` 插入 `{type}_inst_count` 和 `{type}_profiles` 到 components HashMap
- `fill_template_context()` 将 HashMap 中的 Value 反序列化为类型化模型，再转换为 Tera 模板需要的 JSON 格式
- `json!({...})` 内的字段名必须与 Tera 模板 `{{ p.xxx }}` 一致

### 步骤 2：注册插件

只需在两个文件各加 1 行：

**(A)** `src-tauri/src/plugins/mod.rs`：
```rust
pub mod newcomp;  // 追加
```

**(B)** `src-tauri/src/lib.rs` — `run()` 函数中的注册块：
```rust
registry.register(Box::new(plugins::newcomp::NewCompPlugin));  // 追加
```

**无需修改**：`models.rs`、`commands/profile_io.rs`、`commands/build.rs`、`defaults.rs`。

### 步骤 3：更新 Tera 模板

**文件**：`src-tauri/templates/ESTA_Profile.c.j2`

**(A)** 顶层结构体初始化：

```c
.newcomp_inst_count = {{ newcomp_inst_count }},
```

**(B)** profile 循环内字段：

```c
            .newcomp_x_origin = {{ p.newcomp_x_origin }},
            .newcomp_y_origin = {{ p.newcomp_y_origin }},
            .newcomp_x_width = {{ p.newcomp_x_width }},
            .newcomp_y_width = {{ p.newcomp_y_width }},
            .newcomp_theme_type = {{ p.newcomp_theme_type }},
```

**(C)** 文件末尾追加两个函数模板（以 BARCHART 的 `ToBARCHART_Config` / `ApplyBARCHART` 为模板）：

```c
bool ESTA_Profile_ToNEWCOMP_Config(const ESTA_Profile_TypeDef *profile,
    NEWCOMP_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));

    ESTA_ConfigSetPositionAndSize((ESTA_BaseConfig *)out_config,
        profile->newcomp_x_origin, profile->newcomp_y_origin,
        profile->newcomp_x_width, profile->newcomp_y_width);
    NEWCOMP_ConfigSetXxx(out_config, profile->newcomp_xxx);

    return true;
}

ESTA_StatusTypeDef ESTA_Profile_ApplyNEWCOMP(int inst_idx,
    const ESTA_Profile_TypeDef *profile) {
    NEWCOMP_Config_TypeDef config;
    if (!ESTA_Profile_ToNEWCOMP_Config(profile, &config)) {
        return ESTA_ERROR;
    }
    return NEWCOMP_Init(inst_idx, &config);
}
```

---

## 第四章：TypeScript 前端集成

### 步骤 1：扩展类型定义

**文件**：`src/lib/types.ts`

```typescript
// (A) ProfileSet 追加实例计数
export interface ProfileSet {
  inst_count: number;
  bar_inst_count: number;
  newcomp_inst_count: number;
  profiles: EstaProfile[];
}

// (B) EstaProfile 追加字段
export interface EstaProfile {
  // ... 现有字段 ...
  bar_theme_type: string;
  newcomp_x_origin: number;
  newcomp_y_origin: number;
  newcomp_x_width: number;
  newcomp_y_width: number;
  newcomp_theme_type: string;
}

// (C) 主题选项
export const NEWCOMP_THEME_OPTIONS = [
  ["NEWCOMP_THEME_DEFAULT", "Default"],
  ["NEWCOMP_THEME_LIGHT", "Light"],
] as const;

// (D) 最大实例数
export const MAX_NEWCOMP_INST = 2;
```

### 步骤 2：创建编辑器组件

**文件**：`src/components/NewCompEditor.tsx` — **新建**

以 `BarChartEditor.tsx` 为模板，遵循相同的模式：

```typescript
import type { EstaProfile } from "../lib/types";
import { NEWCOMP_THEME_OPTIONS } from "../lib/types";

interface Props {
  profile: EstaProfile;
  onChange: (p: EstaProfile) => void;
}

// spin 辅助函数（数字输入）
function spin(value: number, min: number, max: number, onChange: (v: number) => void) {
  return <input type="number" value={value} min={min} max={max}
    onChange={(e) => onChange(Number(e.target.value) || 0)} />;
}

export default function NewCompEditor({ profile, onChange }: Props) {
  const set = (key: keyof EstaProfile, value: unknown) =>
    onChange({ ...profile, [key]: value });

  return (
    <div>
      <fieldset className="group-box">
        <legend>位置与尺寸</legend>
        <div className="form-row">
          <label>newcomp_x_origin</label>
          {spin(profile.newcomp_x_origin, 0, 65535, (v) => set("newcomp_x_origin", v))}
        </div>
        {/* ... 更多字段 ... */}
      </fieldset>

      <fieldset className="group-box">
        <legend>显示选项</legend>
        <div className="form-row">
          <label>newcomp_theme_type</label>
          <select value={profile.newcomp_theme_type}
            onChange={(e) => set("newcomp_theme_type", e.target.value)}>
            {NEWCOMP_THEME_OPTIONS.map(([value, text]) => (
              <option key={value} value={value}>{text}</option>
            ))}
          </select>
        </div>
      </fieldset>
    </div>
  );
}
```

关键规则：
- Props 接口固定为 `{ profile: EstaProfile; onChange: (p: EstaProfile) => void }`
- 使用 `set("field_name", value)` 更新任意字段
- 数字输入使用 `spin()` 辅助函数
- 布尔字段使用 `<input type="checkbox">`
- 主题选择使用 `<select>` + `XXX_THEME_OPTIONS`
- 布局使用 `fieldset.group-box > div.form-row > label + input`

### 步骤 3：修改主应用 App.tsx

**文件**：`src/App.tsx` — 共 8 处修改。

#### 3a. 导入

```typescript
import NewCompEditor from "./components/NewCompEditor";
import { ..., MAX_NEWCOMP_INST } from "./lib/types";
```

#### 3b. EMPTY_PROFILE 默认值

```typescript
const EMPTY_PROFILE: EstaProfile = {
  // ... 现有 WAVE + BARCHART 默认值 ...
  newcomp_x_origin: 10, newcomp_y_origin: 0,
  newcomp_x_width: 200, newcomp_y_width: 100,
  newcomp_theme_type: "NEWCOMP_THEME_DEFAULT",
};
```

#### 3c. 验证函数

```typescript
function validateNewComp(profiles: EstaProfile[], count: number): string | null {
  for (let i = 0; i < count; i++) {
    const p = profiles[i];
    // ... 组件专属校验逻辑 ...
  }
  return null;
}
```

#### 3d. 标签索引计算

2 组件时：
```typescript
const waveCount = data.inst_count;
const barCount = data.bar_inst_count;
```

3 组件时扩展为：
```typescript
const waveCount = data.inst_count;
const barCount = data.bar_inst_count;
const newcompCount = data.newcomp_inst_count;
const totalTabs = waveCount + barCount + newcompCount;

const isWaveTab = activeTab < waveCount;
const isBarTab = activeTab >= waveCount && activeTab < waveCount + barCount;
const isNewCompTab = activeTab >= waveCount + barCount;

const profileIndex = isWaveTab
  ? activeTab
  : isBarTab
    ? activeTab - waveCount
    : activeTab - waveCount - barCount;
```

#### 3e. 标签页渲染

在 BARCHART 标签块之后追加：

```typescript
{barCount > 0 && newcompCount > 0 && <span className="tab-sep" />}
{Array.from({ length: newcompCount }, (_, i) => (
  <button
    key={`n${i}`}
    className={`tab ${activeTab === waveCount + barCount + i ? "active" : ""}`}
    onClick={() => setActiveTab(waveCount + barCount + i)}
  >
    NEWCOMP{i}
  </button>
))}
```

#### 3f. 编辑器切换

```typescript
{totalTabs === 0 ? (
  <div style={{ color: "#999", padding: 24 }}>请设置组件数量</div>
) : isWaveTab ? (
  <ProfileEditor profile={currentProfile} onChange={updateProfile} />
) : isBarTab ? (
  <BarChartEditor profile={currentProfile} onChange={updateProfile} />
) : (
  <NewCompEditor profile={currentProfile} onChange={updateProfile} />
)}
```

#### 3g. 工具栏计数控件

在 BARCHART 输入框之后追加：

```typescript
<label style={{ marginLeft: 12 }}>NEWCOMP</label>
<input type="number" value={data.newcomp_inst_count} min={0} max={MAX_NEWCOMP_INST}
  onChange={(e) => setData({
    ...data,
    newcomp_inst_count: Math.max(0, Math.min(MAX_NEWCOMP_INST, Number(e.target.value) || 0)),
  })}
/>
```

#### 3h. saveProfile 调用

```typescript
// handleGenerate 和 handleBuildRun 两处都需更新
const xxxErr = validateNewComp(data.profiles, newcompCount);
if (xxxErr) { showStatus({ type: "error", msg: xxxErr }); return; }

await api.saveProfile({
  inst_count: waveCount,
  bar_inst_count: barCount,
  newcomp_inst_count: newcompCount,
  profiles: data.profiles.slice(0, Math.max(waveCount, barCount, newcompCount)),
});
```

### 步骤 4：样式

**文件**：`src/styles/app.css` — 通常无需修改。仅当新组件需要非标准布局（非 `form-row` 模式）时才添加样式。

---

## 第五章：端到端验证清单

完成所有步骤后，依次执行：

| # | 命令 | 预期结果 |
|:---:|------|------|
| 1 | `cd tools/profile-gui && npx tsc --noEmit` | TypeScript 无错误 |
| 2 | `cd tools/profile-gui/src-tauri && cargo check` | Rust 无错误 |
| 3 | `cmake -B build -S . && cmake --build build` | C 编译通过 |
| 4 | `cd tools/profile-gui && npm run tauri dev` | GUI 启动 |
| 4a | — 新组件标签页可见 | NEWCOMP0 标签显示 |
| 4b | — 字段可编辑 | 修改数字、切换复选框、选择主题均正常 |
| 4c | — 点击"生成" | 无模板渲染错误，状态显示成功 |
| 4d | — 检查生成的 `core/ESTA_Profile.c` | 包含 `newcomp_` 字段和 `ToNEWCOMP_Config`/`ApplyNEWCOMP` 函数 |
| 5 | `.\build\ESTA_Simulator.exe` | 模拟器运行 |
| 5a | — 新组件可见 | NEWCOMP 正确渲染在屏幕上 |
| 5b | — 与现有组件共存 | WAVE 和 BARCHART 不受影响 |
| 5c | — 关闭窗口 | 无崩溃，正常退出 |

---

## 附录 A：主题色表结构

每个组件在自身 `.c` 文件中定义独立本地色表，颜色索引从 0 开始：

| 组件 | 色表 | 维度 |
|------|------|------|
| WAVE | `WAVE_ColorTable` | `[2][7]` (DEFAULT/LIGHT × 7 色) |
| BARCHART | `BARCHART_ColorTable` | `[2][8]` (DEFAULT/LIGHT × 8 色) |

新组件按同样模式自建色表，无需全局槽位协调。详见 `docs/COMPONENT_SPEC.md` 第九章。

## 附录 B：Profile 字段前缀约定

| 组件 | 前缀 | 示例字段 |
|------|------|------|
| WAVE | 无前缀 | `x_origin`, `theme_type` |
| BARCHART | `bar_` | `bar_x_origin`, `bar_theme_type` |
| 新组件 | `newcomp_` | `newcomp_x_origin`, `newcomp_theme_type` |

> **规则**：第一个组件（WAVE）无前缀，后续组件使用描述性前缀。前缀用于在共享的 `ESTA_Profile_TypeDef` 中区分字段归属。

## 附录 C：快速检查清单

- [ ] `core/NEWCOMP.h` 新建
- [ ] `core/NEWCOMP.c` 新建
- [ ] `core/ESTA_Profile.h` 添加 include + 字段 + 计数 + 函数声明
- [ ] `core/ESTA_Profile.c` 添加默认值 + ToConfig + Apply
- [ ] `core/ESTA_Profile.json` 添加字段
- [ ] `simulator/sim_scenario.h` 添加声明
- [ ] `simulator/sim_scenario.c` 添加实现
- [ ] `simulator/main.c` 添加 include + 初始化 + 循环更新
- [ ] `src-tauri/src/plugins/newcomp.rs` **新建**（模型 + Plugin impl，自包含）
- [ ] `src-tauri/src/plugins/mod.rs` 追加 `pub mod newcomp;`
- [ ] `src-tauri/src/lib.rs` 追加 `registry.register(Box::new(plugins::newcomp::NewCompPlugin));`
- [ ] `src-tauri/templates/ESTA_Profile.c.j2` 模板字段 + ToConfig + Apply
- [ ] `src/lib/types.ts` 类型 + 主题选项 + MAX 常量
- [ ] `src/components/NewCompEditor.tsx` 新建
- [ ] `src/App.tsx` 8 处修改
- [ ] TypeScript 检查通过
- [ ] Rust 检查通过
- [ ] C 编译通过
- [ ] GUI 生成测试通过
- [ ] 模拟器运行测试通过
