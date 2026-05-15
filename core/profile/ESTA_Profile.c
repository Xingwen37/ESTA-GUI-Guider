#include "profile/ESTA_Profile.h"

#include <string.h>

static const ESTA_ProfileSet_TypeDef g_default_profiles = {
    .wave_inst_count = 1,
    .bar_inst_count = 1,
    .table_inst_count = 1,
    .button_count = 4,
    .wave_profiles = {

        {
            .x_origin = 0,
            .y_origin = 0,
            .x_width = 400,
            .y_width = 256,
            .display_num_min = 0,
            .display_num_max = 256,
            .x_scale = 2,
            .channel_num = 4,
            .channel_mask = CH0 | CH1 | CH2 | CH3,
            .is_display_ruler_y = true,
            .ruler_y = { 0, 32, 64, 96, 128, 160, 192, 224, 256, 0 },
            .ruler_label_y = {

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = -128, .float_value = -128 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = -96, .float_value = -96 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = -64, .float_value = -64 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = -32, .float_value = -32 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 32, .float_value = 32 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 64, .float_value = 64 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 96, .float_value = 96 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 128, .float_value = 128 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 }

            },
            .ruler_unit_y = "mV",
            .ruler_precision_y = 0,
            .ruler_count_y = 9,
            .ruler_num_digits_y = 4,
            .ruler_font_size_y = ESTA_FONT_1206,
            .is_display_ruler_x = true,
            .ruler_x = { 20, 40, 60, 80, 100, 120, 140, 160, 180, 200 },
            .ruler_label_x = {

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = -1 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = -0.75 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = -0.5 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = -0.25 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = 0.25 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = 0.5 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = 0.75 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = 1 },

                { .value_type = WAVE_RULER_LABEL_FLOAT, .int_value = 0, .float_value = 1.25 }

            },
            .ruler_unit_x = "ms",
            .ruler_precision_x = 2,
            .ruler_count_x = 10,
            .ruler_zero_value_x = 0,
            .ruler_full_value_x = 200,
            .ruler_num_digits_x = 3,
            .ruler_font_size_x = ESTA_FONT_1206,
            .theme_type = WAVE_THEME_LIGHT,
            .is_auto_clear = true,
            .is_use_batch_draw = false
        }

    },
    .bar_profiles = {

        {
            .x_origin = 300,
            .y_origin = 301,
            .x_width = 130,
            .y_width = 110,
            .display_num_min = 0,
            .display_num_max = 120,
            .bar_count = 6,
            .bar_width = 20,
            .bar_spacing = 0,
            .is_display_value = true,
            .is_display_axis = true,
            .font_size = ESTA_FONT_1608,
            .theme_type = BARCHART_THEME_LIGHT
        }

    },
    .table_profiles = {

        {
            .x_origin = 210,
            .y_origin = 300,
            .x_width = 110,
            .y_width = 72,
            .row_count = 2,
            .row_height = 20,
            .label_col_width = 32,
            .value_col_width = 48,
            .unit_col_width = 24,
            .is_auto_col_width = true,
            .is_show_frame = true,
            .is_show_row_line = false,
            .is_fill_background = true,
            .font_size = ESTA_FONT_1206,
            .theme_type = TABLE_THEME_DEFAULT,
            .rows = {

                {
                    .label = "Vpp",
                    .value_kind = TABLE_VALUE_NUMBER,
                    .number_type = TABLE_NUMBER_UINT32,
                    .unit = "mV",
                    .precision = 0,
                    .default_u32 = 1000,
                    .default_float = 0,
                    .default_text = ""
                },

                {
                    .label = "Fre",
                    .value_kind = TABLE_VALUE_NUMBER,
                    .number_type = TABLE_NUMBER_UINT32,
                    .unit = "Hz",
                    .precision = 0,
                    .default_u32 = 1230,
                    .default_float = 0,
                    .default_text = ""
                }

            }
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
    if (WAVE_ConfigSetXScale(out_config, profile->x_scale) != ESTA_OK) return false;
    if (WAVE_ConfigSetChannelNum(out_config, profile->channel_num) != ESTA_OK) return false;
    if (WAVE_ConfigSetChannelEnabled(out_config, profile->channel_mask) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerY(out_config, profile->is_display_ruler_y,
                             (uint16_t *)profile->ruler_y, profile->ruler_count_y,
                             profile->ruler_num_digits_y) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerX(out_config, profile->is_display_ruler_x,
                             (uint16_t *)profile->ruler_x, profile->ruler_count_x,
                             profile->ruler_zero_value_x, profile->ruler_full_value_x,
                             profile->ruler_num_digits_x) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerLabelY(out_config, profile->ruler_label_y,
                                  profile->ruler_unit_y,
                                  profile->ruler_precision_y) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerLabelX(out_config, profile->ruler_label_x,
                                  profile->ruler_unit_x,
                                  profile->ruler_precision_x) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerFontSize(out_config, profile->ruler_font_size_x,
                                    profile->ruler_font_size_y) != ESTA_OK) return false;
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
    if (BARCHART_ConfigSetFontSize(out_config, profile->font_size) != ESTA_OK) return false;
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

bool ESTA_Profile_ToTABLE_Config(const ESTA_TableProfile_TypeDef *profile,
                                 TABLE_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));

    if (ESTA_ConfigSetPositionAndSize((ESTA_BaseConfig *)out_config, profile->x_origin,
                                      profile->y_origin, profile->x_width,
                                      profile->y_width) != ESTA_OK) return false;
    if (TABLE_ConfigSetRows(out_config, profile->rows, profile->row_count) != ESTA_OK) return false;
    if (TABLE_ConfigSetLayout(out_config, profile->row_height, profile->label_col_width,
                              profile->value_col_width, profile->unit_col_width,
                              profile->is_auto_col_width) != ESTA_OK) return false;
    if (TABLE_ConfigSetDisplayOptions(out_config, profile->is_show_frame,
                                      profile->is_show_row_line,
                                      profile->is_fill_background) != ESTA_OK) return false;
    if (TABLE_ConfigSetFontSize(out_config, profile->font_size) != ESTA_OK) return false;
    if (TABLE_ConfigSetTheme(out_config, profile->theme_type) != ESTA_OK) return false;

    return true;
}

ESTA_StatusTypeDef ESTA_Profile_ApplyTABLE(int inst_idx,
                                           const ESTA_TableProfile_TypeDef *profile) {
    TABLE_Config_TypeDef config;
    if (!ESTA_Profile_ToTABLE_Config(profile, &config)) {
        return ESTA_ERROR;
    }
    return TABLE_Init(inst_idx, &config);
}

ESTA_StatusTypeDef ESTA_Profile_ApplyEvents(const ESTA_ProfileSet_TypeDef *profile_set) {
    (void)profile_set;
    ESTA_EventInit();
    return ESTA_OK;
}
