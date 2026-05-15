#ifndef __TABLE_LIB
#define __TABLE_LIB

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "infra/helper.h"
#include "infra/ui_base.h"
#include "infra/ui_theme.h"

#ifndef TABLE_MAX_NUM
#define TABLE_MAX_NUM            4
#endif
#ifndef TABLE_MAX_ROWS
#define TABLE_MAX_ROWS           8
#endif
#ifndef TABLE_MAX_COLS
#define TABLE_MAX_COLS           6
#endif
#define TABLE_MAX_STRING_LEN     16

#define TABLE_INST(i)                   (i)
#define TABLE_INST_ADDR(i)              TABLE_State[TABLE_INST(i)]

typedef enum {
    TABLE_THEME_DEFAULT = 0,
    TABLE_THEME_LIGHT,
    TABLE_THEME_COUNT
} TABLE_theme_type;

typedef enum {
    TABLE_THEME_FRAME_INDEX = 0,
    TABLE_THEME_BACKGROUND_INDEX,
    TABLE_THEME_HEADER_INDEX,
    TABLE_THEME_TEXT_INDEX,
    TABLE_THEME_LINE_INDEX,
    TABLE_THEME_INDEX_COUNT
} TABLE_theme_color_index_type;

typedef enum {
    TABLE_CELL_TEXT = 0,
    TABLE_CELL_UINT32,
    TABLE_CELL_FLOAT,
    TABLE_CELL_TYPE_COUNT
} TABLE_CellType;

typedef struct {
    char header[TABLE_MAX_STRING_LEN + 1];
    TABLE_CellType cell_type;
    uint16_t width;
    uint8_t precision;
} TABLE_ColConfig_TypeDef;

typedef union {
    uint32_t u32;
    float f32;
    char text[TABLE_MAX_STRING_LEN + 1];
} TABLE_CellValue;

#define TABLE_CONFIG_MEMBER(inst, reg_name)  \
            TABLE_INST_ADDR(inst).TABLE_Config.reg_name
#define TABLE_PRIVATE_MEMBER(inst, reg_name)  \
            TABLE_INST_ADDR(inst).TABLE_Private.reg_name

#define TABLE_CONFIG_MEMBER_ARRAY(inst, reg_name, NO)  \
            TABLE_INST_ADDR(inst).TABLE_Config.reg_name[NO]
#define TABLE_PRIVATE_MEMBER_ARRAY(inst, reg_name, NO)  \
            TABLE_INST_ADDR(inst).TABLE_Private.reg_name[NO]

#define TABLE_WRITE_CONFIG(inst, reg_name, reg_value)   \
            TABLE_INST_ADDR(inst).TABLE_Config.reg_name = reg_value
#define TABLE_WRITE_PRIVATE(inst, reg_name, reg_value)   \
            TABLE_INST_ADDR(inst).TABLE_Private.reg_name = reg_value

#define TABLE_WRITE_CONFIG_INIT(inst, reg_name)         \
            TABLE_INST_ADDR(inst).TABLE_Config.reg_name = TABLE_Init->reg_name

#define IS_VALID_TABLE_INST(x)  ((int)(x) < TABLE_MAX_NUM && (int)(x) >= 0)
#define IS_VALID_TABLE_THEME(x) ((int)(x) < TABLE_THEME_COUNT && (int)(x) >= 0)
#define IS_VALID_TABLE_ROW(x)   ((uint16_t)(x) < TABLE_MAX_ROWS)
#define IS_VALID_TABLE_COL(x)   ((uint16_t)(x) < TABLE_MAX_COLS)

typedef struct {
    uint16_t       x_origin;
    uint16_t       y_origin;
    uint16_t       x_width;
    uint16_t       y_width;

    uint16_t       row_count;
    uint16_t       col_count;
    uint16_t       row_height;
    volatile bool  is_show_header;
    volatile bool  is_show_frame;
    volatile bool  is_show_row_line;
    volatile bool  is_show_col_line;
    volatile bool  is_fill_background;

    const TABLE_ColConfig_TypeDef *cols;
    ESTA_FontSize  font_size;
    TABLE_theme_type theme_type;
} TABLE_Config_TypeDef;

typedef struct {
    TABLE_ColConfig_TypeDef cols[TABLE_MAX_COLS];
    TABLE_CellValue cells[TABLE_MAX_ROWS][TABLE_MAX_COLS];
    uint16_t col_width_actual[TABLE_MAX_COLS];
} TABLE_Private_Typedef;

typedef struct {
    TABLE_Config_TypeDef     TABLE_Config;
    TABLE_Private_Typedef    TABLE_Private;
} TABLE_TypeDef;

extern TABLE_TypeDef TABLE_State[TABLE_MAX_NUM];

ESTA_StatusTypeDef TABLE_ConfigSetCols(TABLE_Config_TypeDef *config,
    const TABLE_ColConfig_TypeDef *cols, uint16_t col_count);
ESTA_StatusTypeDef TABLE_ConfigSetRowCount(TABLE_Config_TypeDef *config,
    uint16_t row_count);
ESTA_StatusTypeDef TABLE_ConfigSetRowHeight(TABLE_Config_TypeDef *config,
    uint16_t row_height);
ESTA_StatusTypeDef TABLE_ConfigSetDisplayOptions(TABLE_Config_TypeDef *config,
    bool is_show_header, bool is_show_frame, bool is_show_row_line,
    bool is_show_col_line, bool is_fill_background);
ESTA_StatusTypeDef TABLE_ConfigSetFontSize(TABLE_Config_TypeDef *config,
    ESTA_FontSize font_size);
ESTA_StatusTypeDef TABLE_ConfigSetTheme(TABLE_Config_TypeDef *config,
    TABLE_theme_type theme_type);

ESTA_StatusTypeDef TABLE_Init(int inst, TABLE_Config_TypeDef *pConfig);
ESTA_StatusTypeDef TABLE_DeInit(int inst);
ESTA_StatusTypeDef TABLE_Clear(int inst);
ESTA_StatusTypeDef TABLE_ReDraw(int inst);

ESTA_StatusTypeDef TABLE_UpdateCell(int inst, uint16_t row, uint16_t col,
    const TABLE_CellValue *value);
ESTA_StatusTypeDef TABLE_UpdateUInt32(int inst, uint16_t row, uint16_t col,
    uint32_t value);
ESTA_StatusTypeDef TABLE_UpdateFloat(int inst, uint16_t row, uint16_t col,
    float value);
ESTA_StatusTypeDef TABLE_UpdateText(int inst, uint16_t row, uint16_t col,
    const char *value);

#endif
