#include "ESTA_Profile.h"

#include <string.h>

static const ESTA_ProfileSet_TypeDef g_default_profiles = {
    .inst_count = 2,
    .profiles = {

        {
            .x_origin = 10,
            .y_origin = 0,
            .x_width = 200,
            .y_width = 120,
            .display_num_min = 0,
            .display_num_max = 4095,
            .channel_num = 4,
            .channel_mask = CH0 | CH1 | CH3,
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
            .is_auto_clear = true
        },

        {
            .x_origin = 0,
            .y_origin = 120,
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
            .ruler_x = { 30, 40, 90, 0, 0 },
            .ruler_count_x = 3,
            .ruler_zero_value_x = 0,
            .ruler_full_value_x = 100,
            .ruler_num_digits_x = 8,
            .theme_type = WAVE_THEME_DEFAULT,
            .is_auto_clear = true
        }

    }
};

const ESTA_ProfileSet_TypeDef *ESTA_Profile_GetDefault(void) {
    return &g_default_profiles;
}

bool ESTA_Profile_ToConfig(const ESTA_Profile_TypeDef *profile, WAVE_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));

    WAVE_ConfigSetPositionAndSize(out_config, profile->x_origin, profile->y_origin,
                                 profile->x_width, profile->y_width);
    WAVE_ConfigSetDisplayRange(out_config, profile->display_num_min, profile->display_num_max);
    WAVE_ConfigSetChannelNum(out_config, profile->channel_num);
    WAVE_ConfigSetChannelEnabled(out_config, profile->channel_mask);
    WAVE_ConfigSetRulerY(out_config, profile->is_display_ruler_y, (uint16_t *)profile->ruler_y,
                        profile->ruler_count_y, profile->ruler_num_digits_y);
    WAVE_ConfigSetRulerX(out_config, profile->is_display_ruler_x, (uint16_t *)profile->ruler_x,
                        profile->ruler_count_x, profile->ruler_zero_value_x,
                        profile->ruler_full_value_x, profile->ruler_num_digits_x);
    WAVE_ConfigSetTheme(out_config, profile->theme_type);
    WAVE_ConfigSetAutoClear(out_config, profile->is_auto_clear);

    return true;
}

WAVE_StatusTypeDef ESTA_Profile_Apply(int inst_idx, const ESTA_Profile_TypeDef *profile) {
    WAVE_Config_TypeDef config;
    if (!ESTA_Profile_ToConfig(profile, &config)) {
        return WAVE_ERROR;
    }
    return WAVE_Init(inst_idx, &config);
}
