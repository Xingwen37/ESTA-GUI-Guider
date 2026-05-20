#ifndef __WAVE_LIB
#define __WAVE_LIB

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "infra/helper.h"
#include "infra/ui_base.h"
#include "infra/ui_theme.h"

#define MAX_WAVE_NUM 4
#define MAX_WAVE_CHANNEL 4

#define WAVE_MAX_RULER_Y_NUM 11
#define WAVE_MAX_RULER_X_NUM 11
#define WAVE_MAX_RULER_UNIT_LEN 16
#define WAVE_MAX_RULER_LABEL_TEXT_LEN 32

#define WAVE_INST(i) (i)
#define WAVE_INST_ADDR(i) WAVE_State[WAVE_INST(i)]

typedef enum {
    WAVE_THEME_DEFAULT = 0,
    WAVE_THEME_LIGHT,
    WAVE_THEME_COUNT
} WAVE_theme_type;

typedef enum {
    WAVE_THEME_FRAME_INDEX = 0,
    WAVE_THEME_RULER_INDEX,
    WAVE_THEME_WAVE_CH0_INDEX,
    WAVE_THEME_WAVE_CH1_INDEX,
    WAVE_THEME_WAVE_CH2_INDEX,
    WAVE_THEME_WAVE_CH3_INDEX,
    WAVE_THEME_BACKGROUND_INDEX,
    WAVE_THEME_INDEX_COUNT
} WAVE_theme_color_index_type;

typedef enum {
    WAVE_RULER_LABEL_INT = 0,
    WAVE_RULER_LABEL_FLOAT,
    WAVE_RULER_LABEL_TYPE_COUNT
} WAVE_RulerLabelValueType;

typedef struct {
    float value;
} WAVE_RulerLabel_TypeDef;

#define WAVE_CONFIG_MEMBER(OSCx, reg_name) \
    WAVE_INST_ADDR(OSCx).WAVE_Config.reg_name
#define WAVE_PRIVATE_MEMBER(OSCx, reg_name) \
    WAVE_INST_ADDR(OSCx).WAVE_Private.reg_name

#define WAVE_CONFIG_MEMBER_ARRAY(OSCx, reg_name, NO) \
    WAVE_INST_ADDR(OSCx).WAVE_Config.reg_name[NO]
#define WAVE_PRIVATE_MEMBER_ARRAY(OSCx, reg_name, NO) \
    WAVE_INST_ADDR(OSCx).WAVE_Private.reg_name[NO]

#define WAVE_WRITE_CONFIG(OSCx, reg_name, reg_value) \
    WAVE_INST_ADDR(OSCx).WAVE_Config.reg_name = reg_value
#define WAVE_WRITE_PRIVATE(OSCx, reg_name, reg_value) \
    WAVE_INST_ADDR(OSCx).WAVE_Private.reg_name = reg_value

#define WAVE_WRITE_CONFIG_INIT(OSCx, reg_name) \
    WAVE_INST_ADDR(OSCx).WAVE_Config.reg_name = WAVE_Init->reg_name

#define IS_VALID_WAVE_INST(OSCx) ((int)OSCx < MAX_WAVE_NUM && (int)OSCx >= 0)
#define IS_VALID_THEME(THEMEx) ((int)THEMEx < WAVE_THEME_COUNT && (int)THEMEx >= 0)
#define IS_VALID_CHNUM(CHNUM) ((int)CHNUM < MAX_WAVE_CHANNEL && (int)CHNUM >= 0)
#define IS_VALID_WAVE_RULER_LABEL_TYPE(TYPE) \
    ((int)(TYPE) >= 0 && (int)(TYPE) < WAVE_RULER_LABEL_TYPE_COUNT)

typedef struct {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t x_width;
    uint16_t y_width;

    uint16_t display_num_min;
    uint16_t display_num_max;
    uint16_t x_scale;

    uint16_t channel_num;
    uint8_t channel_mask;

    volatile bool is_display_ruler_x;
    volatile bool is_display_ruler_y;

    uint16_t *ruler_y;
    const WAVE_RulerLabel_TypeDef *ruler_label_y;
    const char *ruler_unit_y;
    uint8_t ruler_precision_y;
    uint16_t ruler_count_y;

    uint16_t *ruler_x;
    const WAVE_RulerLabel_TypeDef *ruler_label_x;
    const char *ruler_unit_x;
    uint8_t ruler_precision_x;
    uint16_t ruler_count_x;
    uint16_t ruler_zero_value_x;
    uint16_t ruler_full_value_x;

    ESTA_FontSize ruler_font_size;
    volatile bool is_auto_clear;
    WAVE_theme_type theme_type;
} WAVE_Config_TypeDef;

typedef struct {
    uint16_t ruler_buff_y[WAVE_MAX_RULER_Y_NUM];
    uint16_t ruler_buff_x[WAVE_MAX_RULER_X_NUM];
    WAVE_RulerLabel_TypeDef ruler_label_buff_y[WAVE_MAX_RULER_Y_NUM];
    WAVE_RulerLabel_TypeDef ruler_label_buff_x[WAVE_MAX_RULER_X_NUM];
    char ruler_unit_buff_y[WAVE_MAX_RULER_UNIT_LEN + 1];
    char ruler_unit_buff_x[WAVE_MAX_RULER_UNIT_LEN + 1];
    uint16_t last_index;
    uint16_t x_coor_last;
    uint16_t y_coor_last_CH[MAX_WAVE_CHANNEL];
} WAVE_Private_Typedef;

typedef struct {
    WAVE_Config_TypeDef WAVE_Config;
    WAVE_Private_Typedef WAVE_Private;
} WAVE_TypeDef;

extern WAVE_TypeDef WAVE_State[MAX_WAVE_NUM];

ESTA_StatusTypeDef WAVE_ConfigSetDisplayRange(WAVE_Config_TypeDef *config,
    uint16_t display_num_min, uint16_t display_num_max);
ESTA_StatusTypeDef WAVE_ConfigSetXScale(WAVE_Config_TypeDef *config,
    uint16_t x_scale);
ESTA_StatusTypeDef WAVE_ConfigSetChannelNum(WAVE_Config_TypeDef *config,
    uint16_t channel_num);
ESTA_StatusTypeDef WAVE_ConfigSetChannelEnabled(WAVE_Config_TypeDef *config,
    uint8_t channel_mask);
ESTA_StatusTypeDef WAVE_ConfigSetChannelDisabled(WAVE_Config_TypeDef *config,
    uint8_t channel_mask);
ESTA_StatusTypeDef WAVE_ConfigSetRulerY(WAVE_Config_TypeDef *config,
    bool is_display, uint16_t *ruler_y, uint16_t ruler_count_y);
ESTA_StatusTypeDef WAVE_ConfigSetRulerX(WAVE_Config_TypeDef *config,
    bool is_display, uint16_t *ruler_x, uint16_t ruler_count_x,
    uint16_t ruler_zero_value_x, uint16_t ruler_full_value_x);
ESTA_StatusTypeDef WAVE_ConfigSetRulerLabelY(WAVE_Config_TypeDef *config,
    const WAVE_RulerLabel_TypeDef *ruler_label_y, const char *ruler_unit_y,
    uint8_t ruler_precision_y);
ESTA_StatusTypeDef WAVE_ConfigSetRulerLabelX(WAVE_Config_TypeDef *config,
    const WAVE_RulerLabel_TypeDef *ruler_label_x, const char *ruler_unit_x,
    uint8_t ruler_precision_x);
ESTA_StatusTypeDef WAVE_ConfigSetRulerFontSize(WAVE_Config_TypeDef *config,
    ESTA_FontSize font_size);
ESTA_StatusTypeDef WAVE_ConfigSetTheme(WAVE_Config_TypeDef *config,
    WAVE_theme_type theme_type);
ESTA_StatusTypeDef WAVE_ConfigSetAutoClear(WAVE_Config_TypeDef *config,
    bool is_auto_clear);

ESTA_StatusTypeDef WAVE_Init(int OSCx, WAVE_Config_TypeDef *WAVE_Init);
ESTA_StatusTypeDef WAVE_DeInit(int OSCx);
ESTA_StatusTypeDef WAVE_RulerDisplay(int OSCx);
ESTA_StatusTypeDef WAVE_FrameDisplay(int OSCx);
ESTA_StatusTypeDef WAVE_CurveClear(int OSCx);
ESTA_StatusTypeDef WAVE_ReDraw(int OSCx);
ESTA_StatusTypeDef WAVE_UpdateRulerUnit(int OSCx, uint8_t axis, const char *unit);
ESTA_StatusTypeDef WAVE_CurveDraw(int OSCx, uint16_t data_CH[]);
ESTA_StatusTypeDef WAVE_CurveDrawBatch(int OSCx, int ch_idx,
    const uint16_t *data, uint16_t count);
uint16_t WAVE_GetPlotWidth(int OSCx);
uint16_t WAVE_GetPlotHeight(int OSCx);
uint16_t WAVE_GetSampleCapacity(int OSCx);

#endif
