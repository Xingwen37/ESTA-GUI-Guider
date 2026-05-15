#ifndef __TABLE_LIB
#define __TABLE_LIB

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "infra/helper.h"
#include "infra/ui_base.h"
#include "infra/ui_theme.h"

#define TABLE_MAX_NUM            4
#define TABLE_MAX_ROWS           8
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
    TABLE_THEME_LABEL_INDEX,
    TABLE_THEME_VALUE_INDEX,
    TABLE_THEME_UNIT_INDEX,
    TABLE_THEME_LINE_INDEX,
    TABLE_THEME_INDEX_COUNT
} TABLE_theme_color_index_type;

typedef enum {
    TABLE_VALUE_TEXT = 0,
    TABLE_VALUE_NUMBER,
    TABLE_VALUE_KIND_COUNT
} TABLE_ValueKind;

typedef enum {
    TABLE_NUMBER_UINT32 = 0,
    TABLE_NUMBER_FLOAT,
    TABLE_NUMBER_TYPE_COUNT
} TABLE_NumberType;

typedef struct {
    char label[TABLE_MAX_STRING_LEN + 1];
    TABLE_ValueKind value_kind;
    TABLE_NumberType number_type;
    char unit[TABLE_MAX_STRING_LEN + 1];
    uint8_t precision;
    uint32_t default_u32;
    float default_float;
    char default_text[TABLE_MAX_STRING_LEN + 1];
} TABLE_RowConfig_TypeDef;

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

typedef struct {
    uint16_t       x_origin;
    uint16_t       y_origin;
    uint16_t       x_width;
    uint16_t       y_width;

    uint16_t       row_count;
    uint16_t       row_height;
    uint16_t       label_col_width;
    uint16_t       value_col_width;
    uint16_t       unit_col_width;
    volatile bool  is_auto_col_width;
    volatile bool  is_show_frame;
    volatile bool  is_show_row_line;
    volatile bool  is_fill_background;

    const TABLE_RowConfig_TypeDef *rows;
    ESTA_FontSize font_size;
    TABLE_theme_type theme_type;
} TABLE_Config_TypeDef;

typedef union {
    uint32_t u32;
    float f32;
    char text[TABLE_MAX_STRING_LEN + 1];
} TABLE_CellValue;

typedef struct {
    TABLE_RowConfig_TypeDef rows[TABLE_MAX_ROWS];
    TABLE_CellValue values[TABLE_MAX_ROWS];
    uint16_t label_col_width_actual;
    uint16_t value_col_width_actual;
    uint16_t unit_col_width_actual;
} TABLE_Private_Typedef;

typedef struct {
    TABLE_Config_TypeDef     TABLE_Config;
    TABLE_Private_Typedef    TABLE_Private;
} TABLE_TypeDef;

extern TABLE_TypeDef TABLE_State[TABLE_MAX_NUM];

ESTA_StatusTypeDef TABLE_ConfigSetRows(TABLE_Config_TypeDef *config,
    const TABLE_RowConfig_TypeDef *rows, uint16_t row_count);
ESTA_StatusTypeDef TABLE_ConfigSetLayout(TABLE_Config_TypeDef *config,
    uint16_t row_height, uint16_t label_col_width,
    uint16_t value_col_width, uint16_t unit_col_width,
    bool is_auto_col_width);
ESTA_StatusTypeDef TABLE_ConfigSetDisplayOptions(TABLE_Config_TypeDef *config,
    bool is_show_frame, bool is_show_row_line, bool is_fill_background);
ESTA_StatusTypeDef TABLE_ConfigSetFontSize(TABLE_Config_TypeDef *config,
    ESTA_FontSize font_size);
ESTA_StatusTypeDef TABLE_ConfigSetTheme(TABLE_Config_TypeDef *config,
    TABLE_theme_type theme_type);

ESTA_StatusTypeDef TABLE_Init(int inst, TABLE_Config_TypeDef *pConfig);
ESTA_StatusTypeDef TABLE_DeInit(int inst);
ESTA_StatusTypeDef TABLE_Clear(int inst);
ESTA_StatusTypeDef TABLE_ReDraw(int inst);

ESTA_StatusTypeDef TABLE_UpdateUInt32(int inst, uint16_t row, uint32_t value);
ESTA_StatusTypeDef TABLE_UpdateFloat(int inst, uint16_t row, float value);
ESTA_StatusTypeDef TABLE_UpdateText(int inst, uint16_t row, const char *value);

#endif
