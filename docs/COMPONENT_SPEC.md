# TL-ESTA 嵌入式 GUI 框架 —— 组件开发规范

参考实现：**WAVE 示波器组件**（`core/WAVE.h` / `core/WAVE.c` / `core/ESTA_Profile.c`）
版本：v1.0
适用范围：所有位于 `core/` 目录下的 GUI 组件

---

## 第一章：总则与架构

### 1.1 框架分层架构

```
应用层 (Application)
    |
模拟场景层 (Scenario)          -- simulator/sim_scenario.c
    |
Profile 配置层 (Profile)       -- core/ESTA_Profile.h/.c  (强制配套)
    |
组件层 (Component)             -- core/WAVE.h/.c  (新组件均位于 core/)
    |
UI 基础层 (UI Base)            -- core/ui_base.h/.c    提供 HAL 宏与通用工具
    |
UI 主题层 (UI Theme)           -- core/ui_theme.h/.c   全局颜色表
    |
辅助层 (Helper)                -- core/helper.h/.c     通道掩码工具
    |
移植层 (Port Layer)            -- port/esta_port_sdl2.*   平台抽象
    |
硬件层 (Hardware)              -- ILI9341 / SDL2 / 自定义
```

### 1.2 各层职责与关键文件

| 层次 | 职责 | 关键文件 | 组件是否直接依赖 |
|------|------|----------|:---:|
| Port Layer | 平台初始化/退出/延时 + 4 个绘图原语 | `port/esta_port_sdl2.*` | **否**（仅通过 `ui_base.h` 宏） |
| UI Base | HAL 宏 (`SCREEN_DRAW_*`)、字体度量、坐标工具、RGB565 颜色 | `core/ui_base.h/.c` | **是** |
| UI Theme | `ESTA_THEME_COLOR` 宏，组件自建本地色表 | `core/infra/ui_theme.h` | **是** |
| Helper | 通道掩码工具函数 | `core/helper.h/.c` | 按需 |
| Component | 具体组件实现 | `core/XXX.h/.c` | —（本规范适用层） |
| Profile | 组件配置的人性化包装 + 代码生成 | `core/ESTA_Profile.*` | **强制配套** |

### 1.3 依赖规则

1. 组件**必须**包含 `ui_base.h` 和 `ui_theme.h`
2. 组件**只能**通过 `SCREEN_DRAW_*` 宏进行绘制，严禁直接调用 Port 层函数
3. 组件**必须**通过 Profile 模块进行外部配置，不得要求用户直接构造 `Config_TypeDef`

---

## 第二章：文件与命名规范

### 2.1 组件文件

- **头文件**：`core/{COMPONENT_NAME}.h`
- **源文件**：`core/{COMPONENT_NAME}.c`
- 组件名全大写（如 `WAVE`），文件名保持一致

### 2.2 前缀体系

| 对象 | 前缀规则 | 示例 |
|------|----------|------|
| 组件函数/宏/类型 | `{NAME}_`（全大写 + 下划线） | `WAVE_Init`、`WAVE_Config_TypeDef` |
| 全局工具函数 | `ui_`（小写） | `ui_coor_normal`、`ui_limit` |
| 主题访问器 | `UI_`（全大写） | `UI_GetThemeColor` |
| 通道工具 | 小写字母前缀 | `is_channel_enabled`、`channel_to_mask` |
| Port 层函数 | `ESTA_SDL2_` | `ESTA_SDL2_Init`、`ESTA_SDL2_DrawLine` |
| Profile 函数 | `ESTA_Profile_` | `ESTA_Profile_Apply` |

### 2.3 头文件保护宏

```c
#ifndef __{NAME}_LIB
#define __{NAME}_LIB
// ...
#endif
```

示例：`__WAVE_LIB`、`__UI_BASE_H`、`__UI_THEME_H`

### 2.4 标准包含顺序

每个组件头文件必须按以下固定顺序包含依赖（需要哪个包含哪个，不允许冗余）：

```c
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper.h"      // 如需通道掩码工具
#include "ui_base.h"     // 必须
#include "ui_theme.h"    // 必须
```

### 2.5 常量命名

| 类型 | 模式 | 示例 |
|------|------|------|
| 最大实例数 | `MAX_{NAME}_NUM` | `MAX_WAVE_NUM` |
| 最大通道/元素数 | `MAX_{NAME}_{FEATURE}` | `MAX_WAVE_CHANNEL`、`MAX_WAVE_RULER_Y_NUM` |
| 其他限制常量 | `{NAME}_MAX_{FEATURE}_NUM` | `WAVE_MAX_RULER_Y_NUM` |

---

## 第三章：OOC 三结构体模式

### 3.1 模式概述

每个组件使用三个结构体实现面向对象风格的封装，模拟硬件外设的"寄存器映射"：

```
{NAME}_TypeDef                     ← "类"的聚合结构体（实例本体）
├── {NAME}_Config_TypeDef          ← 公开"寄存器"（用户可读写）
└── {NAME}_Private_Typedef         ← 私有状态（仅组件内部使用）
```

**命名约定**（有意保留拼写差异以区分访问级别）：

| 结构体 | 后缀 | 访问级别 | 含义 |
|--------|------|:---:|------|
| Config | `_TypeDef` | 公开 | 用户可自由读写的配置参数 |
| Private | `_Typedef` | 私有 | 组件内部运行时状态，用户不应直接访问 |
| Aggregate | `_TypeDef` | — | 将 Config 和 Private 封装为一个实例 |

### 3.2 Config 结构体模板

```c
typedef struct {
    /* 坐标与尺寸 */
    uint16_t       x_origin;
    uint16_t       y_origin;
    uint16_t       x_width;
    uint16_t       y_width;

    /* 业务相关配置 */
    uint16_t       display_num_min;
    uint16_t       display_num_max;
    uint16_t       channel_num;
    uint8_t        channel_mask;

    /* 标志位（使用 volatile bool） */
    volatile bool  is_auto_clear;
    volatile bool  is_display_ruler_x;

    /* 指针型配置（仅在 Init 时使用，Init 后置 NULL） */
    uint16_t       *ruler_y;

    /* 主题选择 */
    {NAME}_theme_type theme_type;
} {NAME}_Config_TypeDef;
```

规则：
- 坐标和尺寸字段（`x_origin`、`y_origin`、`x_width`、`y_width`）是所有可视化组件的**必选字段**
- 标志位字段使用 `volatile bool` 类型
- 仅在 Init 阶段传递数据的指针字段必须在 Init 结束后置为 NULL
- `theme_type` 字段为所有支持主题的组件的必选字段

### 3.3 Private 结构体模板

```c
typedef struct {
    /* 内部缓冲区 */
    uint16_t    buffer[BUFFER_SIZE];

    /* 内部状态追踪 */
    uint16_t    last_index;
    uint16_t    last_coor;

    /* ... 按组件特性添加 ... */
} {NAME}_Private_Typedef;
```

规则：
- Private 中的字段**绝不**在组件外部直接访问
- 所有 Private 访问必须通过 `{NAME}_PRIVATE_MEMBER` / `{NAME}_WRITE_PRIVATE` 宏

### 3.4 Aggregate 结构体模板

```c
/* be like Class in C++ */
typedef struct {
    /* Public regs */
    {NAME}_Config_TypeDef     {NAME}_Config;
    /* Private regs */
    {NAME}_Private_Typedef    {NAME}_Private;
} {NAME}_TypeDef;
```

字段命名固定为 `{NAME}_Config` 和 `{NAME}_Private`。

### 3.5 全局状态数组

在 `.c` 文件中定义全局实例数组：

```c
// 不要修改这个数组的名称，我们根据其来寻址
// 也不要直接通过数组修改里面的内容，而是通过库函数修改配置
{NAME}_TypeDef {NAME}_State[MAX_{NAME}_NUM];
```

规则：
- 数组名固定为 `{NAME}_State`，不可更改
- 为零初始化全局变量（C 标准保证）
- 严禁外部代码直接通过数组下标访问成员——所有访问必须通过宏（第四章）或 API 函数

### 3.6 实例句柄

实例以 `int` 类型的索引表示，通过身份宏包装：

```c
#define {NAME}_INST(i)      (i)
#define {NAME}_INST_ADDR(i) {NAME}_State[{NAME}_INST(i)]
```

- `{NAME}_INST(i)` 是身份宏（纯文档作用，运行时为零开销）
- `{NAME}_INST_ADDR(i)` 解析为全局数组中对应实例的引用

---

## 第四章：宏定义规范

### 4.1 实例宏

```c
#define {NAME}_INST(i)                   (i)
#define {NAME}_INST_ADDR(i)              {NAME}_State[{NAME}_INST(i)]
```

### 4.2 寄存器访问宏 —— 标量

```c
/* 读取配置字段 */
#define {NAME}_CONFIG_MEMBER(inst, field) \
    {NAME}_INST_ADDR(inst).{NAME}_Config.field

/* 读取私有字段 */
#define {NAME}_PRIVATE_MEMBER(inst, field) \
    {NAME}_INST_ADDR(inst).{NAME}_Private.field

/* 写入配置字段 */
#define {NAME}_WRITE_CONFIG(inst, field, val) \
    {NAME}_INST_ADDR(inst).{NAME}_Config.field = (val)

/* 写入私有字段 */
#define {NAME}_WRITE_PRIVATE(inst, field, val) \
    {NAME}_INST_ADDR(inst).{NAME}_Private.field = (val)

/* 从 Init 参数复制到实例（专用于 Init 函数内部） */
#define {NAME}_WRITE_CONFIG_INIT(inst, field) \
    {NAME}_INST_ADDR(inst).{NAME}_Config.field = {NAME}_Init->field
```

### 4.3 寄存器访问宏 —— 数组

```c
#define {NAME}_CONFIG_MEMBER_ARRAY(inst, field, idx) \
    {NAME}_INST_ADDR(inst).{NAME}_Config.field[idx]

#define {NAME}_PRIVATE_MEMBER_ARRAY(inst, field, idx) \
    {NAME}_INST_ADDR(inst).{NAME}_Private.field[idx]
```

### 4.4 主题颜色宏

`ESTA_THEME_COLOR` 是**全局共享**宏，定义在 `core/infra/ui_theme.h` 中：

```c
#define ESTA_THEME_COLOR(table, theme, idx) \
    ((table)[(int)(theme)][(int)(idx)])
```

每个组件在自身的 `.c` 文件中定义**本地静态色表**，通过此宏访问：

```c
// 组件 .c 文件中的本地色表
static const uint16_t {NAME}_ColorTable[{NAME}_THEME_COUNT][{NAME}_THEME_INDEX_COUNT] = {
    [{NAME}_THEME_DEFAULT] = { [0]=COLOR1, [1]=COLOR2, ... },
    [{NAME}_THEME_LIGHT]  = { [0]=COLOR1, [1]=COLOR2, ... },
};

// 使用时
uint16_t color = ESTA_THEME_COLOR({NAME}_ColorTable, theme_type, {NAME}_THEME_FRAME_INDEX);
```

颜色索引从 0 开始，各组件独立，无需全局槽位协调。

### 4.5 强制规则

1. **所有 Config/Private 字段访问必须通过宏**，严禁直接写 `{NAME}_State[i].{NAME}_Config.xxx`
2. 宏名中组件前缀必须全大写
3. `_ARRAY` 后缀专门标识数组访问宏，与标量宏区分
4. `_WRITE_CONFIG_INIT` **仅限** Init 函数内使用，其隐含依赖名为 `{NAME}_Init` 的局部指针变量
5. 宏的参数命名约定：
   - `inst`：实例索引（`int`）
   - `field`：结构体字段名
   - `val`：要写入的值
   - `idx`：数组索引

---

## 第五章：状态码与错误处理

### 5.1 统一状态枚举

`ESTA_StatusTypeDef` 是**全局共享**状态类型，定义在 `core/ui_base.h` 中。所有组件共用同一个状态枚举，不再各自定义：

```c
typedef enum
{
    ESTA_OK       = 0x00,  /* 成功 */
    ESTA_ERROR    = 0x01,  /* 通用错误（参数无效、NULL 指针、越界等） */
    ESTA_FULL     = 0x02   /* 缓冲区满（仅当组件存在数据积压场景时定义） */
} ESTA_StatusTypeDef;
```

规则：
- `ESTA_OK` 固定为 `0x00`（C 语言约定：零表示成功）
- `ESTA_ERROR` 为通用错误码 `0x01`
- `ESTA_FULL` 为可选特殊状态码 `0x02`
- 所有组件函数返回 `ESTA_StatusTypeDef`，使用 `ESTA_OK` / `ESTA_ERROR` / `ESTA_FULL` 作为返回值

### 5.2 返回值约定

| 函数类别 | 返回类型 | 说明 |
|----------|----------|------|
| 生命周期函数 (Init/DeInit) | `ESTA_StatusTypeDef` | 参数错误或操作失败返回 ESTA_ERROR |
| 显示/操作函数 | `ESTA_StatusTypeDef` | 同上 |
| Config Setter | `ESTA_StatusTypeDef` | 不得使用 void（见第八章） |

### 5.3 错误传播宏

`ESTA_RETURN_IF_ERROR` 定义在 `core/ui_base.h` 中，用于简洁的错误传播：

```c
#define ESTA_RETURN_IF_ERROR(expr) \
    do { if ((expr) != ESTA_OK) return ESTA_ERROR; } while(0)
```

用法：
```c
ESTA_StatusTypeDef {NAME}_SomeOp(int inst) {
    if (!IS_VALID_{NAME}_INST(inst)) return ESTA_ERROR;
    ESTA_RETURN_IF_ERROR(SubFunction(inst));
    return ESTA_OK;
}
```

---

## 第六章：验证宏与防御性编程

### 6.1 实例验证宏

```c
#define IS_VALID_{NAME}_INST(x) \
    ((int)(x) < MAX_{NAME}_NUM && (int)(x) >= 0)
```

必须检查 `0 <= x < MAX`，使用 `(int)` 强制转换以避免符号比较警告。

### 6.2 主题/域值验证

```c
#define IS_VALID_THEME(x) \
    ((int)(x) < {NAME}_THEME_COUNT && (int)(x) >= 0)

#define IS_VALID_{NAME}_FIELD(x) \
    ((x) >= MIN_VALUE && (x) <= MAX_VALUE)
```

### 6.3 边界哨兵枚举模式

所有需要做范围检查的枚举必须在末尾放置哨兵值：

```c
typedef enum {
    {NAME}_THEME_DEFAULT = 0,
    {NAME}_THEME_LIGHT,
    // 新主题类型在此之上添加
    {NAME}_THEME_COUNT      /* 哨兵，不是有效主题 */
} {NAME}_theme_type;

typedef enum {
    {NAME}_THEME_FRAME_INDEX = 0,
    {NAME}_THEME_RULER_INDEX,
    // 新颜色索引在此之上添加
    {NAME}_THEME_INDEX_COUNT /* 哨兵，不是有效颜色索引 */
} {NAME}_theme_color_index_type;
```

- `_COUNT` 和 `_INDEX_COUNT` 哨兵值**不是**有效值，仅用于数组尺寸和范围检查
- 所有有效的枚举值必须定义在哨兵之前

### 6.4 NULL 指针检查

所有接受指针参数的函数必须在校验后使用：

```c
ESTA_StatusTypeDef {NAME}_ConfigSetXxx({NAME}_Config_TypeDef *config, ...) {
    if (config == NULL) return ESTA_ERROR;
    // ... 正常逻辑 ...
}
```

### 6.5 防御性编程总则

1. 所有函数入口处**先做参数验证**（实例范围、NULL 检查），失败立即返回
2. 除法前检查除数不为零（如 `value_max_range == 0` 保护）
3. 使用 `ui_limit` 钳位坐标值，避免无符号整数溢出
4. 涉及数值相减后判断 `>= 0` 时使用有符号类型（`(int16_t)` 强制转换）

---

## 第七章：生命周期函数

### 7.1 Init 函数

```c
/**
 * @brief  初始化组件实例
 * @param  inst        : 组件实例，如 {NAME}_INST(0)
 * @param  pConfig     : 指向配置结构体的指针
 * @return ESTA_StatusTypeDef
 * @note   将配置深拷贝到实例的 Config 寄存器，初始化 Private 状态
 */
ESTA_StatusTypeDef {NAME}_Init(int inst, {NAME}_Config_TypeDef *pConfig) {
    if (!IS_VALID_{NAME}_INST(inst)) return ESTA_ERROR;
    if (pConfig == NULL) return ESTA_ERROR;

    /* ---- 写入 Config 字段 ---- */
    {NAME}_WRITE_CONFIG_INIT(inst, x_origin);
    {NAME}_WRITE_CONFIG_INIT(inst, y_origin);
    {NAME}_WRITE_CONFIG_INIT(inst, x_width);
    {NAME}_WRITE_CONFIG_INIT(inst, y_width);
    // ... 其他字段含验证逻辑 ...

    /* ---- 初始化 Private 字段 ---- */
    {NAME}_WRITE_PRIVATE(inst, state_field, 0);

    return ESTA_OK;
}
```

**Init 函数合同**（必须履行的职责）：

| 步骤 | 说明 |
|------|------|
| 参数验证 | `IS_VALID_{NAME}_INST` + `pConfig != NULL` + 各字段合法性 |
| 字段深拷贝 | 通过 `WRITE_CONFIG_INIT` 将所有配置复制到实例 |
| 指针消耗 | 若 Config 中有仅用于传参的指针（如标尺数据），复制完毕后**立即置 NULL** |
| 数据钳位 | 超出 `MAX` 的值应钳位而非报错（如标尺数量 > MAX 时截断为 MAX） |
| 私有初始化 | 将所有 Private 字段设置为初始值（零值或从 Config 派生的初始坐标） |

### 7.2 DeInit 函数

```c
/**
 * @brief  复位（反初始化）组件实例
 * @param  inst        : 组件实例
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef {NAME}_DeInit(int inst) {
    if (!IS_VALID_{NAME}_INST(inst)) return ESTA_ERROR;

    /* 将所有 Config 字段重置为默认值（零/false/NULL/默认主题） */
    {NAME}_WRITE_CONFIG(inst, x_origin, 0);
    // ...

    /* 将所有 Private 字段清零 */
    {NAME}_WRITE_PRIVATE(inst, state_field, 0);
    // ...

    return ESTA_OK;
}
```

**DeInit 合同**：
- 所有 Config 字段复位到零值/安全默认值
- 所有 Private 字段归零
- 不涉及动态内存释放（嵌入式环境无堆分配）

### 7.3 显示/操作函数

```c
ESTA_StatusTypeDef {NAME}_OperationX(int inst, ...);
ESTA_StatusTypeDef {NAME}_ReDraw(int inst);  // 全量重绘
```

规则：
- 首个参数固定为 `int inst`（实例句柄）
- 返回 `ESTA_StatusTypeDef`
- 函数内部先获取配置快照（局部变量缓存），避免反复通过宏读取
- 内部子函数失败时向上传播错误

### 7.4 生命周期状态机

```
[未初始化] ── Init() ──→ [已初始化，可操作]
[已初始化] ── DeInit() ──→ [已复位（未初始化）]
[已初始化] ── Init() ──→ [重新初始化]（幂等，覆盖旧配置）
```

---

## 第八章：Config Setter 函数规范

### 8.1 核心规则（新规）

**所有 Config Setter 必须返回 `ESTA_StatusTypeDef`**，不再使用 `void`。

原因：调用者 Setter 无法区分"正常执行"和"传入 NULL 被静默忽略"，在生产环境中难以排查配置遗漏问题。

### 8.2 函数签名模板

```c
ESTA_StatusTypeDef {NAME}_ConfigSetPositionAndSize(
    {NAME}_Config_TypeDef *config,
    uint16_t x_origin, uint16_t y_origin,
    uint16_t x_width, uint16_t y_width);

ESTA_StatusTypeDef {NAME}_ConfigSet{Feature}(
    {NAME}_Config_TypeDef *config,
    <其他参数>);
```

### 8.3 实现模板

```c
ESTA_StatusTypeDef {NAME}_ConfigSetXxx(
    {NAME}_Config_TypeDef *config, param1, param2, ...)
{
    if (config == NULL) return ESTA_ERROR;

    // 参数合法性检查
    // if (param1 > MAX_VALUE) return ESTA_ERROR;

    config->field1 = param1;
    config->field2 = param2;

    return ESTA_OK;
}
```

### 8.4 规则汇总

1. 首个参数固定为 `{NAME}_Config_TypeDef *config`
2. 必须检查 `config == NULL` → 返回 ERROR
3. 必须检查参数合法性（范围、边界），不合理则返回 ERROR
4. 返回 `ESTA_OK` 或 `ESTA_ERROR`
5. 一个 Setter 仅操作一个逻辑字段或一组紧密相关的字段
6. 命名格式：`{NAME}_ConfigSet{FeatureName}`（PascalCase 连接特性名）

### 8.5 分组参考（以 WAVE 为例）

| 分组 | 函数名 | 设置字段 |
|------|--------|----------|
| 位置尺寸 | `ConfigSetPositionAndSize` | x_origin, y_origin, x_width, y_width |
| 显示范围 | `ConfigSetDisplayRange` | display_num_min, display_num_max |
| 通道数量 | `ConfigSetChannelNum` | channel_num |
| 通道使能 | `ConfigSetChannelEnabled` | channel_mask (OR) |
| 通道禁能 | `ConfigSetChannelDisabled` | channel_mask (AND-NOT) |
| 标尺 Y/X | `ConfigSetRulerY` / `ConfigSetRulerX` | 标尺全套参数 |
| 主题 | `ConfigSetTheme` | theme_type |
| 自动清除 | `ConfigSetAutoClear` | is_auto_clear |

---

## 第九章：主题集成规范

### 9.1 主题枚举定义

每个组件定义两个主题相关枚举：

```c
/* 主题类型枚举 */
typedef enum {
    {NAME}_THEME_DEFAULT = 0,
    {NAME}_THEME_LIGHT,
    // 新主题类型在此添加
    {NAME}_THEME_COUNT      // 边界哨兵
} {NAME}_theme_type;

/* 颜色索引枚举 */
typedef enum {
    {NAME}_THEME_FRAME_INDEX = 0,
    {NAME}_THEME_RULER_INDEX,
    {NAME}_THEME_ITEM_A_INDEX,
    {NAME}_THEME_ITEM_B_INDEX,
    {NAME}_THEME_BACKGROUND_INDEX,
    // 新颜色索引在此添加
    {NAME}_THEME_INDEX_COUNT  // 边界哨兵
} {NAME}_theme_color_index_type;
```

规则：
- 枚举值必须从 0 开始自增
- `_COUNT` 和 `_INDEX_COUNT` 是哨兵值，必须位于枚举末尾
- 颜色索引为组件本地值，无需全局协调

### 9.2 本地色表定义

每个组件在自身的 `.c` 文件中定义**本地静态色表**，不依赖全局表：

```c
// 组件 .c 文件中，紧接在实例数组之后
static const uint16_t {NAME}_ColorTable[{NAME}_THEME_COUNT][{NAME}_THEME_INDEX_COUNT] = {
    [{NAME}_THEME_DEFAULT] = {
        [{NAME}_THEME_FRAME_INDEX]      = __WHITE,
        [{NAME}_THEME_RULER_INDEX]      = __GRAY,
        [{NAME}_THEME_BACKGROUND_INDEX] = __BLACK,
    },
    [{NAME}_THEME_LIGHT] = {
        [{NAME}_THEME_FRAME_INDEX]      = __BLACK,
        [{NAME}_THEME_RULER_INDEX]      = __GRAY,
        [{NAME}_THEME_BACKGROUND_INDEX] = __WHITE,
    },
};
```

每个组件的色表维度精确匹配其主题数 × 颜色索引数，无内存浪费。

**新组件加入时**：仅需在组件自身的 `.c` 文件中定义色表，无需修改任何基础设施文件。

### 9.3 颜色取值方式

通过全局宏 `ESTA_THEME_COLOR`（定义在 `core/infra/ui_theme.h`）访问：

```c
uint16_t theme_type = {NAME}_CONFIG_MEMBER(inst, theme_type);
uint16_t color = ESTA_THEME_COLOR({NAME}_ColorTable, theme_type, {NAME}_THEME_FRAME_INDEX);
SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, color);
```

### 9.5 可用颜色宏

所有颜色宏（RGB565 格式）定义在 `core/ui_base.h` 中：

| 宏 | 色值 | 描述 |
|------|------|------|
| `__WHITE` | 0xFFFF | 白色 |
| `__BLACK` | 0x0000 | 黑色 |
| `__GRAY` | 0x8410 | 灰色 |
| `__RED` | 0xF800 | 红色 |
| `__GREEN` | 0x07E0 | 绿色 |
| `__BLUE` | 0x001F | 蓝色 |
| `__YELLOW` | 0xFFE0 | 黄色 |
| `__ORANGE` | 0xFD20 | 橙色 |
| `__GBLUE` | 0x07FF | 浅蓝绿 |
| `__DEEP_BLUE` | 0x0018 | 深蓝 |
| `__BROWN` | 0x9260 | 棕色 |
| `__BRED` | 0xFA0A | 酒红 |
| `__GRED` | 0xAFE5 | 灰红 |
| `__BRRED` | 0xD104 | 棕红 |
| `__PURPLE` | 0xC01C | 紫色 |

---

## 第十章：Profile 模块规范（强制）

### 10.1 设计目标

Profile 模块将"配置数据"与"配置方法"分离。用户只需填写人性化的数据结构，不需要了解组件的 `Config_TypeDef` 内部布局或 `ConfigSet*` 调用顺序。

### 10.2 数据结构

`ESTA_Profile_TypeDef`（定义在 `core/ESTA_Profile.h`）是所有组件的共享配置描述符。新组件加入时需要在此结构体中添加自己需要的字段。

```c
typedef struct {
    /* ---- WAVE 组件字段 ---- */
    uint16_t       x_origin;
    uint16_t       y_origin;
    uint16_t       x_width;
    uint16_t       y_width;
    uint16_t       display_num_min;
    uint16_t       display_num_max;
    uint16_t       channel_num;
    uint8_t        channel_mask;
    uint16_t       ruler_value_y[WAVE_MAX_RULER_Y_NUM];
    uint16_t       ruler_count_y;
    uint16_t       ruler_num_digits_y;
    bool           is_display_ruler_y;
    uint16_t       ruler_value_x[WAVE_MAX_RULER_X_NUM];
    uint16_t       ruler_count_x;
    uint16_t       ruler_num_digits_x;
    uint16_t       ruler_zero_value_x;
    uint16_t       ruler_full_value_x;
    bool           is_display_ruler_x;
    WAVE_theme_type theme_type;
    bool           is_auto_clear;

    /* ---- 新组件字段在此添加 ---- */
} ESTA_Profile_TypeDef;

typedef struct {
    uint16_t inst_count;
    ESTA_Profile_TypeDef profiles[ESTA_PROFILE_MAX_INST];
} ESTA_ProfileSet_TypeDef;
```

### 10.3 三个强制函数

#### GetDefault

```c
/**
 * @brief  获取默认配置集合（只读）
 * @return 指向静态常量默认配置的指针
 */
const ESTA_ProfileSet_TypeDef *ESTA_Profile_GetDefault(void);
```

实现模式：返回 `.c` 文件中用指定初始化器定义的 `static const` 全局变量。

#### ToConfig

```c
/**
 * @brief  将 Profile 转换为组件 Config
 * @param  profile     : 输入的 Profile 数据
 * @param  out_config  : 输出的组件 Config
 * @return true 成功, false 失败（NULL 指针）
 */
bool ESTA_Profile_ToConfig(const ESTA_Profile_TypeDef *profile,
                           {NAME}_Config_TypeDef *out_config);
```

实现规则：
- 入口检查 `profile == NULL || out_config == NULL` → 返回 `false`
- 先用 `memset(out_config, 0, sizeof(*out_config))` 清零
- 依次调用组件的所有 `ConfigSet*` 函数完成映射
- 返回 `true` 表示成功

#### Apply

```c
/**
 * @brief  ToConfig + Init 的二合一封装
 * @param  inst_idx : 组件实例索引
 * @param  profile  : Profile 数据
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef ESTA_Profile_Apply(int inst_idx,
                                         const ESTA_Profile_TypeDef *profile);
```

实现模式：
```c
ESTA_StatusTypeDef ESTA_Profile_Apply(int inst_idx,
                                         const ESTA_Profile_TypeDef *profile) {
    {NAME}_Config_TypeDef config;
    if (!ESTA_Profile_ToConfig(profile, &config)) {
        return ESTA_ERROR;
    }
    return {NAME}_Init(inst_idx, &config);
}
```

### 10.4 代码生成链路

`core/ESTA_Profile.c` 由 Tauri GUI 工具从 `core/ESTA_Profile.json` 自动生成：

```
ESTA_Profile.json（JSON 数据源）
    → Rust serde 反序列化
    → Tera 模板（src-tauri/templates/ESTA_Profile.c.j2）
    → core/ESTA_Profile.c（生成的 C 代码）
```

新组件接入代码生成需修改以下文件：

| 文件 | 修改内容 |
|------|----------|
| `core/ESTA_Profile.h` | 在 `ESTA_Profile_TypeDef` 中添加新字段 |
| `core/ESTA_Profile.json` | 添加新组件的默认 JSON 数据 |
| `tools/profile-gui/src-tauri/templates/ESTA_Profile.c.j2` | 添加新组件的 Tera 渲染逻辑 |
| `tools/profile-gui/src-tauri/src/models.rs` | 添加 Rust 结构体字段 |
| `tools/profile-gui/src/lib/types.ts` | 添加 TypeScript 类型字段 |

---

## 第十一章：移植层抽象合约

### 11.1 系统控制函数

| 函数 | 用途 |
|------|------|
| `ESTA_SDL2_Init(void)` | 平台初始化（窗口创建、驱动初始化） |
| `ESTA_SDL2_Update(void)` | 刷新显示（将缓冲区推送到屏幕） |
| `ESTA_SDL2_Quit(void)` | 平台退出（释放资源） |
| `ESTA_SDL2_Delay(uint32_t ms)` | 毫秒级阻塞延时 |

### 11.2 四基元绘制宏

组件只能通过 `ui_base.h` 中的以下宏与显示硬件交互：

| 宏 | 签名 | 用途 |
|------|------|------|
| `SCREEN_DRAW_LINE` | `(x1, y1, x2, y2, COLOR)` | 从 (x1,y1) 到 (x2,y2) 画直线 |
| `SCREEN_DRAW_RECTANGLE` | `(x1, y1, x2, y2, COLOR)` | 画矩形边框 |
| `SCREEN_FILL` | `(x1, y1, x2, y2, COLOR)` | 填充实心矩形 |
| `SCREEN_DRAW_NUM` | `(x, y, num, len, COLOR)` | 用 MCU 位图字体渲染数字 |

- 所有坐标以像素为单位，屏幕坐标系原点为左上角
- 所有颜色为 `uint16_t` RGB565 格式
- 字体尺寸：宽 `CHAR_PIXEL_WIDTH`（8px），高 `CHAR_PIXEL_HEIGHT`（16px）

### 11.3 组件与 Port 层的隔离

```
组件代码
    ↓ (仅调用)
SCREEN_DRAW_* 宏（定义在 ui_base.h）
    ↓ (编译时展开)
ESTA_SDL2_* 函数（port/esta_port_sdl2.c）
    ↓
SDL2 / ILI9341 硬件
```

组件代码**严禁**直接 include port 层头文件或直接调用 `ESTA_SDL2_*` 函数。所有绘图需求必须通过 `SCREEN_DRAW_*` 宏完成。

### 11.4 添加新平台的步骤

1. 在 `core/ui_base.h` 中添加新的 `#define SCREEN_USE_XXX`
2. 实现新的 port 文件（如 `port/esta_port_xxx.c/.h`）
3. 在 `ui_base.h` 的 `#elif` 链中添加新平台的宏映射

组件的 C 代码完全不受此过程影响。

---

## 第十二章：编码风格规范

### 12.1 大小写约定

| 对象 | 风格 | 示例 |
|------|------|------|
| 类型/结构体 | PascalCase + 后缀 | `WAVE_Config_TypeDef` |
| 枚举类型 | snake_case（小写） | `WAVE_theme_type` |
| 枚举值 | UPPER_SNAKE_CASE | `WAVE_THEME_DEFAULT` |
| 函数 | PascalCase（组件前缀大写） | `WAVE_Init`、`WAVE_ConfigSetChannelNum` |
| 宏 | UPPER_SNAKE_CASE | `MAX_WAVE_NUM`、`WAVE_INST` |
| 局部变量 | snake_case（小写） | `uint16_t frame_color`、`int inst_count` |
| 全局变量 | `g_` 前缀 + snake_case | `g_default_profiles` |
| 文件作用域数组 | PascalCase | `WAVE_State` |

### 12.2 注释规范

所有对外 API 函数使用 Doxygen 风格注释：

```c
/**
 * @brief  函数简要描述（一句话说明功能）
 * @param  参数名 : 参数说明
 * @return 返回值说明
 * @note   注意事项、边界条件、特殊说明（可选）
 */
```

- 内部静态函数可简化注释
- 关键逻辑需有中文注释解释
- TODO 标记格式：`// TODO: 待实现内容说明`
- 文件头部需包含：项目名、模块说明、主要贡献者、设计特点、版本记录

### 12.3 代码格式

- **缩进**：4 空格（不使用 Tab）
- **大括号**：函数定义左大括号另起一行，控制语句（if/for/while）左大括号同行
- 不同类型的 enum/struct 定义之间保留一个空行
- 宏定义组之间保留注释分隔行
- 每行不超过 100 字符（建议，非强制）

### 12.4 volatile 使用

- Config 结构体中的布尔标志位使用 `volatile bool`（可能被中断或外部修改）
- 从 Config 读取到局部变量快照后，不再需要 volatile

---

## 第十三章：新组件开发清单

以下为新组件从零到完整集成的 **15 步清单**。

### 第 1 步：创建文件

在 `core/` 下创建 `XXX.h` 和 `XXX.c`。

### 第 2 步：定义基础常量

```c
#define MAX_XXX_NUM       N   // 最大实例数
#define MAX_XXX_CHANNEL   N   // 最大通道数（如需要）
```

### 第 3 步：定义状态枚举

```c
typedef enum { XXX_OK = 0x00, XXX_ERROR = 0x01 } XXX_StatusTypeDef;
```

### 第 4 步：定义实例宏

```c
#define XXX_INST(i)      (i)
#define XXX_INST_ADDR(i) XXX_State[XXX_INST(i)]
```

### 第 5 步：定义寄存器访问宏

```c
#define XXX_CONFIG_MEMBER(inst, field)   ...
#define XXX_PRIVATE_MEMBER(inst, field)  ...
#define XXX_WRITE_CONFIG(inst, field, val) ...
#define XXX_WRITE_PRIVATE(inst, field, val) ...
#define XXX_WRITE_CONFIG_INIT(inst, field) ...
#define XXX_CONFIG_MEMBER_ARRAY(inst, field, idx) ...
#define XXX_PRIVATE_MEMBER_ARRAY(inst, field, idx) ...
```

### 第 6 步：定义验证宏

```c
#define IS_VALID_XXX_INST(x)  ((int)(x) < MAX_XXX_NUM && (int)(x) >= 0)
```

### 第 7 步：定义三结构体

```c
typedef struct { /* Config 字段 */ } XXX_Config_TypeDef;
typedef struct { /* Private 字段 */ } XXX_Private_Typedef;
typedef struct {
    XXX_Config_TypeDef   XXX_Config;
    XXX_Private_Typedef  XXX_Private;
} XXX_TypeDef;
```

在 `.c` 文件中定义：`XXX_TypeDef XXX_State[MAX_XXX_NUM];`

### 第 8 步：实现 Config Setter

所有 `XXX_ConfigSet*(config, ...)` 返回 `XXX_StatusTypeDef`，含 NULL 检查和参数校验。

### 第 9 步：实现生命周期函数

`XXX_Init(int inst, XXX_Config_TypeDef *pConfig)` 和 `XXX_DeInit(int inst)`。

### 第 10 步：实现显示/操作函数

所有函数返回 `XXX_StatusTypeDef`，首个参数为 `int inst`。

### 第 11 步：集成主题

1. 定义 `XXX_theme_type` 和 `XXX_theme_color_index_type` 枚举（值从 0 开始）
2. 在组件 `.c` 文件中定义本地静态色表 `XXX_ColorTable[THEME_COUNT][INDEX_COUNT]`
3. 通过 `ESTA_THEME_COLOR(XXX_ColorTable, theme, idx)` 获取颜色

### 第 12 步：集成 Profile

1. 在 `ESTA_Profile_TypeDef` 中添加 XXX 相关字段
2. 在 `core/ESTA_Profile.c` 中实现 `ToConfig` 和 `Apply` 对 XXX 的支持
3. 更新 `core/ESTA_Profile.json`
4. 更新 Tera 模板
5. 更新 Rust `models.rs`
6. 更新 TypeScript `types.ts`

### 第 13 步：添加到构建系统

`CMakeLists.txt` 使用 `file(GLOB SOURCES "core/*.c")`，新 `.c` 文件自动包含。

### 第 14 步：编写场景测试

在 `simulator/sim_scenario.h/.c` 中添加新组件的测试数据生成逻辑。

### 第 15 步：更新文档

1. 更新 `README.md` 的 API 列表
2. 更新 `CLAUDE.md` 的架构说明
3. 更新本规范文档的槽位分配表和变更日志

---

## 附录 A：完整组件结构模板

以下模板可作为新组件的起点，直接复制并替换 `XXX` 为实际组件名前缀。

### XXX.h

```c
#ifndef __XXX_LIB
#define __XXX_LIB

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper.h"
#include "ui_base.h"
#include "ui_theme.h"

/* 最大实例数 */
#define MAX_XXX_NUM          N

/* 实例宏 */
#define XXX_INST(i)                   (i)
#define XXX_INST_ADDR(i)              XXX_State[XXX_INST(i)]

/* 主题类型 */
typedef enum {
    XXX_THEME_DEFAULT = 0,
    XXX_THEME_COUNT
} XXX_theme_type;

/* 颜色索引 */
typedef enum {
    XXX_THEME_ITEM_A_INDEX = 0,
    XXX_THEME_ITEM_B_INDEX,
    XXX_THEME_BACKGROUND_INDEX,
    XXX_THEME_INDEX_COUNT
} XXX_theme_color_index_type;

/* 主题桥接宏 */
#define XXX_GetThemeColor(theme, idx) \
    UI_GetThemeColor((int)(theme), (int)(idx))

/* 寄存器访问宏 — 标量 */
#define XXX_CONFIG_MEMBER(inst, field)   XXX_INST_ADDR(inst).XXX_Config.field
#define XXX_PRIVATE_MEMBER(inst, field)  XXX_INST_ADDR(inst).XXX_Private.field
#define XXX_WRITE_CONFIG(inst, f, v)     XXX_INST_ADDR(inst).XXX_Config.f = (v)
#define XXX_WRITE_PRIVATE(inst, f, v)    XXX_INST_ADDR(inst).XXX_Private.f = (v)
#define XXX_WRITE_CONFIG_INIT(inst, f)   XXX_INST_ADDR(inst).XXX_Config.f = XXX_Init->f

/* 寄存器访问宏 — 数组 */
#define XXX_CONFIG_MEMBER_ARRAY(inst, f, i)   XXX_INST_ADDR(inst).XXX_Config.f[i]
#define XXX_PRIVATE_MEMBER_ARRAY(inst, f, i)  XXX_INST_ADDR(inst).XXX_Private.f[i]

/* 状态枚举 */
typedef enum {
    XXX_OK    = 0x00,
    XXX_ERROR = 0x01
} XXX_StatusTypeDef;

/* 验证宏 */
#define IS_VALID_XXX_INST(x)  ((int)(x) < MAX_XXX_NUM && (int)(x) >= 0)

/* === 三结构体 === */

typedef struct {
    uint16_t       x_origin;
    uint16_t       y_origin;
    uint16_t       x_width;
    uint16_t       y_width;
    /* ... 业务配置字段 ... */
    XXX_theme_type theme_type;
} XXX_Config_TypeDef;

typedef struct {
    /* ... 私有运行时状态 ... */
} XXX_Private_Typedef;

typedef struct {
    XXX_Config_TypeDef   XXX_Config;
    XXX_Private_Typedef  XXX_Private;
} XXX_TypeDef;

/* === API 声明 === */

XXX_StatusTypeDef XXX_ConfigSetPositionAndSize(XXX_Config_TypeDef *config,
    uint16_t x_origin, uint16_t y_origin, uint16_t x_width, uint16_t y_width);
XXX_StatusTypeDef XXX_ConfigSetTheme(XXX_Config_TypeDef *config,
    XXX_theme_type theme_type);

XXX_StatusTypeDef XXX_Init(int inst, XXX_Config_TypeDef *pConfig);
XXX_StatusTypeDef XXX_DeInit(int inst);

XXX_StatusTypeDef XXX_ReDraw(int inst);
/* ... 其他操作函数 ... */

#endif /* __XXX_LIB */
```

### XXX.c

```c
#include "XXX.h"
#include <string.h>

XXX_TypeDef XXX_State[MAX_XXX_NUM];

/* === Config Setter === */

XXX_StatusTypeDef XXX_ConfigSetPositionAndSize(XXX_Config_TypeDef *config,
    uint16_t x_origin, uint16_t y_origin, uint16_t x_width, uint16_t y_width) {
    if (config == NULL) return XXX_ERROR;
    config->x_origin = x_origin;
    config->y_origin = y_origin;
    config->x_width = x_width;
    config->y_width = y_width;
    return XXX_OK;
}

XXX_StatusTypeDef XXX_ConfigSetTheme(XXX_Config_TypeDef *config,
    XXX_theme_type theme_type) {
    if (config == NULL) return XXX_ERROR;
    config->theme_type = theme_type;
    return XXX_OK;
}

/* === Lifecycle === */

XXX_StatusTypeDef XXX_Init(int inst, XXX_Config_TypeDef *pConfig) {
    if (!IS_VALID_XXX_INST(inst)) return XXX_ERROR;
    if (pConfig == NULL) return XXX_ERROR;

    XXX_WRITE_CONFIG_INIT(inst, x_origin);
    XXX_WRITE_CONFIG_INIT(inst, y_origin);
    XXX_WRITE_CONFIG_INIT(inst, x_width);
    XXX_WRITE_CONFIG_INIT(inst, y_width);
    /* ... 字段验证与拷贝 ... */

    return XXX_OK;
}

XXX_StatusTypeDef XXX_DeInit(int inst) {
    if (!IS_VALID_XXX_INST(inst)) return XXX_ERROR;

    XXX_WRITE_CONFIG(inst, x_origin, 0);
    XXX_WRITE_CONFIG(inst, y_origin, 0);
    XXX_WRITE_CONFIG(inst, x_width, 0);
    XXX_WRITE_CONFIG(inst, y_width, 0);
    /* ... */

    return XXX_OK;
}

/* === Display / Operations === */

XXX_StatusTypeDef XXX_ReDraw(int inst) {
    if (!IS_VALID_XXX_INST(inst)) return XXX_ERROR;
    /* ... */
    return XXX_OK;
}
```

---

## 附录 B：主题槽位分配表

| 槽位范围 | 占用数 | 组件 | 状态 | 注册日期 |
|----------|:---:|------|:---:|----------|
| 0 - 6 | 7 | WAVE | 已注册 | 2026-05-08 |
| 7 - 14 | 8 | BARCHART | 已注册 | 2026-05-08 |
| 15 | 1 | **预留** | — | — |

---

## 附录 C：错误码分配参考

| 值 | 常量名 | 语义 | 适用场景 |
|:---:|------|------|----------|
| 0x00 | `_OK` | 成功 | 所有组件 |
| 0x01 | `_ERROR` | 通用错误 | 参数无效、NULL 指针、越界、内部错误 |
| 0x02 | `_FULL` | 缓冲区满 | 数据积压、屏幕已满且未启用自动清除 |
| 0x03+ | 组件自定义 | 按需 | 组件特有的错误类型 |

---

## 附录 D：新组件需修改的文件清单

| # | 文件 | 操作 | 说明 |
|---|------|:---:|------|
| 1 | `core/XXX.h` | **新建** | 组件头文件 |
| 2 | `core/XXX.c` | **新建** | 组件源文件 |
| 3 | `core/ESTA_Profile.h` | 修改 | 在 `ESTA_Profile_TypeDef` 中添加 XXX 字段 |
| 4 | `core/ESTA_Profile.c` | 修改 | 添加 ToConfig/Apply 对 XXX 的支持 |
| 5 | `core/ESTA_Profile.json` | 修改 | 添加默认配置数据 |
| 6 | `tools/profile-gui/src-tauri/templates/ESTA_Profile.c.j2` | 修改 | 添加模板渲染逻辑 |
| 7 | `tools/profile-gui/src-tauri/src/models.rs` | 修改 | 添加 Rust 结构体字段 |
| 8 | `tools/profile-gui/src/lib/types.ts` | 修改 | 添加 TypeScript 类型字段 |
| 9 | `simulator/sim_scenario.h` | 修改 | 添加场景测试声明 |
| 10 | `simulator/sim_scenario.c` | 修改 | 添加场景测试实现 |
| 11 | `docs/COMPONENT_SPEC.md` | 修改 | 更新变更记录 |

> **注**：`CMakeLists.txt` 使用 `file(GLOB SOURCES "core/*.c")` 自动收集源文件，第 2 步创建的新 `.c` 文件会被自动纳入构建，无需手动修改构建脚本。
