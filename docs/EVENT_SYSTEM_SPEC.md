# 事件绑定系统规格（EVENT_SYSTEM_SPEC）

本文档描述 ESTA 的数据驱动事件绑定系统。该系统允许用户通过 Profile 配置（而非硬编码）来定义"什么输入触发什么动作"。

## 架构概览

```
┌─────────────────────────────────────────────────────────────────┐
│  事件源                                                          │
│  ├─ 外部输入：按钮按下/松开、编码器旋转                            │
│  ├─ 内部事件：MENU 选中某项、定时器到期                            │
│  └─ Flag 信号：ESTA_FlagSignal() 置位状态位并推送 FLAG 事件         │
│                         │                                        │
│                         ▼                                        │
│  ┌─────────────────────────────────┐                             │
│  │  事件队列 (core/event/event.h)   │  容量 16，FIFO             │
│  │  ESTA_EventPush / ESTA_EventPoll │                             │
│  └─────────────────────────────────┘                             │
│                         │                                        │
│                         ▼                                        │
│  ┌─────────────────────────────────┐                             │
│  │  订阅分发 (core/app/app_event.c) │                             │
│  │  App_DispatchEvents()            │                             │
│  │  匹配: type + source_id + event_id                            │
│  │  优先级: 精确匹配 > wildcard                                   │
│  └─────────────────────────────────┘                             │
│                         │                                        │
│                         ▼                                        │
│  ┌─────────────────────────────────┐                             │
│  │  Action Handler (app_action.c)   │                             │
│  │  通过 App_BindingContext 获取     │                             │
│  │  target_type + target_inst       │                             │
│  └─────────────────────────────────┘                             │
└─────────────────────────────────────────────────────────────────┘
```

## 核心数据结构

### ESTA_Event（事件队列中的事件）

```c
// core/event/event.h
typedef struct {
    uint8_t  type;           // ESTA_EventType 枚举值
    uint8_t  source;         // 物理源：按钮索引、编码器索引、组件实例
    uint16_t id;             // 语义 ID：菜单项 event_id、timer_id 等
    uint32_t timestamp;      // 系统 tick (ms)
    uint8_t  flag_snapshot;  // Flag 状态快照（入队时由 ESTA_EventPush 自动填入）
} ESTA_Event;
```

### ESTA_EventType（事件类型枚举）

```c
// core/event/event.h
typedef enum {
    ESTA_EVENT_NONE           = 0,
    ESTA_EVENT_BUTTON_PRESS   = 1,
    ESTA_EVENT_BUTTON_RELEASE = 2,
    ESTA_EVENT_MENU_SELECT    = 3,
    ESTA_EVENT_ENCODER_ROTATE = 4,
    ESTA_EVENT_TIMER          = 5,
    ESTA_EVENT_FLAG           = 6,
    ESTA_EVENT_CUSTOM         = 0xFF
} ESTA_EventType;
```

### ESTA_EventBinding_TypeDef（绑定配置）

```c
// core/app/app_action.h
typedef struct {
    uint8_t  trigger;      // ESTA_EventType 值
    uint8_t  source_id;    // 源索引，0xFF = 匹配所有
    uint16_t trigger_id;   // 事件 ID 过滤，0xFFFF = 匹配所有
    uint8_t  target_type;  // ESTA_TargetType
    uint8_t  target_inst;  // 目标实例索引
    uint8_t  action;       // ESTA_ActionType
    uint8_t  param;        // action 参数（TEXT_SET: 字符串索引；SEQUENCE: 序列索引；CUSTOM: custom_id）
} ESTA_EventBinding_TypeDef;
```

## 绑定字段语义

| 字段 | 含义 | 特殊值 |
|------|------|--------|
| `trigger` | 触发事件类型 | — |
| `source_id` | 事件源（按钮 ID / MENU 实例） | `0xFF` = 匹配任意源 |
| `trigger_id` | 事件语义 ID（MENU 项的 event_id） | `0xFFFF` = 匹配任意 ID |
| `target_type` | 动作目标组件类型 | — |
| `target_inst` | 目标组件实例索引 | PAGE/GLOBAL 时为 0；SEQUENCE/CUSTOM 时无意义（填 0） |
| `action` | 执行的动作 | — |
| `param` | action 参数 | TEXT_SET: 字符串表索引；SEQUENCE: 序列索引；CUSTOM: custom_id (0~7)；其他: 0 |

### 各事件类型的字段映射

| trigger | source_id 含义 | trigger_id 含义 |
|---------|---------------|----------------|
| BUTTON_PRESS (1) | 按钮编号 (0~N) | 无意义（填 0xFFFF） |
| BUTTON_RELEASE (2) | 按钮编号 (0~N) | 无意义（填 0xFFFF） |
| MENU_SELECT (3) | MENU 实例编号 (0~3) | 菜单项的 event_id |
| ENCODER_ROTATE (4) | 编码器编号 | 方向/步数（预留） |
| TIMER (5) | — | timer_id（预留） |
| FLAG (6) | Flag ID (0~7) | 无意义（填 0xFFFF） |

## 目标类型与动作

### ESTA_TargetType

```c
typedef enum {
    ESTA_TARGET_WAVE = 0,
    ESTA_TARGET_BARCHART,
    ESTA_TARGET_TABLE,
    ESTA_TARGET_MENU,
    ESTA_TARGET_PAGE,
    ESTA_TARGET_GLOBAL,
    ESTA_TARGET_FLAG,
} ESTA_TargetType;
```

### ESTA_ActionType

```c
typedef enum {
    ESTA_ACTION_NONE          = 0,
    ESTA_ACTION_PAGE_NEXT     = 1,
    ESTA_ACTION_PAGE_PREV     = 2,
    ESTA_ACTION_THEME_TOGGLE  = 3,
    ESTA_ACTION_WAVE_REDRAW   = 4,   // 即时：通过回调直接绘制
    ESTA_ACTION_MENU_UP       = 5,
    ESTA_ACTION_MENU_DOWN     = 6,
    ESTA_ACTION_MENU_ENTER    = 7,
    ESTA_ACTION_MENU_BACK     = 8,
    ESTA_ACTION_FLAG_SET      = 9,   // 仅置位状态位，不推送事件（与 FLAG_CLEAR 对称）
    ESTA_ACTION_TEXT_SET      = 10,  // 参数化：写入字符串表中的预定义文本
    ESTA_ACTION_SEQUENCE      = 11,  // 序列：触发一组子动作
    ESTA_ACTION_FLAG_CLEAR    = 12,  // 清除 Flag 状态位
    ESTA_ACTION_FLAG_SIGNAL         = 13,  // 置位状态位 + 推送 FLAG 事件
    ESTA_ACTION_BARCHART_REDRAW     = 14,  // 即时：通过回调直接绘制柱形图
    ESTA_ACTION_CUSTOM              = 0xFF // 自定义：由用户注册的 handler
} ESTA_ActionType;
```

### 合法组合约束表（GUI 侧过滤）

| target_type | 可用 action |
|-------------|------------|
| WAVE (0) | THEME_TOGGLE, WAVE_REDRAW, SEQUENCE, CUSTOM |
| BARCHART (1) | THEME_TOGGLE, BARCHART_REDRAW, SEQUENCE, CUSTOM |
| TABLE (2) | TEXT_SET, SEQUENCE, CUSTOM |
| MENU (3) | MENU_UP, MENU_DOWN, MENU_ENTER, MENU_BACK, TEXT_SET, SEQUENCE, CUSTOM |
| PAGE (4) | PAGE_NEXT, PAGE_PREV, SEQUENCE, CUSTOM |
| GLOBAL (5) | THEME_TOGGLE, SEQUENCE, CUSTOM |
| FLAG (6) | FLAG_SET, FLAG_CLEAR, FLAG_SIGNAL, SEQUENCE, CUSTOM |

> SEQUENCE 和 CUSTOM 对所有 target_type 均可用，因为实际目标由序列步骤或自定义 handler 内部决定。

## 分发机制

`App_DispatchEvents()`（在 `App_MainTick()` 中每帧调用）从队列中取出事件，两轮匹配：

1. **Pass 1（精确匹配）**：`source_id != ANY` 的订阅，按 `type + source_id + event_id` 匹配。匹配成功且 handler 返回 true 则消费事件。
2. **Pass 2（wildcard）**：`source_id == ANY` 的订阅，仅匹配 `type + event_id`。作为兜底处理。

两轮匹配均在执行 handler 前检查 guard 条件（见下文 Guard 机制）。

### Guard 机制

每个 binding 可携带三个 guard 掩码，用于在 dispatch 时过滤不满足条件的事件：

| 字段 | 语义 |
|------|------|
| `guard_and_mask` | 所有指定 Flag 必须置位（AND），0 = 不检查 |
| `guard_or_mask` | 至少一个指定 Flag 必须置位（OR），0 = 不检查 |
| `guard_inv_mask` | 所有指定 Flag 必须清零（AND-NOT），0 = 不检查 |

**快照语义**：guard 检查使用 `evt.flag_snapshot`（事件入队时的 Flag 状态快照），而非 dispatch 时的实时状态。快照由 `ESTA_EventPush` 在写入队列时自动填入，确保同一帧内多个事件的 guard 结果由各自入队时刻的 Flag 状态决定，与 dispatch 处理顺序无关。

> **guard 状态存储推荐使用 MANUAL 模式 Flag**：MANUAL Flag 持久保持置位状态，能跨越多帧被事件快照捕获到。

> 注意：`ESTA_FlagSet(flag_id)` 先置位 `s_flags[flag_id]`，再调用 `ESTA_EventPush`，因此快照会包含刚被 set 的 Flag——这是正确的语义（"此事件产生时 Flag 的状态"）。

```c
int App_Subscribe(ESTA_EventType type, uint8_t source_id, uint16_t event_id,
                  ESTA_EventHandler handler, void *user_data);
```

## Handler 上下文

Handler 签名不变：`bool (*)(const ESTA_Event *event, void *user_data)`

`user_data` 指向 `App_BindingContext`，handler 从中获取目标信息：

```c
typedef struct {
    void *app;            // App_MainState 指针
    uint8_t target_type;  // ESTA_TargetType
    uint8_t target_inst;  // 目标实例索引
    uint8_t param;        // action 参数（TEXT_SET: 字符串表索引；SEQUENCE: 序列索引；CUSTOM: custom_id）
    uint8_t guard_and_mask; // AND guard：所有指定 Flag 必须置位
    uint8_t guard_or_mask;  // OR guard：至少一个指定 Flag 必须置位
    uint8_t guard_inv_mask; // AND-NOT guard：所有指定 Flag 必须清零
} App_BindingContext;
```

Handler 示例：
```c
static bool action_theme_toggle(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    int target = ctx->target_inst;
    // ... 对 WAVE[target] 切换主题
}
```

## Profile 中的绑定配置

### JSON 格式

```json
{
  "binding_count": 2,
  "bindings": [
    { "trigger": 1, "source_id": 1, "trigger_id": 65535, "target_type": 3, "target_inst": 0, "action": 5, "param": 0 },
    { "trigger": 1, "source_id": 0, "trigger_id": 65535, "target_type": 2, "target_inst": 0, "action": 10, "param": 0 }
  ],
  "string_count": 1,
  "strings": [
    { "sub_addr": 1, "text": "Running" }
  ],
  "sequence_count": 0,
  "sequences": []
}
```

### C 生成代码

```c
.bindings = {
    { .trigger = 1, .source_id = 1, .trigger_id = 65535, .target_type = 3, .target_inst = 0, .action = 5, .param = 0 },
    { .trigger = 1, .source_id = 0, .trigger_id = 65535, .target_type = 2, .target_inst = 0, .action = 10, .param = 0 }
},
.string_count = 1,
.strings = {
    { .sub_addr = 1, .text = "Running" }
},
.sequence_count = 0,
.sequences = {
}
```

### 初始化流程

`App_MainInit()` → `ESTA_Profile_ApplyEvents(profiles, state)`：
1. 遍历 `flag_configs[]` 数组，为每个 flag 调用 `ESTA_FlagRegister(i, &config)`
2. 遍历 `bindings[]` 数组
3. 为每条 binding 创建 `App_BindingContext`（静态数组 `g_binding_ctx[]`）
4. 调用 `App_Subscribe(trigger, source_id, trigger_id, handler, &ctx)`

## 仿真器键盘映射

键盘数字键 N 直接映射到 Button N（`sim_input.c`）：

| 键盘 | Button | 默认绑定（可通过 Profile 修改） |
|------|--------|-------------------------------|
| 0 | Button 0 | （未绑定） |
| 1 | Button 1 | MENU #0 UP |
| 2 | Button 2 | MENU #0 DOWN |
| 3 | Button 3 | MENU #0 ENTER |
| 4 | Button 4 | MENU #0 BACK |
| 5~9 | Button 5~9 | （未绑定） |

## 回调驱动的即时 Redraw 模式

WAVE_REDRAW 和 BARCHART_REDRAW 均为即时 action——handler 通过回调函数直接执行绘制，而非设置标志位等待轮询。两者遵循相同的架构模式。

数据流模型：
```
SimFeed 每帧采样 → 写入数据缓冲区（模拟 ADC / 传感器连续采集）
                    ↓ 不绘制
WAVE_REDRAW / BARCHART_REDRAW handler → 调用 redraw_fn(inst, ctx) → 读取缓冲区并绘制一帧
```

> `BARCHART_UpdateAll` 现为纯数据写入（不触发绘制）；`BARCHART_ReDraw` 通过 `bar_redraw_fn` 回调由事件驱动调用。

`App_MainState` 中的回调字段：
```c
typedef void (*App_WaveRedrawFn)(int inst, void *ctx);
typedef void (*App_BarRedrawFn)(int inst, void *ctx);

typedef struct {
    App_PageState page_state;
    int wave_inst_count, bar_inst_count, table_inst_count, menu_inst_count;
    App_WaveRedrawFn wave_redraw_fn;  // 平台注册的波形绘制回调
    void *wave_redraw_ctx;
    App_BarRedrawFn  bar_redraw_fn;   // 平台注册的柱形图绘制回调
    void *bar_redraw_ctx;
} App_MainState;
```

Handler 实现（以 BARCHART_REDRAW 为例）：
```c
static bool action_barchart_redraw(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL || s->bar_redraw_fn == NULL) return false;
    s->bar_redraw_fn(ctx->target_inst, s->bar_redraw_ctx);
    return true;
}
```

仿真器注册：
```c
g_app.wave_redraw_fn = (App_WaveRedrawFn)SimFeed_RedrawWaveInst;
g_app.wave_redraw_ctx = NULL;
g_app.bar_redraw_fn  = (App_BarRedrawFn)SimFeed_RedrawBarInst;
g_app.bar_redraw_ctx = NULL;
```

MCU 移植时，用户实现自己的绘制回调（从数据缓冲区读取并调用对应组件的 ReDraw / CurveDrawBatch）。

## Flag 系统（`core/event/event_flag.h/.c`）

Flag 是一种轻量级的内部信号机制，用于跨模块通信和 guard 状态存储。支持两种模式。

### 模式

| 模式 | 值 | 典型用途 |
|------|:---:|---------|
| `ESTA_FLAG_MODE_DISABLED` | 0 | 未配置的槽位（UI 中不显示） |
| `ESTA_FLAG_MODE_MANUAL` | 1 | 状态位：持久保存，用于 guard 门控；`ESTA_FlagSignal()` 置位时同时推送 FLAG 事件供 binding 订阅 |

### Flag 的双重角色

MANUAL Flag 同时扮演两个角色：

**状态位（guard 门控）**：Flag 被置位后持续保持为 1，直到 `ESTA_FlagClear()` 或 FLAG_CLEAR action 显式清除。状态稳定，能跨越多帧被 TIMER 事件的 `flag_snapshot` 捕获到，适合作为 guard 门控条件。

**事件触发源**：`ESTA_FlagSignal()` 在置位状态位的同时，向事件队列推送一条 `ESTA_EVENT_FLAG` 事件（`source = flag_id`），binding 可直接订阅该事件。

```
按键1按下 → FLAG_SET(0) → FlagSignal(0) → s_flags[0]=1（持续保持）+ 推送 FLAG(source=0)
每帧 SoftTimer 推送 TIMER 事件 → flag_snapshot=0b01 → guard_and_mask=1 通过 → WAVE_REDRAW 执行
按键3按下 → FLAG_CLEAR(0) → s_flags[0]=0 → 下一帧 guard 失败 → 停止刷新
```

### FlagConfig 结构体

```c
typedef struct {
    ESTA_FlagMode mode;  // DISABLED(0) / MANUAL(1)
} ESTA_FlagConfig;
```

### API

```c
void ESTA_FlagInit(void);
void ESTA_FlagRegister(uint8_t flag_id, const ESTA_FlagConfig *config);
void ESTA_FlagSignal(uint8_t flag_id);  // 置位状态位 + 推送 FLAG 事件（FLAG_SET action 调用此函数）
void ESTA_FlagSet(uint8_t flag_id);     // 仅置位状态位，不推送事件（纯状态写入）
bool ESTA_FlagCheck(uint8_t flag_id);   // 检查并清除（consume）
bool ESTA_FlagPeek(uint8_t flag_id);    // 仅查看，不清除
void ESTA_FlagClear(uint8_t flag_id);   // 主动清除 flag（FLAG_CLEAR action 调用此函数）
uint8_t ESTA_FlagReadAll(void);         // 读取所有 flag 状态位（供 flag_snapshot 使用）
```

`ESTA_FlagSet` 与 `ESTA_FlagClear` 是对称的纯状态操作，不产生事件。`ESTA_FlagSignal` 是"通知系统"操作，置位状态位并推送事件。

```json
{ "trigger": 6, "source_id": 2, "trigger_id": 65535, "target_type": 4, "target_inst": 0, "action": 1 }
```
含义：Flag #2 被设置时 → 切换到下一页。

### FLAG_SET action

FLAG_SET 是纯状态写入——仅置位状态位，不推送事件，与 FLAG_CLEAR 对称：

```c
static bool action_flag_set(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    ESTA_FlagSet(ctx->target_inst);  // target_inst = flag_id
    return true;
}
```

FLAG_SIGNAL 置位状态位并推送 FLAG 事件，是最常用的 Flag 触发方式：

```c
static bool action_flag_signal(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    ESTA_FlagSignal(ctx->target_inst);  // target_inst = flag_id
    return true;
}
```

### Action 语义分类

| 类别 | Action | 行为 |
|------|--------|------|
| 即时 | PAGE_NEXT/PREV, THEME_TOGGLE, WAVE_REDRAW, BARCHART_REDRAW, MENU_* | handler 直接执行效果 |
| 状态写入 | FLAG_SET, FLAG_CLEAR | 纯状态位操作，不产生事件 |
| 通知 | FLAG_SIGNAL | 置位状态位 + 推送 FLAG 事件 |
| 参数化 | TEXT_SET | handler 从字符串表取数据，写入目标组件 |
| 序列 | SEQUENCE | handler 依次执行 Profile 中定义的子动作步骤 |
| 自定义 | CUSTOM | handler 分发到用户注册的函数（运行时注册） |

### 容量

`ESTA_FLAG_MAX = 8`，Flag ID 范围 0~7。

## SoftTimer 系统（`core/event/soft_timer.h/.c`）

SoftTimer 是独立的周期事件驱动模块，每个 timer 按配置的 `period_ms` 周期向事件队列推送事件。最多支持 4 个 timer（`ESTA_SOFT_TIMER_MAX`）。

### 数据结构

```c
#define ESTA_SOFT_TIMER_MAX 4

typedef struct {
    uint16_t period_ms;    // 触发周期（ms），0 = 禁用
    uint8_t  event_type;   // 到期时推送的事件类型（Emit Type）
    uint8_t  event_source; // 推送事件的 source 字段（Emit Source）
    uint16_t event_id;     // 推送事件的 id 字段（Emit ID）
} ESTA_SoftTimerConfig;
```

`event_type/event_source/event_id` 是**发出配置**，描述"定时器到期时向队列推送什么事件"，而非过滤条件。SoftTimer 可以推送任意类型的事件，不限于 TIMER 类型——这使得它能直接驱动任何已有的 binding，无需为"定时触发"单独写一套 binding。

例如配置 `event_type=BUTTON_PRESS, event_source=0`，即可每隔 `period_ms` 模拟一次 Button 0 按下，触发所有订阅该按钮的 binding（自动演示、屏保、自动翻页等场景）。

### API

```c
void ESTA_SoftTimerInit(void);
void ESTA_SoftTimerRegister(uint8_t timer_id, const ESTA_SoftTimerConfig *config);
void ESTA_SoftTimerTick(uint16_t delta_ms);
```

`ESTA_SoftTimerTick(delta_ms)` 在仿真器主循环每帧调用（`delta_ms = SIM_TARGET_FRAME_MS`）。MCU 移植时在主循环中以实际帧间隔调用。

### Profile JSON 格式

```json
{
  "timer_count": 1,
  "timer_configs": [
    { "period_ms": 20, "event_type": 5, "event_source": 0, "event_id": 0 }
  ]
}
```

### 典型用法：周期刷新波形

配置一个 period_ms=20 的 TIMER 事件 timer，再配置 binding：`TIMER(source=0) → WAVE_REDRAW(inst=0)`，即可实现 50Hz 波形持续刷新。

## 字符串表系统

字符串表是 Profile 中的静态数据池，供 TEXT_SET action 引用。每个条目仅存储子地址和文本，目标组件信息由 binding（或 sequence 步骤）的 `target_type` / `target_inst` 决定，避免冗余并支持同一条目被不同 target 的 binding 复用。

### 数据结构

```c
#define ESTA_MAX_STRING_ENTRIES 16
#define ESTA_STRING_MAX_LEN     16

typedef struct {
    uint8_t  sub_addr;      // 组件内子地址（TABLE: row*MAX_COLS+col, MENU: item_idx）
    char     text[ESTA_STRING_MAX_LEN + 1];
} ESTA_StringEntry_TypeDef;
```

### 绑定结构体

```c
typedef struct {
    uint8_t  trigger;
    uint8_t  source_id;
    uint16_t trigger_id;
    uint8_t  target_type;   // 决定 sub_addr 的解释方式
    uint8_t  target_inst;   // 决定写入哪个组件实例
    uint8_t  action;        // = ESTA_ACTION_TEXT_SET (10)
    uint8_t  param;         // 字符串表索引
} ESTA_EventBinding_TypeDef;
```

当 `action == TEXT_SET` 时，handler 从 `profiles->strings[param]` 取出条目，结合 `ctx->target_type` 和 `ctx->target_inst` 分发到对应组件的更新 API。

### Handler 实现

```c
static bool action_text_set(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL) return false;

    uint8_t str_idx = ctx->param;
    const ESTA_ProfileSet_TypeDef *p = s->page_state.profiles;
    if (str_idx >= p->string_count) return false;
    const ESTA_StringEntry_TypeDef *entry = &p->strings[str_idx];

    switch (ctx->target_type) {          // 从 binding context 取，不从 entry 取
        case ESTA_TARGET_TABLE: {
            uint8_t row = entry->sub_addr / TABLE_MAX_COLS;
            uint8_t col = entry->sub_addr % TABLE_MAX_COLS;
            TABLE_UpdateText(ctx->target_inst, row, col, entry->text);
            TABLE_ReDraw(ctx->target_inst);
            return true;
        }
        case ESTA_TARGET_MENU: {
            MENU_UpdateItemLabel(ctx->target_inst, entry->sub_addr, entry->text);
            MENU_ReDraw(ctx->target_inst);
            return true;
        }
        default: return false;
    }
}
```

### sub_addr 编码

| target_type | sub_addr 含义 | 计算方式 |
|-------------|--------------|---------|
| WAVE (0) | 轴选择 | `0` = Y 轴单位字符串, `1` = X 轴单位字符串 |
| TABLE (2) | 单元格位置 | `row * TABLE_MAX_COLS + col` |
| MENU (3) | 菜单项索引 | `item_idx`（0~31） |

## 全链路文件清单

| 层 | 文件 | 职责 |
|----|------|------|
| C 事件队列 | `core/event/event.h/.c` | ESTA_Event 结构体（含 flag_snapshot）、队列、Emit 函数 |
| C Flag 系统 | `core/event/event_flag.h/.c` | Flag 设置/检查/清除，FlagSignal 推送 FLAG 事件，FlagReadAll 供快照使用 |
| C SoftTimer | `core/event/soft_timer.h/.c` | 周期事件驱动（period_ms 配置） |
| C 订阅分发 | `core/app/app_event.h/.c` | App_Subscribe、App_DispatchEvents（guard 用 flag_snapshot） |
| C Action 注册 | `core/app/app_action.h/.c` | 枚举、handler 实现、g_action_table、自定义 action 注册 |
| C 应用骨架 | `core/app/app_main.h/.c` | App_MainState（含 wave_redraw_fn / bar_redraw_fn 回调） |
| C Profile 绑定 | `core/profile/ESTA_Profile.h/.c` | ApplyEvents、g_binding_ctx、StringEntry、ActionSequence、FlagConfig、SoftTimerConfig |
| C MENU 导航 | `core/ui/MENU.h/.c` | MENU_NavUp/Down/Enter/Back、MENU_UpdateItemLabel |
| JSON 数据 | `core/profile/ESTA_Profile.json` | bindings + strings + sequences + flag_profiles + timer_configs 数组 |
| Rust 插件 | `src-tauri/src/plugins/flag.rs` | FlagConfig 模型（仅 mode 字段）+ FlagPlugin（默认值 + 模板上下文） |
| Rust 插件 | `src-tauri/src/plugins/soft_timer.rs` | SoftTimerConfig 模型 + SoftTimerPlugin（timer_count/timer_configs） |
| Rust 命令 | `src-tauri/src/commands/profile_io.rs` | 模板数据构建（注册表驱动，无需修改） |
| Tera 模板 | `src-tauri/templates/ESTA_Profile.c.j2` | C 代码生成（含 ApplyEvents 函数体 + FlagRegister + SoftTimerRegister 循环） |
| TS 类型 | `src/lib/types.ts` | EventBinding（含 guard 字段）、FlagConfig、SoftTimerConfig 接口、常量、约束表 |
| React UI | `src/components/EventEditor.tsx` | 事件绑定编辑器（含 Guard 展开面板） |
| React UI | `src/components/StringTableEditor.tsx` | 字符串表编辑器（sub_addr + text，含 WAVE 轴选择） |
| React UI | `src/components/SequenceEditor.tsx` | 序列编辑器（序列列表 + 步骤列表） |
| React UI | `src/components/FlagConfigEditor.tsx` | Flag 配置编辑器（8 槽位，按需启用/禁用，MANUAL 模式） |
| React UI | `src/components/SoftTimerEditor.tsx` | SoftTimer 编辑器（timer 列表：period_ms + event 参数） |

## Action 序列系统（SEQUENCE）

一条 binding 触发多个子动作。`param` 字段指向 Profile 中 `sequences[]` 数组的索引。

### 数据结构

```c
#define ESTA_MAX_SEQUENCES      4
#define ESTA_MAX_SEQUENCE_STEPS 4

typedef struct {
    uint8_t action;
    uint8_t target_type;
    uint8_t target_inst;
    uint8_t param;
} ESTA_ActionStep_TypeDef;

typedef struct {
    uint8_t step_count;
    ESTA_ActionStep_TypeDef steps[ESTA_MAX_SEQUENCE_STEPS];
} ESTA_ActionSequence_TypeDef;

// 在 ESTA_ProfileSet_TypeDef 中：
uint8_t sequence_count;
ESTA_ActionSequence_TypeDef sequences[ESTA_MAX_SEQUENCES];
```

每个步骤拥有独立的 `target_type`、`target_inst`、`param`，与触发 binding 的 target 无关。

### Handler 实现

```c
static bool action_sequence(const ESTA_Event *evt, void *user_data) {
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL) return false;
    uint8_t seq_idx = ctx->param;
    const ESTA_ProfileSet_TypeDef *p = s->page_state.profiles;
    if (seq_idx >= p->sequence_count) return false;
    const ESTA_ActionSequence_TypeDef *seq = &p->sequences[seq_idx];
    bool any = false;
    for (uint8_t i = 0; i < seq->step_count; i++) {
        const ESTA_ActionStep_TypeDef *step = &seq->steps[i];
        ESTA_EventHandler h = App_ActionGetHandler((ESTA_ActionType)step->action);
        if (h == NULL) continue;
        App_BindingContext step_ctx = {
            .app = ctx->app,
            .target_type = step->target_type,
            .target_inst = step->target_inst,
            .param = step->param,
        };
        if (h(evt, &step_ctx)) any = true;
    }
    return any;
}
```

### Profile JSON 格式

```json
{
  "binding_count": 1,
  "bindings": [
    { "trigger": 1, "source_id": 0, "trigger_id": 65535,
      "target_type": 0, "target_inst": 0, "action": 11, "param": 0 }
  ],
  "sequence_count": 1,
  "sequences": [
    {
      "step_count": 2,
      "steps": [
        { "action": 3, "target_type": 0, "target_inst": 0, "param": 0 },
        { "action": 10, "target_type": 2, "target_inst": 0, "param": 1 }
      ]
    }
  ]
}
```

含义：Button 0 按下 → 执行序列 #0 → ① WAVE #0 切换主题；② TABLE #0 写入字符串表 #1。

> binding 的 `target_type`/`target_inst` 在 SEQUENCE action 下无实际意义（步骤各自携带目标信息），填 0 即可。

---

## 自定义 Action 注册（CUSTOM）

允许用户在运行时注册自己的 handler，无需修改 Profile 或 action 枚举。

### API

```c
// core/app/app_action.h
#define ESTA_CUSTOM_ACTION_MAX 8

void App_RegisterCustomAction(uint8_t custom_id, ESTA_EventHandler handler);
```

`custom_id` 范围 0~7，对应 binding 的 `param` 字段。

### 使用方式

```c
// 用户代码（MCU 初始化阶段）
static bool my_handler(const ESTA_Event *evt, void *user_data) {
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    // ctx->target_type / ctx->target_inst 来自 binding 配置
    // 执行自定义逻辑...
    return true;
}

App_RegisterCustomAction(0, my_handler);  // 注册为 custom_id = 0
```

对应 binding 配置：

```json
{ "trigger": 1, "source_id": 2, "trigger_id": 65535,
  "target_type": 2, "target_inst": 0, "action": 255, "param": 0 }
```

含义：Button 2 按下 → 调用 custom_id=0 的 handler，target 为 TABLE #0。

### 分发机制

```c
static ESTA_EventHandler g_custom_actions[ESTA_CUSTOM_ACTION_MAX];

static bool action_custom_dispatch(const ESTA_Event *evt, void *user_data) {
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    uint8_t id = ctx->param;
    if (id >= ESTA_CUSTOM_ACTION_MAX || g_custom_actions[id] == NULL) return false;
    return g_custom_actions[id](evt, user_data);
}
```

`App_ActionGetHandler(ESTA_ACTION_CUSTOM)` 返回 `action_custom_dispatch`（而非 NULL），由它根据 `ctx->param` 路由到具体 handler。

### 容量

`ESTA_CUSTOM_ACTION_MAX = 8`，custom_id 范围 0~7。

---

## 扩展指南

### 添加新 Action

1. `app_action.h`：`ESTA_ActionType` 枚举新增值
2. `app_action.c`：实现 handler 函数，加入 `g_action_table`
3. `types.ts`：`ACTION_TYPE_OPTIONS` 新增选项，`VALID_ACTIONS` 更新约束
4. 无需改动 dispatch 逻辑或 binding 结构体

> 若新 action 需要 `param` 字段携带额外信息，在 `EventEditor.tsx` 的 param 列中为该 action 添加对应的 UI 控件（参考 TEXT_SET 的字符串下拉、SEQUENCE 的序列下拉、CUSTOM 的数字输入）。

### 添加新事件类型

1. `event.h`：`ESTA_EventType` 枚举新增值
2. 实现对应的 `ESTA_EventEmitXxx()` 函数（或通过 `ESTA_EventPush` 直接推送）
3. `types.ts`：`TRIGGER_OPTIONS` 新增选项
4. `EventEditor.tsx`：根据新 trigger 类型决定 source/trigger_id 的 UI 呈现

### 添加新目标类型

1. `app_action.h`：`ESTA_TargetType` 枚举新增值
2. `types.ts`：`TARGET_TYPE_OPTIONS` 新增选项，`VALID_ACTIONS` 新增约束条目
3. `EventEditor.tsx`：根据新 target_type 决定 target_inst 的 UI 呈现（下拉/隐藏）
4. 实现对应的 action handler（通过 `App_BindingContext.target_inst` 获取实例）
