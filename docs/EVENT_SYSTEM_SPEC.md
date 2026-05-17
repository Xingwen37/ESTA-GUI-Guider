# 事件绑定系统规格（EVENT_SYSTEM_SPEC）

本文档描述 ESTA 的数据驱动事件绑定系统。该系统允许用户通过 Profile 配置（而非硬编码）来定义"什么输入触发什么动作"。

## 架构概览

```
┌─────────────────────────────────────────────────────────────────┐
│  事件源                                                          │
│  ├─ 外部输入：按钮按下/松开、编码器旋转                            │
│  └─ 内部事件：MENU 选中某项、定时器到期                            │
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
    uint8_t  type;       // ESTA_EventType 枚举值
    uint8_t  source;     // 物理源：按钮索引、编码器索引、组件实例
    uint16_t id;         // 语义 ID：菜单项 event_id、timer_id 等
    uint32_t timestamp;  // 系统 tick (ms)
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
} ESTA_EventBinding_TypeDef;
```

## 绑定字段语义

| 字段 | 含义 | 特殊值 |
|------|------|--------|
| `trigger` | 触发事件类型 | — |
| `source_id` | 事件源（按钮 ID / MENU 实例） | `0xFF` = 匹配任意源 |
| `trigger_id` | 事件语义 ID（MENU 项的 event_id） | `0xFFFF` = 匹配任意 ID |
| `target_type` | 动作目标组件类型 | — |
| `target_inst` | 目标组件实例索引 | PAGE/GLOBAL 时为 0 |
| `action` | 执行的动作 | — |

### 各事件类型的字段映射

| trigger | source_id 含义 | trigger_id 含义 |
|---------|---------------|----------------|
| BUTTON_PRESS (1) | 按钮编号 (0~N) | 无意义（填 0xFFFF） |
| BUTTON_RELEASE (2) | 按钮编号 (0~N) | 无意义（填 0xFFFF） |
| MENU_SELECT (3) | MENU 实例编号 (0~3) | 菜单项的 event_id |
| ENCODER_ROTATE (4) | 编码器编号 | 方向/步数（预留） |
| TIMER (5) | — | timer_id（预留） |

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
} ESTA_TargetType;
```

### ESTA_ActionType

```c
typedef enum {
    ESTA_ACTION_NONE          = 0,
    ESTA_ACTION_PAGE_NEXT     = 1,
    ESTA_ACTION_PAGE_PREV     = 2,
    ESTA_ACTION_THEME_TOGGLE  = 3,
    ESTA_ACTION_WAVE_REDRAW   = 4,
    ESTA_ACTION_MENU_UP       = 5,
    ESTA_ACTION_MENU_DOWN     = 6,
    ESTA_ACTION_MENU_ENTER    = 7,
    ESTA_ACTION_MENU_BACK     = 8,
    ESTA_ACTION_CUSTOM        = 0xFF
} ESTA_ActionType;
```

### 合法组合约束表（GUI 侧过滤）

| target_type | 可用 action |
|-------------|------------|
| WAVE (0) | THEME_TOGGLE, WAVE_REDRAW |
| BARCHART (1) | THEME_TOGGLE |
| TABLE (2) | （暂无） |
| MENU (3) | MENU_UP, MENU_DOWN, MENU_ENTER, MENU_BACK |
| PAGE (4) | PAGE_NEXT, PAGE_PREV |
| GLOBAL (5) | THEME_TOGGLE |

## 分发机制

`App_DispatchEvents()`（在 `App_MainTick()` 中每帧调用）从队列中取出事件，两轮匹配：

1. **Pass 1（精确匹配）**：`source_id != ANY` 的订阅，按 `type + source_id + event_id` 匹配。匹配成功且 handler 返回 true 则消费事件。
2. **Pass 2（wildcard）**：`source_id == ANY` 的订阅，仅匹配 `type + event_id`。作为兜底处理。

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
  "binding_count": 5,
  "bindings": [
    { "trigger": 1, "source_id": 1, "trigger_id": 65535, "target_type": 3, "target_inst": 0, "action": 5 },
    { "trigger": 3, "source_id": 0, "trigger_id": 10,    "target_type": 0, "target_inst": 0, "action": 3 }
  ]
}
```

### C 生成代码

```c
.bindings = {
    { .trigger = 1, .source_id = 1, .trigger_id = 65535, .target_type = 3, .target_inst = 0, .action = 5 },
    { .trigger = 3, .source_id = 0, .trigger_id = 10,    .target_type = 0, .target_inst = 0, .action = 3 }
}
```

### 初始化流程

`App_MainInit()` → `ESTA_Profile_ApplyEvents(profiles, state)`：
1. 遍历 `bindings[]` 数组
2. 为每条 binding 创建 `App_BindingContext`（静态数组 `g_binding_ctx[]`）
3. 调用 `App_Subscribe(trigger, source_id, trigger_id, handler, &ctx)`

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

## 波形触发模式

`App_MainState.wave_trigger`（bool）控制 SimFeed 是否执行波形数据采集：
- `action_wave_redraw` handler 设置 `s->wave_trigger = true`
- `sim_feed_wave()` 检查标志，触发后消费（单次触发）
- 默认波形静止，按键触发后刷新一帧

## 全链路文件清单

| 层 | 文件 | 职责 |
|----|------|------|
| C 事件队列 | `core/event/event.h/.c` | ESTA_Event 结构体、队列、Emit 函数 |
| C 订阅分发 | `core/app/app_event.h/.c` | App_Subscribe、App_DispatchEvents |
| C Action 注册 | `core/app/app_action.h/.c` | 枚举、handler 实现、g_action_table |
| C Profile 绑定 | `core/profile/ESTA_Profile.c` | ApplyEvents、g_binding_ctx |
| C MENU 导航 | `core/ui/MENU.h/.c` | MENU_NavUp/Down/Enter/Back |
| JSON 数据 | `core/profile/ESTA_Profile.json` | bindings 数组 |
| Rust 模型 | `src-tauri/src/models.rs` | EventBinding struct |
| Rust 命令 | `src-tauri/src/commands.rs` | 模板数据构建 |
| Tera 模板 | `src-tauri/templates/ESTA_Profile.c.j2` | C 代码生成 |
| TS 类型 | `src/lib/types.ts` | EventBinding 接口、常量、约束表 |
| React UI | `src/components/EventEditor.tsx` | 事件绑定编辑器 |

## 扩展指南

### 添加新 Action

1. `app_action.h`：`ESTA_ActionType` 枚举新增值
2. `app_action.c`：实现 handler 函数，加入 `g_action_table`
3. `types.ts`：`ACTION_TYPE_OPTIONS` 新增选项，`VALID_ACTIONS` 更新约束
4. 无需改动 dispatch 逻辑或 binding 结构体

### 添加新事件类型

1. `event.h`：`ESTA_EventType` 枚举新增值
2. 实现对应的 `ESTA_EventEmitXxx()` 函数
3. `types.ts`：`TRIGGER_OPTIONS` 新增选项
4. `EventEditor.tsx`：根据新 trigger 类型决定 source/trigger_id 的 UI 呈现
