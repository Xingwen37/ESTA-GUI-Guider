#include "profile/ESTA_Profile.h"

#include <string.h>

static const ESTA_ProfileSet_TypeDef g_default_profiles = {
    .wave_inst_count = 1,
    .bar_inst_count = 1,
    .table_inst_count = 1,
    .menu_inst_count = 1,
    .button_count = 4,
    .wave_profiles = {

        {
            .x_origin = 0,
            .y_origin = 0,
            .x_width = 200,
            .y_width = 120,
            .display_num_min = 0,
            .display_num_max = 4095,
            .x_scale = 1,
            .channel_num = 4,
            .channel_mask = CH0 | CH1 | CH2 | CH3,
            .is_display_ruler_y = true,
            .ruler_y = { 1000, 2000, 3000, 4000, 0, 0, 0, 0, 0, 0 },
            .ruler_label_y = {

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 1000, .float_value = 1000 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 2000, .float_value = 2000 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 3000, .float_value = 3000 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 4000, .float_value = 4000 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 }

            },
            .ruler_unit_y = "",
            .ruler_precision_y = 0,
            .ruler_count_y = 4,
            .ruler_num_digits_y = 4,
            .ruler_font_size_y = ESTA_FONT_1608,
            .is_display_ruler_x = true,
            .ruler_x = { 30, 50, 90, 0, 0, 0, 0, 0, 0, 0 },
            .ruler_label_x = {

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 30, .float_value = 30 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 50, .float_value = 50 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 90, .float_value = 90 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 },

                { .value_type = WAVE_RULER_LABEL_INT, .int_value = 0, .float_value = 0 }

            },
            .ruler_unit_x = "",
            .ruler_precision_x = 0,
            .ruler_count_x = 3,
            .ruler_zero_value_x = 0,
            .ruler_full_value_x = 100,
            .ruler_num_digits_x = 8,
            .ruler_font_size_x = ESTA_FONT_1608,
            .theme_type = WAVE_THEME_DEFAULT,
            .is_auto_clear = true,
            .is_use_batch_draw = false
        }

    },
    .bar_profiles = {

        {
            .x_origin = 200,
            .y_origin = 230,
            .x_width = 93,
            .y_width = 88,
            .display_num_min = 0,
            .display_num_max = 7,
            .bar_count = 6,
            .bar_width = 10,
            .bar_spacing = 5,
            .is_display_value = true,
            .is_display_axis = true,
            .font_size = ESTA_FONT_1206,
            .theme_type = BARCHART_THEME_DEFAULT
        }

    },
    .table_profiles = {

        {
            .x_origin = 200,
            .y_origin = 230,
            .x_width = 168,
            .y_width = 48,
            .row_count = 2,
            .col_count = 5,
            .row_height = 16,
            .is_show_header = true,
            .is_show_frame = true,
            .is_show_row_line = false,
            .is_show_col_line = false,
            .is_fill_background = true,
            .font_size = ESTA_FONT_1206,
            .theme_type = TABLE_THEME_DEFAULT,
            .cols = {

                { .header = "WAVE", .cell_type = TABLE_CELL_TEXT, .width = 0, .precision = 0 },

                { .header = "Value", .cell_type = TABLE_CELL_UINT32, .width = 0, .precision = 0 },

                { .header = "uint", .cell_type = TABLE_CELL_TEXT, .width = 0, .precision = 0 },

                { .header = "Value", .cell_type = TABLE_CELL_UINT32, .width = 0, .precision = 0 },

                { .header = "uint", .cell_type = TABLE_CELL_TEXT, .width = 0, .precision = 0 }

            },
            .default_cells = {

                { { .text = "Wave1" }, { .u32 = 1000 }, { .text = "mV" }, { .u32 = 1000 }, { .text = "Hz" } },

                { { .text = "Wave2" }, { .u32 = 1230 }, { .text = "mV" }, { .u32 = 2000 }, { .text = "Hz" } }

            }
        }

    },
    .menu_profiles = {

        {
            .x_origin = 10,
            .y_origin = 230,
            .x_width = 98,
            .y_width = 80,
            .item_count = 8,
            .item_height = 20,
            .breadcrumb_height = 0,
            .is_show_frame = true,
            .is_show_breadcrumb = false,
            .is_fill_background = true,
            .font_size = ESTA_FONT_1608,
            .theme_type = MENU_THEME_DEFAULT,
            .items = {

                {
                    .label = "Settings",
                    .parent_idx = 255,
                    .is_submenu = true,
                    .event_id = 0
                },

                {
                    .label = "Display",
                    .parent_idx = 0,
                    .is_submenu = true,
                    .event_id = 0
                },

                {
                    .label = "Brightness",
                    .parent_idx = 1,
                    .is_submenu = false,
                    .event_id = 10
                },

                {
                    .label = "Background",
                    .parent_idx = 255,
                    .is_submenu = true,
                    .event_id = 0
                },

                {
                    .label = "defult",
                    .parent_idx = 3,
                    .is_submenu = false,
                    .event_id = 0
                },

                {
                    .label = "light",
                    .parent_idx = 3,
                    .is_submenu = false,
                    .event_id = 0
                },

                {
                    .label = "Item",
                    .parent_idx = 255,
                    .is_submenu = false,
                    .event_id = 0
                },

                {
                    .label = "Item",
                    .parent_idx = 255,
                    .is_submenu = false,
                    .event_id = 0
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
    if (TABLE_ConfigSetCols(out_config, profile->cols, profile->col_count) != ESTA_OK) return false;
    if (TABLE_ConfigSetRowCount(out_config, profile->row_count) != ESTA_OK) return false;
    if (TABLE_ConfigSetRowHeight(out_config, profile->row_height) != ESTA_OK) return false;
    if (TABLE_ConfigSetDisplayOptions(out_config, profile->is_show_header,
                                      profile->is_show_frame, profile->is_show_row_line,
                                      profile->is_show_col_line,
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
    ESTA_StatusTypeDef ret = TABLE_Init(inst_idx, &config);
    if (ret != ESTA_OK) return ret;

    for (uint16_t r = 0; r < profile->row_count; r++) {
        for (uint16_t c = 0; c < profile->col_count; c++) {
            TABLE_UpdateCell(inst_idx, r, c, &profile->default_cells[r][c]);
        }
    }
    return ESTA_OK;
}

bool ESTA_Profile_ToMENU_Config(const ESTA_MenuProfile_TypeDef *profile,
                                MENU_Config_TypeDef *out_config) {
    if (profile == NULL || out_config == NULL) return false;
    memset(out_config, 0, sizeof(*out_config));

    if (ESTA_ConfigSetPositionAndSize((ESTA_BaseConfig *)out_config, profile->x_origin,
                                      profile->y_origin, profile->x_width,
                                      profile->y_width) != ESTA_OK) return false;
    if (MENU_ConfigSetItems(out_config, profile->items, profile->item_count) != ESTA_OK) return false;
    if (MENU_ConfigSetItemHeight(out_config, profile->item_height) != ESTA_OK) return false;
    if (MENU_ConfigSetBreadcrumbHeight(out_config, profile->breadcrumb_height) != ESTA_OK) return false;
    if (MENU_ConfigSetDisplayOptions(out_config, profile->is_show_frame,
                                     profile->is_show_breadcrumb,
                                     profile->is_fill_background) != ESTA_OK) return false;
    if (MENU_ConfigSetFontSize(out_config, profile->font_size) != ESTA_OK) return false;
    if (MENU_ConfigSetTheme(out_config, profile->theme_type) != ESTA_OK) return false;

    return true;
}

ESTA_StatusTypeDef ESTA_Profile_ApplyMENU(int inst_idx,
                                           const ESTA_MenuProfile_TypeDef *profile) {
    MENU_Config_TypeDef config;
    if (!ESTA_Profile_ToMENU_Config(profile, &config)) {
        return ESTA_ERROR;
    }
    return MENU_Init(inst_idx, &config);
}

ESTA_StatusTypeDef ESTA_Profile_ApplyEvents(const ESTA_ProfileSet_TypeDef *profile_set) {
    (void)profile_set;
    ESTA_EventInit();
    return ESTA_OK;
}
