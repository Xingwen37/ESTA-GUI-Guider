#include "profile/ESTA_Profile.h"

#include <string.h>

static uint8_t g_active_page = 0;

static const ESTA_ProfileSet_TypeDef g_default_profiles = {
    .wave_inst_count = 1,
    .bar_inst_count = 1,
    .table_inst_count = 2,
    .menu_inst_count = 1,
    .button_count = 6,
    .page_count = 2,
    .wave_profiles = {

        {
            .x_origin = 0,
            .y_origin = 0,
            .x_width = 200,
            .y_width = 256,
            .display_num_min = 0,
            .display_num_max = 256,
            .x_scale = 1,
            .channel_num = 4,
            .channel_mask = CH0 | CH1 | CH2 | CH3,
            .is_display_ruler_y = true,
            .ruler_y = { 0, 32, 64, 96, 128, 160, 192, 224, 256, 0, 0 },
            .ruler_label_y = {

                { .value = -4 },

                { .value = -3 },

                { .value = -2 },

                { .value = -1 },

                { .value = 0 },

                { .value = 1 },

                { .value = 2 },

                { .value = 3 },

                { .value = 4 },

                { .value = 0 },

                { .value = 0 }

            },
            .ruler_unit_y = "0.1V",
            .ruler_precision_y = 0,
            .ruler_count_y = 9,
            .is_display_ruler_x = true,
            .ruler_x = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 },
            .ruler_label_x = {

                { .value = 0 },

                { .value = 1 },

                { .value = 2 },

                { .value = 3 },

                { .value = 4 },

                { .value = 5 },

                { .value = 6 },

                { .value = 7 },

                { .value = 8 },

                { .value = 9 },

                { .value = 10 }

            },
            .ruler_unit_x = "ms",
            .ruler_precision_x = 0,
            .ruler_count_x = 11,
            .ruler_zero_value_x = 0,
            .ruler_full_value_x = 100,
            .ruler_font_size = ESTA_FONT_1206,
            .theme_type = WAVE_THEME_DEFAULT,
            .is_auto_clear = true,
            .is_use_batch_draw = false,
            .page = 0
        }

    },
    .bar_profiles = {

        {
            .x_origin = 0,
            .y_origin = 76,
            .x_width = 93,
            .y_width = 76,
            .display_num_min = 0,
            .display_num_max = 100,
            .bar_count = 6,
            .bar_width = 10,
            .bar_spacing = 5,
            .is_display_value = true,
            .is_display_axis = true,
            .font_size = ESTA_FONT_1206,
            .theme_type = BARCHART_THEME_DEFAULT,
            .page = 1
        }

    },
    .table_profiles = {

        {
            .x_origin = 0,
            .y_origin = 260,
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

            },
            .page = 0
        },

        {
            .x_origin = 0,
            .y_origin = 0,
            .x_width = 110,
            .y_width = 72,
            .row_count = 2,
            .col_count = 3,
            .row_height = 20,
            .is_show_header = false,
            .is_show_frame = true,
            .is_show_row_line = false,
            .is_show_col_line = false,
            .is_fill_background = true,
            .font_size = ESTA_FONT_1608,
            .theme_type = TABLE_THEME_LIGHT,
            .cols = {

                { .header = "Name", .cell_type = TABLE_CELL_TEXT, .width = 0, .precision = 0 },

                { .header = "Value", .cell_type = TABLE_CELL_UINT32, .width = 0, .precision = 0 },

                { .header = "Unit", .cell_type = TABLE_CELL_TEXT, .width = 0, .precision = 0 }

            },
            .default_cells = {

                { { .text = "Vpp" }, { .u32 = 1000 }, { .text = "mV" } },

                { { .text = "Fre" }, { .u32 = 1230 }, { .text = "Hz" } }

            },
            .page = 1
        }

    },
    .menu_profiles = {

        {
            .x_origin = 114,
            .y_origin = 0,
            .x_width = 78,
            .y_width = 64,
            .item_count = 8,
            .item_height = 16,
            .breadcrumb_height = 0,
            .is_show_frame = true,
            .is_show_breadcrumb = false,
            .is_fill_background = true,
            .font_size = ESTA_FONT_1206,
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

            },
            .page = 1
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
                             (uint16_t *)profile->ruler_y, profile->ruler_count_y) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerX(out_config, profile->is_display_ruler_x,
                             (uint16_t *)profile->ruler_x, profile->ruler_count_x,
                             profile->ruler_zero_value_x, profile->ruler_full_value_x) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerLabelY(out_config, profile->ruler_label_y,
                                  profile->ruler_unit_y,
                                  profile->ruler_precision_y) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerLabelX(out_config, profile->ruler_label_x,
                                  profile->ruler_unit_x,
                                  profile->ruler_precision_x) != ESTA_OK) return false;
    if (WAVE_ConfigSetRulerFontSize(out_config, profile->ruler_font_size) != ESTA_OK) return false;
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

void ESTA_Profile_SetActivePage(uint8_t page) {
    g_active_page = page;
}

uint8_t ESTA_Profile_GetActivePage(void) {
    return g_active_page;
}

ESTA_StatusTypeDef ESTA_Profile_ApplyPage(const ESTA_ProfileSet_TypeDef *profile_set, uint8_t page) {
    if (profile_set == NULL) return ESTA_ERROR;
    g_active_page = page;

    for (uint16_t i = 0; i < profile_set->wave_inst_count; i++) {
        if (profile_set->wave_profiles[i].page == page) {
            ESTA_StatusTypeDef ret = ESTA_Profile_Apply(i, &profile_set->wave_profiles[i]);
            if (ret != ESTA_OK) return ret;
        }
    }
    for (uint16_t i = 0; i < profile_set->bar_inst_count; i++) {
        if (profile_set->bar_profiles[i].page == page) {
            ESTA_StatusTypeDef ret = ESTA_Profile_ApplyBARCHART(i, &profile_set->bar_profiles[i]);
            if (ret != ESTA_OK) return ret;
        }
    }
    for (uint16_t i = 0; i < profile_set->table_inst_count; i++) {
        if (profile_set->table_profiles[i].page == page) {
            ESTA_StatusTypeDef ret = ESTA_Profile_ApplyTABLE(i, &profile_set->table_profiles[i]);
            if (ret != ESTA_OK) return ret;
        }
    }
    for (uint16_t i = 0; i < profile_set->menu_inst_count; i++) {
        if (profile_set->menu_profiles[i].page == page) {
            ESTA_StatusTypeDef ret = ESTA_Profile_ApplyMENU(i, &profile_set->menu_profiles[i]);
            if (ret != ESTA_OK) return ret;
        }
    }
    return ESTA_OK;
}
