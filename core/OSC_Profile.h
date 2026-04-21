#ifndef __OSC_PROFILE_LIB
#define __OSC_PROFILE_LIB

#include <stdbool.h>
#include <stdint.h>

#include "OSC.h"

#define OSC_PROFILE_MAX_INST 2

typedef struct {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t x_width;
    uint16_t y_width;

    uint16_t display_num_min;
    uint16_t display_num_max;

    uint16_t channel_num;
    uint8_t channel_mask;

    bool is_display_ruler_y;
    uint16_t ruler_y[OSC_MAX_RULER_Y_NUM];
    uint16_t ruler_count_y;
    uint16_t ruler_num_digits_y;

    bool is_display_ruler_x;
    uint16_t ruler_x[OSC_MAX_RULER_X_NUM];
    uint16_t ruler_count_x;
    uint16_t ruler_zero_value_x;
    uint16_t ruler_full_value_x;
    uint16_t ruler_num_digits_x;

    OSC_theme_type theme_type;
    bool is_auto_clear;
} OSC_Profile_TypeDef;

typedef struct {
    uint16_t osc_count;
    OSC_Profile_TypeDef profiles[OSC_PROFILE_MAX_INST];
} OSC_ProfileSet_TypeDef;

const OSC_ProfileSet_TypeDef *OSC_Profile_GetDefault(void);
bool OSC_Profile_ToConfig(const OSC_Profile_TypeDef *profile, OSC_Config_TypeDef *out_config);
OSC_StatusTypeDef OSC_Profile_Apply(int osc_idx, const OSC_Profile_TypeDef *profile);

#endif
