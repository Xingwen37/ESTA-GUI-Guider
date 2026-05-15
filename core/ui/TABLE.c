#include "ui/TABLE.h"
#include <string.h>

TABLE_TypeDef TABLE_State[TABLE_MAX_NUM];

static const uint16_t TABLE_ColorTable[TABLE_THEME_COUNT][TABLE_THEME_INDEX_COUNT] = {
    [TABLE_THEME_DEFAULT] = {
        [TABLE_THEME_FRAME_INDEX]      = __WHITE,
        [TABLE_THEME_BACKGROUND_INDEX] = __BLACK,
        [TABLE_THEME_HEADER_INDEX]     = __GBLUE,
        [TABLE_THEME_TEXT_INDEX]       = __WHITE,
        [TABLE_THEME_LINE_INDEX]       = __GRAY,
    },
    [TABLE_THEME_LIGHT] = {
        [TABLE_THEME_FRAME_INDEX]      = __BLACK,
        [TABLE_THEME_BACKGROUND_INDEX] = __WHITE,
        [TABLE_THEME_HEADER_INDEX]     = __DEEP_BLUE,
        [TABLE_THEME_TEXT_INDEX]       = __BLACK,
        [TABLE_THEME_LINE_INDEX]       = __GRAY,
    },
};

static void TABLE_CopyString(char dst[TABLE_MAX_STRING_LEN + 1], const char *src) {
    if (dst == NULL) return;
    if (src == NULL) { dst[0] = '\0'; return; }
    strncpy(dst, src, TABLE_MAX_STRING_LEN);
    dst[TABLE_MAX_STRING_LEN] = '\0';
}

static uint8_t TABLE_StrLen16(const char *s) {
    uint8_t len = 0;
    if (s == NULL) return 0;
    while (len < TABLE_MAX_STRING_LEN && s[len] != '\0') len++;
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

/* PLACEHOLDER_CONTINUE */

static uint32_t TABLE_Pow10(uint8_t precision) {
    uint32_t scale = 1;
    while (precision > 0) { scale *= 10U; precision--; }
    return scale;
}

static uint8_t TABLE_FormatFloat(float value, uint8_t precision, char *buf, uint8_t size) {
    if (buf == NULL || size == 0) return 0;
    if (precision > 4) precision = 4;
    uint8_t pos = 0;
    if (value < 0.0f && pos + 1 < size) { buf[pos++] = '-'; value = -value; }
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

static uint8_t TABLE_FormatCell(const TABLE_ColConfig_TypeDef *col,
    const TABLE_CellValue *cell, char *buf, uint8_t size) {
    switch (col->cell_type) {
        case TABLE_CELL_UINT32:
            return TABLE_FormatUInt32(cell->u32, buf, size);
        case TABLE_CELL_FLOAT:
            return TABLE_FormatFloat(cell->f32, col->precision, buf, size);
        case TABLE_CELL_TEXT:
        default:
            TABLE_CopyString(buf, cell->text);
            return TABLE_StrLen16(buf);
    }
}

static void TABLE_CalcColumns(int inst) {
    uint16_t col_count = TABLE_CONFIG_MEMBER(inst, col_count);
    uint16_t row_count = TABLE_CONFIG_MEMBER(inst, row_count);
    ESTA_FontSize font_size = TABLE_CONFIG_MEMBER(inst, font_size);
    uint16_t font_width = ui_font_width(font_size);
    uint16_t total_fixed = 0;
    uint16_t auto_count = 0;
    char buf[TABLE_MAX_STRING_LEN + 1];

    for (uint16_t c = 0; c < col_count; c++) {
        TABLE_ColConfig_TypeDef *col = &TABLE_INST_ADDR(inst).TABLE_Private.cols[c];
        if (col->width > 0) {
            TABLE_INST_ADDR(inst).TABLE_Private.col_width_actual[c] = col->width;
            total_fixed += col->width;
        } else {
            uint8_t max_len = TABLE_StrLen16(col->header);
            for (uint16_t r = 0; r < row_count; r++) {
                TABLE_FormatCell(col, &TABLE_INST_ADDR(inst).TABLE_Private.cells[r][c], buf, sizeof(buf));
                uint8_t len = TABLE_StrLen16(buf);
                if (len > max_len) max_len = len;
            }
            uint16_t w = (uint16_t)((max_len + 1U) * font_width);
            TABLE_INST_ADDR(inst).TABLE_Private.col_width_actual[c] = w;
            total_fixed += w;
            auto_count++;
        }
    }

    uint16_t x_width = TABLE_CONFIG_MEMBER(inst, x_width);
    if (total_fixed < x_width && auto_count > 0) {
        uint16_t extra = (uint16_t)((x_width - total_fixed) / auto_count);
        for (uint16_t c = 0; c < col_count; c++) {
            if (TABLE_INST_ADDR(inst).TABLE_Private.cols[c].width == 0) {
                TABLE_INST_ADDR(inst).TABLE_Private.col_width_actual[c] += extra;
            }
        }
    }
}

/* PLACEHOLDER_API */

ESTA_StatusTypeDef TABLE_ConfigSetCols(TABLE_Config_TypeDef *config,
    const TABLE_ColConfig_TypeDef *cols, uint16_t col_count) {
    if (config == NULL) return ESTA_ERROR;
    if (col_count > TABLE_MAX_COLS) return ESTA_ERROR;
    if (col_count > 0 && cols == NULL) return ESTA_ERROR;
    config->cols = cols;
    config->col_count = col_count;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetRowCount(TABLE_Config_TypeDef *config, uint16_t row_count) {
    if (config == NULL) return ESTA_ERROR;
    if (row_count > TABLE_MAX_ROWS) return ESTA_ERROR;
    config->row_count = row_count;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetRowHeight(TABLE_Config_TypeDef *config, uint16_t row_height) {
    if (config == NULL) return ESTA_ERROR;
    config->row_height = row_height;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetDisplayOptions(TABLE_Config_TypeDef *config,
    bool is_show_header, bool is_show_frame, bool is_show_row_line,
    bool is_show_col_line, bool is_fill_background) {
    if (config == NULL) return ESTA_ERROR;
    config->is_show_header = is_show_header;
    config->is_show_frame = is_show_frame;
    config->is_show_row_line = is_show_row_line;
    config->is_show_col_line = is_show_col_line;
    config->is_fill_background = is_fill_background;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetFontSize(TABLE_Config_TypeDef *config, ESTA_FontSize font_size) {
    if (config == NULL) return ESTA_ERROR;
    if ((int)font_size < 0 || font_size >= ESTA_FONT_SIZE_COUNT) return ESTA_ERROR;
    config->font_size = font_size;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_ConfigSetTheme(TABLE_Config_TypeDef *config, TABLE_theme_type theme_type) {
    if (config == NULL) return ESTA_ERROR;
    config->theme_type = theme_type;
    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_Init(int inst, TABLE_Config_TypeDef *TABLE_Init) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    if (TABLE_Init == NULL) return ESTA_ERROR;
    if (!IS_VALID_TABLE_THEME(TABLE_Init->theme_type)) return ESTA_ERROR;
    if (TABLE_Init->col_count > TABLE_MAX_COLS) return ESTA_ERROR;
    if (TABLE_Init->row_count > TABLE_MAX_ROWS) return ESTA_ERROR;
    if (TABLE_Init->col_count > 0 && TABLE_Init->cols == NULL) return ESTA_ERROR;

    TABLE_WRITE_CONFIG_INIT(inst, x_origin);
    TABLE_WRITE_CONFIG_INIT(inst, y_origin);
    TABLE_WRITE_CONFIG_INIT(inst, x_width);
    TABLE_WRITE_CONFIG_INIT(inst, y_width);
    TABLE_WRITE_CONFIG_INIT(inst, row_count);
    TABLE_WRITE_CONFIG_INIT(inst, col_count);
    TABLE_WRITE_CONFIG_INIT(inst, row_height);
    TABLE_WRITE_CONFIG_INIT(inst, is_show_header);
    TABLE_WRITE_CONFIG_INIT(inst, is_show_frame);
    TABLE_WRITE_CONFIG_INIT(inst, is_show_row_line);
    TABLE_WRITE_CONFIG_INIT(inst, is_show_col_line);
    TABLE_WRITE_CONFIG_INIT(inst, is_fill_background);
    TABLE_WRITE_CONFIG_INIT(inst, font_size);
    TABLE_WRITE_CONFIG_INIT(inst, theme_type);
    TABLE_WRITE_CONFIG(inst, cols, NULL);

    memset(TABLE_INST_ADDR(inst).TABLE_Private.cols, 0,
           sizeof(TABLE_INST_ADDR(inst).TABLE_Private.cols));
    memset(TABLE_INST_ADDR(inst).TABLE_Private.cells, 0,
           sizeof(TABLE_INST_ADDR(inst).TABLE_Private.cells));

    for (uint16_t c = 0; c < TABLE_Init->col_count; c++) {
        TABLE_ColConfig_TypeDef *dst = &TABLE_INST_ADDR(inst).TABLE_Private.cols[c];
        const TABLE_ColConfig_TypeDef *src = &TABLE_Init->cols[c];
        *dst = *src;
        TABLE_CopyString(dst->header, src->header);
    }

    TABLE_CalcColumns(inst);
    return ESTA_OK;
}

/* PLACEHOLDER_DRAW */

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
    uint16_t row_count = TABLE_CONFIG_MEMBER(inst, row_count);
    uint16_t col_count = TABLE_CONFIG_MEMBER(inst, col_count);
    uint16_t theme = TABLE_CONFIG_MEMBER(inst, theme_type);
    ESTA_FontSize font_size = TABLE_CONFIG_MEMBER(inst, font_size);
    uint16_t font_height = ui_font_height(font_size);

    uint16_t frame_c = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_FRAME_INDEX);
    uint16_t bg = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_BACKGROUND_INDEX);
    uint16_t header_c = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_HEADER_INDEX);
    uint16_t text_c = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_TEXT_INDEX);
    uint16_t line_c = ESTA_THEME_COLOR(TABLE_ColorTable, theme, TABLE_THEME_LINE_INDEX);

    if (TABLE_CONFIG_MEMBER(inst, is_fill_background)) {
        SCREEN_FILL(x, y, x + w, y + h, bg);
    }
    if (TABLE_CONFIG_MEMBER(inst, is_show_frame)) {
        SCREEN_DRAW_RECTANGLE(x, y, x + w, y + h, frame_c);
    }

    uint16_t content_y = y;
    char buf[TABLE_MAX_STRING_LEN + 1];

    if (TABLE_CONFIG_MEMBER(inst, is_show_header)) {
        uint16_t col_x = x;
        uint16_t text_y = (uint16_t)(content_y + (row_h > font_height ? (row_h - font_height) / 2 : 0));
        for (uint16_t c = 0; c < col_count; c++) {
            TABLE_ColConfig_TypeDef *col = &TABLE_INST_ADDR(inst).TABLE_Private.cols[c];
            uint8_t hdr_len = TABLE_StrLen16(col->header);
            SCREEN_DRAW_STRING_FONT(col_x + 2, text_y, col->header, hdr_len, font_size, header_c);
            col_x += TABLE_INST_ADDR(inst).TABLE_Private.col_width_actual[c];
        }
        content_y = (uint16_t)(content_y + row_h);
        if (TABLE_CONFIG_MEMBER(inst, is_show_row_line)) {
            SCREEN_DRAW_LINE(x, content_y, x + w, content_y, line_c);
        }
    }

    for (uint16_t r = 0; r < row_count; r++) {
        uint16_t row_y = (uint16_t)(content_y + r * row_h);
        if (row_y + row_h > y + h) break;
        uint16_t text_y = (uint16_t)(row_y + (row_h > font_height ? (row_h - font_height) / 2 : 0));
        uint16_t col_x = x;
        for (uint16_t c = 0; c < col_count; c++) {
            TABLE_ColConfig_TypeDef *col = &TABLE_INST_ADDR(inst).TABLE_Private.cols[c];
            TABLE_CellValue *cell = &TABLE_INST_ADDR(inst).TABLE_Private.cells[r][c];
            TABLE_FormatCell(col, cell, buf, sizeof(buf));
            uint8_t len = TABLE_StrLen16(buf);
            SCREEN_DRAW_STRING_FONT(col_x + 2, text_y, buf, len, font_size, text_c);
            col_x += TABLE_INST_ADDR(inst).TABLE_Private.col_width_actual[c];
        }
        if (TABLE_CONFIG_MEMBER(inst, is_show_row_line) && r + 1 < row_count) {
            uint16_t line_y = (uint16_t)(content_y + (r + 1) * row_h);
            SCREEN_DRAW_LINE(x, line_y, x + w, line_y, line_c);
        }
    }

    if (TABLE_CONFIG_MEMBER(inst, is_show_col_line)) {
        uint16_t col_x = x;
        for (uint16_t c = 0; c + 1 < col_count; c++) {
            col_x += TABLE_INST_ADDR(inst).TABLE_Private.col_width_actual[c];
            SCREEN_DRAW_LINE(col_x, y, col_x, y + h, line_c);
        }
    }

    return ESTA_OK;
}

ESTA_StatusTypeDef TABLE_UpdateCell(int inst, uint16_t row, uint16_t col,
    const TABLE_CellValue *value) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    if (row >= TABLE_CONFIG_MEMBER(inst, row_count)) return ESTA_ERROR;
    if (col >= TABLE_CONFIG_MEMBER(inst, col_count)) return ESTA_ERROR;
    if (value == NULL) return ESTA_ERROR;
    TABLE_CellType type = TABLE_INST_ADDR(inst).TABLE_Private.cols[col].cell_type;
    if (type == TABLE_CELL_TEXT) {
        TABLE_CopyString(TABLE_INST_ADDR(inst).TABLE_Private.cells[row][col].text, value->text);
    } else {
        TABLE_INST_ADDR(inst).TABLE_Private.cells[row][col] = *value;
    }
    return TABLE_ReDraw(inst);
}

ESTA_StatusTypeDef TABLE_UpdateUInt32(int inst, uint16_t row, uint16_t col, uint32_t value) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    if (row >= TABLE_CONFIG_MEMBER(inst, row_count)) return ESTA_ERROR;
    if (col >= TABLE_CONFIG_MEMBER(inst, col_count)) return ESTA_ERROR;
    if (TABLE_INST_ADDR(inst).TABLE_Private.cols[col].cell_type != TABLE_CELL_UINT32)
        return ESTA_ERROR;
    TABLE_INST_ADDR(inst).TABLE_Private.cells[row][col].u32 = value;
    return TABLE_ReDraw(inst);
}

ESTA_StatusTypeDef TABLE_UpdateFloat(int inst, uint16_t row, uint16_t col, float value) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    if (row >= TABLE_CONFIG_MEMBER(inst, row_count)) return ESTA_ERROR;
    if (col >= TABLE_CONFIG_MEMBER(inst, col_count)) return ESTA_ERROR;
    if (TABLE_INST_ADDR(inst).TABLE_Private.cols[col].cell_type != TABLE_CELL_FLOAT)
        return ESTA_ERROR;
    TABLE_INST_ADDR(inst).TABLE_Private.cells[row][col].f32 = value;
    return TABLE_ReDraw(inst);
}

ESTA_StatusTypeDef TABLE_UpdateText(int inst, uint16_t row, uint16_t col, const char *value) {
    if (!IS_VALID_TABLE_INST(inst)) return ESTA_ERROR;
    if (row >= TABLE_CONFIG_MEMBER(inst, row_count)) return ESTA_ERROR;
    if (col >= TABLE_CONFIG_MEMBER(inst, col_count)) return ESTA_ERROR;
    if (TABLE_INST_ADDR(inst).TABLE_Private.cols[col].cell_type != TABLE_CELL_TEXT)
        return ESTA_ERROR;
    TABLE_CopyString(TABLE_INST_ADDR(inst).TABLE_Private.cells[row][col].text, value);
    return TABLE_ReDraw(inst);
}
