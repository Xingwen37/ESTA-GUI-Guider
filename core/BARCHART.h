#ifndef __BARCHART_LIB
#define __BARCHART_LIB

/* INCLUDE */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper.h"
#include "ui_base.h"
#include "ui_theme.h"

/* GLOBAL MARCO */
// 最大柱状图实例个数
#define BARCHART_MAX_NUM         4
// 单个实例最大柱体数量
#define BARCHART_MAX_BARS        32

/* INST MARCO */
#define BARCHART_INST(i)                   (i)
#define BARCHART_INST_ADDR(i)              BARCHART_State[BARCHART_INST(i)]

/* THEME SETTINGS */
typedef enum {
    BARCHART_THEME_DEFAULT = 0,
    BARCHART_THEME_LIGHT ,
    BARCHART_THEME_COUNT
} BARCHART_theme_type;

typedef enum {
    BARCHART_THEME_FRAME_INDEX      = 7,
    BARCHART_THEME_AXIS_INDEX       = 8,
    BARCHART_THEME_BAR_INDEX        = 9,
    BARCHART_THEME_BAR_CH1_INDEX    = 10,
    BARCHART_THEME_BAR_CH2_INDEX    = 11,
    BARCHART_THEME_BAR_CH3_INDEX    = 12,
    BARCHART_THEME_BACKGROUND_INDEX = 13,
    BARCHART_THEME_LABEL_INDEX      = 14,
    BARCHART_THEME_INDEX_COUNT
} BARCHART_theme_color_index_type;

/* WRITE/READ REGS MARCO */
#define BARCHART_CONFIG_MEMBER(inst, reg_name)  \
            BARCHART_INST_ADDR(inst).BARCHART_Config.reg_name
#define BARCHART_PRIVATE_MEMBER(inst, reg_name)  \
            BARCHART_INST_ADDR(inst).BARCHART_Private.reg_name

#define BARCHART_CONFIG_MEMBER_ARRAY(inst, reg_name, NO)  \
            BARCHART_INST_ADDR(inst).BARCHART_Config.reg_name[NO]
#define BARCHART_PRIVATE_MEMBER_ARRAY(inst, reg_name, NO)  \
            BARCHART_INST_ADDR(inst).BARCHART_Private.reg_name[NO]

#define BARCHART_WRITE_CONFIG(inst, reg_name, reg_value)   \
            BARCHART_INST_ADDR(inst).BARCHART_Config.reg_name = reg_value
#define BARCHART_WRITE_PRIVATE(inst, reg_name, reg_value)   \
            BARCHART_INST_ADDR(inst).BARCHART_Private.reg_name = reg_value

#define BARCHART_WRITE_CONFIG_INIT(inst, reg_name)         \
            BARCHART_INST_ADDR(inst).BARCHART_Config.reg_name = BARCHART_Init->reg_name

/* ASSERT MARCO */
#define IS_VALID_BARCHART_INST(OSCx)  ((int)OSCx < BARCHART_MAX_NUM && (int)OSCx >= 0)
#define IS_VALID_BARCHART_THEME(THEMEx)  ((int)THEMEx < BARCHART_THEME_COUNT && (int)THEMEx >= 0)
#define IS_VALID_BARCHART_BAR_IDX(idx)  ((int)idx >= 0 && (int)idx < BARCHART_MAX_BARS)

/* Software REGS typedef of BARCHART */
typedef struct {
    /* 坐标与尺寸（与 ESTA_BaseConfig 兼容） */
    uint16_t       x_origin;
    uint16_t       y_origin;
    uint16_t       x_width;
    uint16_t       y_width;
    /* 显示数值范围 */
    uint16_t       display_num_min;
    uint16_t       display_num_max;
    /* 柱体布局 */
    uint16_t       bar_count;
    uint16_t       bar_width;       // 0 = 自动计算
    uint16_t       bar_spacing;     // 0 = 自动计算
    /* 数据源（仅 Init 时用于传参，Init 后置 NULL） */
    uint16_t      *data_values;
    /* 显示选项 */
    volatile bool  is_display_value;
    volatile bool  is_display_axis;
    /* 主题 */
    BARCHART_theme_type theme_type;
} BARCHART_Config_TypeDef;

typedef struct {
    /* Private regs */
    uint16_t    data_buff[BARCHART_MAX_BARS];
    uint16_t    area_ox;
    uint16_t    area_oy;
    uint16_t    area_width;
    uint16_t    area_height;
    uint16_t    bar_width_actual;
    uint16_t    bar_spacing_actual;
    uint16_t    bar_count_actual;
    uint16_t    baseline_y;
} BARCHART_Private_Typedef;

/* be like Class in C++ */
typedef struct {
    /* Public regs */
    BARCHART_Config_TypeDef     BARCHART_Config;
    /* Private regs */
    BARCHART_Private_Typedef    BARCHART_Private;
} BARCHART_TypeDef;

/* function prototype */
ESTA_StatusTypeDef BARCHART_ConfigSetDisplayRange(BARCHART_Config_TypeDef *config,
    uint16_t display_num_min, uint16_t display_num_max);
ESTA_StatusTypeDef BARCHART_ConfigSetBarCount(BARCHART_Config_TypeDef *config,
    uint16_t bar_count);
ESTA_StatusTypeDef BARCHART_ConfigSetBarLayout(BARCHART_Config_TypeDef *config,
    uint16_t bar_width, uint16_t bar_spacing);
ESTA_StatusTypeDef BARCHART_ConfigSetData(BARCHART_Config_TypeDef *config,
    uint16_t *data_values, uint16_t data_count);
ESTA_StatusTypeDef BARCHART_ConfigSetDisplayOptions(BARCHART_Config_TypeDef *config,
    bool is_display_value, bool is_display_axis);
ESTA_StatusTypeDef BARCHART_ConfigSetTheme(BARCHART_Config_TypeDef *config,
    BARCHART_theme_type theme_type);

ESTA_StatusTypeDef BARCHART_Init(int inst, BARCHART_Config_TypeDef *pConfig);
ESTA_StatusTypeDef BARCHART_DeInit(int inst);

ESTA_StatusTypeDef BARCHART_FrameDisplay(int inst);
ESTA_StatusTypeDef BARCHART_BarDisplay(int inst);
ESTA_StatusTypeDef BARCHART_ReDraw(int inst);
ESTA_StatusTypeDef BARCHART_Clear(int inst);
ESTA_StatusTypeDef BARCHART_UpdateBar(int inst, uint16_t bar_index, uint16_t new_value);
ESTA_StatusTypeDef BARCHART_UpdateAll(int inst, const uint16_t *new_values, uint16_t count);

#endif
