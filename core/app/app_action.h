#ifndef __APP_ACTION_H
#define __APP_ACTION_H

#include "app/app_event.h"

#define ESTA_PROFILE_MAX_BINDINGS 8
#define ESTA_SOURCE_ANY 0xFF
#define ESTA_TRIGGER_ID_ANY 0xFFFF
#define ESTA_CUSTOM_ACTION_MAX 8

typedef enum {
    ESTA_TARGET_WAVE = 0,
    ESTA_TARGET_BARCHART,
    ESTA_TARGET_TABLE,
    ESTA_TARGET_MENU,
    ESTA_TARGET_PAGE,
    ESTA_TARGET_GLOBAL,
    ESTA_TARGET_FLAG,
} ESTA_TargetType;

typedef enum {
    ESTA_ACTION_NONE          = 0,
    ESTA_ACTION_PAGE_NEXT     = 1,
    ESTA_ACTION_PAGE_PREV     = 2,
    ESTA_ACTION_THEME_TOGGLE  = 3,
    ESTA_ACTION_WAVE_REDRAW   = 4,
    ESTA_ACTION_MENU_UP       = 5,
    ESTA_ACTION_MENU_DOWN     = 6,
    ESTA_ACTION_MENU_ENTER    = 7,
    ESTA_ACTION_MENU_BACK     = 8,
    ESTA_ACTION_FLAG_SET      = 9,   /* 仅置位状态位，不推送事件（与 FLAG_CLEAR 对称） */
    ESTA_ACTION_TEXT_SET      = 10,
    ESTA_ACTION_SEQUENCE      = 11,
    ESTA_ACTION_FLAG_CLEAR    = 12,
    ESTA_ACTION_FLAG_SIGNAL   = 13,  /* 置位状态位 + 推送 FLAG 事件 */
    ESTA_ACTION_BARCHART_REDRAW = 14,
    ESTA_ACTION_CUSTOM        = 0xFF
} ESTA_ActionType;

typedef struct {
    uint8_t  trigger;      /* ESTA_EventType */
    uint8_t  source_id;    /* source index, ESTA_SOURCE_ANY = match all */
    uint16_t trigger_id;   /* event ID filter, ESTA_TRIGGER_ID_ANY = match all */
    uint8_t  target_type;  /* ESTA_TargetType */
    uint8_t  target_inst;  /* target instance (0 for PAGE/GLOBAL) */
    uint8_t  action;       /* ESTA_ActionType */
    uint8_t  param;        /* action parameter (TEXT_SET: string table index) */
    uint8_t  guard_and_mask; /* all these Flags must be set (AND), 0 = no check */
    uint8_t  guard_or_mask;  /* at least one of these Flags must be set (OR), 0 = no check */
    uint8_t  guard_inv_mask; /* all these Flags must be clear (AND-NOT), 0 = no check */
} ESTA_EventBinding_TypeDef;

typedef struct {
    void *app;
    uint8_t target_type;
    uint8_t target_inst;
    uint8_t param;
    uint8_t guard_and_mask;
    uint8_t guard_or_mask;
    uint8_t guard_inv_mask;
} App_BindingContext;

ESTA_EventHandler App_ActionGetHandler(ESTA_ActionType action);
void App_RegisterCustomAction(uint8_t custom_id, ESTA_EventHandler handler);

#endif
