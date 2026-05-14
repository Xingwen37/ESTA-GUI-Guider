#include "profile/ESTA_Profile.h"

#include <string.h>

static const ESTA_ProfileSet_TypeDef g_default_profiles = {
    .wave_inst_count = 2,
    .bar_inst_count = 1,
    .button_count = 4,
    .wave_profiles = {

        {
            .x_origin = 10,
            .y_origin = 0,
            .x_width = 200,
            .y_width = 120,
            .display_num_min = 0,
            .display_num_max = 4095,
            .channel_num = 4,
            .channel_mask = CH0 | CH1 | CH2 | CH3,
            .is_display_ruler_y = true,
            .ruler_y = { 1000, 2000, 3000, 4000, 0 },
            .ruler_count_y = 4,
            .ruler_num_digits_y = 4,
            .is_display_ruler_x = true,
            .ruler_x = { 30, 50, 70, 120, 0 },
            .ruler_count_x = 4,
            .ruler_zero_value_x = 0,
            .ruler_full_value_x = 100,
            .ruler_num_digits_x = 8,
            .theme_type = WAVE_THEME_LIGHT,
            .is_auto_clear = true,
            .is_use_batch_draw = true
        },

        {
            .x_origin = 0,
            .y_origin = 125,
            .x_width = 200,
            .y_width = 120,
            .display_num_min = 0,
            .display_num_max = 4095,
            .channel_num = 4,
            .channel_mask = CH0 | CH1 | CH2 | CH3,
            .is_display_ruler_y = true,
            .ruler_y = { 1000, 2000, 3000, 4000, 0 },
            .ruler_count_y = 4,
            .ruler_num_digits_y = 4,
            .is_display_ruler_x = true,
            .ruler_x = { 30, 50, 90, 0, 0 },
            .ruler_count_x = 3,
            .ruler_zero_value_x = 0,
            .ruler_full_value_x = 100,
            .ruler_num_digits_x = 8,
            .theme_type = WAVE_THEME_DEFAULT,
            .is_auto_clear = true,
            .is_use_batch_draw = false
        }

    },
    .bar_profiles = {

        {
            .x_origin = 200,
            .y_origin = 125,
            .x_width = 130,
            .y_width = 110,
            .display_num_min = 0,
            .display_num_max = 120,
            .bar_count = 6,
            .bar_width = 20,
            .bar_spacing = 0,
            .is_display_value = true,
            .is_display_axis = true,
            .theme_type = BARCHART_THEME_LIGHT
        }

    }
};

const ESTA_ProfileSet_TypeDef *ESTA_Profile_GetDefault(void) {
    return &g_default_profiles;
}

bool ESTA_Profile_ToConfig(const ESTA_WaveProfile_TypeDef *profile, WAVE_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));

    if (ESTA_ConfigSetPositionAndSize((ESTA_BaseConfig *)out_config, profile->x_origin,
                                      profile->y_origin, profile->x_width,
                                      profile->y_width) != ESTA_OK) return false;
    if (WAVE_ConfigSetDisplayRange(out_config, profile->display_num_min,
                                   profile->display_num_max) != ESTA_OK) return false;
    if (WAVE_ConfigSetChannelNum(out_config, profile->channel_num) != ESTA_OK) return false;
    if (WAVE_ConfigSetChannelEnabled(out_config, profile->channel_mask) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerY(out_config, profile->is_display_ruler_y,
                             (uint16_t *)profile->ruler_y, profile->ruler_count_y,
                             profile->ruler_num_digits_y) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerX(out_config, profile->is_display_ruler_x,
                             (uint16_t *)profile->ruler_x, profile->ruler_count_x,
                             profile->ruler_zero_value_x, profile->ruler_full_value_x,
                             profile->ruler_num_digits_x) != ESTA_OK) return false;
    if (WAVE_ConfigSetTheme(out_config, profile->theme_type) != ESTA_OK) return false;
    if (WAVE_ConfigSetAutoClear(out_config, profile->is_auto_clear) != ESTA_OK) return false;

    return true;
}

ESTA_StatusTypeDef ESTA_Profile_Apply(int inst_idx, const ESTA_WaveProfile_TypeDef *profile) {
    WAVE_Config_TypeDef config;
    if (!ESTA_Profile_ToConfig(profile, &config)) {
        return ESTA_ERROR;
    }
    return WAVE_Init(inst_idx, &config);
}

bool ESTA_Profile_ToBARCHART_Config(const ESTA_BarChartProfile_TypeDef *profile,
                                    BARCHART_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));

    if (ESTA_ConfigSetPositionAndSize((ESTA_BaseConfig *)out_config, profile->x_origin,
                                      profile->y_origin, profile->x_width,
                                      profile->y_width) != ESTA_OK) return false;
    if (BARCHART_ConfigSetDisplayRange(out_config, profile->display_num_min,
                                       profile->display_num_max) != ESTA_OK) return false;
    if (BARCHART_ConfigSetBarCount(out_config, profile->bar_count) != ESTA_OK) return false;
    if (BARCHART_ConfigSetBarLayout(out_config, profile->bar_width,
                                    profile->bar_spacing) != ESTA_OK) return false;
    if (BARCHART_ConfigSetDisplayOptions(out_config, profile->is_display_value,
                                         profile->is_display_axis) != ESTA_OK) return false;
    if (BARCHART_ConfigSetTheme(out_config, profile->theme_type) != ESTA_OK) return false;

    return true;
}

ESTA_StatusTypeDef ESTA_Profile_ApplyBARCHART(int inst_idx,
                                              const ESTA_BarChartProfile_TypeDef *profile) {
    BARCHART_Config_TypeDef config;
    if (!ESTA_Profile_ToBARCHART_Config(profile, &config)) {
        return ESTA_ERROR;
    }
    return BARCHART_Init(inst_idx, &config);
}

ESTA_StatusTypeDef ESTA_Profile_ApplyEvents(const ESTA_ProfileSet_TypeDef *profile_set) {
    (void)profile_set;
    ESTA_EventInit();
    return ESTA_OK;
}
