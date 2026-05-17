#ifndef __APP_ACTION_H
#define __APP_ACTION_H

#include "app/app_event.h"

#define ESTA_PROFILE_MAX_BINDINGS 8
#define ESTA_SOURCE_ANY 0xFF
#define ESTA_TRIGGER_ID_ANY 0xFFFF

typedef enum {
    ESTA_TARGET_WAVE = 0,
    ESTA_TARGET_BARCHART,
    ESTA_TARGET_TABLE,
    ESTA_TARGET_MENU,
    ESTA_TARGET_PAGE,
    ESTA_TARGET_GLOBAL,
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
    ESTA_ACTION_CUSTOM        = 0xFF
} ESTA_ActionType;

typedef struct {
    uint8_t  trigger;      /* ESTA_EventType */
    uint8_t  source_id;    /* source index, ESTA_SOURCE_ANY = match all */
    uint16_t trigger_id;   /* event ID filter, ESTA_TRIGGER_ID_ANY = match all */
    uint8_t  target_type;  /* ESTA_TargetType */
    uint8_t  target_inst;  /* target instance (0 for PAGE/GLOBAL) */
    uint8_t  action;       /* ESTA_ActionType */
} ESTA_EventBinding_TypeDef;

typedef struct {
    void *app;
    uint8_t target_type;
    uint8_t target_inst;
} App_BindingContext;

ESTA_EventHandler App_ActionGetHandler(ESTA_ActionType action);

#endif
