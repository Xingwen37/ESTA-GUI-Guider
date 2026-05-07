#include "ESTA_Profile.h"

#include <string.h>

static const ESTA_ProfileSet_TypeDef g_default_profiles = {
    .ESTA_count = 2,
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
            .theme_type = ESTA_THEME_LIGHT,
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
            .theme_type = ESTA_THEME_DEFAULT,
            .is_auto_clear = true
        }

    }
};

const ESTA_ProfileSet_TypeDef *ESTA_Profile_GetDefault(void) {
    return &g_default_profiles;
}

bool ESTA_Profile_ToConfig(const ESTA_Profile_TypeDef *profile, ESTA_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));

    ESTA_ConfigSetPositionAndSize(out_config, profile->x_origin, profile->y_origin,
                                 profile->x_width, profile->y_width);
    ESTA_ConfigSetDisplayRange(out_config, profile->display_num_min, profile->display_num_max);
    ESTA_ConfigSetChannelNum(out_config, profile->channel_num);
    ESTA_ConfigSetChannelEnabled(out_config, profile->channel_mask);
    ESTA_ConfigSetRulerY(out_config, profile->is_display_ruler_y, (uint16_t *)profile->ruler_y,
                        profile->ruler_count_y, profile->ruler_num_digits_y);
    ESTA_ConfigSetRulerX(out_config, profile->is_display_ruler_x, (uint16_t *)profile->ruler_x,
                        profile->ruler_count_x, profile->ruler_zero_value_x,
                        profile->ruler_full_value_x, profile->ruler_num_digits_x);
    ESTA_ConfigSetTheme(out_config, profile->theme_type);
    ESTA_ConfigSetAutoClear(out_config, profile->is_auto_clear);

    return true;
}

ESTA_StatusTypeDef ESTA_Profile_Apply(int ESTA_idx, const ESTA_Profile_TypeDef *profile) {
    ESTA_Config_TypeDef config;
    if (!ESTA_Profile_ToConfig(profile, &config)) {
        return ESTA_ERROR;
    }
    return ESTA_Init(ESTA_idx, &config);
}
