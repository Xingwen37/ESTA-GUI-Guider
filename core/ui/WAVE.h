#ifndef __WAVE_LIB
#define __WAVE_LIB

/* INCLUDE */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "infra/helper.h"
#include "infra/ui_base.h"
#include "infra/ui_theme.h"

/* GLOBAL MARCO */
// 最大示波器实例个数
#define MAX_WAVE_NUM         4

// 单个示波器实例通道数
//TODO: 实现多通道的示波器 ,现在支持至四通道,可拓展至八通道
#define MAX_WAVE_CHANNEL     4

// 最大标尺个数，标尺过多且宽度不足可能导致标尺重叠
#define WAVE_MAX_RULER_Y_NUM   5
#define WAVE_MAX_RULER_X_NUM   5


/* USER MARCO OR ENUM */
/* INST MARCO */
#define WAVE_INST(i)                   (i)
#define WAVE_INST_ADDR(i)              WAVE_State[WAVE_INST(i)]


/* THEME SETTINGS */
typedef enum {
    WAVE_THEME_DEFAULT = 0,
    WAVE_THEME_LIGHT ,
    //用于检查边界条件，并非主题类型！所有添加的类型都放在WAVE_THEME_COUNT 这个枚举变量上面
    WAVE_THEME_COUNT
} WAVE_theme_type;

typedef enum {
    WAVE_THEME_FRAME_INDEX = 0  ,
    WAVE_THEME_RULER_INDEX      ,
    WAVE_THEME_WAVE_CH0_INDEX   ,
    WAVE_THEME_WAVE_CH1_INDEX   ,
    WAVE_THEME_WAVE_CH2_INDEX   ,
    WAVE_THEME_WAVE_CH3_INDEX   ,
    WAVE_THEME_BACKGROUND_INDEX ,
    //用于检查边界条件，并非主题类型！所有添加的类型都放在WAVE_THEME_INDEX_COUNT 这个枚举变量上面
    WAVE_THEME_INDEX_COUNT
} WAVE_theme_color_index_type;

/* WRITE/READ REGS MARCO */
#define WAVE_CONFIG_MEMBER(OSCx, reg_name)  \
            WAVE_INST_ADDR(OSCx).WAVE_Config.reg_name
#define WAVE_PRIVATE_MEMBER(OSCx, reg_name)  \
            WAVE_INST_ADDR(OSCx).WAVE_Private.reg_name

#define WAVE_CONFIG_MEMBER_ARRAY(OSCx, reg_name, NO)  \
            WAVE_INST_ADDR(OSCx).WAVE_Config.reg_name[NO]
#define WAVE_PRIVATE_MEMBER_ARRAY(OSCx, reg_name, NO)  \
            WAVE_INST_ADDR(OSCx).WAVE_Private.reg_name[NO]

#define WAVE_WRITE_CONFIG(OSCx, reg_name, reg_value)   \
            WAVE_INST_ADDR(OSCx).WAVE_Config.reg_name = reg_value
#define WAVE_WRITE_PRIVATE(OSCx, reg_name, reg_value)   \
            WAVE_INST_ADDR(OSCx).WAVE_Private.reg_name = reg_value

#define WAVE_WRITE_CONFIG_INIT(OSCx, reg_name)         \
            WAVE_INST_ADDR(OSCx).WAVE_Config.reg_name = WAVE_Init->reg_name


#define IS_VALID_WAVE_INST(OSCx) ((int)OSCx < MAX_WAVE_NUM && (int)OSCx >= 0)
#define IS_VALID_THEME(THEMEx)  ((int)THEMEx < WAVE_THEME_COUNT && (int)THEMEx >= 0)
#define IS_VALID_CHNUM(CHNUM)   ((int)CHNUM < MAX_WAVE_CHANNEL && (int)CHNUM >= 0)


/* Software REGS typedef of WAVE */
typedef struct {
    /* config regs */
    /* 示波器的坐标原点(x_origin, y_origin) */
    uint16_t       x_origin;
    uint16_t       y_origin;
    /* 示波器的x,y宽度 x_width, y_width */
    uint16_t       x_width;
    uint16_t       y_width;
    /* 示波器显示数字的最大值和最小值 */
    uint16_t       display_num_min;
    uint16_t       display_num_max;
    /* 示波器通道数量（至多两个） */
    uint16_t       channel_num;
    /* 示波器通道掩码 */
    uint8_t        channel_mask;
    /* 标尺设置 */
    // TODO: 标尺显示负数,小数的情况
    // TODO: 非线性标尺（如dB）
    volatile bool  is_display_ruler_x; // 是否显示x轴标尺
    volatile bool  is_display_ruler_y; // 是否显示y轴标尺
    /* ruler 这个地址只在初始化时用于传参，用于将标尺数据保存在Private中, 其余时间为NULL */
    uint16_t       *ruler_y;           // y轴标尺传入的地址
    uint16_t       ruler_count_y;      // y轴标尺数量，最多WAVE_MAX_RULER_NUM
    uint16_t       ruler_num_digits_y; // y轴标尺显示数字的最大位数
    /* ruler 这个地址只在初始化时用于传参，用于将标尺数据保存在Private中, 其余时间为NULL */
    uint16_t       *ruler_x;           // x轴标尺传入的地址
    uint16_t       ruler_count_x;      // x轴标尺数量，最多WAVE_MAX_RULER_NUM
    uint16_t       ruler_zero_value_x; // x轴标尺坐标为0的值
    uint16_t       ruler_full_value_x; // x轴标尺坐标最大的值
    uint16_t       ruler_num_digits_x; // x轴标尺显示数字的最大位数
    /* 屏幕满是否自动刷新屏幕 */
    volatile bool  is_auto_clear;
    /* 示波器主题 */
    WAVE_theme_type theme_type;
} WAVE_Config_TypeDef;

typedef struct {
    /* Private regs */
    /* 标尺的数据存储在这里 */
    uint16_t    ruler_buff_y[WAVE_MAX_RULER_Y_NUM];
    uint16_t    ruler_buff_x[WAVE_MAX_RULER_X_NUM];
    uint16_t    last_index;
    uint16_t    x_coor_last;
    uint16_t    y_coor_last_CH[MAX_WAVE_CHANNEL];
} WAVE_Private_Typedef;

/* be like Class in C++ */
typedef struct {
    /* Public regs */
    WAVE_Config_TypeDef     WAVE_Config;
    /* Private regs */
    WAVE_Private_Typedef    WAVE_Private;
} WAVE_TypeDef;

extern WAVE_TypeDef WAVE_State[MAX_WAVE_NUM];

/* function prototype */
ESTA_StatusTypeDef WAVE_ConfigSetDisplayRange(WAVE_Config_TypeDef *config,
                               uint16_t display_num_min, uint16_t display_num_max);
ESTA_StatusTypeDef WAVE_ConfigSetChannelNum(WAVE_Config_TypeDef *config, uint16_t channel_num);
ESTA_StatusTypeDef WAVE_ConfigSetChannelEnabled(WAVE_Config_TypeDef *config, uint8_t channel_mask);
ESTA_StatusTypeDef WAVE_ConfigSetChannelDisabled(WAVE_Config_TypeDef *config, uint8_t channel_mask);
ESTA_StatusTypeDef WAVE_ConfigSetRulerY(WAVE_Config_TypeDef *config, bool is_display,
                         uint16_t *ruler_y, uint16_t ruler_count_y,
                         uint16_t ruler_num_digits_y);
ESTA_StatusTypeDef WAVE_ConfigSetRulerX(WAVE_Config_TypeDef *config, bool is_display,
                         uint16_t *ruler_x, uint16_t ruler_count_x,
                         uint16_t ruler_zero_value_x, uint16_t ruler_full_value_x,
                         uint16_t ruler_num_digits_x);
ESTA_StatusTypeDef WAVE_ConfigSetTheme(WAVE_Config_TypeDef *config, WAVE_theme_type theme_type);
ESTA_StatusTypeDef WAVE_ConfigSetAutoClear(WAVE_Config_TypeDef *config, bool is_auto_clear);

ESTA_StatusTypeDef WAVE_Init(int OSCx, WAVE_Config_TypeDef *WAVE_Init);
ESTA_StatusTypeDef WAVE_DeInit(int OSCx);
ESTA_StatusTypeDef WAVE_RulerDisplay(int OSCx);
ESTA_StatusTypeDef WAVE_FrameDisplay(int OSCx);
ESTA_StatusTypeDef WAVE_CurveClear(int OSCx);
ESTA_StatusTypeDef WAVE_ReDraw(int OSCx);
ESTA_StatusTypeDef WAVE_CurveDraw(int OSCx, uint16_t data_CH[]);
ESTA_StatusTypeDef WAVE_CurveDrawBatch(int OSCx, int ch_idx,
                        const uint16_t *data, uint16_t count);

#endif
