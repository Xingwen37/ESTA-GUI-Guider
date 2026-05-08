# TL-ESTA 项目模拟器

[English Version](#english-version) | [中文版本](#chinese-version)

---

<h2 id="chinese-version">中文版本</h2>

TL-ESTA 是一个基于 OOP-in-C (OOC) 架构的轻量级嵌入式波形显示 GUI 库，配套提供基于 SDL2 的跨平台 PC 模拟器，旨在实现“PC端验证，零修改移植”。

### 1. 核心特性
* **OOP-in-C 架构**：支持多实例、多通道独立运行，数据与配置严格隔离。
* **极简移植**：显示接口高度解耦，仅需实现 4 个基础画图宏即可接入任意屏幕硬件。
* **免烧录验证**：内建 PC 模拟器（SDL2），支持在 Linux/WSL 环境下进行 UI 逻辑测试。
* **像素级还原**：复用单片机点阵字库，模拟器显示效果与真实硬件 100% 一致。

### 2. 核心 API
| API | 功能说明 |
| :--- | :--- |
| `WAVE_Init()` | 初始化显示实例并载入配置。 |
| `WAVE_DeInit()` | 复位显示实例，清空数据。 |
| `WAVE_CurveDraw()` | 压入通道数据并触发波形绘制。 |
| `WAVE_ReDraw()` | 强制重绘边框、背景与坐标轴。 |
| `ESTA_GetThemeColor()` | 获取当前主题下指定组件的颜色值 (RGB565)。 |

### 3. 模拟器编译与运行

**Linux / WSL**

安装依赖：
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev
```

编译执行：
```bash
cmake -B build
cmake --build build
./build/ESTA_Simulator
```

**Windows (MinGW64)**

前置条件：安装 [MinGW-w64](https://www.mingw-w64.org/) 及 SDL2 开发库。

使用 `build.ps1` 一键构建：
```powershell
.\build.ps1
```

或手动执行：
```powershell
$env:PATH = "<mingw64>/bin;" + $env:PATH
cmake -G "MinGW Makefiles" -B build -S .
cmake --build build
.\build\ESTA_Simulator.exe
```

清理：
```bash
rm -rf build/
```

### 4. 图形化配置器（`tools/profile-gui`）

基于 **Tauri v2 + React + Rust** 的桌面配置器，替代原 PySide6 方案，二进制体积 ~5MB。

**前置条件**
- [Rust](https://www.rust-lang.org/) (>= 1.70)
- [Node.js](https://nodejs.org/) (>= 18)
- Windows：系统自带 WebView2；Linux：需安装 `libwebkit2gtk-4.1-dev`

**启动（开发模式）**
```bash
cd tools/profile-gui
npm install
npm run tauri dev
```

**打包为独立可执行文件**
```bash
cd tools/profile-gui
npm run tauri build
```

**使用方式**
- 界面中编辑 ESTA 实例参数（坐标、通道、标尺、主题）。
- 点击「生成 ESTA_Profile.c」：从 JSON 源文件渲染写入 `core/ESTA_Profile.c`。
- 点击「Build & Run」：自动保存、编译并启动模拟器。

**数据流**
- 配置源文件：`core/ESTA_Profile.json`
- 生成输出：`core/ESTA_Profile.c`
- 模板引擎：Tera（语法兼容 Jinja2）

### 5. 移植方式
**步骤**
1. 在目标工程中保留 `core/WAVE.c`、`core/WAVE.h`、`core/ESTA_Profile.c`、`core/ESTA_Profile.h`。  
2. 在 `ESTA.h` 中适配 4 个底层绘图宏：`SCREEN_DRAW_LINE`、`SCREEN_DRAW_RECTANGLE`、`SCREEN_FILL`、`SCREEN_DRAW_NUM`。  
3. 调用 `ESTA_Profile_GetDefault()` 读取配置，使用 `ESTA_Profile_Apply()` 初始化实例。  
4. 主循环中持续调用 `WAVE_CurveDraw()` 输入采样数据，并按需调用 `WAVE_ReDraw()`。  

**最小 `main.c` 示例（可裁剪）**
```c
#include "ESTA.h"
#include "ESTA_Profile.h"

int main(void) {
    /* 平台初始化：时钟/屏幕/端口 */
    Platform_Init();

    const ESTA_ProfileSet_TypeDef *profiles = ESTA_Profile_GetDefault();
    if (profiles == NULL) return -1;

    for (int i = 0; i < profiles->inst_count; ++i) {
        if (ESTA_Profile_Apply(WAVE_INST(i), &profiles->profiles[i]) != ESTA_OK) return -1;
        if (WAVE_ReDraw(WAVE_INST(i)) != ESTA_OK) return -1;
    }

    while (1) {
        uint16_t ch_data[MAX_WAVE_CHANNEL] = {0};
        AcquireSignal(ch_data);                 /* 用户实现：采样或读取缓存 */
        WAVE_CurveDraw(WAVE_INST(0), ch_data);   /* 可扩展到多实例 */
    }
}
```

---

<h2 id="english-version">English Version</h2>

TL-ESTA is a lightweight embedded waveform display GUI library built on an OOP-in-C architecture. It includes an SDL2-based cross-platform PC simulator, enabling UI verification on PC before seamless porting to hardware.

### 1. Core Features
* **OOP-in-C** : Supports multi-instance and multi-channel operations with isolated data/configuration.
* **Minimal Porting** : Highly decoupled display layer; requires only 4 basic drawing macros to port.
* **No-Flash Verification** : Built-in PC simulator (SDL2) for UI logic testing on Linux/WSL.
* **Pixel-Perfect** : Reuses MCU dot-matrix fonts to accurately replicate hardware visuals.

### 2. Core APIs
| API | Description |
| :--- | :--- |
| `WAVE_Init()` | Initialize an display instance with configuration. |
| `WAVE_DeInit()` | Reset instance and clear data. |
| `WAVE_CurveDraw()` | Push channel data and trigger waveform drawing. |
| `WAVE_ReDraw()` | Force redraw of frames, backgrounds, and coordinate rulers. |
| `ESTA_GetThemeColor()` | Get RGB565 color value for specific UI components. |

### 3. Build & Run Simulator

**Linux / WSL**

Install dependencies:
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev
```

Build and run:
```bash
cmake -B build
cmake --build build
./build/ESTA_Simulator
```

**Windows (MinGW64)**

Prerequisites: [MinGW-w64](https://www.mingw-w64.org/) with SDL2 development libraries.

One-click build with `build.ps1`:
```powershell
.\build.ps1
```

Or manually:
```powershell
$env:PATH = "<mingw64>/bin;" + $env:PATH
cmake -G "MinGW Makefiles" -B build -S .
cmake --build build
.\build\ESTA_Simulator.exe
```

Clean:
```bash
rm -rf build/
```

---

### 4. Profile GUI (`tools/profile-gui`)

A **Tauri v2 + React + Rust** desktop configurator replacing the previous PySide6 solution. Binary size ~5MB.

**Prerequisites**
- [Rust](https://www.rust-lang.org/) (>= 1.70)
- [Node.js](https://nodejs.org/) (>= 18)
- Windows: WebView2 included with OS; Linux: install `libwebkit2gtk-4.1-dev`

**Development mode**
```bash
cd tools/profile-gui
npm install
npm run tauri dev
```

**Package as standalone executable**
```bash
cd tools/profile-gui
npm run tauri build
```

**Usage**
- Edit ESTA instance parameters (position, channels, rulers, theme) in the GUI.
- Click "Generate ESTA_Profile.c" to render and write `core/ESTA_Profile.c` from the JSON source.
- Click "Build & Run" to save, compile, and launch the simulator.

**Data flow**
- Source of truth: `core/ESTA_Profile.json`
- Generated output: `core/ESTA_Profile.c`
- Template engine: Tera (Jinja2-compatible syntax)

---

### 5. Porting Guide
1. Keep `core/WAVE.c`, `core/WAVE.h`, `core/ESTA_Profile.c`, `core/ESTA_Profile.h` in your target project.
2. In `ESTA.h`, adapt 4 drawing macros: `SCREEN_DRAW_LINE`, `SCREEN_DRAW_RECTANGLE`, `SCREEN_FILL`, `SCREEN_DRAW_NUM`.
3. Call `ESTA_Profile_GetDefault()` to read configuration, use `ESTA_Profile_Apply()` to initialize instances.
4. In the main loop, call `WAVE_CurveDraw()` to push sample data, use `WAVE_ReDraw()` as needed.

**Minimal `main.c` example**
```c
#include "ESTA.h"
#include "ESTA_Profile.h"

int main(void) {
    Platform_Init();

    const ESTA_ProfileSet_TypeDef *profiles = ESTA_Profile_GetDefault();
    if (profiles == NULL) return -1;

    for (int i = 0; i < profiles->inst_count; ++i) {
        if (ESTA_Profile_Apply(WAVE_INST(i), &profiles->profiles[i]) != ESTA_OK) return -1;
        if (WAVE_ReDraw(WAVE_INST(i)) != ESTA_OK) return -1;
    }

    while (1) {
        uint16_t ch_data[MAX_WAVE_CHANNEL] = {0};
        AcquireSignal(ch_data);
        WAVE_CurveDraw(WAVE_INST(0), ch_data);
    }
}
```

---

### 6. Project Structure
```
ESTA-GUI-Guider/
├── core/                   # Waveform display library core
│   ├── ESTA.h / ESTA.c       # Core rendering engine
│   ├── ESTA_Profile.h/.c    # Instance configuration (auto-generated)
│   └── ESTA_Profile.json    # Configuration source of truth
├── port/                   # Platform abstraction layer
│   └── esta_port_sdl2.c     # SDL2 backend
├── simulator/              # PC simulator entry
│   ├── main.c
│   └── sim_scenario.c
├── tools/
│   └── profile-gui/        # Tauri-based profile configurator
│       ├── src/            # React TypeScript frontend
│       ├── src-tauri/      # Rust backend (Tera templates + commands)
│       └── package.json
├── CMakeLists.txt          # C project build
├── build.ps1               # Windows MinGW build script
└── README.md
```