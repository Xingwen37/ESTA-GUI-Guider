# TL-ESTA · v0.1.0-beta

![version](https://img.shields.io/badge/version-v0.1.0--beta-orange)
![platform](https://img.shields.io/badge/platform-MCU%20%7C%20PC%20Simulator-blue)
![license](https://img.shields.io/badge/license-MIT-green)

中文版本 | [English Version](README.md)

---

TL-ESTA 是一个面向嵌入式 MCU 的轻量级 GUI 库，采用 OOP-in-C 架构，支持波形、柱状图、表格、菜单等多种可视化组件，内建事件驱动系统实现按键/编码器/定时器驱动的 UI 交互。配套提供基于 SDL2 的 PC 仿真器和基于 Tauri v2 的可视化配置器，实现"PC 端验证，零修改移植"。

> ⚠️ **测试版声明**：当前为 `v0.1.0-beta`，API 与配置格式可能在正式版前发生变更。

### 1. 核心特性

- **OOP-in-C 多实例架构**：每个组件支持多实例独立运行，数据与配置严格隔离，通过寄存器风格宏访问。
- **事件驱动 UI**：按键、编码器、软件定时器、Flag 均可触发组件刷新、页面切换、菜单导航等 Action，支持 Guard 条件与 Action 序列。
- **多组件支持**：WAVE（波形）、BARCHART（柱状图）、TABLE（数据表格）、MENU（树形菜单）。
- **多页系统**：组件按页分组，支持运行时页面切换。
- **可视化配置器**（`tools/profile-gui`）：Tauri v2 + React 桌面应用，所见即所得编辑布局与事件绑定，一键 Build & Run，内置截图预览。
- **像素级仿真**：PC 仿真器复用 MCU 点阵字库，显示效果与真实硬件 100% 一致。

### 2. 快速开始（PC 仿真器）

**Linux / WSL**

```bash
sudo apt install build-essential cmake libsdl2-dev
cmake -B build && cmake --build build
./build/ESTA_Simulator
```

**Windows（MinGW64）**

前置条件：安装 [MinGW-w64](https://www.mingw-w64.org/) 及 SDL2 开发库。

```powershell
.\build.ps1                              # 一键配置 + 构建
.\build\ESTA_Simulator.exe               # 运行仿真器
.\build\ESTA_Simulator.exe --screenshot build\preview  # 截图模式
```

### 3. Profile 配置器（`tools/profile-gui`）

基于 **Tauri v2 + React + Rust** 的桌面配置器，二进制体积约 5 MB。

**前置条件**
- [Rust](https://www.rust-lang.org/) >= 1.70
- [Node.js](https://nodejs.org/) >= 18
- Windows：系统自带 WebView2；Linux：需安装 `libwebkit2gtk-4.1-dev`

**启动（开发模式）**
```bash
cd tools/profile-gui
npm install
npm run tauri dev
```

**打包为独立可执行文件**
```bash
npm run tauri build
```

**功能一览**

| 面板 | 功能 |
|---|---|
| WAVE / BARCHART / TABLE / MENU | 组件布局、参数、主题可视化编辑 |
| Event Bindings | 事件绑定：触发源 → Action → 目标，支持 Guard 条件 |
| Action Sequences | 一个触发对应多个 Action 的序列编辑 |
| String Table | 预定义文本资源池 |
| Soft Timers | 软件定时器配置（最多 4 个） |
| Flag Config | Flag 状态位配置（最多 8 个） |

**数据流**
```
ESTA_Profile.json  →  Rust serde + Tera 模板  →  core/profile/ESTA_Profile.c
```

### 4. 移植指南

1. 将 `core/` 目录整体加入目标工程，并加入 include path。
2. 在 port 层实现 7 个绘图宏（`SCREEN_DRAW_LINE`、`SCREEN_DRAW_RECTANGLE`、`SCREEN_FILL`、`SCREEN_DRAW_NUM`、`SCREEN_DRAW_STRING`、`SCREEN_DRAW_NUM_FONT`、`SCREEN_DRAW_STRING_FONT`），映射到目标 LCD 驱动。
3. 调用 `App_MainInit(&app, LCD_WIDTH, LCD_HEIGHT)` 完成 Profile 加载与组件初始化。
4. 主循环中调用 `App_MainTick(&app)` `ESTA_SoftTimerTick(SIM_TARGET_FRAME_MS)`;。

详细移植参考 `docs/INTEGRATION_SPEC.md`。

### 5. 项目结构

```
ESTA-GUI-Guider/
├── core/
│   ├── ui/          # 可视化组件（WAVE, BARCHART, TABLE, MENU）
│   ├── infra/       # 基础设施（helper, font, ui_base, ui_theme）
│   ├── profile/     # Profile 系统（ESTA_Profile.h/.c/.json）
│   ├── event/       # 事件队列、Flag、软件定时器
│   └── app/         # 可移植应用骨架（app_main, app_page, app_event, app_action）
├── port/            # 平台抽象层（SDL2 后端）
├── simulator/       # PC 仿真器（main.c, sim_scenario, sim_feed, sim_input）
├── tools/
│   └── profile-gui/ # Tauri 配置器（src/ React 前端 + src-tauri/ Rust 后端）
├── docs/            # 规格文档（COMPONENT_SPEC, EVENT_SYSTEM_SPEC 等）
├── CMakeLists.txt
└── build.ps1        # Windows MinGW 一键构建脚本
```
