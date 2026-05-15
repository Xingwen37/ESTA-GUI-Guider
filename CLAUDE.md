# CLAUDE.md

此文件为 Claude Code (claude.ai/code) 在本仓库中工作时提供指导。

## 项目概述

TL-ESTA 是一个嵌入式波形显示 GUI 库，用 C 语言以 OOP-in-C 风格编写，附带基于 SDL2 的 PC 仿真器用于桌面验证，以及一个基于 Tauri v2 的配置器用于代码生成。核心库面向带小型 LCD 的微控制器，但通过硬件抽象层可在桌面端完全一致地构建和运行。

## 目录结构

```
core/
├── ui/          # 可视化组件（WAVE, BARCHART, TABLE）
├── infra/       # 基础设施（helper, font, ui_base, ui_theme）
├── profile/     # Profile 系统（ESTA_Profile.*, ESTA_Profile.json）
└── event/       # 事件队列（button press/release）
port/            # 平台抽象层（SDL2 后端）
simulator/       # PC 仿真器入口（main.c, sim_scenario, btn_ui）
tools/profile-gui/  # Tauri 配置器 GUI
```

所有 `#include` 以 `core/` 为根目录（CMake 将其加入 include path）。跨目录引用使用带子目录的路径，如 `#include "infra/helper.h"`、`#include "ui/WAVE.h"`。

## 构建与运行命令

### C 仿真器（Windows MinGW）

```powershell
.\build.ps1                           # 配置 + 构建，输出 build/ESTA_Simulator.exe
.\build\ESTA_Simulator.exe            # 运行仿真器
```

`build.ps1` 将 `$env:CC` 设为 MinGW 的 gcc，执行 `cmake -G "MinGW Makefiles" -B build -S .` 然后 `cmake --build build`。MinGW 路径硬编码在脚本中——如果你的 MinGW 安装路径不同，请相应修改。

### C 仿真器（Linux/WSL）

```bash
cmake -B build && cmake --build build
./build/ESTA_Simulator
```

### Tauri 配置器 GUI

```bash
cd tools/profile-gui
npm install                           # 安装前端依赖（React、Vite、Tauri API）
npm run tauri dev                     # 开发模式，支持热重载
npm run tauri build                   # 生产构建
cargo check                           # Rust 类型检查（在 src-tauri/ 下运行）
npx tsc --noEmit                      # TypeScript 类型检查（在 tools/profile-gui/ 下运行）
```

## 架构

### OOP-in-C 模式（`core/ui/WAVE.h` 和 `core/ui/WAVE.c`）

波形引擎使用类似类的结构体层次，通过寄存器风格的宏来访问：

```
WAVE_TypeDef              ← 每个实例一个（静态数组 WAVE_State[MAX_WAVE_NUM]）
├── WAVE_Config_TypeDef   ← 公共"寄存器"（位置、尺寸、通道、标尺、主题）
└── WAVE_Private_Typedef  ← 私有状态（标尺缓冲区、上次坐标）
```

所有访问必须通过宏，绝不直接访问字段：
- `WAVE_CONFIG_MEMBER(inst, field)` / `WAVE_PRIVATE_MEMBER(inst, field)` — 读取
- `WAVE_WRITE_CONFIG(inst, field, val)` / `WAVE_WRITE_PRIVATE(inst, field, val)` — 写入

`WAVE_ConfigSet*` 系列函数（如 `WAVE_ConfigSetPositionAndSize`）是配置结构体的公开 API。然后 `WAVE_Init(inst_idx, &config)` 将配置深拷贝到实例的配置寄存器中，并初始化私有状态（将标尺数组拷贝到私有缓冲区、重置坐标状态）。

最多支持 4 个实例（`MAX_WAVE_NUM`），每个实例最多 4 个通道（`MAX_WAVE_CHANNEL`）。实例之间完全隔离——每个实例拥有独立的配置、私有状态和屏幕区域。

标尺容量：Y/X 轴各最多 10 个标尺（`WAVE_MAX_RULER_Y_NUM` / `WAVE_MAX_RULER_X_NUM`）。

关键 API：
| API | 说明 |
|-----|------|
| `WAVE_Init()` | 初始化实例并载入配置 |
| `WAVE_CurveDraw()` | 压入单帧通道数据并触发波形绘制 |
| `WAVE_CurveDrawBatch()` | 批量压入多采样点数据（滑动窗口），减少逐点绘制开销 |
| `WAVE_ReDraw()` | 强制重绘边框、背景与坐标轴 |
| `WAVE_GetSampleCapacity()` | 返回绘图区可容纳的采样点数 |

### TABLE 组件（`core/ui/TABLE.h`）

表格组件，支持每行显示 label / value / unit 三列。每行 value 可为文本、uint32 或 float。最多 4 个实例、每实例最多 8 行、字符串最长 16 字符。API 包括 `TABLE_UpdateUInt32`、`TABLE_UpdateFloat`、`TABLE_UpdateText` 按行更新。

### 事件系统（`core/event/event.h`）

简易事件队列（容量 16），当前支持按键按下/释放事件（`ESTA_EVENT_BUTTON_PRESS` / `ESTA_EVENT_BUTTON_RELEASE`）。仿真器在 `simulator/btn_ui.c` 中实现按键 UI，通过 `ESTA_EventPush()` 注入事件，核心库可通过 `ESTA_EventPoll()` 消费。

### 硬件抽象边界

核心库（`core/`）与任何具体的显示技术无关。七个绘制宏构成了移植面：

| 宏 | 用途 |
|---|---|
| `SCREEN_DRAW_LINE(x0, y0, x1, y1, color)` | 绘制直线 |
| `SCREEN_DRAW_RECTANGLE(x, y, w, h, color)` | 绘制矩形边框 |
| `SCREEN_FILL(x, y, w, h, color)` | 填充矩形 |
| `SCREEN_DRAW_NUM(x, y, num, digits, color)` | 使用默认字体绘制数字 |
| `SCREEN_DRAW_STRING(x, y, str, len, color)` | 使用默认字体绘制字符串 |
| `SCREEN_DRAW_NUM_FONT(x, y, num, digits, font, color)` | 使用指定字体绘制数字 |
| `SCREEN_DRAW_STRING_FONT(x, y, str, len, font, color)` | 使用指定字体绘制字符串 |

字体大小由 `ESTA_FontSize` 枚举定义：`ESTA_FONT_1206`（12×6）、`ESTA_FONT_1608`（16×8，默认）、`ESTA_FONT_2412`（24×12）。

在实际硬件上，这些宏映射到 ILI9341 LCD 驱动调用。在仿真器上，它们映射到 SDL2 渲染（`port/esta_port_sdl2.c`），使用虚拟的 400×320 纹理以 2 倍缩放渲染。字体数据（`core/infra/font.c`）使用的是真实的 MCU 点阵字库 `asc2_1608`——与芯片上使用的位图完全一致。

### Profile 系统与代码生成（`core/profile/ESTA_Profile.*`）

`ESTA_Profile_TypeDef` 是一个可序列化的配置描述符（包含所有组件的配置字段，外加以内联数组形式存储的标尺数据）。`ESTA_Profile_Apply()` 将 profile 转换为各组件的 `Config_TypeDef` 并调用各自的 `Init()`。

文件 `core/profile/ESTA_Profile.c` 由 Tauri 配置器 GUI 从 `core/profile/ESTA_Profile.json` **自动生成**。生成流程：

```
ESTA_Profile.json（JSON 数据源）
    → Rust serde 反序列化
    → Tera 模板（src-tauri/templates/ESTA_Profile.c.j2）
    → core/profile/ESTA_Profile.c（生成的 C 代码）
```

GUI 在每次保存时也会回写 `ESTA_Profile.json`，保持二者同步。

### Tauri GUI 命令流（`tools/profile-gui/src-tauri/`）

四个 Tauri 命令连接前端与文件系统和构建系统：

| 命令 | 功能 |
|---|---|
| `load_profile` | 读取 `core/profile/ESTA_Profile.json`，如有 BOM 则剥离，解析为 `ProfileSet`。文件不存在时回退到硬编码的默认值。 |
| `save_profile` | 将 profile 数据序列化为 JSON 上下文，渲染 Tera 模板 → 写入 `core/profile/ESTA_Profile.c`（含 `.c.bak` 备份），写入 `core/profile/ESTA_Profile.json`。 |
| `build_simulator` | 在仓库根目录下运行 `cmake` 配置 + 构建（硬编码使用 "MinGW Makefiles" 生成器）。 |
| `run_simulator` | 启动 `build/ESTA_Simulator.exe`。 |

`repo_root()`（在 `commands.rs` 中）从 `std::env::current_dir()` 出发向上遍历目录树，查找包含 `CMakeLists.txt` 的目录。Tauri 的 `AppState` 保存解析后的 `repo_root` 和预加载了 `src-tauri/templates/*.j2` 的 `Tera` 引擎。

### Profile 配置结构体的字段映射

`EstaProfile` 结构体出现在三个地方，必须保持同步：
- **C**：`core/profile/ESTA_Profile.h` 中的 `ESTA_Profile_TypeDef`
- **Rust**：`src-tauri/src/models.rs` 中的 `EstaProfile`
- **TypeScript**：`src/lib/types.ts` 中的 `EstaProfile`

关键字段约定：
- 各组件的 `x_origin`/`y_origin`/`x_width`/`y_width` 以组件前缀命名：WAVE 无前缀（`x_origin`），BARCHART 用 `bar_` 前缀（`bar_x_origin`），TABLE 用 `table_` 前缀（`table_x_origin`），新组件类推。
- `channel_mask`：已启用通道的位掩码（bit 0 = CH0，以此类推）。Rust 模型的 `channel_mask_expr()` 方法会生成 C 语言的 OR 表达式。
- `ruler_y` / `ruler_x`：固定大小的数组。实际仅使用前 `ruler_count_y`/`ruler_count_x` 个元素。
- `theme_type`：字符串，取值为 `WAVE_THEME_DEFAULT` 或 `WAVE_THEME_LIGHT`（Rust 后端将其原样传入 C 模板）。各组件有独立的 theme 枚举。

### 前端布局（`tools/profile-gui/src/`）

- `App.tsx`：根组件。挂载时加载 profile，渲染工具栏（inst_count 数值框、生成/Build&Run 按钮、状态栏）+ 标签栏 + 各组件 Editor。在生成/构建前执行校验。**注意**：错误状态仅在 `data` 非空时渲染（loading 提前返回时也会显示错误信息）。
- `ProfileEditor.tsx`：WAVE 编辑——基础参数、通道使能复选框（位掩码操作）、Y 标尺设置、X 标尺设置。
- `BarChartEditor.tsx`：BARCHART 编辑——位置尺寸、柱体数量、主题等。
- `TableEditor.tsx`：TABLE 编辑——位置尺寸、行配置、列宽、主题等。

## 常见坑点

### Windows 下的 UTF-8 BOM

PowerShell 和某些 Windows 编辑器写入 UTF-8 文件时会附带 BOM（`EF BB BF`）。`serde_json` 和 `tauri` 的 JSON 解析器遇到带 BOM 的文件会报 "expected value at line 1 column 1" 错误。`load_profile` 的 Rust 命令现在会在解析前剥离 BOM，但在创建或编辑 JSON 文件时仍需注意。在 PowerShell 中使用 `[System.Text.UTF8Encoding]::new($false)` 来写入无 BOM 的文件。

### .gitignore 作用域

含 `/` 的模式（如 `src-tauri/target/`）相对于仓库根目录匹配。不含 `/` 的模式（如 `target/`）匹配任意深度的目录。`target/` 模式会覆盖所有 Rust 构建目录，包括 `tools/profile-gui/src-tauri/target/`。

### CMake 编译器标志

`-Wall -Wextra` 仅适用于 GCC/Clang。MSVC 改用 `/W4`。CMakeLists.txt 会检测编译器并条件性地设置标志。

## 项目命名

项目最初命名为 OSC，之后改为 ESTA，核心渲染引擎最近又从 ESTA 重命名为 WAVE。当前的命名约定为：
- **WAVE** — 波形渲染引擎（`core/ui/WAVE.h`、`WAVE_TypeDef`、`WAVE_Init` 等）
- **ESTA** — profile/配置系统及整体项目名称（`core/profile/ESTA_Profile.*`、`ESTA_Simulator`）
- Tauri GUI 的 crate 名称为 `esta-profile-gui`

## 组件开发规范（速查）

完整规范文档：`docs/COMPONENT_SPEC.md`。所有新 UI 组件必须遵循该规范。

### 组件文件模板

```
core/ui/{NAME}.h + core/ui/{NAME}.c   // 组件代码（如 WAVE.h/WAVE.c）
```

### 三结构体模式（OOC）

```c
{NAME}_Config_TypeDef      // 公开配置"寄存器"（TypeDef 后缀）
{NAME}_Private_Typedef     // 私有运行时状态（Typedef 后缀 — 注意拼写差异）
{NAME}_TypeDef             // 聚合实例（{NAME}_Config + {NAME}_Private）
```

全局实例数组：`{NAME}_TypeDef {NAME}_State[MAX_{NAME}_NUM];`

### 核心宏速查

| 宏 | 用途 |
|------|------|
| `{NAME}_INST(i)` | 实例身份宏（展开为 `(i)`） |
| `{NAME}_INST_ADDR(i)` | 获取实例引用 → `State[i]` |
| `{NAME}_CONFIG_MEMBER(inst, f)` | 读取 Config 字段 |
| `{NAME}_PRIVATE_MEMBER(inst, f)` | 读取 Private 字段 |
| `{NAME}_WRITE_CONFIG(inst, f, v)` | 写入 Config 字段 |
| `{NAME}_WRITE_PRIVATE(inst, f, v)` | 写入 Private 字段 |
| `{NAME}_WRITE_CONFIG_INIT(inst, f)` | 从 Init 参数复制到实例（仅限 Init 内） |
| `{NAME}_CONFIG_MEMBER_ARRAY(inst, f, i)` | 数组版读取 |
| `{NAME}_PRIVATE_MEMBER_ARRAY(inst, f, i)` | 数组版读取 |
| `IS_VALID_{NAME}_INST(x)` | 实例索引范围检查 |

### 共享基础设施（所有组件通用）

| 定义 | 位置 | 用途 |
|------|------|------|
| `ESTA_StatusTypeDef` | `core/infra/ui_base.h` | 统一状态码（OK/ERROR/FULL） |
| `ESTA_BaseConfig` | `core/infra/ui_base.h` | 公共配置基类（x_origin, y_origin, x_width, y_width） |
| `ESTA_ConfigSetPositionAndSize` | `core/infra/ui_base.c` | 公共位置尺寸 Setter |
| `ESTA_RETURN_IF_ERROR(expr)` | `core/infra/ui_base.h` | 错误传播宏 |
| `ESTA_GetThemeColor(t, idx)` | `core/infra/ui_theme.h` | 主题颜色访问（所有组件共用） |
| `CH0`–`CH7` | `core/infra/helper.h` | 通道掩码常量 |
| `ESTA_FontSize` | `core/infra/ui_base.h` | 字体大小枚举（1206/1608/2412） |

### 关键规则

1. **所有 Config/Private 访问必须通过宏**，严禁 `State[i].Config.field` 直接访问
2. **Config Setter 必须返回 `ESTA_StatusTypeDef`**（新规，不再 void），必须 NULL 检查
3. **状态码统一使用 `ESTA_OK`/`ESTA_ERROR`/`ESTA_FULL`**（定义在 `ui_base.h`）
4. **绘制仅通过 `SCREEN_DRAW_*` 宏**，禁止直接调 Port 层函数
5. **每个组件必须配套 Profile**（GetDefault + ToConfig + Apply）
6. **主题颜色通过 `ESTA_GetThemeColor` 获取**（定义在 `ui_theme.h`）
7. **`ESTA_ConfigSetPositionAndSize` 公共 Setter**，组件不再各自实现
8. **错误传播使用 `ESTA_RETURN_IF_ERROR(expr)`** 宏
9. **Config 结构体前 4 字段必须与 `ESTA_BaseConfig` 一致**（x_origin, y_origin, x_width, y_width）
10. **Config 指针型字段**（仅 Init 传参用）：Init 后必须置 NULL
11. **命名**：Config 用 `TypeDef`，Private 用 `Typedef`（有意区分 Public/Private）
12. **通道掩码常量 CH0-CH7** 定义在 `helper.h` 中

### 主题颜色系统

各组件在自身 `.c` 文件中定义独立本地色表，通过全局宏 `ESTA_THEME_COLOR(table, theme, idx)` 访问，颜色索引从 0 开始：

| 组件 | 色表 | 维度 |
|------|------|------|
| WAVE | `WAVE_ColorTable` | `[2][7]` |
| BARCHART | `BARCHART_ColorTable` | `[2][8]` |
| TABLE | `TABLE_ColorTable` | `[2][7]` |

新组件自建色表，无需修改 `ui_theme.h` 或 `ui_theme.c`。详见 `docs/COMPONENT_SPEC.md` 第九章。

### 新组件集成需修改的文件（约 14 个）

`core/ui/XXX.h`, `core/ui/XXX.c`（新建），`core/profile/ESTA_Profile.h`, `core/profile/ESTA_Profile.c`, `core/profile/ESTA_Profile.json`, `tools/profile-gui/src-tauri/templates/ESTA_Profile.c.j2`, `tools/profile-gui/src-tauri/src/models.rs`, `tools/profile-gui/src-tauri/src/commands.rs`, `tools/profile-gui/src/lib/types.ts`, `tools/profile-gui/src/components/XXXEditor.tsx`（新建），`tools/profile-gui/src/App.tsx`, `simulator/sim_scenario.h/.c`, `simulator/main.c`, `docs/COMPONENT_SPEC.md`

完整集成步骤参见 `docs/INTEGRATION_SPEC.md`（三层架构：C核心 → Rust后端 → TS前端，含精确修改位置和代码模板）。

profile-gui 侧的集成模式（TS 前端 + Rust 后端 + Tera 模板）详见 `docs/PROFILE_GUI_SPEC.md`（含新组件 6 步骤、新事件 v1/v2 模式、App.tsx 核心模式、修改速查表）。

### Profile 字段前缀约定

| 组件 | 前缀 | 示例 |
|------|------|------|
| WAVE | (无) | `x_origin`, `theme_type` |
| BARCHART | `bar_` | `bar_x_origin`, `bar_theme_type` |
| TABLE | `table_` | `table_x_origin`, `table_theme_type` |
| 新组件 | `xxx_` | `xxx_x_origin`, `xxx_theme_type` |

### 标准包含顺序

```c
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "infra/helper.h"
#include "infra/ui_base.h"
#include "infra/ui_theme.h"
```
