# profile-gui 新组件与事件支持规范

版本：v1.0
适用范围：为 profile-gui 代码配置器与生成器添加新 UI 组件和新事件类型支持的全部步骤
参考实现：BARCHART（第二个组件）、button_event（第一个事件类型）

---

## 第一章：架构总览

### 1.1 profile-gui 在系统中的位置

```
┌─────────────────────────────────────────────────────────────┐
│ TypeScript 前端 (tools/profile-gui/src/)                     │
│   types.ts → App.tsx → XxxEditor.tsx                        │
│   用户编辑 ProfileSet → 调用 saveProfile()                   │
└──────────────────────┬──────────────────────────────────────┘
                       │ invoke("save_profile", { data })
                       ▼
┌─────────────────────────────────────────────────────────────┐
│ Rust 后端 (tools/profile-gui/src-tauri/)                     │
│   models.rs → commands.rs → Tera 模板渲染                    │
│   ProfileSet → json!({...}) 上下文 → ESTA_Profile.c.j2      │
└──────────────────────┬──────────────────────────────────────┘
                       │ 写入文件
                       ▼
┌─────────────────────────────────────────────────────────────┐
│ C 代码生成产物 (core/profile/)                                │
│   ESTA_Profile.c (自动生成) + ESTA_Profile.json (数据源)     │
└─────────────────────────────────────────────────────────────┘
```

> **注意**：C 核心层、模拟器、Port 层的集成规范见 `docs/INTEGRATION_SPEC.md`。本规范仅覆盖上图中 profile-gui 内部的三层（TS → Rust → Tera → 生成文件）。

### 1.2 当前目录映射

| profile-gui 文件 | 生成的 C 文件 | 说明 |
|------|------|------|
| `src/lib/types.ts` | — | TS 类型定义 |
| `src/components/XxxEditor.tsx` | — | 编辑器组件 |
| `src/App.tsx` | — | 主应用 |
| `src-tauri/src/models.rs` | — | Rust 数据模型 |
| `src-tauri/src/commands.rs` | `core/profile/ESTA_Profile.c` | 生成逻辑 |
| `src-tauri/templates/ESTA_Profile.c.j2` | 同上 | Tera 模板 |

C 代码库目录结构：
```
core/
  event/     — 事件系统 (event.h/c)
  infra/     — 基础设施 (ui_base, ui_theme, font, helper)
  profile/   — Profile 系统 (ESTA_Profile.h/c/json)
  ui/        — UI 组件 (WAVE.h/c, BARCHART.h/c)
```

### 1.3 涉及文件总览

| 层 | 文件 | 组件集成 | 事件集成 |
|----|------|:---:|:---:|
| TS | `src/lib/types.ts` | 改 | 改 |
| TS | `src/components/XxxEditor.tsx` | **新建** | 按需 |
| TS | `src/App.tsx` | 改 (8处) | 改 (3处) |
| Rust | `src-tauri/src/models.rs` | 改 | 改 |
| Rust | `src-tauri/src/commands.rs` | 改 (3处) | 改 (2处) |
| Tera | `src-tauri/templates/ESTA_Profile.c.j2` | 改 (3处) | 改 (1处) |

### 1.4 数据模型约定

**两层结构**：

| 结构 | 级别 | 内容 |
|------|------|------|
| `ProfileSet` | 全局 | 各组件的实例计数 + 事件类型计数 + profiles 数组 |
| `EstaProfile` | 每个实例 | 单个 UI 实例的所有可编辑字段 |

**字段前缀约定**（在共享的 `EstaProfile` 中区分组件归属）：

| 组件 | 前缀 | 示例 |
|------|------|------|
| WAVE | 无前缀 | `x_origin`, `theme_type` |
| BARCHART | `bar_` | `bar_x_origin`, `bar_theme_type` |
| 后续组件 | 描述性前缀 | `newcomp_x_origin`, `newcomp_theme_type` |

**事件字段**放 `ProfileSet` 级别（全局，不属于某个组件实例）。

**命名一致性**：C 层 `snake_case` → Rust 层 `snake_case` → TS 层 `snake_case`。

---

## 第二章：新组件集成

本章以虚构组件 `NEWCOMP` 为例（前缀 `newcomp_`），以 `BARCHART` 为参考实现。

### 步骤 1：扩展 TypeScript 类型

**文件**：`src/lib/types.ts`

**(A)** `EstaProfile` 接口末尾追加 `newcomp_` 字段块：

```typescript
export interface EstaProfile {
  // ... 现有 WAVE 字段 ...
  bar_theme_type: string;
  // ---- NEWCOMP 字段 ----
  newcomp_x_origin: number;
  newcomp_y_origin: number;
  newcomp_x_width: number;
  newcomp_y_width: number;
  // ... 其他组件专属字段 ...
  newcomp_theme_type: string;
}
```

**(B)** 追加主题选项常量：

```typescript
export const NEWCOMP_THEME_OPTIONS = [
  ["NEWCOMP_THEME_DEFAULT", "Default"],
  ["NEWCOMP_THEME_LIGHT", "Light"],
] as const;
```

**(C)** 追加最大实例数常量：

```typescript
export const MAX_NEWCOMP_INST = 2;
```

### 步骤 2：创建编辑器组件

**文件**：`src/components/NewCompEditor.tsx` — **新建**

标准模板（约 100 行），以 `BarChartEditor.tsx` 为参考：

```typescript
import type { EstaProfile } from "../lib/types";
import { NEWCOMP_THEME_OPTIONS } from "../lib/types";

interface Props {
  profile: EstaProfile;
  onChange: (p: EstaProfile) => void;
}

function spin(value: number, min: number, max: number, onChange: (v: number) => void) {
  return (
    <input
      type="number"
      value={value}
      min={min}
      max={max}
      onChange={(e) => onChange(Number(e.target.value) || 0)}
    />
  );
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
        {/* ... newcomp_y_origin, newcomp_x_width, newcomp_y_width ... */}
      </fieldset>

      <fieldset className="group-box">
        <legend>组件专属配置</legend>
        {/* 组件特定的数字/布尔字段 */}
      </fieldset>

      <fieldset className="group-box">
        <legend>显示选项</legend>
        <div className="form-row">
          <label>newcomp_theme_type</label>
          <select
            value={profile.newcomp_theme_type}
            onChange={(e) => set("newcomp_theme_type", e.target.value)}
          >
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

**关键规则**：
- Props 固定为 `{ profile: EstaProfile; onChange: (p: EstaProfile) => void }`
- 使用 `set("field_name", value)` 更新任意字段（展开 + 覆盖模式）
- 数字输入使用 `spin()` 辅助函数（含 min/max/默认值保护）
- 布尔字段使用 `<input type="checkbox">` + `e.target.checked`
- 主题选择使用 `<select>` + `XXX_THEME_OPTIONS.map()`
- 布局使用 `fieldset.group-box` > `legend` + `div.form-row` > `label` + 控件
- 提示文字使用 `<span className="hint">`

### 步骤 3：修改 App.tsx — 8 处变更

**文件**：`src/App.tsx`

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
    // 组件专属校验逻辑
    // if (p.newcomp_xxx < 1) return `NEWCOMP${i}: xxx 无效`;
  }
  return null;
}
```

#### 3d. 标签索引计算

当前 2 组件公式：
```typescript
const waveCount = data.inst_count;
const barCount = data.bar_inst_count;
```

扩展为 3 组件：
```typescript
const waveCount = data.inst_count;
const barCount = data.bar_inst_count;
const newcompCount = data.newcomp_inst_count;
const totalTabs = waveCount + barCount + newcompCount;

const isWaveTab    = activeTab < waveCount;
const isBarTab     = activeTab >= waveCount && activeTab < waveCount + barCount;
const isNewCompTab = activeTab >= waveCount + barCount;

const profileIndex = isWaveTab
  ? activeTab
  : isBarTab
    ? activeTab - waveCount
    : activeTab - waveCount - barCount;
```

**通用递推公式**（N 个组件）：
```
counts = [count0, count1, count2, ...]
totalTabs = sum(counts)
offset[k] = sum(counts[0..k-1])

is[k]Tab = activeTab >= offset[k] && activeTab < offset[k] + counts[k]
profileIndex = activeTab - offset[k]   (其中 k 满足 is[k]Tab)
```

#### 3e. 标签页渲染

在 BARCHART 标签块之后追加：

```tsx
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

```tsx
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

> **设计决策**：≤3 组件时使用嵌套三元，≥4 组件时重构为配置驱动的数组方案。

#### 3g. 工具栏计数控件

在 BARCHART 输入框之后追加：

```tsx
<label style={{ marginLeft: 12 }}>NEWCOMP</label>
<input
  type="number"
  value={data.newcomp_inst_count}
  min={0}
  max={MAX_NEWCOMP_INST}
  onChange={(e) =>
    setData({
      ...data,
      newcomp_inst_count: Math.max(0, Math.min(MAX_NEWCOMP_INST, Number(e.target.value) || 0)),
    })
  }
/>
```

#### 3h. saveProfile 调用

`handleGenerate` 和 `handleBuildRun` 两处都需更新：

```typescript
const newcompErr = validateNewComp(data.profiles, newcompCount);
if (newcompErr) { showStatus({ type: "error", msg: newcompErr }); return; }

await api.saveProfile({
  inst_count: waveCount,
  bar_inst_count: barCount,
  newcomp_inst_count: newcompCount,
  button_count: data.button_count,
  profiles: data.profiles.slice(0, Math.max(waveCount, barCount, newcompCount)),
});
```

`Math.max(...)` 参数 = 所有组件实例数的最大值，确保 profiles 数组足够长。

### 步骤 4：扩展 Rust 数据模型

**文件**：`src-tauri/src/models.rs`

**(A)** `EstaProfile` 结构体末尾追加字段：

```rust
pub struct EstaProfile {
    // ... 现有字段 ...
    pub bar_theme_type: String,
    // ---- NEWCOMP 字段 ----
    pub newcomp_x_origin: u16,
    pub newcomp_y_origin: u16,
    pub newcomp_x_width: u16,
    pub newcomp_y_width: u16,
    pub newcomp_theme_type: String,
}
```

**(B)** `ProfileSet` 结构体追加实例计数：

```rust
pub struct ProfileSet {
    pub inst_count: u16,
    pub bar_inst_count: u16,
    pub newcomp_inst_count: u16,   // 新增
    pub button_count: u16,
    pub profiles: Vec<EstaProfile>,
}
```

> 如需特殊序列化方法（类似 `channel_mask_expr()`），在 `impl EstaProfile` 块中添加。

### 步骤 5：更新命令处理

**文件**：`src-tauri/src/commands.rs`

**(A)** `default_profile()` 函数 — 两处修改：

ProfileSet 构造追加计数：
```rust
ProfileSet {
    inst_count: 2,
    bar_inst_count: 1,
    newcomp_inst_count: 1,   // 新增
    button_count: 2,
    profiles: vec![ ... ],
}
```

每个 `EstaProfile { ... }` 实例化追加默认字段值：
```rust
EstaProfile {
    // ... 现有字段 ...
    bar_theme_type: "BARCHART_THEME_DEFAULT".into(),
    newcomp_x_origin: 10,
    newcomp_y_origin: 0,
    newcomp_x_width: 200,
    newcomp_y_width: 100,
    newcomp_theme_type: "NEWCOMP_THEME_DEFAULT".into(),
}
```

**(B)** `save_profile()` 函数 — `json!({...})` 映射块（第 50-83 行区域）：

```rust
"bar_theme_type": p.bar_theme_type,
// ---- NEWCOMP 字段 ----
"newcomp_x_origin": p.newcomp_x_origin,
"newcomp_y_origin": p.newcomp_y_origin,
"newcomp_x_width": p.newcomp_x_width,
"newcomp_y_width": p.newcomp_y_width,
"newcomp_theme_type": p.newcomp_theme_type,
```

**(C)** `save_profile()` 函数 — `ctx.insert()` 块（第 87-91 行区域）：

```rust
ctx.insert("inst_count", &data.inst_count);
ctx.insert("bar_inst_count", &data.bar_inst_count);
ctx.insert("newcomp_inst_count", &data.newcomp_inst_count);
ctx.insert("button_count", &data.button_count);
ctx.insert("profiles", &profiles_for_template);
```

### 步骤 6：更新 Tera 模板

**文件**：`src-tauri/templates/ESTA_Profile.c.j2`

**(A)** 顶层结构体初始化（`.bar_inst_count` 之后）：

```c
.bar_inst_count = {{ bar_inst_count }},
.newcomp_inst_count = {{ newcomp_inst_count }},
```

**(B)** profile 循环内（BARCHART 字段之后，右花括号之前）：

```c
            .bar_theme_type = {{ p.bar_theme_type }},
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
    // ... 其他 Setter 调用 ...

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

## 第三章：新事件类型集成

### 3.1 事件数据模型分层

事件的 profile-gui 数据分为两个层级：

| 层级 | 位置 | 示例（v1） | 说明 |
|------|------|------|------|
| 事件计数 | `ProfileSet` 顶层字段 | `button_count: u16` | 该事件类型有多少个实例 |
| 实例配置 | `ProfileSet` 顶层数组（v2） | `buttons: [{id, name}]` | 每个事件实例的详细参数 |

### 3.2 v1 模式：简单计数型

适用场景：事件实例无需自定义参数，只需知道"有几个"。以 `button_event` 为参考实现。

**修改清单**（8 处）：

| # | 文件 | 操作 |
|:---:|------|------|
| 1 | `types.ts` | `ProfileSet` 加 `button_count: number` |
| 2 | `types.ts` | 加 `export const MAX_BUTTON_COUNT = 8` |
| 3 | `App.tsx` 工具栏 | 加 `<label>BUTTON</label>` + `<input>` 数量控件 |
| 4 | `App.tsx` saveProfile | 传 `button_count: data.button_count` |
| 5 | `models.rs` | `ProfileSet` 加 `pub button_count: u16` |
| 6 | `commands.rs` default_profile | 加 `button_count: N` |
| 7 | `commands.rs` ctx.insert | 加 `("button_count", &data.button_count)` |
| 8 | `ESTA_Profile.c.j2` | 加 `.button_count = {{ button_count }}` |

**类型定义**：

```typescript
// types.ts
export interface ProfileSet {
  // ... 组件实例计数 ...
  button_count: number;
  profiles: EstaProfile[];
}
export const MAX_BUTTON_COUNT = 8;
```

```rust
// models.rs
pub struct ProfileSet {
    pub inst_count: u16,
    pub bar_inst_count: u16,
    pub button_count: u16,   // 事件计数
    pub profiles: Vec<EstaProfile>,
}
```

**工具栏控件**（App.tsx）：

```tsx
<label style={{ marginLeft: 12 }}>BUTTON</label>
<input type="number" value={data.button_count} min={0} max={MAX_BUTTON_COUNT}
  onChange={(e) => setData({
    ...data,
    button_count: Math.max(0, Math.min(MAX_BUTTON_COUNT, Number(e.target.value) || 0)),
  })} />
```

> 简单计数型事件**不需要**新建编辑器组件——仅一个工具栏数字输入框即足够。

### 3.3 v2 扩展框架：实例配置型

当事件需要每个实例的自定义参数时（如按钮名称、触摸区域坐标），扩展为此模式。

**以自定义按钮名称为例**，数据模型变化：

```typescript
// types.ts — 新增
export interface ButtonDef {
  id: number;
  name: string;
}

export interface ProfileSet {
  // ... 现有字段 ...
  button_count: number;
  buttons: ButtonDef[];          // 新增：实例配置数组
  profiles: EstaProfile[];
}
```

```rust
// models.rs — 新增
#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ButtonDef {
    pub id: u8,
    pub name: String,
}

pub struct ProfileSet {
    // ... 现有字段 ...
    pub button_count: u16,
    pub buttons: Vec<ButtonDef>, // 新增
    pub profiles: Vec<EstaProfile>,
}
```

**编辑器组件**（`src/components/ButtonEditor.tsx` — 新建）：

```typescript
interface ButtonDef {
  id: number;
  name: string;
}

interface Props {
  buttons: ButtonDef[];
  onChange: (btns: ButtonDef[]) => void;
}

export default function ButtonEditor({ buttons, onChange }: Props) {
  const update = (i: number, name: string) => {
    const next = [...buttons];
    next[i] = { ...next[i], name };
    onChange(next);
  };

  return (
    <fieldset className="group-box">
      <legend>按钮配置</legend>
      {buttons.map((btn, i) => (
        <div className="form-row" key={btn.id}>
          <label>BTN_{btn.id}</label>
          <input type="text" value={btn.name}
            onChange={(e) => update(i, e.target.value)} />
        </div>
      ))}
    </fieldset>
  );
}
```

**App.tsx 集成**：
- 新增 `BUTTON` 标签页（位于组件标签之后），渲染 `ButtonEditor`
- 标签索引公式扩展（buttons 视为新增的"标签组"）
- `saveProfile` 传 `buttons: data.buttons`

**Tera 模板**：

```c
{% for btn in buttons %}
.button_{{ btn.id }}_name = "{{ btn.name }}",
{% endfor %}
```

> **v2 模式当前为预留设计**，待有具体需求时按此框架实现。v1 仅需实现 3.2 节的简单计数型。

### 3.4 事件类型注册清单（通用模板）

每种新事件类型需在以下 8 个位置注册（以未来的 `touch_event` 为例）：

| # | 文件 | 操作 |
|:---:|------|------|
| 1 | `types.ts` ProfileSet | 加 `touch_count: number` |
| 2 | `types.ts` | 加 `MAX_TOUCH_COUNT` 常量 |
| 3 | `App.tsx` 工具栏 | 加 TOUCH 数量输入框 |
| 4 | `App.tsx` saveProfile | 传 `touch_count: data.touch_count` |
| 5 | `models.rs` ProfileSet | 加 `pub touch_count: u16` |
| 6 | `commands.rs` default_profile | 加 `touch_count: N` |
| 7 | `commands.rs` ctx.insert | 加 `("touch_count", &data.touch_count)` |
| 8 | `ESTA_Profile.c.j2` | 加 `.touch_count = {{ touch_count }}` |

---

## 第四章：App.tsx 核心模式

### 4.1 标签索引系统

**当前布局**（2 组件 + 事件设置）：
```
[WAVE0] [WAVE1] | [BARCHART0] [BARCHART1]          [BUTTON: 工具栏]
 ←── waveCount ──→   ←──── barCount ────→
```

**3 组件扩展后**：
```
[WAVE0] [WAVE1] | [BARCHART0] | [NEWCOMP0]          [BUTTON: 工具栏]
```

**关键原则**：
- 组件标签按加入顺序水平排列
- 不同组件组之间用 `<span className="tab-sep" />` 分隔
- 全局事件设置（button_count 等）不放标签栏，放工具栏
- 每个组件的标签起始索引 = 前面所有组件实例数之和

**当前标签索引公式**（2 组件）：
```typescript
const waveCount = data.inst_count;
const barCount = data.bar_inst_count;
const totalTabs = waveCount + barCount;

const isWaveTab = activeTab < waveCount;
const isBarTab  = activeTab >= waveCount;  // 即 activeTab >= waveCount && activeTab < totalTabs
const profileIndex = isWaveTab ? activeTab : activeTab - waveCount;
```

### 4.2 编辑器切换

嵌套三元模式：
```tsx
{totalTabs === 0 ? (
  <div style={{ color: "#999", padding: 24 }}>请设置组件数量</div>
) : isWaveTab ? (
  <ProfileEditor profile={currentProfile} onChange={updateProfile} />
) : isBarTab ? (
  <BarChartEditor profile={currentProfile} onChange={updateProfile} />
) : isNewCompTab ? (
  <NewCompEditor profile={currentProfile} onChange={updateProfile} />
) : (
  <div style={{ color: "#999", padding: 24 }}>未知标签页</div>
)}
```

> **渐进式设计**：≤3 组件保持嵌套三元（可读性尚可），≥4 组件时重构为配置驱动的数组方案。

### 4.3 验证模式

每个组件一个验证函数，签名统一：
```typescript
function validateXxx(profiles: EstaProfile[], count: number): string | null
```

返回第一条错误信息字符串，无错误返回 `null`。在 `handleGenerate` 和 `handleBuildRun` 中依次调用：

```typescript
const waveErr = validateWave(data.profiles, waveCount);
if (waveErr) { showStatus({ type: "error", msg: waveErr }); return; }
const barErr = validateBar(data.profiles, barCount);
if (barErr) { showStatus({ type: "error", msg: barErr }); return; }
```

### 4.4 saveProfile 调用模式

```typescript
await api.saveProfile({
  inst_count: waveCount,
  bar_inst_count: barCount,
  newcomp_inst_count: newcompCount,     // 每增加一个组件加一行
  button_count: data.button_count,      // 每增加一个事件类型加一行
  profiles: data.profiles.slice(0, Math.max(waveCount, barCount, newcompCount)),
});
```

`Math.max(...)` 确保 `profiles` 数组至少覆盖最大实例索引。每个新组件都需加入该参数。

### 4.5 工具栏布局约定

- 组件实例数控件在左侧，格式：`<label>NAME</label> <input>`，间距 `marginLeft: 12`
- 事件计数控件紧随组件控件之后
- `toolbar-spacer` 分隔左侧控件和右侧按钮
- 生成和 Build&Run 按钮在右侧

---

## 第五章：端到端验证清单

| # | 命令/操作 | 预期 |
|:---:|------|------|
| 1 | `cd tools/profile-gui && npx tsc --noEmit` | TypeScript 无错误 |
| 2 | `cd tools/profile-gui/src-tauri && cargo check` | Rust 无错误 |
| 3 | `cd tools/profile-gui && npm run tauri dev` | GUI 启动 |
| 3a | — 新组件标签页可见 | NEWCOMP0 标签显示 |
| 3b | — 字段可编辑 | 修改数字、切换复选框、选择主题均正常 |
| 3c | — 点击"生成" | 无模板渲染错误，状态栏显示成功 |
| 3d | — 检查生成的 `core/profile/ESTA_Profile.c` | 包含 `newcomp_` 字段和 `ToNEWCOMP_Config` / `ApplyNEWCOMP` 函数 |
| 3e | — 检查 `core/profile/ESTA_Profile.json` | JSON 含 `newcomp_inst_count` 和所有 `newcomp_*` 字段 |
| 4 | `cmake --build build` | C 编译通过 |

---

## 附录 A：文件修改速查表

### A.1 组件集成修改点

| # | 文件 | 修改位置 | 操作 |
|:---:|------|------|:---:|
| 1 | `types.ts` | `EstaProfile` 接口 | 追加 `newcomp_*` 字段 |
| 2 | `types.ts` | 常量区 | 追加 `NEWCOMP_THEME_OPTIONS` |
| 3 | `types.ts` | 常量区 | 追加 `MAX_NEWCOMP_INST` |
| 4 | `NewCompEditor.tsx` | — | **新建** |
| 5 | `App.tsx` | 导入区 | 导入 `NewCompEditor` + `MAX_NEWCOMP_INST` |
| 6 | `App.tsx` | `EMPTY_PROFILE` | 追加默认字段值 |
| 7 | `App.tsx` | 验证函数区 | 新增 `validateNewComp()` |
| 8 | `App.tsx` | 标签索引 | 扩展 `xxxCount`/`totalTabs`/`isXxxTab`/`profileIndex` |
| 9 | `App.tsx` | 标签渲染 | 追加 `tab-sep` + `Array.from` 按钮组 |
| 10 | `App.tsx` | 编辑器切换 | 追加三元分支 |
| 11 | `App.tsx` | 工具栏 | 追加 `<label>` + `<input>` |
| 12 | `App.tsx` | `handleGenerate` | 追加验证 + 更新 saveProfile |
| 13 | `App.tsx` | `handleBuildRun` | 追加验证 + 更新 saveProfile |
| 14 | `models.rs` | `EstaProfile` | 追加 `newcomp_*` 字段 |
| 15 | `models.rs` | `ProfileSet` | 追加 `newcomp_inst_count` |
| 16 | `commands.rs` | `default_profile()` ProfileSet | 追加 `newcomp_inst_count` |
| 17 | `commands.rs` | `default_profile()` EstaProfile | 追加默认字段值 |
| 18 | `commands.rs` | `save_profile()` json! 映射 | 追加所有 `newcomp_*` 条目 |
| 19 | `commands.rs` | `save_profile()` ctx.insert | 追加 `"newcomp_inst_count"` |
| 20 | `ESTA_Profile.c.j2` | 顶层结构体 | 追加 `.newcomp_inst_count` |
| 21 | `ESTA_Profile.c.j2` | profile 循环 | 追加所有 `newcomp_*` 字段 |
| 22 | `ESTA_Profile.c.j2` | 文件末尾 | 追加 `ToNEWCOMP_Config` + `ApplyNEWCOMP` |

### A.2 事件集成修改点（v1 简单计数型）

| # | 文件 | 修改位置 | 操作 |
|:---:|------|------|:---:|
| 1 | `types.ts` | `ProfileSet` 接口 | 追加 `xxx_count: number` |
| 2 | `types.ts` | 常量区 | 追加 `MAX_XXX_COUNT` |
| 3 | `App.tsx` | 工具栏 | 追加 `<label>` + `<input>` |
| 4 | `App.tsx` | saveProfile 调用 (×2) | 追加 `xxx_count: data.xxx_count` |
| 5 | `models.rs` | `ProfileSet` | 追加 `pub xxx_count: u16` |
| 6 | `commands.rs` | `default_profile()` | 追加 `xxx_count: N` |
| 7 | `commands.rs` | `ctx.insert()` | 追加 `("xxx_count", ...)` |
| 8 | `ESTA_Profile.c.j2` | 顶层结构体 | 追加 `.xxx_count = {{ xxx_count }}` |

---

## 附录 B：与 INTEGRATION_SPEC.md 的关系

| 方面 | 本规范 | INTEGRATION_SPEC.md |
|------|------|------|
| 覆盖范围 | profile-gui 内部（TS + Rust + Tera） | 全链路（C 核心 + Rust 后端 + TS 前端 + 模拟器） |
| 组件集成 | 6 步骤（仅 profile-gui 侧） | 7 步骤（含 C 组件代码 + 主题注册 + 模拟器） |
| 事件集成 | 含 v1/v2 模式 + 扩展框架 | 不含（编写时事件系统尚未实现） |
| App.tsx 模式 | 深入分析（标签索引、编辑器切换、验证、saveProfile） | 简要概述 |
| 适用读者 | profile-gui 开发者 | 全栈开发者 |

两份文档互补——本规范深入 profile-gui 细节，INTEGRATION_SPEC.md 覆盖完整数据流。添加新组件时建议先读 INTEGRATION_SPEC.md 了解全局，再按本规范操作 profile-gui 侧。

---

## 附录 C：BARCHART 完整修改参考

以下列出 BARCHART 作为第二个组件集成到 profile-gui 时的所有实际修改，作为 `NEWCOMP` 的对照参考。

### types.ts（+13 行）

```typescript
// EstaProfile 追加：
bar_x_origin: number; bar_y_origin: number; bar_x_width: number; bar_y_width: number;
bar_display_num_min: number; bar_display_num_max: number;
bar_count: number; bar_width: number; bar_spacing: number;
bar_is_display_value: boolean; bar_is_display_axis: boolean;
bar_theme_type: string;

// ProfileSet 追加：
bar_inst_count: number;

// 常量：
export const BAR_THEME_OPTIONS = [
  ["BARCHART_THEME_DEFAULT", "Default"],
  ["BARCHART_THEME_LIGHT", "Light"],
] as const;
export const MAX_BAR_INST = 2;
```

### BarChartEditor.tsx（新建，115 行）

4 个 fieldset：位置与尺寸 / 数据与柱体 / 柱体布局 / 显示选项。见第 2.2 节模板。

### App.tsx（8 处修改）

见步骤 3a-3h 的模式，BARCHART 是这些模式的参考实现。

### models.rs（+16 行）

```rust
// EstaProfile 追加 12 个 bar_* 字段
// ProfileSet 追加 pub bar_inst_count: u16
```

### commands.rs（+16 行）

default_profile 加 `bar_inst_count: 1` + 每个 profile 的 12 个 bar_* 默认值；
json! 映射加 12 个 `"bar_xxx": p.bar_xxx`；ctx.insert 加 `"bar_inst_count"`。

### ESTA_Profile.c.j2（+28 行）

顶层 `.bar_inst_count`；循环内 12 个 `.bar_*` 字段；末尾 `ToBARCHART_Config` + `ApplyBARCHART` 函数。
