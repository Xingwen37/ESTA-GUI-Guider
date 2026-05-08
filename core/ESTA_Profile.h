#ifndef __ESTA_PROFILE_LIB
#define __ESTA_PROFILE_LIB

#include <stdbool.h>
#include <stdint.h>

#include "WAVE.h"
#include "BARCHART.h"

#define ESTA_PROFILE_MAX_INST 2

typedef struct {
    /* ---- WAVE 组件字段 ---- */
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t x_width;
    uint16_t y_width;

    uint16_t display_num_min;
    uint16_t display_num_max;

    uint16_t channel_num;
    uint8_t channel_mask;

    bool is_display_ruler_y;
    uint16_t ruler_y[WAVE_MAX_RULER_Y_NUM];
    uint16_t ruler_count_y;
    uint16_t ruler_num_digits_y;

    bool is_display_ruler_x;
    uint16_t ruler_x[WAVE_MAX_RULER_X_NUM];
    uint16_t ruler_count_x;
    uint16_t ruler_zero_value_x;
    uint16_t ruler_full_value_x;
    uint16_t ruler_num_digits_x;

    WAVE_theme_type theme_type;
    bool is_auto_clear;

    /* ---- BARCHART 组件字段 ---- */
    uint16_t bar_x_origin;
    uint16_t bar_y_origin;
    uint16_t bar_x_width;
    uint16_t bar_y_width;

    uint16_t bar_display_num_min;
    uint16_t bar_display_num_max;

    uint16_t bar_count;
    uint16_t bar_width;
    uint16_t bar_spacing;

    bool bar_is_display_value;
    bool bar_is_display_axis;

    BARCHART_theme_type bar_theme_type;
} ESTA_Profile_TypeDef;

typedef struct {
    uint16_t inst_count;
    ESTA_Profile_TypeDef profiles[ESTA_PROFILE_MAX_INST];
} ESTA_ProfileSet_TypeDef;

const ESTA_ProfileSet_TypeDef *ESTA_Profile_GetDefault(void);
bool ESTA_Profile_ToConfig(const ESTA_Profile_TypeDef *profile, WAVE_Config_TypeDef *out_config);
ESTA_StatusTypeDef ESTA_Profile_Apply(int inst_idx, const ESTA_Profile_TypeDef *profile);
bool ESTA_Profile_ToBARCHART_Config(const ESTA_Profile_TypeDef *profile, BARCHART_Config_TypeDef *out_config);
ESTA_StatusTypeDef ESTA_Profile_ApplyBARCHART(int inst_idx, const ESTA_Profile_TypeDef *profile);

#endif
