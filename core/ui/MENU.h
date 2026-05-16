#ifndef __MENU_LIB
#define __MENU_LIB

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "infra/helper.h"
#include "infra/ui_base.h"
#include "infra/ui_theme.h"

#define MENU_MAX_NUM            4
#define MENU_MAX_ITEMS          32
#define MENU_MAX_STRING_LEN     16
#define MENU_MAX_DEPTH          8

#define MENU_INST(i)                   (i)
#define MENU_INST_ADDR(i)              MENU_State[MENU_INST(i)]

typedef enum {
    MENU_THEME_DEFAULT = 0,
    MENU_THEME_LIGHT,
    MENU_THEME_COUNT
} MENU_theme_type;

typedef enum {
    MENU_THEME_BACKGROUND_INDEX = 0,
    MENU_THEME_TEXT_INDEX,
    MENU_THEME_SELECTED_BG_INDEX,
    MENU_THEME_SELECTED_TEXT_INDEX,
    MENU_THEME_BREADCRUMB_INDEX,
    MENU_THEME_FRAME_INDEX,
    MENU_THEME_ARROW_INDEX,
    MENU_THEME_INDEX_COUNT
} MENU_theme_color_index_type;

typedef struct {
    char label[MENU_MAX_STRING_LEN + 1];
    uint8_t parent_idx;    /* 0xFF = root level */
    bool is_submenu;       /* true = opens submenu, false = leaf item */
    uint8_t event_id;      /* pushed as ESTA_EVENT_MENU_SELECT when leaf selected */
} MENU_ItemConfig;

#define MENU_CONFIG_MEMBER(inst, reg_name)  \
            MENU_INST_ADDR(inst).MENU_Config.reg_name
#define MENU_PRIVATE_MEMBER(inst, reg_name)  \
            MENU_INST_ADDR(inst).MENU_Private.reg_name

#define MENU_CONFIG_MEMBER_ARRAY(inst, reg_name, NO)  \
            MENU_INST_ADDR(inst).MENU_Config.reg_name[NO]
#define MENU_PRIVATE_MEMBER_ARRAY(inst, reg_name, NO)  \
            MENU_INST_ADDR(inst).MENU_Private.reg_name[NO]

#define MENU_WRITE_CONFIG(inst, reg_name, reg_value)   \
            MENU_INST_ADDR(inst).MENU_Config.reg_name = reg_value
#define MENU_WRITE_PRIVATE(inst, reg_name, reg_value)   \
            MENU_INST_ADDR(inst).MENU_Private.reg_name = reg_value
#define MENU_WRITE_CONFIG_ARRAY(inst, reg_name, idx, reg_value)   \
            MENU_INST_ADDR(inst).MENU_Config.reg_name[idx] = reg_value
#define MENU_WRITE_PRIVATE_ARRAY(inst, reg_name, idx, reg_value)   \
            MENU_INST_ADDR(inst).MENU_Private.reg_name[idx] = reg_value

#define MENU_WRITE_CONFIG_INIT(inst, reg_name)         \
            MENU_INST_ADDR(inst).MENU_Config.reg_name = MENU_Init->reg_name

#define IS_VALID_MENU_INST(x)   ((int)(x) < MENU_MAX_NUM && (int)(x) >= 0)
#define IS_VALID_MENU_THEME(x)  ((int)(x) < MENU_THEME_COUNT && (int)(x) >= 0)
#define IS_VALID_MENU_ITEM(x)   ((uint16_t)(x) < MENU_MAX_ITEMS)

typedef struct {
    uint16_t       x_origin;
    uint16_t       y_origin;
    uint16_t       x_width;
    uint16_t       y_width;

    uint16_t       item_count;
    uint16_t       item_height;
    uint16_t       breadcrumb_height;
    volatile bool  is_show_frame;
    volatile bool  is_show_breadcrumb;
    volatile bool  is_fill_background;

    const MENU_ItemConfig *items;
    ESTA_FontSize  font_size;
    MENU_theme_type theme_type;
} MENU_Config_TypeDef;

typedef struct {
    MENU_ItemConfig items[MENU_MAX_ITEMS];
    uint8_t nav_stack[MENU_MAX_DEPTH];
    uint8_t nav_depth;
    uint8_t local_items[MENU_MAX_ITEMS];
    uint8_t local_count;
    uint8_t selected_idx;
} MENU_Private_Typedef;

typedef struct {
    MENU_Config_TypeDef     MENU_Config;
    MENU_Private_Typedef    MENU_Private;
} MENU_TypeDef;

extern MENU_TypeDef MENU_State[MENU_MAX_NUM];

ESTA_StatusTypeDef MENU_ConfigSetItems(MENU_Config_TypeDef *config,
    const MENU_ItemConfig *items, uint16_t item_count);
ESTA_StatusTypeDef MENU_ConfigSetItemHeight(MENU_Config_TypeDef *config,
    uint16_t item_height);
ESTA_StatusTypeDef MENU_ConfigSetBreadcrumbHeight(MENU_Config_TypeDef *config,
    uint16_t breadcrumb_height);
ESTA_StatusTypeDef MENU_ConfigSetDisplayOptions(MENU_Config_TypeDef *config,
    bool is_show_frame, bool is_show_breadcrumb, bool is_fill_background);
ESTA_StatusTypeDef MENU_ConfigSetFontSize(MENU_Config_TypeDef *config,
    ESTA_FontSize font_size);
ESTA_StatusTypeDef MENU_ConfigSetTheme(MENU_Config_TypeDef *config,
    MENU_theme_type theme_type);

ESTA_StatusTypeDef MENU_Init(int inst, MENU_Config_TypeDef *pConfig);
ESTA_StatusTypeDef MENU_DeInit(int inst);
ESTA_StatusTypeDef MENU_Clear(int inst);
ESTA_StatusTypeDef MENU_ReDraw(int inst);
void              MENU_ProcessInput(int inst);

#include "event/event.h"
bool              MENU_HandleEvent(int inst, const ESTA_Event *event);

#endif
