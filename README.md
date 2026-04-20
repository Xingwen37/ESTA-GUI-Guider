# TL-OSC 项目模拟器

[English Version](#english-version) | [中文版本](#chinese-version)

---

<h2 id="chinese-version">中文版本</h2>

TL-OSC 是一个基于 OOP-in-C (OOC) 架构的轻量级嵌入式示波器 GUI 库，配套提供基于 SDL2 的跨平台 PC 模拟器，旨在实现“PC端验证，零修改移植”。

### 1. 核心特性
* **OOP-in-C 架构**：支持多实例、多通道独立运行，数据与配置严格隔离。
* **极简移植**：显示接口高度解耦，仅需实现 4 个基础画图宏即可接入任意屏幕硬件。
* **免烧录验证**：内建 PC 模拟器（SDL2），支持在 Linux/WSL 环境下进行 UI 逻辑测试。
* **像素级还原**：复用单片机点阵字库，模拟器显示效果与真实硬件 100% 一致。

### 2. 核心 API
| API | 功能说明 |
| :--- | :--- |
| `OSC_Init()` | 初始化示波器实例并载入配置。 |
| `OSC_DeInit()` | 复位示波器实例，清空数据。 |
| `OSC_CurveDraw()` | 压入通道数据并触发波形绘制。 |
| `OSC_ReDraw()` | 强制重绘边框、背景与坐标轴。 |
| `OSC_GetThemeColor()` | 获取当前主题下指定组件的颜色值 (RGB565)。 |

### 3. 模拟器编译与运行 (Linux/WSL)
**安装依赖**
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev
```

**编译执行**
```bash
cmake -B build
cmake --build build
./build/OSC_Simulator
```

**清理工程**
```bash
rm -rf build/
```

---

<h2 id="english-version">English Version</h2>

TL-OSC is a lightweight embedded oscilloscope GUI library built on an OOP-in-C architecture. It includes an SDL2-based cross-platform PC simulator, enabling UI verification on PC before seamless porting to hardware.

### 1. Core Features
* **OOP-in-C** : Supports multi-instance and multi-channel operations with isolated data/configuration.
* **Minimal Porting** : Highly decoupled display layer; requires only 4 basic drawing macros to port.
* **No-Flash Verification** : Built-in PC simulator (SDL2) for UI logic testing on Linux/WSL.
* **Pixel-Perfect** : Reuses MCU dot-matrix fonts to accurately replicate hardware visuals.

### 2. Core APIs
| API | Description |
| :--- | :--- |
| `OSC_Init()` | Initialize an oscilloscope instance with configuration. |
| `OSC_DeInit()` | Reset instance and clear data. |
| `OSC_CurveDraw()` | Push channel data and trigger waveform drawing. |
| `OSC_ReDraw()` | Force redraw of frames, backgrounds, and coordinate rulers. |
| `OSC_GetThemeColor()` | Get RGB565 color value for specific UI components. |

### 3. Build & Run Simulator (Linux/WSL)
**Dependencies**
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev
```

**Build and Run**
```bash
cmake -B build
cmake --build build
./build/OSC_Simulator
```

**Clean**
```bash
rm -rf build/
```