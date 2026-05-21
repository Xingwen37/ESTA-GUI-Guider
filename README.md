# TL-ESTA · v0.1.0-beta

![version](https://img.shields.io/badge/version-v0.1.0--beta-orange)
![platform](https://img.shields.io/badge/platform-MCU%20%7C%20PC%20Simulator-blue)
![license](https://img.shields.io/badge/license-MIT-green)

[中文版本](README.zh.md) | English Version

---

TL-ESTA is a lightweight embedded GUI library for MCUs, built on an OOP-in-C architecture. It supports waveform, bar chart, table, and menu components with a built-in event-driven system for button/encoder/timer-driven UI interactions. A SDL2-based PC simulator and a Tauri v2 visual configurator are included for PC-side verification before hardware porting.

> ⚠️ **Beta Notice**: This is `v0.1.0-beta`. APIs and configuration formats may change before the stable release.

### 1. Core Features

- **OOP-in-C Multi-Instance**: Each component supports multiple independent instances with isolated data and configuration, accessed via register-style macros.
- **Event-Driven UI**: Buttons, encoders, soft timers, and Flags can trigger component redraws, page switches, menu navigation, and more — with Guard conditions and Action sequences.
- **Multiple Components**: WAVE (waveform), BARCHART (bar chart), TABLE (data table), MENU (tree menu).
- **Multi-Page System**: Components are grouped by page with runtime page switching.
- **Visual Configurator** (`tools/profile-gui`): Tauri v2 + React desktop app with WYSIWYG layout and event binding editing, one-click Build & Run, and screenshot preview.
- **Pixel-Perfect Simulation**: PC simulator reuses MCU dot-matrix fonts — display output is 100% identical to real hardware.

### 2. Quick Start (PC Simulator)

**Linux / WSL**

```bash
sudo apt install build-essential cmake libsdl2-dev
cmake -B build && cmake --build build
./build/ESTA_Simulator
```

**Windows (MinGW64)**

Prerequisites: [MinGW-w64](https://www.mingw-w64.org/) with SDL2 development libraries.

```powershell
.\build.ps1                              # Configure + build
.\build\ESTA_Simulator.exe               # Run simulator
.\build\ESTA_Simulator.exe --screenshot build\preview  # Screenshot mode
```

### 3. Profile Configurator (`tools/profile-gui`)

A **Tauri v2 + React + Rust** desktop configurator, ~5 MB binary.

**Prerequisites**
- [Rust](https://www.rust-lang.org/) >= 1.70
- [Node.js](https://nodejs.org/) >= 18
- Windows: WebView2 included with OS; Linux: install `libwebkit2gtk-4.1-dev`

**Development mode**
```bash
cd tools/profile-gui
npm install
npm run tauri dev
```

**Package as standalone executable**
```bash
npm run tauri build
```

**Feature Overview**

| Panel | Function |
|---|---|
| WAVE / BARCHART / TABLE / MENU | Visual editing of layout, parameters, and theme |
| Event Bindings | Bind trigger sources to Actions with optional Guard conditions |
| Action Sequences | Multi-step Action sequences for a single trigger |
| String Table | Predefined text resource pool |
| Soft Timers | Software timer configuration (up to 4) |
| Flag Config | Flag state bit configuration (up to 8) |

**Data Flow**
```
ESTA_Profile.json  →  Rust serde + Tera template  →  core/profile/ESTA_Profile.c
```

### 4. Porting Guide

1. Include the entire `core/` directory in your target project and add it to the include path.
2. Implement 7 drawing macros in your port layer (`SCREEN_DRAW_LINE`, `SCREEN_DRAW_RECTANGLE`, `SCREEN_FILL`, `SCREEN_DRAW_NUM`, `SCREEN_DRAW_STRING`, `SCREEN_DRAW_NUM_FONT`, `SCREEN_DRAW_STRING_FONT`), mapping them to your LCD driver.
3. Call `App_MainInit(&app, LCD_WIDTH, LCD_HEIGHT)` to load the Profile and initialize all components.
4. In the main loop, call `App_MainTick(&app)` and `ESTA_SoftTimerTick(delta_ms)`.

See `docs/INTEGRATION_SPEC.md` for the full porting reference.

### 5. Project Structure

```
ESTA-GUI-Guider/
├── core/
│   ├── ui/          # Visual components (WAVE, BARCHART, TABLE, MENU)
│   ├── infra/       # Infrastructure (helper, font, ui_base, ui_theme)
│   ├── profile/     # Profile system (ESTA_Profile.h/.c/.json)
│   ├── event/       # Event queue, Flags, soft timers
│   └── app/         # Portable app skeleton (app_main, app_page, app_event, app_action)
├── port/            # Platform abstraction layer (SDL2 backend)
├── simulator/       # PC simulator (main.c, sim_scenario, sim_feed, sim_input)
├── tools/
│   └── profile-gui/ # Tauri configurator (src/ React frontend + src-tauri/ Rust backend)
├── docs/            # Spec documents (COMPONENT_SPEC, EVENT_SYSTEM_SPEC, etc.)
├── CMakeLists.txt
└── build.ps1        # Windows MinGW one-click build script
```
