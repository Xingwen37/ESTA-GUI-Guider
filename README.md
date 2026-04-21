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

### 4. 图形化配置（`tools/profile_gui`）
**安装依赖**
```bash
sudo apt install python3.12-venv
cd tools/profile_gui
python -m venv .venv
source .venv/bin/activate
pip install pyside6 jinja2
pip install -r requirements.txt
```

**启动工具**
```bash
cd tools/profile_gui
source .venv/bin/activate
python app.py
```

**使用方式**
- 在界面中编辑实例参数（坐标、通道、标尺、主题）。
- 点击 `生成 OSC_Profile.c`：写入 `core/OSC_Profile.c`。
- 点击 `Run Simulator`：自动生成配置、构建并启动 `OSC_Simulator`。

**生成结果**
- 输出文件：`core/OSC_Profile.c`
- 备份文件：`core/OSC_Profile.c.bak`

### 5. 移植方式
**步骤**
1. 在目标工程中保留 `core/OSC.c`、`core/OSC.h`、`core/OSC_Profile.c`、`core/OSC_Profile.h`。  
2. 在 `OSC.h` 中适配 4 个底层绘图宏：`SCREEN_DRAW_LINE`、`SCREEN_DRAW_RECTANGLE`、`SCREEN_FILL`、`SCREEN_DRAW_NUM`。  
3. 调用 `OSC_Profile_GetDefault()` 读取配置，使用 `OSC_Profile_Apply()` 初始化实例。  
4. 主循环中持续调用 `OSC_CurveDraw()` 输入采样数据，并按需调用 `OSC_ReDraw()`。  

**最小 `main.c` 示例（可裁剪）**
```c
#include "OSC.h"
#include "OSC_Profile.h"

int main(void) {
    /* 平台初始化：时钟/屏幕/端口 */
    Platform_Init();

    const OSC_ProfileSet_TypeDef *profiles = OSC_Profile_GetDefault();
    if (profiles == NULL) return -1;

    for (int i = 0; i < profiles->osc_count; ++i) {
        if (OSC_Profile_Apply(OSC_INST(i), &profiles->profiles[i]) != OSC_OK) return -1;
        if (OSC_ReDraw(OSC_INST(i)) != OSC_OK) return -1;
    }

    while (1) {
        uint16_t ch_data[MAX_OSC_CHANNEL] = {0};
        AcquireSignal(ch_data);                 /* 用户实现：采样或读取缓存 */
        OSC_CurveDraw(OSC_INST(0), ch_data);   /* 可扩展到多实例 */
    }
}
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