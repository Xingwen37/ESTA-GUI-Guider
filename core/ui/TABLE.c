#include "ui/TABLE.h"
#include <string.h>

TABLE_TypeDef TABLE_State[TABLE_MAX_NUM];

static const uint16_t TABLE_ColorTable[TABLE_THEME_COUNT][TABLE_THEME_INDEX_COUNT] = {
    [TABLE_THEME_DEFAULT] = {
        [TABLE_THEME_FRAME_INDEX]      = __WHITE,
        [TABLE_THEME_BACKGROUND_INDEX] = __BLACK,
        [TABLE_THEME_LABEL_INDEX]      = __GBLUE,
        [TABLE_THEME_VALUE_INDEX]      = __WHITE,
        [TABLE_THEME_UNIT_INDEX]       = __GRAY,
        [TABLE_THEME_LINE_INDEX]       = __GRAY,
    },
    [TABLE_THEME_LIGHT] = {
        [TABLE_THEME_FRAME_INDEX]      = __BLACK,
        [TABLE_THEME_BACKGROUND_INDEX] = __WHITE,
        [TABLE_THEME_LABEL_INDEX]      = __DEEP_BLUE,
        [TABLE_THEME_VALUE_INDEX]      = __BLACK,
        [TABLE_THEME_UNIT_INDEX]       = __GRAY,
        [TABLE_THEME_LINE_INDEX]       = __GRAY,
    },
};

static void TABLE_CopyString(char dst[TABLE_MAX_STRING_LEN + 1], const char *src) {
    if (dst == NULL) return;
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, TABLE_MAX_STRING_LEN);
    dst[TABLE_MAX_STRING_LEN] = '\0';
}

static uint8_t TABLE_StrLen16(const char *s) {
    uint8_t len = 0;
    if (s == NULL) return 0;
    while (len < TABLE_MAX_STRING_LEN && s[len] != '\0') {
        len++;
    }
    return len;
}

static uint8_t TABLE_FormatUInt32(uint32_t value, char *buf, uint8_t size) {
    char tmp[10];
    uint8_t len = 0;
    if (buf == NULL || size == 0) return 0;
    if (value == 0) {
        buf[0] = '0';
        if (size > 1) buf[1] = '\0';
        return 1;
    }
    while (value > 0 && len < sizeof(tmp)) {
        tmp[len++] = (char)('0' + (value % 10U));
        value /= 10U;
    }
    uint8_t out_len = 0;
    while (len > 0 && out_len + 1 < size) {
        buf[out_len++] = tmp[--len];
    }
    buf[out_len] = '\0';
    return out_len;
}

static uint32_t TABLE_Pow10(uint8_t precision) {
    uint32_t scale = 1;
    while (precision > 0) {
        scale *= 10U;
        precision--;
    }
    return scale;
}

static uint8_t TABLE_FormatFloat(float value, uint8_t precision, char *buf, uint8_t size) {
    if (buf == NULL || size == 0) return 0;
    if (precision > 4) precision = 4;

    uint8_t pos = 0;
    if (value < 0.0f && pos + 1 < size) {
        buf[pos++] = '-';
        value = -value;
    }

    uint32_t scale = TABLE_Pow10(precision);
    uint32_t scaled = (uint32_t)(value * (float)scale + 0.5f);
    uint32_t integer_part = scaled / scale;
    uint32_t frac_part = scaled % scale;

    pos += TABLE_FormatUInt32(integer_part, &buf[pos], (uint8_t)(size - pos));
    if (precision > 0 && pos + 1 < size) {
        buf[pos++] = '.';
        uint32_t div = scale / 10U;
        for (uint8_t i = 0; i < precision && pos + 1 < size; i++) {
            buf[pos++] = (char)('0' + (frac_part / div) % 10U);
            if (div > 1U) div /= 10U;
        }
    }
    buf[pos] = '\0';
    return pos;
}

static void TABLE_FormatValue(int inst, uint16_t row, char *buf, uint8_t size) {
    TABLE_RowConfig_TypeDef *cfg = &TABLE_PRIVATE_MEMBER_ARRAY(inst, rows, row);
    TABLE_CellValue *value = &TABLE_PRIVATE_MEMBER_ARRAY(inst, values, row);
    if (cfg->value_kind == TABLE_VALUE_TEXT) {
        TABLE_CopyString(buf, value->text);
    } else if (cfg->number_type == TABLE_NUMBER_FLOAT) {
        TABLE_FormatFloat(value->f32, cfg->precision, buf, size);
    } else {
        TABLE_FormatUInt32(value->u32, buf, size);
    }
}

static void TABLE_CalcColumns(int inst) {
    uint16_t row_count = TABLE_CONFIG_MEMBER(inst, row_count);
    uint16_t label_w = TABLE_CONFIG_MEMBER(inst, label_col_width);
    uint16_t value_w = TABLE_CONFIG_MEMBER(inst, value_col_width);
    uint16_t unit_w = TABLE_CONFIG_MEMBER(inst, unit_col_width);
    ESTA_FontSize font_size = TABLE_CONFIG_MEMBER(inst, font_size);
    uint16_t font_width = ui_font_width(font_size);

    if (TABLE_CONFIG_MEMBER(inst, is_auto_col_width)) {
        uint8_t max_label = 1;
        uint8_t max_value = 1;
        uint8_t max_unit = 1;
        char value_buf[TABLE_MAX_STRING_LEN + 1];
        for (uint16_t i = 0; i < row_count; i++) {
            TABLE_RowConfig_TypeDef *row = &TABLE_PRIVATE_MEMBER_ARRAY(inst, rows, i);
            uint8_t label_len = TABLE_StrLen16(row->label);
            uint8_t unit_len = TABLE_StrLen16(row->unit);
            TABLE_FormatValue(inst, i, value_buf, sizeof(value_buf));
            uint8_t value_len = TABLE_StrLen16(value_buf);
            if (label_len > max_label) max_label = label_len;
            if (value_len > max_value) max_value = value_len;
            if (unit_len > max_unit) max_unit = unit_len;
        }
        label_w = (uint16_t)((max_label + 1U) * font_width);
        value_w = (uint16_t)((max_value + 1U) * font_width);
        unit_w = (uint16_t)((max_unit + 1U) * font_width);
        uint16_t total = label_w + value_w + unit_w;
        uint16_t x_width = TABLE_CONFIG_MEMBER(inst, x_width);
        if (total < x_width) {
            value_w += (uint16_t)(x_width - total);
        }
    }

    TABLE_WRITE_PRIVATE(inst, label_col_width_actual, label_w);
    TABLE_WRITE_PRIVATE(inst, value_col_width_actual, value_w);
    TABLE_WRITE_PRIVATE(inst, unit_col_width_actual, unit_w);
}

ESTA_StatusTypeDef TABLE_ConfigSetRows(TABLE_Config_TypeDef *config,
    const TABLE_RowConfig_TypeDef *rows, uint16_t row_count) {
    if (config == NULL) return ESTA_ERROR;
    if (row_count > TABLE_MAX_ROWS) return ESTA_ERROR;
    if (row_count > 0 && rows == NULL) return ESTA_ERROR;
    config->rows = rows;
    config->row_count = row_count;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetLayout(TABLE_Config_TypeDef *config,
    uint16_t row_height, uint16_t label_col_width,
    uint16_t value_col_width, uint16_t unit_col_width,
    bool is_auto_col_width) {
    if (config == NULL) return ESTA_ERROR;
    config->row_height = row_height;
    config->label_col_width = label_col_width;
    config->value_col_width = value_col_width;
    config->unit_col_width = unit_col_width;
    config->is_auto_col_width = is_auto_col_width;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetDisplayOptions(TABLE_Config_TypeDef *config,
    bool is_show_frame, bool is_show_row_line, bool is_fill_background) {
    if (config == NULL) return ESTA_ERROR;
    config->is_show_frame = is_show_frame;
    config->is_show_row_line = is_show_row_line;
    config->is_fill_background = is_fill_background;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetFontSize(TABLE_Config_TypeDef *config,
    ESTA_FontSize font_size) {
    if (config == NULL) return ESTA_ERROR;
    if ((int)font_size < 0 || font_size >= ESTA_FONT_SIZE_COUNT) return ESTA_ERROR;
    config->font_size = font_size;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetTheme(TABLE_Config_TypeDef *config,
    TABLE_theme_type theme_type) {
    if (config == NULL) return ESTA_ERROR;
    config->theme_type = theme_type;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_Init(int inst, TABLE_Config_TypeDef *TABLE_Init) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    if (TABLE_Init == NULL) return ESTA_ERROR;
    if (!IS_VALID_TABLE_THEME(TABLE_Init->theme_type)) return ESTA_ERROR;
    if (TABLE_Init->row_count > TABLE_MAX_ROWS) return ESTA_ERROR;
    if (TABLE_Init->row_count > 0 && TABLE_Init->rows == NULL) return ESTA_ERROR;

    TABLE_WRITE_CONFIG_INIT(inst, x_origin);
    TABLE_WRITE_CONFIG_INIT(inst, y_origin);
    TABLE_WRITE_CONFIG_INIT(inst, x_width);
    TABLE_WRITE_CONFIG_INIT(inst, y_width);
    TABLE_WRITE_CONFIG_INIT(inst, row_count);
    TABLE_WRITE_CONFIG_INIT(inst, row_height);
    TABLE_WRITE_CONFIG_INIT(inst, label_col_width);
    TABLE_WRITE_CONFIG_INIT(inst, value_col_width);
    TABLE_WRITE_CONFIG_INIT(inst, unit_col_width);
    TABLE_WRITE_CONFIG_INIT(inst, is_auto_col_width);
    TABLE_WRITE_CONFIG_INIT(inst, is_show_frame);
    TABLE_WRITE_CONFIG_INIT(inst, is_show_row_line);
    TABLE_WRITE_CONFIG_INIT(inst, is_fill_background);
    TABLE_WRITE_CONFIG_INIT(inst, font_size);
    TABLE_WRITE_CONFIG_INIT(inst, theme_type);
    TABLE_WRITE_CONFIG(inst, rows, NULL);

    memset(TABLE_INST_ADDR(inst).TABLE_Private.rows, 0,
           sizeof(TABLE_INST_ADDR(inst).TABLE_Private.rows));
    memset(TABLE_INST_ADDR(inst).TABLE_Private.values, 0,
           sizeof(TABLE_INST_ADDR(inst).TABLE_Private.values));

    for (uint16_t i = 0; i < TABLE_Init->row_count; i++) {
        TABLE_RowConfig_TypeDef *dst = &TABLE_PRIVATE_MEMBER_ARRAY(inst, rows, i);
        const TABLE_RowConfig_TypeDef *src = &TABLE_Init->rows[i];
        *dst = *src;
        TABLE_CopyString(dst->label, src->label);
        TABLE_CopyString(dst->unit, src->unit);
        TABLE_CopyString(dst->default_text, src->default_text);
        if (dst->value_kind == TABLE_VALUE_TEXT) {
            TABLE_CopyString(TABLE_PRIVATE_MEMBER_ARRAY(inst, values, i).text, dst->default_text);
        } else if (dst->number_type == TABLE_NUMBER_FLOAT) {
            TABLE_PRIVATE_MEMBER_ARRAY(inst, values, i).f32 = dst->default_float;
        } else {
            TABLE_PRIVATE_MEMBER_ARRAY(inst, values, i).u32 = dst->default_u32;
        }
    }

    TABLE_CalcColumns(inst);
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_DeInit(int inst) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    memset(&TABLE_INST_ADDR(inst), 0, sizeof(TABLE_INST_ADDR(inst)));
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_Clear(int inst) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    uint16_t theme = TABLE_CONFIG_MEMBER(inst, theme_type);
    uint16_t bg = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_BACKGROUND_INDEX);
    SCREEN_FILL(TABLE_CONFIG_MEMBER(inst, x_origin), TABLE_CONFIG_MEMBER(inst, y_origin),
                TABLE_CONFIG_MEMBER(inst, x_origin) + TABLE_CONFIG_MEMBER(inst, x_width),
                TABLE_CONFIG_MEMBER(inst, y_origin) + TABLE_CONFIG_MEMBER(inst, y_width), bg);
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ReDraw(int inst) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    TABLE_CalcColumns(inst);
    uint16_t x = TABLE_CONFIG_MEMBER(inst, x_origin);
    uint16_t y = TABLE_CONFIG_MEMBER(inst, y_origin);
    uint16_t w = TABLE_CONFIG_MEMBER(inst, x_width);
    uint16_t h = TABLE_CONFIG_MEMBER(inst, y_width);
    uint16_t row_h = TABLE_CONFIG_MEMBER(inst, row_height);
    uint16_t rows = TABLE_CONFIG_MEMBER(inst, row_count);
    uint16_t theme = TABLE_CONFIG_MEMBER(inst, theme_type);
    ESTA_FontSize font_size = TABLE_CONFIG_MEMBER(inst, font_size);
    uint16_t font_height = ui_font_height(font_size);

    uint16_t frame = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_FRAME_INDEX);
    uint16_t bg = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_BACKGROUND_INDEX);
    uint16_t label = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_LABEL_INDEX);
    uint16_t value = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_VALUE_INDEX);
    uint16_t unit = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_UNIT_INDEX);
    uint16_t line = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_LINE_INDEX);

    if (TABLE_CONFIG_MEMBER(inst, is_fill_background)) {
        SCREEN_FILL(x, y, x + w, y + h, bg);
    }
    if (TABLE_CONFIG_MEMBER(inst, is_show_frame)) {
        SCREEN_DRAW_RECTANGLE(x, y, x + w, y + h, frame);
    }

    uint16_t label_w = TABLE_PRIVATE_MEMBER(inst, label_col_width_actual);
    uint16_t value_w = TABLE_PRIVATE_MEMBER(inst, value_col_width_actual);
    char value_buf[TABLE_MAX_STRING_LEN + 1];

    for (uint16_t i = 0; i < rows; i++) {
        uint16_t row_y = (uint16_t)(y + i * row_h + ((row_h > font_height) ?
                         (row_h - font_height) / 2 : 0));
        TABLE_RowConfig_TypeDef *row = &TABLE_PRIVATE_MEMBER_ARRAY(inst, rows, i);
        TABLE_FormatValue(inst, i, value_buf, sizeof(value_buf));
        SCREEN_DRAW_STRING_FONT(x + 2, row_y, row->label, TABLE_StrLen16(row->label),
                                font_size, label);
        SCREEN_DRAW_STRING_FONT(x + label_w, row_y, value_buf, TABLE_StrLen16(value_buf),
                                font_size, value);
        SCREEN_DRAW_STRING_FONT(x + label_w + value_w, row_y, row->unit, TABLE_StrLen16(row->unit),
                                font_size, unit);
        if (TABLE_CONFIG_MEMBER(inst, is_show_row_line) && i + 1 < rows) {
            uint16_t line_y = (uint16_t)(y + (i + 1) * row_h);
            SCREEN_DRAW_LINE(x, line_y, x + w, line_y, line);
        }
    }

    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_UpdateUInt32(int inst, uint16_t row, uint32_t value) {
    if (!IS_VALID_TABLE_INST(inst) || !IS_VALID_TABLE_ROW(row)) return ESTA_ERROR;
    if (row >= TABLE_CONFIG_MEMBER(inst, row_count)) return ESTA_ERROR;
    TABLE_RowConfig_TypeDef *cfg = &TABLE_PRIVATE_MEMBER_ARRAY(inst, rows, row);
    if (cfg->value_kind != TABLE_VALUE_NUMBER || cfg->number_type != TABLE_NUMBER_UINT32)
        return ESTA_ERROR;
    TABLE_PRIVATE_MEMBER_ARRAY(inst, values, row).u32 = value;
    return TABLE_ReDraw(inst);
}

ESTA_StatusTypeDef TABLE_UpdateFloat(int inst, uint16_t row, float value) {
    if (!IS_VALID_TABLE_INST(inst) || !IS_VALID_TABLE_ROW(row)) return ESTA_ERROR;
    if (row >= TABLE_CONFIG_MEMBER(inst, row_count)) return ESTA_ERROR;
    TABLE_RowConfig_TypeDef *cfg = &TABLE_PRIVATE_MEMBER_ARRAY(inst, rows, row);
    if (cfg->value_kind != TABLE_VALUE_NUMBER || cfg->number_type != TABLE_NUMBER_FLOAT)
        return ESTA_ERROR;
    TABLE_PRIVATE_MEMBER_ARRAY(inst, values, row).f32 = value;
    return TABLE_ReDraw(inst);
}

ESTA_StatusTypeDef TABLE_UpdateText(int inst, uint16_t row, const char *value) {
    if (!IS_VALID_TABLE_INST(inst) || !IS_VALID_TABLE_ROW(row)) return ESTA_ERROR;
    if (row >= TABLE_CONFIG_MEMBER(inst, row_count)) return ESTA_ERROR;
    TABLE_RowConfig_TypeDef *cfg = &TABLE_PRIVATE_MEMBER_ARRAY(inst, rows, row);
    if (cfg->value_kind != TABLE_VALUE_TEXT) return ESTA_ERROR;
    TABLE_CopyString(TABLE_PRIVATE_MEMBER_ARRAY(inst, values, row).text, value);
    return TABLE_ReDraw(inst);
}
