#include "ui/MENU.h"
#include "event/event.h"
#include <string.h>

MENU_TypeDef MENU_State[MENU_MAX_NUM];

static const uint16_t MENU_ColorTable[MENU_THEME_COUNT][MENU_THEME_INDEX_COUNT] = {
    [MENU_THEME_DEFAULT] = {
        [MENU_THEME_BACKGROUND_INDEX]      = __BLACK,
        [MENU_THEME_TEXT_INDEX]            = __WHITE,
        [MENU_THEME_SELECTED_BG_INDEX]     = __GBLUE,
        [MENU_THEME_SELECTED_TEXT_INDEX]   = __WHITE,
        [MENU_THEME_BREADCRUMB_INDEX]      = __GRAY,
        [MENU_THEME_FRAME_INDEX]           = __WHITE,
        [MENU_THEME_ARROW_INDEX]           = __GRAY,
    },
    [MENU_THEME_LIGHT] = {
        [MENU_THEME_BACKGROUND_INDEX]      = __WHITE,
        [MENU_THEME_TEXT_INDEX]            = __BLACK,
        [MENU_THEME_SELECTED_BG_INDEX]     = __DEEP_BLUE,
        [MENU_THEME_SELECTED_TEXT_INDEX]   = __WHITE,
        [MENU_THEME_BREADCRUMB_INDEX]      = __GRAY,
        [MENU_THEME_FRAME_INDEX]           = __BLACK,
        [MENU_THEME_ARROW_INDEX]           = __GRAY,
    },
};

static void MENU_CopyString(char dst[MENU_MAX_STRING_LEN + 1], const char *src) {
    if (dst == NULL) return;
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, MENU_MAX_STRING_LEN);
    dst[MENU_MAX_STRING_LEN] = '\0';
}

static uint8_t MENU_StrLen16(const char *s) {
    uint8_t len = 0;
    if (s == NULL) return 0;
    while (len < MENU_MAX_STRING_LEN && s[len] != '\0') {
        len++;
    }
    return len;
}

static uint8_t MENU_GetCurrentParent(int inst) {
    uint8_t depth = MENU_PRIVATE_MEMBER(inst, nav_depth);
    if (depth == 0) {
        return 0xFF; /* root level */
    }
    return MENU_PRIVATE_MEMBER_ARRAY(inst, nav_stack, (uint8_t)(depth - 1U));
}

static void MENU_RebuildLocalList(int inst) {
    uint16_t item_count = MENU_CONFIG_MEMBER(inst, item_count);
    uint8_t parent = MENU_GetCurrentParent(inst);
    uint8_t count = 0;

    for (uint16_t i = 0; i < item_count && count < MENU_MAX_ITEMS; i++) {
        const MENU_ItemConfig *item = &MENU_PRIVATE_MEMBER_ARRAY(inst, items, i);
        if (item->parent_idx == parent) {
            MENU_WRITE_PRIVATE_ARRAY(inst, local_items, count, (uint8_t)i);
            count++;
        }
    }

    MENU_WRITE_PRIVATE(inst, local_count, count);
    MENU_WRITE_PRIVATE(inst, selected_idx, 0);
}

static void MENU_BuildBreadcrumb(int inst, char *buf, uint8_t buf_size) {
    uint8_t depth = MENU_PRIVATE_MEMBER(inst, nav_depth);
    uint8_t pos = 0;
    const char *sep = " > ";

    if (depth == 0) {
        if (pos < buf_size) buf[pos++] = '\0';
        buf[buf_size - 1] = '\0';
        return;
    }

    for (uint8_t d = 0; d < depth; d++) {
        uint8_t item_idx = MENU_PRIVATE_MEMBER_ARRAY(inst, nav_stack, d);
        const MENU_ItemConfig *item = &MENU_PRIVATE_MEMBER_ARRAY(inst, items, item_idx);
        uint8_t label_len = MENU_StrLen16(item->label);

        /* separator */
        if (d > 0) {
            for (uint8_t s = 0; s < 3 && pos + 1 < buf_size; s++) {
                buf[pos++] = sep[s];
            }
        }

        for (uint8_t l = 0; l < label_len && pos + 1 < buf_size; l++) {
            buf[pos++] = item->label[l];
        }
    }

    buf[pos] = '\0';
}

ESTA_StatusTypeDef MENU_ConfigSetItems(MENU_Config_TypeDef *config,
    const MENU_ItemConfig *items, uint16_t item_count) {
    if (config == NULL) return ESTA_ERROR;
    if (item_count > MENU_MAX_ITEMS) return ESTA_ERROR;
    if (item_count > 0 && items == NULL) return ESTA_ERROR;
    config->items = items;
    config->item_count = item_count;
    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_ConfigSetItemHeight(MENU_Config_TypeDef *config,
    uint16_t item_height) {
    if (config == NULL) return ESTA_ERROR;
    config->item_height = item_height;
    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_ConfigSetBreadcrumbHeight(MENU_Config_TypeDef *config,
    uint16_t breadcrumb_height) {
    if (config == NULL) return ESTA_ERROR;
    config->breadcrumb_height = breadcrumb_height;
    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_ConfigSetDisplayOptions(MENU_Config_TypeDef *config,
    bool is_show_frame, bool is_show_breadcrumb, bool is_fill_background) {
    if (config == NULL) return ESTA_ERROR;
    config->is_show_frame = is_show_frame;
    config->is_show_breadcrumb = is_show_breadcrumb;
    config->is_fill_background = is_fill_background;
    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_ConfigSetFontSize(MENU_Config_TypeDef *config,
    ESTA_FontSize font_size) {
    if (config == NULL) return ESTA_ERROR;
    if ((int)font_size < 0 || font_size >= ESTA_FONT_SIZE_COUNT) return ESTA_ERROR;
    config->font_size = font_size;
    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_ConfigSetTheme(MENU_Config_TypeDef *config,
    MENU_theme_type theme_type) {
    if (config == NULL) return ESTA_ERROR;
    config->theme_type = theme_type;
    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_Init(int inst, MENU_Config_TypeDef *MENU_Init) {
    if (!IS_VALID_MENU_INST(inst)) return ESTA_ERROR;
    if (MENU_Init == NULL) return ESTA_ERROR;
    if (!IS_VALID_MENU_THEME(MENU_Init->theme_type)) return ESTA_ERROR;
    if (MENU_Init->item_count > MENU_MAX_ITEMS) return ESTA_ERROR;
    if (MENU_Init->item_count > 0 && MENU_Init->items == NULL) return ESTA_ERROR;

    MENU_WRITE_CONFIG_INIT(inst, x_origin);
    MENU_WRITE_CONFIG_INIT(inst, y_origin);
    MENU_WRITE_CONFIG_INIT(inst, x_width);
    MENU_WRITE_CONFIG_INIT(inst, y_width);
    MENU_WRITE_CONFIG_INIT(inst, item_count);
    MENU_WRITE_CONFIG_INIT(inst, item_height);
    MENU_WRITE_CONFIG_INIT(inst, breadcrumb_height);
    MENU_WRITE_CONFIG_INIT(inst, is_show_frame);
    MENU_WRITE_CONFIG_INIT(inst, is_show_breadcrumb);
    MENU_WRITE_CONFIG_INIT(inst, is_fill_background);
    MENU_WRITE_CONFIG_INIT(inst, font_size);
    MENU_WRITE_CONFIG_INIT(inst, theme_type);
    MENU_WRITE_CONFIG(inst, items, NULL);

    memset(MENU_INST_ADDR(inst).MENU_Private.items, 0,
           sizeof(MENU_INST_ADDR(inst).MENU_Private.items));
    memset(MENU_INST_ADDR(inst).MENU_Private.nav_stack, 0,
           sizeof(MENU_INST_ADDR(inst).MENU_Private.nav_stack));
    memset(MENU_INST_ADDR(inst).MENU_Private.local_items, 0,
           sizeof(MENU_INST_ADDR(inst).MENU_Private.local_items));

    for (uint16_t i = 0; i < MENU_Init->item_count; i++) {
        MENU_ItemConfig *dst = &MENU_PRIVATE_MEMBER_ARRAY(inst, items, i);
        const MENU_ItemConfig *src = &MENU_Init->items[i];
        *dst = *src;
        MENU_CopyString(dst->label, src->label);
    }

    MENU_WRITE_PRIVATE(inst, nav_depth, 0);
    MENU_WRITE_PRIVATE(inst, selected_idx, 0);
    MENU_RebuildLocalList(inst);

    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_DeInit(int inst) {
    if (!IS_VALID_MENU_INST(inst)) return ESTA_ERROR;
    memset(&MENU_INST_ADDR(inst), 0, sizeof(MENU_INST_ADDR(inst)));
    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_Clear(int inst) {
    if (!IS_VALID_MENU_INST(inst)) return ESTA_ERROR;
    uint16_t theme = MENU_CONFIG_MEMBER(inst, theme_type);
    uint16_t bg = ESTA_THEME_COLOR(MENU_ColorTable, theme, MENU_THEME_BACKGROUND_INDEX);
    SCREEN_FILL(MENU_CONFIG_MEMBER(inst, x_origin), MENU_CONFIG_MEMBER(inst, y_origin),
                MENU_CONFIG_MEMBER(inst, x_origin) + MENU_CONFIG_MEMBER(inst, x_width),
                MENU_CONFIG_MEMBER(inst, y_origin) + MENU_CONFIG_MEMBER(inst, y_width), bg);
    return ESTA_OK;
}

ESTA_StatusTypeDef MENU_ReDraw(int inst) {
    if (!IS_VALID_MENU_INST(inst)) return ESTA_ERROR;

    uint16_t x = MENU_CONFIG_MEMBER(inst, x_origin);
    uint16_t y = MENU_CONFIG_MEMBER(inst, y_origin);
    uint16_t w = MENU_CONFIG_MEMBER(inst, x_width);
    uint16_t h = MENU_CONFIG_MEMBER(inst, y_width);
    uint16_t item_h = MENU_CONFIG_MEMBER(inst, item_height);
    uint16_t bread_h = MENU_CONFIG_MEMBER(inst, breadcrumb_height);
    uint16_t theme = MENU_CONFIG_MEMBER(inst, theme_type);
    ESTA_FontSize font_size = MENU_CONFIG_MEMBER(inst, font_size);
    uint16_t font_height = ui_font_height(font_size);
    uint8_t local_count = MENU_PRIVATE_MEMBER(inst, local_count);
    uint8_t selected = MENU_PRIVATE_MEMBER(inst, selected_idx);

    uint16_t bg = ESTA_THEME_COLOR(MENU_ColorTable, theme, MENU_THEME_BACKGROUND_INDEX);
    uint16_t text_color = ESTA_THEME_COLOR(MENU_ColorTable, theme, MENU_THEME_TEXT_INDEX);
    uint16_t sel_bg = ESTA_THEME_COLOR(MENU_ColorTable, theme, MENU_THEME_SELECTED_BG_INDEX);
    uint16_t sel_text = ESTA_THEME_COLOR(MENU_ColorTable, theme, MENU_THEME_SELECTED_TEXT_INDEX);
    uint16_t bread_color = ESTA_THEME_COLOR(MENU_ColorTable, theme, MENU_THEME_BREADCRUMB_INDEX);
    uint16_t frame = ESTA_THEME_COLOR(MENU_ColorTable, theme, MENU_THEME_FRAME_INDEX);
    uint16_t arrow = ESTA_THEME_COLOR(MENU_ColorTable, theme, MENU_THEME_ARROW_INDEX);

    if (MENU_CONFIG_MEMBER(inst, is_fill_background)) {
        SCREEN_FILL(x, y, x + w, y + h, bg);
    }

    if (MENU_CONFIG_MEMBER(inst, is_show_frame)) {
        SCREEN_DRAW_RECTANGLE(x, y, x + w, y + h, frame);
    }

    uint16_t content_y = y;

    /* Breadcrumb */
    if (MENU_CONFIG_MEMBER(inst, is_show_breadcrumb) && bread_h > 0) {
        char bread_buf[MENU_MAX_STRING_LEN * 3 + 16];
        MENU_BuildBreadcrumb(inst, bread_buf, sizeof(bread_buf));
        uint8_t bread_len = MENU_StrLen16(bread_buf);
        if (bread_len > 0) {
            uint16_t bread_y = (uint16_t)(content_y + (bread_h > font_height ?
                                    (bread_h - font_height) / 2 : 0));
            SCREEN_DRAW_STRING_FONT(x + 2, bread_y, bread_buf, bread_len,
                                    font_size, bread_color);
        }
        content_y = (uint16_t)(content_y + bread_h);
    }

    /* Items */
    for (uint8_t i = 0; i < local_count; i++) {
        uint16_t row_y_abs = (uint16_t)(content_y + i * item_h);
        uint16_t row_bottom = (uint16_t)(row_y_abs + item_h);

        if (row_bottom > y + h) {
            break; /* no scroll support, stop drawing */
        }

        uint8_t global_idx = MENU_PRIVATE_MEMBER_ARRAY(inst, local_items, i);
        const MENU_ItemConfig *item = &MENU_PRIVATE_MEMBER_ARRAY(inst, items, global_idx);
        uint8_t label_len = MENU_StrLen16(item->label);
        uint16_t text_y = (uint16_t)(row_y_abs + (item_h > font_height ?
                                     (item_h - font_height) / 2 : 0));

        if (i == selected) {
            SCREEN_FILL(x, row_y_abs, x + w, row_bottom, sel_bg);
        }

        /* label */
        SCREEN_DRAW_STRING_FONT(x + 2, text_y, item->label, label_len,
                                font_size, (i == selected) ? sel_text : text_color);

        /* submenu arrow indicator */
        if (item->is_submenu) {
            uint16_t arrow_x = (uint16_t)(x + w - 12U);
            if (arrow_x > x + label_len * ui_font_width(font_size) + 4U) {
                SCREEN_DRAW_STRING_FONT(arrow_x, text_y, ">", 1,
                                        font_size, arrow);
            }
        }
    }

    return ESTA_OK;
}

void MENU_ProcessInput(int inst) {
    if (!IS_VALID_MENU_INST(inst)) return;

    ESTA_Event evt;
    bool state_changed = false;

    /* Gather and re-push helper */
    ESTA_Event held[16];
    uint8_t held_count = 0;

    while (ESTA_EventPoll(&evt)) {
        if (evt.type == ESTA_EVENT_BUTTON_PRESS) {
            switch (evt.source) {
                case 0: /* UP */
                    if (MENU_PRIVATE_MEMBER(inst, selected_idx) > 0) {
                        MENU_WRITE_PRIVATE(inst, selected_idx,
                            (uint8_t)(MENU_PRIVATE_MEMBER(inst, selected_idx) - 1U));
                        state_changed = true;
                    }
                    break;

                case 1: /* DOWN */
                    if (MENU_PRIVATE_MEMBER(inst, selected_idx) + 1U <
                        MENU_PRIVATE_MEMBER(inst, local_count)) {
                        MENU_WRITE_PRIVATE(inst, selected_idx,
                            (uint8_t)(MENU_PRIVATE_MEMBER(inst, selected_idx) + 1U));
                        state_changed = true;
                    }
                    break;

                case 2: /* ENTER */
                    {
                        uint8_t sel = MENU_PRIVATE_MEMBER(inst, selected_idx);
                        if (sel < MENU_PRIVATE_MEMBER(inst, local_count)) {
                            uint8_t global_idx = MENU_PRIVATE_MEMBER_ARRAY(inst, local_items, sel);
                            const MENU_ItemConfig *item =
                                &MENU_PRIVATE_MEMBER_ARRAY(inst, items, global_idx);
                            if (item->is_submenu) {
                                uint8_t depth = MENU_PRIVATE_MEMBER(inst, nav_depth);
                                if (depth < MENU_MAX_DEPTH) {
                                    MENU_WRITE_PRIVATE_ARRAY(inst, nav_stack, depth, global_idx);
                                    MENU_WRITE_PRIVATE(inst, nav_depth, (uint8_t)(depth + 1U));
                                    MENU_RebuildLocalList(inst);
                                    state_changed = true;
                                }
                            } else {
                                ESTA_EventEmitMenuSelect((uint8_t)inst, item->event_id);
                            }
                        }
                    }
                    break;

                case 3: /* BACK */
                    {
                        uint8_t depth = MENU_PRIVATE_MEMBER(inst, nav_depth);
                        if (depth > 0) {
                            MENU_WRITE_PRIVATE(inst, nav_depth, (uint8_t)(depth - 1U));
                            MENU_RebuildLocalList(inst);
                            state_changed = true;
                        }
                    }
                    break;

                default:
                    held[held_count++] = evt;
                    break;
            }
        } else {
            held[held_count++] = evt;
        }
    }

    /* Re-push unconsumed events */
    for (uint8_t i = 0; i < held_count; i++) {
        ESTA_EventPush(&held[i]);
    }

    if (state_changed) {
        MENU_ReDraw(inst);
    }
}

bool MENU_HandleEvent(int inst, const ESTA_Event *event) {
    if (!IS_VALID_MENU_INST(inst)) return false;
    if (event == NULL) return false;
    if (event->type != ESTA_EVENT_BUTTON_PRESS) return false;

    bool state_changed = false;

    switch (event->source) {
        case 0: /* UP */
            if (MENU_PRIVATE_MEMBER(inst, selected_idx) > 0) {
                MENU_WRITE_PRIVATE(inst, selected_idx,
                    (uint8_t)(MENU_PRIVATE_MEMBER(inst, selected_idx) - 1U));
                state_changed = true;
            }
            break;

        case 1: /* DOWN */
            if (MENU_PRIVATE_MEMBER(inst, selected_idx) + 1U <
                MENU_PRIVATE_MEMBER(inst, local_count)) {
                MENU_WRITE_PRIVATE(inst, selected_idx,
                    (uint8_t)(MENU_PRIVATE_MEMBER(inst, selected_idx) + 1U));
                state_changed = true;
            }
            break;

        case 2: /* ENTER */
            {
                uint8_t sel = MENU_PRIVATE_MEMBER(inst, selected_idx);
                if (sel < MENU_PRIVATE_MEMBER(inst, local_count)) {
                    uint8_t global_idx = MENU_PRIVATE_MEMBER_ARRAY(inst, local_items, sel);
                    const MENU_ItemConfig *item =
                        &MENU_PRIVATE_MEMBER_ARRAY(inst, items, global_idx);
                    if (item->is_submenu) {
                        uint8_t depth = MENU_PRIVATE_MEMBER(inst, nav_depth);
                        if (depth < MENU_MAX_DEPTH) {
                            MENU_WRITE_PRIVATE_ARRAY(inst, nav_stack, depth, global_idx);
                            MENU_WRITE_PRIVATE(inst, nav_depth, (uint8_t)(depth + 1U));
                            MENU_RebuildLocalList(inst);
                            state_changed = true;
                        }
                    } else {
                        ESTA_EventEmitMenuSelect((uint8_t)inst, item->event_id);
                    }
                }
            }
            break;

        case 3: /* BACK */
            {
                uint8_t depth = MENU_PRIVATE_MEMBER(inst, nav_depth);
                if (depth > 0) {
                    MENU_WRITE_PRIVATE(inst, nav_depth, (uint8_t)(depth - 1U));
                    MENU_RebuildLocalList(inst);
                    state_changed = true;
                }
            }
            break;

        default:
            return false;
    }

    if (state_changed) {
        MENU_ReDraw(inst);
    }
    return true;
}

bool MENU_NavUp(int inst) {
    if (!IS_VALID_MENU_INST(inst)) return false;
    if (MENU_PRIVATE_MEMBER(inst, selected_idx) > 0) {
        MENU_WRITE_PRIVATE(inst, selected_idx,
            (uint8_t)(MENU_PRIVATE_MEMBER(inst, selected_idx) - 1U));
        MENU_ReDraw(inst);
        return true;
    }
    return false;
}

bool MENU_NavDown(int inst) {
    if (!IS_VALID_MENU_INST(inst)) return false;
    if (MENU_PRIVATE_MEMBER(inst, selected_idx) + 1U <
        MENU_PRIVATE_MEMBER(inst, local_count)) {
        MENU_WRITE_PRIVATE(inst, selected_idx,
            (uint8_t)(MENU_PRIVATE_MEMBER(inst, selected_idx) + 1U));
        MENU_ReDraw(inst);
        return true;
    }
    return false;
}

bool MENU_NavEnter(int inst) {
    if (!IS_VALID_MENU_INST(inst)) return false;
    uint8_t sel = MENU_PRIVATE_MEMBER(inst, selected_idx);
    if (sel >= MENU_PRIVATE_MEMBER(inst, local_count)) return false;
    uint8_t global_idx = MENU_PRIVATE_MEMBER_ARRAY(inst, local_items, sel);
    const MENU_ItemConfig *item = &MENU_PRIVATE_MEMBER_ARRAY(inst, items, global_idx);
    if (item->is_submenu) {
        uint8_t depth = MENU_PRIVATE_MEMBER(inst, nav_depth);
        if (depth < MENU_MAX_DEPTH) {
            MENU_WRITE_PRIVATE_ARRAY(inst, nav_stack, depth, global_idx);
            MENU_WRITE_PRIVATE(inst, nav_depth, (uint8_t)(depth + 1U));
            MENU_RebuildLocalList(inst);
            MENU_ReDraw(inst);
            return true;
        }
    } else {
        ESTA_EventEmitMenuSelect((uint8_t)inst, item->event_id);
        return true;
    }
    return false;
}

bool MENU_NavBack(int inst) {
    if (!IS_VALID_MENU_INST(inst)) return false;
    uint8_t depth = MENU_PRIVATE_MEMBER(inst, nav_depth);
    if (depth > 0) {
        MENU_WRITE_PRIVATE(inst, nav_depth, (uint8_t)(depth - 1U));
        MENU_RebuildLocalList(inst);
        MENU_ReDraw(inst);
        return true;
    }
    return false;
}

ESTA_StatusTypeDef MENU_UpdateItemLabel(int inst, uint8_t item_idx, const char *label) {
    if (!IS_VALID_MENU_INST(inst) || !IS_VALID_MENU_ITEM(item_idx) || label == NULL)
        return ESTA_ERROR;
    uint16_t item_count = MENU_CONFIG_MEMBER(inst, item_count);
    if (item_idx >= item_count) return ESTA_ERROR;
    strncpy(MENU_PRIVATE_MEMBER_ARRAY(inst, items, item_idx).label, label, MENU_MAX_STRING_LEN);
    MENU_PRIVATE_MEMBER_ARRAY(inst, items, item_idx).label[MENU_MAX_STRING_LEN] = '\0';
    return ESTA_OK;
}