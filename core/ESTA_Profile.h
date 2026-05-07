#ifndef __ESTA_PROFILE_LIB
#define __ESTA_PROFILE_LIB

#include <stdbool.h>
#include <stdint.h>

#include "ESTA.h"

#define ESTA_PROFILE_MAX_INST 2

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
    uint16_t ruler_y[ESTA_MAX_RULER_Y_NUM];
    uint16_t ruler_count_y;
    uint16_t ruler_num_digits_y;

    bool is_display_ruler_x;
    uint16_t ruler_x[ESTA_MAX_RULER_X_NUM];
    uint16_t ruler_count_x;
    uint16_t ruler_zero_value_x;
    uint16_t ruler_full_value_x;
    uint16_t ruler_num_digits_x;

    ESTA_theme_type theme_type;
    bool is_auto_clear;
} ESTA_Profile_TypeDef;

typedef struct {
    uint16_t ESTA_count;
    ESTA_Profile_TypeDef profiles[ESTA_PROFILE_MAX_INST];
} ESTA_ProfileSet_TypeDef;

const ESTA_ProfileSet_TypeDef *ESTA_Profile_GetDefault(void);
bool ESTA_Profile_ToConfig(const ESTA_Profile_TypeDef *profile, ESTA_Config_TypeDef *out_config);
ESTA_StatusTypeDef ESTA_Profile_Apply(int ESTA_idx, const ESTA_Profile_TypeDef *profile);

#endif
