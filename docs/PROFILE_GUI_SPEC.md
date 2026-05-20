# profile-gui 新组件与事件支持规范

版本：v2.0（注册表架构）
适用范围：为 profile-gui 代码配置器与生成器添加新 UI 组件和新事件类型支持的全部步骤
参考实现：BARCHART、TABLE、MENU

---

## 第一章：架构总览

### 1.1 profile-gui 在系统中的位置

```
┌─────────────────────────────────────────────────────────────┐
│ TypeScript 前端 (tools/profile-gui/src/)                     │
│   xxx.registry.ts → componentRegistry.ts → App.tsx          │
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

| profile-gui 文件 | 说明 |
|------|------|
| `src/lib/xxx.registry.ts` | 组件类型、常量、默认值、验证器、ComponentEntry（每组件一个文件） |
| `src/lib/componentRegistry.ts` | 注册表聚合器（仅导入 + 数组） |
| `src/lib/types.ts` | 全局类型（ProfileSet、事件相关类型和常量） |
| `src/components/XxxEditor.tsx` | 编辑器组件 |
| `src/App.tsx` | 主应用（注册表驱动，新增组件无需修改） |
| `src-tauri/src/models.rs` | Rust 数据模型 |
| `src-tauri/src/commands.rs` | 生成逻辑，输出 `core/profile/ESTA_Profile.c` |
| `src-tauri/templates/ESTA_Profile.c.j2` | Tera 模板 |

### 1.3 涉及文件总览

| 层 | 文件 | 组件集成 | 事件集成 |
|----|------|:---:|:---:|
| TS | `src/lib/xxx.registry.ts` | **新建** | — |
| TS | `src/lib/componentRegistry.ts` | 改（2处） | — |
| TS | `src/lib/types.ts` | 改（2处） | 改 |
| TS | `src/components/XxxEditor.tsx` | **新建** | 按需 |
| TS | `src/App.tsx` | **无需修改** | 改（1处） |
| Rust | `src-tauri/src/models.rs` | 改 | 改 |
| Rust | `src-tauri/src/commands.rs` | 改（3处） | 改（2处） |
| Tera | `src-tauri/templates/ESTA_Profile.c.j2` | 改（3处） | 改（1处） |

> **关键变化**：App.tsx 对新组件**无需修改**——工具栏、标签栏、编辑器切换均由 COMPONENT_REGISTRY 自动驱动。

### 1.4 数据模型约定

**两层结构**：

| 结构 | 级别 | 内容 |
|------|------|------|
| `ProfileSet` | 全局 | 各组件的实例计数 + 各组件的 profiles 数组 + 事件相关字段 |
| `XxxProfile` | 每个实例 | 单个 UI 实例的所有可编辑字段（定义在 `xxx.registry.ts`） |

**每组件独立 Profile 类型**（不再共享 `EstaProfile` 平铺结构）：

```typescript
// ProfileSet 中的组件字段示例
wave_inst_count: number;
wave_profiles: WaveProfile[];    // WaveProfile 定义在 wave.registry.ts

bar_inst_count: number;
bar_profiles: BarChartProfile[]; // BarChartProfile 定义在 bar.registry.ts
```

**字段前缀约定**（Rust 模型和 Tera 模板中区分组件归属）：

| 组件 | 前缀 | 示例 |
|------|------|------|
| WAVE | 无前缀 | `x_origin`, `theme_type` |
| BARCHART | `bar_` | `bar_x_origin`, `bar_theme_type` |
| TABLE | `table_` | `table_x_origin`, `table_theme_type` |
| MENU | `menu_` | `menu_x_origin`, `menu_theme_type` |
| 后续组件 | `xxx_` | `xxx_x_origin`, `xxx_theme_type` |

**命名一致性**：C 层 `snake_case` → Rust 层 `snake_case` → TS 层 `snake_case`。

---

## 第二章：新组件集成（注册表架构）

本章以虚构组件 `NEWCOMP` 为例（前缀 `newcomp_`），以 `bar.registry.ts` 为参考实现。

### 步骤 1：创建注册表文件

**文件**：`src/lib/newcomp.registry.ts` — **新建**

```typescript
import type React from "react";
import type { ProfileSet } from "./types";
import NewCompEditor from "../components/NewCompEditor";
import type { ComponentEntry } from "./componentRegistry";

export interface NewCompProfile {
  x_origin: number;
  y_origin: number;
  x_width: number;
  y_width: number;
  // ... 组件专属字段 ...
  theme_type: string;
  page: number;
}

export const MAX_NEWCOMP_INST = 4;

export const NEWCOMP_THEME_OPTIONS = [
  ["NEWCOMP_THEME_DEFAULT", "Default"],
  ["NEWCOMP_THEME_LIGHT", "Light"],
] as const;

const EMPTY_NEWCOMP: NewCompProfile = {
  x_origin: 10, y_origin: 10, x_width: 200, y_width: 100,
  theme_type: "NEWCOMP_THEME_DEFAULT",
  page: 0,
};

export function makeDefaultNewComp(): NewCompProfile {
  return { ...EMPTY_NEWCOMP };
}

function validateNewComp(profiles: NewCompProfile[], count: number): string | null {
  for (let i = 0; i < count; i++) {
    const p = profiles[i];
    if (p.x_width < 1) return `NEWCOMP${i}: x_width 无效`;
  }
  return null;
}

export const newcompEntry: ComponentEntry<NewCompProfile> = {
  key: "newcomp",
  label: "NEWCOMP",
  countField: "newcomp_inst_count" as keyof ProfileSet,
  profilesField: "newcomp_profiles" as keyof ProfileSet,
  maxCount: MAX_NEWCOMP_INST,
  makeDefault: makeDefaultNewComp,
  validate: validateNewComp as ComponentEntry["validate"],
  Editor: NewCompEditor as unknown as React.ComponentType<{
    profile: NewCompProfile; onChange: (p: NewCompProfile) => void
  }>,
};
```

**关键字段**：
- `key`：小写标识符，用于 `instCounts` 查找（需与 `countField` 前缀一致）
- `countField` / `profilesField`：必须与 `ProfileSet` 中的字段名完全一致
- `validate`：返回第一条错误字符串，无错误返回 `null`
- `Editor` 的双重 `as unknown as` 转型是绕过 TypeScript 逆变检查的标准写法

### 步骤 2：创建编辑器组件

**文件**：`src/components/NewCompEditor.tsx` — **新建**

```typescript
import type { NewCompProfile } from "../lib/newcomp.registry";
import { NEWCOMP_THEME_OPTIONS } from "../lib/newcomp.registry";
import { UI_FONT_SIZE_OPTIONS } from "../lib/types";

interface Props {
  profile: NewCompProfile;
  onChange: (p: NewCompProfile) => void;
}

export default function NewCompEditor({ profile, onChange }: Props) {
  const set = <K extends keyof NewCompProfile>(key: K, value: NewCompProfile[K]) =>
    onChange({ ...profile, [key]: value });

  return (
    <div>
      <fieldset className="group-box">
        <legend>位置与尺寸</legend>
        <div className="form-row">
          <label>x_origin</label>
          <input type="number" value={profile.x_origin} min={0} max={65535}
            onChange={(e) => set("x_origin", Number(e.target.value) || 0)} />
        </div>
        {/* y_origin, x_width, y_width ... */}
      </fieldset>
      <fieldset className="group-box">
        <legend>显示选项</legend>
        <div className="form-row">
          <label>theme_type</label>
          <select value={profile.theme_type} onChange={(e) => set("theme_type", e.target.value)}>
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
- Props 固定为 `{ profile: NewCompProfile; onChange: (p: NewCompProfile) => void }`（每组件独立类型）
- 从 `newcomp.registry.ts` 导入类型和常量，从 `types.ts` 导入 `UI_FONT_SIZE_OPTIONS`
- 布局使用 `fieldset.group-box` > `legend` + `div.form-row` > `label` + 控件

### 步骤 3：注册到 componentRegistry.ts

**文件**：`src/lib/componentRegistry.ts` — **改（2处）**

```typescript
import { newcompEntry } from "./newcomp.registry";  // 新增导入

export const COMPONENT_REGISTRY: ComponentEntry[] = [
  waveEntry as unknown as ComponentEntry,
  barEntry as unknown as ComponentEntry,
  tableEntry as unknown as ComponentEntry,
  menuEntry as unknown as ComponentEntry,
  newcompEntry as unknown as ComponentEntry,  // 新增条目
];
```

完成此步骤后，App.tsx 的工具栏、标签栏、编辑器切换**自动包含** NEWCOMP，无需任何其他 App.tsx 修改。

### 步骤 4：扩展 ProfileSet

**文件**：`src/lib/types.ts` — **改（2处）**

```typescript
export interface ProfileSet {
  // ... 现有字段 ...
  newcomp_inst_count: number;   // 新增
  // ...
  newcomp_profiles: import("./newcomp.registry").NewCompProfile[];  // 新增
  // ...
}
```

### 步骤 5：扩展 Rust 数据模型

**文件**：`src-tauri/src/models.rs`

**(A)** `EstaProfile` 结构体末尾追加字段：

```rust
// ---- NEWCOMP 字段 ----
pub newcomp_x_origin: u16,
pub newcomp_y_origin: u16,
pub newcomp_x_width: u16,
pub newcomp_y_width: u16,
pub newcomp_theme_type: String,
```

**(B)** `ProfileSet` 结构体追加：

```rust
pub newcomp_inst_count: u16,
pub newcomp_profiles: Vec<EstaProfile>,
```

### 步骤 6：更新命令处理

**文件**：`src-tauri/src/commands.rs`

**(A)** `default_profile()` — ProfileSet 构造追加：
```rust
newcomp_inst_count: 1,
newcomp_profiles: vec![ EstaProfile { newcomp_x_origin: 10, newcomp_y_origin: 10,
    newcomp_x_width: 200, newcomp_y_width: 100,
    newcomp_theme_type: "NEWCOMP_THEME_DEFAULT".into(), ..Default::default() } ],
```

**(B)** `save_profile()` — `json!({...})` 映射块追加：
```rust
"newcomp_x_origin": p.newcomp_x_origin,
"newcomp_y_origin": p.newcomp_y_origin,
"newcomp_x_width": p.newcomp_x_width,
"newcomp_y_width": p.newcomp_y_width,
"newcomp_theme_type": p.newcomp_theme_type,
```

**(C)** `save_profile()` — `ctx.insert()` 块追加：
```rust
ctx.insert("newcomp_inst_count", &data.newcomp_inst_count);
```

### 步骤 7：更新 Tera 模板

**文件**：`src-tauri/templates/ESTA_Profile.c.j2`

**(A)** 顶层结构体初始化追加：
```c
.newcomp_inst_count = {{ newcomp_inst_count }},
```

**(B)** profile 循环内追加：
```c
.newcomp_x_origin = {{ p.newcomp_x_origin }},
.newcomp_y_origin = {{ p.newcomp_y_origin }},
.newcomp_x_width  = {{ p.newcomp_x_width }},
.newcomp_y_width  = {{ p.newcomp_y_width }},
.newcomp_theme_type = {{ p.newcomp_theme_type }},
```

**(C)** 文件末尾追加两个函数（以 `ToBARCHART_Config` / `ApplyBARCHART` 为模板）：
```c
bool ESTA_Profile_ToNEWCOMP_Config(const ESTA_Profile_TypeDef *profile,
    NEWCOMP_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));
    ESTA_ConfigSetPositionAndSize((ESTA_BaseConfig *)out_config,
        profile->newcomp_x_origin, profile->newcomp_y_origin,
        profile->newcomp_x_width, profile->newcomp_y_width);
    return true;
}

ESTA_StatusTypeDef ESTA_Profile_ApplyNEWCOMP(int inst_idx,
    const ESTA_Profile_TypeDef *profile) {
    NEWCOMP_Config_TypeDef config;
    if (!ESTA_Profile_ToNEWCOMP_Config(profile, &config)) return ESTA_ERROR;
    return NEWCOMP_Init(inst_idx, &config);
}
```

---

## 第三章：新事件类型集成

### 3.1 事件数据模型分层

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
| 4 | `App.tsx` useProfileState | 传 `button_count: data.button_count` |
| 5 | `models.rs` | `ProfileSet` 加 `pub button_count: u16` |
| 6 | `commands.rs` default_profile | 加 `button_count: N` |
| 7 | `commands.rs` ctx.insert | 加 `("button_count", &data.button_count)` |
| 8 | `ESTA_Profile.c.j2` | 加 `.button_count = {{ button_count }}` |

> 简单计数型事件**不需要**新建编辑器组件——仅一个工具栏数字输入框即足够。

### 3.3 事件类型注册清单（通用模板）

每种新事件类型需在以下 8 个位置注册（以未来的 `touch_event` 为例）：

| # | 文件 | 操作 |
|:---:|------|------|
| 1 | `types.ts` ProfileSet | 加 `touch_count: number` |
| 2 | `types.ts` | 加 `MAX_TOUCH_COUNT` 常量 |
| 3 | `App.tsx` 工具栏 | 加 TOUCH 数量输入框 |
| 4 | `App.tsx` useProfileState | 传 `touch_count: data.touch_count` |
| 5 | `models.rs` ProfileSet | 加 `pub touch_count: u16` |
| 6 | `commands.rs` default_profile | 加 `touch_count: N` |
| 7 | `commands.rs` ctx.insert | 加 `("touch_count", ...)` |
| 8 | `ESTA_Profile.c.j2` | 加 `.touch_count = {{ touch_count }}` |

---

## 第四章：App.tsx 核心模式（注册表驱动）

### 4.1 注册表驱动原理

App.tsx 不再包含任何组件专属逻辑。所有组件相关的渲染均由 `COMPONENT_REGISTRY` 数组驱动：

- **工具栏**：遍历 `COMPONENT_REGISTRY`，为每个 entry 渲染 `<label>` + `<input>`
- **标签栏**：遍历 `COMPONENT_REGISTRY`，为每个 entry 的实例渲染 `<button>`
- **编辑器**：使用 `activeEntry.Editor` 动态渲染当前活跃组件的编辑器

### 4.2 标签索引系统

```typescript
// 总标签数 = 所有组件实例数之和
const totalComponentTabs = COMPONENT_REGISTRY.reduce(
  (sum, e) => sum + ((data[e.countField] as number) ?? 0), 0
);

// 查找当前活跃的 entry 和 profileIndex
let tabOffset = 0;
for (const entry of COMPONENT_REGISTRY) {
  const count = (data[entry.countField] as number) ?? 0;
  if (activeSafeTab < tabOffset + count) {
    activeEntry = entry;
    profileIndex = activeSafeTab - tabOffset;
    break;
  }
  tabOffset += count;
}
```

新增组件后，此逻辑**无需修改**——COMPONENT_REGISTRY 的遍历自动覆盖新组件。

### 4.3 instCounts 模式（EventEditor / SequenceEditor）

```typescript
const instCounts = Object.fromEntries(
  COMPONENT_REGISTRY.map((e) => [e.key, (data[e.countField] as number) ?? 0])
);
// 结果示例：{ wave: 2, bar: 1, table: 1, menu: 1 }
```

`EventEditor` 和 `SequenceEditor` 接收 `instCounts: Record<string, number>`，通过 `entry.key` 查找实例数。新增组件后，这两个组件**无需修改**。

### 4.4 验证模式

验证逻辑已移入各组件的 `xxx.registry.ts`，由 `entry.validate` 字段持有。`useProfileState` hook 在 `handleGenerate` / `handleBuildRun` 中遍历 `COMPONENT_REGISTRY` 调用：

```typescript
for (const entry of COMPONENT_REGISTRY) {
  const count = (data[entry.countField] as number) ?? 0;
  const profiles = (data[entry.profilesField] as unknown[]) ?? [];
  const err = entry.validate(profiles as never[], count);
  if (err) { showStatus({ type: "error", msg: err }); return; }
}
```

新增组件后，验证逻辑**无需修改 App.tsx**——只需在 `xxx.registry.ts` 的 `validate` 函数中实现。

### 4.5 工具栏布局约定

- 组件实例数控件由 COMPONENT_REGISTRY 自动生成，格式：`<label>NAME</label> <input>`
- 事件计数控件（如 BUTTON）手动追加在组件控件之后
- `toolbar-spacer` 分隔左侧控件和右侧按钮
- Screen 尺寸控件和操作按钮在右侧

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

### A.1 组件集成修改点（v2.0 注册表架构）

| # | 文件 | 修改位置 | 操作 |
|:---:|------|------|:---:|
| 1 | `newcomp.registry.ts` | — | **新建**（类型 + 常量 + 默认值 + 验证器 + ComponentEntry） |
| 2 | `NewCompEditor.tsx` | — | **新建** |
| 3 | `componentRegistry.ts` | 导入区 | 追加 `import { newcompEntry }` |
| 4 | `componentRegistry.ts` | `COMPONENT_REGISTRY` 数组 | 追加 `newcompEntry as unknown as ComponentEntry` |
| 5 | `types.ts` | `ProfileSet` 接口 | 追加 `newcomp_inst_count: number` |
| 6 | `types.ts` | `ProfileSet` 接口 | 追加 `newcomp_profiles: import(...).NewCompProfile[]` |
| 7 | `models.rs` | `EstaProfile` | 追加 `newcomp_*` 字段 |
| 8 | `models.rs` | `ProfileSet` | 追加 `pub newcomp_inst_count: u16` + `pub newcomp_profiles: Vec<EstaProfile>` |
| 9 | `commands.rs` | `default_profile()` | 追加 `newcomp_inst_count` + 默认 profile |
| 10 | `commands.rs` | `save_profile()` json! 映射 | 追加所有 `newcomp_*` 条目 |
| 11 | `commands.rs` | `save_profile()` ctx.insert | 追加 `"newcomp_inst_count"` |
| 12 | `ESTA_Profile.c.j2` | 顶层结构体 | 追加 `.newcomp_inst_count` |
| 13 | `ESTA_Profile.c.j2` | profile 循环 | 追加所有 `newcomp_*` 字段 |
| 14 | `ESTA_Profile.c.j2` | 文件末尾 | 追加 `ToNEWCOMP_Config` + `ApplyNEWCOMP` |

**对比 v1.0**：修改点从 22 个减少到 14 个，且 App.tsx 完全不需要修改。

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
| 组件集成 | 7 步骤（仅 profile-gui 侧） | 7 步骤（含 C 组件代码 + 主题注册 + 模拟器） |
| 事件集成 | 含 v1/v2 模式 + 扩展框架 | 不含（编写时事件系统尚未实现） |
| App.tsx 模式 | 深入分析（注册表驱动原理） | 简要概述 |
| 适用读者 | profile-gui 开发者 | 全栈开发者 |

两份文档互补——本规范深入 profile-gui 细节，INTEGRATION_SPEC.md 覆盖完整数据流。添加新组件时建议先读 INTEGRATION_SPEC.md 了解全局，再按本规范操作 profile-gui 侧。

---

## 附录 C：BARCHART 完整修改参考（v2.0 注册表架构）

以下列出 BARCHART 作为第二个组件集成到 profile-gui 时的所有实际修改，作为 `NEWCOMP` 的对照参考。

### bar.registry.ts（新建，65 行）

包含：`BarChartProfile` 接口、`MAX_BAR_INST=4`、`BAR_THEME_OPTIONS`、`makeDefaultBar()`、`validateBar()`、`barEntry: ComponentEntry<BarChartProfile>`。

### BarChartEditor.tsx（新建，约 120 行）

4 个 fieldset：位置与尺寸 / 数据范围 / 柱体配置 / 显示选项。
从 `bar.registry.ts` 导入 `BarChartProfile`、`BAR_THEME_OPTIONS`；从 `types.ts` 导入 `UI_FONT_SIZE_OPTIONS`。

### componentRegistry.ts（+2 行）

```typescript
import { barEntry } from "./bar.registry";
// COMPONENT_REGISTRY 数组追加 barEntry as unknown as ComponentEntry
```

### types.ts（+2 行）

```typescript
bar_inst_count: number;
bar_profiles: import("./bar.registry").BarChartProfile[];
```

### models.rs（+13 行）

```rust
// EstaProfile 追加 12 个 bar_* 字段
// ProfileSet 追加 pub bar_inst_count: u16 + pub bar_profiles: Vec<EstaProfile>
```

### commands.rs（+16 行）

default_profile 加 `bar_inst_count: 1` + 每个 profile 的 12 个 bar_* 默认值；
json! 映射加 12 个 `"bar_xxx": p.bar_xxx`；ctx.insert 加 `"bar_inst_count"`。

### ESTA_Profile.c.j2（+28 行）

顶层 `.bar_inst_count`；循环内 12 个 `.bar_*` 字段；末尾 `ToBARCHART_Config` + `ApplyBARCHART` 函数。


---
