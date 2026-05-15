#ifndef __ESTA_PROFILE_LIB
#define __ESTA_PROFILE_LIB

#include <stdbool.h>
#include <stdint.h>

#include "ui/WAVE.h"
#include "ui/BARCHART.h"
#include "ui/TABLE.h"
#include "event/event.h"

#define ESTA_PROFILE_MAX_WAVE_INST 4
#define ESTA_PROFILE_MAX_BARCHART_INST 4
#define ESTA_PROFILE_MAX_TABLE_INST 4

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

    bool is_display_ruler_y;
    uint16_t ruler_y[WAVE_MAX_RULER_Y_NUM];
    WAVE_RulerLabel_TypeDef ruler_label_y[WAVE_MAX_RULER_Y_NUM];
    char ruler_unit_y[WAVE_MAX_RULER_UNIT_LEN + 1];
    uint8_t ruler_precision_y;
    uint16_t ruler_count_y;
    uint16_t ruler_num_digits_y;
    ESTA_FontSize ruler_font_size_y;

    bool is_display_ruler_x;
    uint16_t ruler_x[WAVE_MAX_RULER_X_NUM];
    WAVE_RulerLabel_TypeDef ruler_label_x[WAVE_MAX_RULER_X_NUM];
    char ruler_unit_x[WAVE_MAX_RULER_UNIT_LEN + 1];
    uint8_t ruler_precision_x;
    uint16_t ruler_count_x;
    uint16_t ruler_zero_value_x;
    uint16_t ruler_full_value_x;
    uint16_t ruler_num_digits_x;
    ESTA_FontSize ruler_font_size_x;

    WAVE_theme_type theme_type;
    bool is_auto_clear;
    bool is_use_batch_draw;
} ESTA_WaveProfile_TypeDef;

typedef struct {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t x_width;
    uint16_t y_width;

    uint16_t display_num_min;
    uint16_t display_num_max;

    uint16_t bar_count;
    uint16_t bar_width;
    uint16_t bar_spacing;

    bool is_display_value;
    bool is_display_axis;
    ESTA_FontSize font_size;

    BARCHART_theme_type theme_type;
} ESTA_BarChartProfile_TypeDef;

typedef struct {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t x_width;
    uint16_t y_width;

    uint16_t row_count;
    uint16_t row_height;
    uint16_t label_col_width;
    uint16_t value_col_width;
    uint16_t unit_col_width;
    bool is_auto_col_width;
    bool is_show_frame;
    bool is_show_row_line;
    bool is_fill_background;
    ESTA_FontSize font_size;
    TABLE_theme_type theme_type;
    TABLE_RowConfig_TypeDef rows[TABLE_MAX_ROWS];
} ESTA_TableProfile_TypeDef;

typedef struct {
    uint16_t wave_inst_count;
    uint16_t bar_inst_count;
    uint16_t table_inst_count;
    uint16_t button_count;
    ESTA_WaveProfile_TypeDef wave_profiles[ESTA_PROFILE_MAX_WAVE_INST];
    ESTA_BarChartProfile_TypeDef bar_profiles[ESTA_PROFILE_MAX_BARCHART_INST];
    ESTA_TableProfile_TypeDef table_profiles[ESTA_PROFILE_MAX_TABLE_INST];
} ESTA_ProfileSet_TypeDef;

const ESTA_ProfileSet_TypeDef *ESTA_Profile_GetDefault(void);
bool ESTA_Profile_ToConfig(const ESTA_WaveProfile_TypeDef *profile, WAVE_Config_TypeDef *out_config);
ESTA_StatusTypeDef ESTA_Profile_Apply(int inst_idx, const ESTA_WaveProfile_TypeDef *profile);
bool ESTA_Profile_ToBARCHART_Config(const ESTA_BarChartProfile_TypeDef *profile, BARCHART_Config_TypeDef *out_config);
ESTA_StatusTypeDef ESTA_Profile_ApplyBARCHART(int inst_idx, const ESTA_BarChartProfile_TypeDef *profile);
bool ESTA_Profile_ToTABLE_Config(const ESTA_TableProfile_TypeDef *profile, TABLE_Config_TypeDef *out_config);
ESTA_StatusTypeDef ESTA_Profile_ApplyTABLE(int inst_idx, const ESTA_TableProfile_TypeDef *profile);
ESTA_StatusTypeDef ESTA_Profile_ApplyEvents(const ESTA_ProfileSet_TypeDef *profile_set);

#endif
