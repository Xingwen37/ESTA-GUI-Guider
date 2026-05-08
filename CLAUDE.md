# CLAUDE.md

此文件为 Claude Code (claude.ai/code) 在本仓库中工作时提供指导。

## 项目概述

TL-ESTA 是一个嵌入式波形显示 GUI 库，用 C 语言以 OOP-in-C 风格编写，附带基于 SDL2 的 PC 仿真器用于桌面验证，以及一个基于 Tauri v2 的配置器用于代码生成。核心库面向带小型 LCD 的微控制器，但通过硬件抽象层可在桌面端完全一致地构建和运行。

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

### OOP-in-C 模式（`core/WAVE.h` 和 `core/WAVE.c`）

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

### 硬件抽象边界

核心库（`core/`）与任何具体的显示技术无关。四个绘制宏构成了移植面：

| 宏 | 用途 |
|---|---|
| `SCREEN_DRAW_LINE(x0, y0, x1, y1, color)` | 绘制直线 |
| `SCREEN_DRAW_RECTANGLE(x, y, w, h, color)` | 绘制矩形边框 |
| `SCREEN_FILL(x, y, w, h, color)` | 填充矩形 |
| `SCREEN_DRAW_NUM(x, y, num, digits, color)` | 使用 MCU 字体绘制数字 |

在实际硬件上，这些宏映射到 ILI9341 LCD 驱动调用。在仿真器上，它们映射到 SDL2 渲染（`port/esta_port_sdl2.c`），使用虚拟的 320×240 纹理以 2 倍缩放渲染。字体数据（`core/font.c`）使用的是真实的 MCU 点阵字库 `asc2_1608`——与芯片上使用的位图完全一致。

### Profile 系统与代码生成（`core/ESTA_Profile.*`）

`ESTA_Profile_TypeDef` 是一个可序列化的配置描述符（包含 `WAVE_Config_TypeDef` 的所有字段，外加以内联数组形式存储的标尺数据）。`ESTA_Profile_Apply()` 将 profile 转换为 `WAVE_Config_TypeDef` 并调用 `WAVE_Init()`。

文件 `core/ESTA_Profile.c` 由 Tauri 配置器 GUI 从 `core/ESTA_Profile.json` **自动生成**。生成流程：

```
ESTA_Profile.json（JSON 数据源）
    → Rust serde 反序列化
    → Tera 模板（src-tauri/templates/ESTA_Profile.c.j2）
    → core/ESTA_Profile.c（生成的 C 代码）
```

GUI 在每次保存时也会回写 `ESTA_Profile.json`，保持二者同步。

### Tauri GUI 命令流（`tools/profile-gui/src-tauri/`）

四个 Tauri 命令连接前端与文件系统和构建系统：

| 命令 | 功能 |
|---|---|
| `load_profile` | 读取 `core/ESTA_Profile.json`，如有 BOM 则剥离，解析为 `ProfileSet`。文件不存在时回退到硬编码的默认值。 |
| `save_profile` | 将 profile 数据序列化为 JSON 上下文，渲染 Tera 模板 → 写入 `core/ESTA_Profile.c`（含 `.c.bak` 备份），写入 `core/ESTA_Profile.json`。 |
| `build_simulator` | 在仓库根目录下运行 `cmake` 配置 + 构建（硬编码使用 "MinGW Makefiles" 生成器）。 |
| `run_simulator` | 启动 `build/ESTA_Simulator.exe`。 |

`repo_root()`（在 `commands.rs` 中）从 `std::env::current_dir()` 出发向上遍历目录树，查找包含 `CMakeLists.txt` 的目录。Tauri 的 `AppState` 保存解析后的 `repo_root` 和预加载了 `src-tauri/templates/*.j2` 的 `Tera` 引擎。

### Profile 配置结构体的字段映射

`EstaProfile` 结构体出现在三个地方，必须保持同步：
- **C**：`core/ESTA_Profile.h` 中的 `ESTA_Profile_TypeDef`
- **Rust**：`src-tauri/src/models.rs` 中的 `EstaProfile`
- **TypeScript**：`src/lib/types.ts` 中的 `EstaProfile`

关键字段：
- `channel_mask`：已启用通道的位掩码（bit 0 = CH0，以此类推）。Rust 模型的 `channel_mask_expr()` 方法会生成 C 语言的 OR 表达式，如 `CH0 | CH1 | CH3`。
- `ruler_y` / `ruler_x`：固定大小为 5 的 `u16` 数组。实际仅使用前 `ruler_count_y`/`ruler_count_x` 个元素。
- `theme_type`：字符串，取值为 `WAVE_THEME_DEFAULT` 或 `WAVE_THEME_LIGHT`（Rust 后端将其原样传入 C 模板）。

### 前端布局（`tools/profile-gui/src/`）

- `App.tsx`：根组件。挂载时加载 profile，渲染工具栏（inst_count 数值框、生成/Build&Run 按钮、状态栏）+ 标签栏 + `ProfileEditor`。在生成/构建前执行校验。**注意**：错误状态仅在 `data` 非空时渲染（loading 提前返回时也会显示错误信息）。
- `ProfileEditor.tsx`：四个 fieldset 分组——基础参数、通道使能复选框（位掩码操作）、Y 标尺设置、X 标尺设置。

## 常见坑点

### Windows 下的 UTF-8 BOM

PowerShell 和某些 Windows 编辑器写入 UTF-8 文件时会附带 BOM（`EF BB BF`）。`serde_json` 和 `tauri` 的 JSON 解析器遇到带 BOM 的文件会报 "expected value at line 1 column 1" 错误。`load_profile` 的 Rust 命令现在会在解析前剥离 BOM，但在创建或编辑 JSON 文件时仍需注意。在 PowerShell 中使用 `[System.Text.UTF8Encoding]::new($false)` 来写入无 BOM 的文件。

### .gitignore 作用域

含 `/` 的模式（如 `src-tauri/target/`）相对于仓库根目录匹配。不含 `/` 的模式（如 `target/`）匹配任意深度的目录。`target/` 模式会覆盖所有 Rust 构建目录，包括 `tools/profile-gui/src-tauri/target/`。

### CMake 编译器标志

`-Wall -Wextra` 仅适用于 GCC/Clang。MSVC 改用 `/W4`。CMakeLists.txt 会检测编译器并条件性地设置标志。

## 项目命名

项目最初命名为 OSC，之后改为 ESTA，核心渲染引擎最近又从 ESTA 重命名为 WAVE。当前的命名约定为：
- **WAVE** — 波形渲染引擎（`core/WAVE.h`、`WAVE_TypeDef`、`WAVE_Init` 等）
- **ESTA** — profile/配置系统及整体项目名称（`core/ESTA_Profile.*`、`ESTA_Simulator`）
- Tauri GUI 的 crate 名称为 `esta-profile-gui`

## 组件开发规范（速查）

完整规范文档：`docs/COMPONENT_SPEC.md`。所有新 UI 组件必须遵循该规范。

### 组件文件模板

```
core/{NAME}.h + core/{NAME}.c   // 组件代码（如 WAVE.h/WAVE.c）
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
| `ESTA_StatusTypeDef` | `core/ui_base.h` | 统一状态码（OK/ERROR/FULL） |
| `ESTA_BaseConfig` | `core/ui_base.h` | 公共配置基类（x_origin, y_origin, x_width, y_width） |
| `ESTA_ConfigSetPositionAndSize` | `core/ui_base.c` | 公共位置尺寸 Setter |
| `ESTA_RETURN_IF_ERROR(expr)` | `core/ui_base.h` | 错误传播宏 |
| `ESTA_GetThemeColor(t, idx)` | `core/ui_theme.h` | 主题颜色访问（所有组件共用） |
| `CH0`–`CH7` | `core/helper.h` | 通道掩码常量 |

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

### 主题颜色槽位

| 槽位 | 组件 |
|:---:|------|
| 0-6 | WAVE (FRAME, RULER, CH0-3, BG) |
| 7-15 | 预留 |

新组件需在 `core/ui_theme.c` 中注册颜色（指定初始化器）。

### 新组件集成需修改的文件（共 16 个）

`core/XXX.h`, `core/XXX.c`（新建），`core/ui_theme.c`, `core/ui_theme.h`, `core/ESTA_Profile.h`, `core/ESTA_Profile.c`, `core/ESTA_Profile.json`, `tools/profile-gui/src-tauri/templates/ESTA_Profile.c.j2`, `tools/profile-gui/src-tauri/src/models.rs`, `tools/profile-gui/src-tauri/src/commands.rs`, `tools/profile-gui/src/lib/types.ts`, `tools/profile-gui/src/components/XXXEditor.tsx`（新建），`tools/profile-gui/src/App.tsx`, `simulator/sim_scenario.h/.c`, `simulator/main.c`, `docs/COMPONENT_SPEC.md`

完整集成步骤参见 `docs/INTEGRATION_SPEC.md`（三层架构：C核心 → Rust后端 → TS前端，含精确修改位置和代码模板）。

### 主题颜色槽位分配

| 槽位 | 组件 |
|:---:|------|
| 0-6 | WAVE (FRAME, RULER, CH0-3, BG) |
| 7-14 | BARCHART (FRAME, AXIS, BAR, BAR_CH1-3, BG, LABEL) |
| 15-31 | 预留 |

### Profile 字段前缀约定

| 组件 | 前缀 | 示例 |
|------|------|------|
| WAVE | (无) | `x_origin`, `theme_type` |
| BARCHART | `bar_` | `bar_x_origin`, `bar_theme_type` |
| 新组件 | `xxx_` | `xxx_x_origin`, `xxx_theme_type` |

### 标准包含顺序

```c
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper.h"
#include "ui_base.h"
#include "ui_theme.h"
```
