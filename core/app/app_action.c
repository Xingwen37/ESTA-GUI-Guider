#include "app/app_action.h"
#include "app/app_main.h"
#include "profile/ESTA_Profile.h"
#include "ui/WAVE.h"
#include "ui/MENU.h"
#include "ui/TABLE.h"
#include "event/event_flag.h"

#include <stdio.h>

static bool action_page_next(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL || s->page_state.page_count <= 1) return false;
    App_PageNext(&s->page_state);
    printf("Switched to page %d\n", App_GetActivePage(&s->page_state));
    return true;
}

static bool action_page_prev(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL || s->page_state.page_count <= 1) return false;
    uint8_t cur = App_GetActivePage(&s->page_state);
    uint8_t prev = (cur == 0) ? (s->page_state.page_count - 1) : (cur - 1);
    App_ApplyAndDrawPage(&s->page_state, prev);
    printf("Switched to page %d\n", prev);
    return true;
}

static bool action_theme_toggle(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL || s->wave_inst_count == 0) return false;

    int target = ctx->target_inst;
    if (target >= s->wave_inst_count) return false;

    uint8_t active = App_GetActivePage(&s->page_state);
    if (s->page_state.profiles->wave_profiles[target].page != active) return false;

    WAVE_theme_type cur = WAVE_CONFIG_MEMBER(target, theme_type);
    WAVE_theme_type next = (cur == WAVE_THEME_DEFAULT)
                           ? WAVE_THEME_LIGHT : WAVE_THEME_DEFAULT;
    WAVE_WRITE_CONFIG(target, theme_type, next);
    WAVE_ReDraw(WAVE_INST(target));
    return true;
}

static bool action_wave_redraw(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL || s->wave_redraw_fn == NULL) return false;
    s->wave_redraw_fn(ctx->target_inst, s->wave_redraw_ctx);
    return true;
}

static bool action_menu_up(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL) return false;
    int target = ctx->target_inst;
    if (target >= s->menu_inst_count) return false;
    uint8_t active = App_GetActivePage(&s->page_state);
    if (s->page_state.profiles->menu_profiles[target].page != active) return false;
    return MENU_NavUp(MENU_INST(target));
}

static bool action_menu_down(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL) return false;
    int target = ctx->target_inst;
    if (target >= s->menu_inst_count) return false;
    uint8_t active = App_GetActivePage(&s->page_state);
    if (s->page_state.profiles->menu_profiles[target].page != active) return false;
    return MENU_NavDown(MENU_INST(target));
}

static bool action_menu_enter(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL) return false;
    int target = ctx->target_inst;
    if (target >= s->menu_inst_count) return false;
    uint8_t active = App_GetActivePage(&s->page_state);
    if (s->page_state.profiles->menu_profiles[target].page != active) return false;
    return MENU_NavEnter(MENU_INST(target));
}

static bool action_menu_back(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL) return false;
    int target = ctx->target_inst;
    if (target >= s->menu_inst_count) return false;
    uint8_t active = App_GetActivePage(&s->page_state);
    if (s->page_state.profiles->menu_profiles[target].page != active) return false;
    return MENU_NavBack(MENU_INST(target));
}

static bool action_flag_set(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    ESTA_FlagSet(ctx->target_inst);
    return true;
}

static bool action_text_set(const ESTA_Event *evt, void *user_data) {
    (void)evt;
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL) return false;

    uint8_t str_idx = ctx->param;
    const ESTA_ProfileSet_TypeDef *p = s->page_state.profiles;
    if (str_idx >= p->string_count) return false;

    const ESTA_StringEntry_TypeDef *entry = &p->strings[str_idx];

    switch (ctx->target_type) {
        case ESTA_TARGET_WAVE: {
            WAVE_UpdateRulerUnit(ctx->target_inst, entry->sub_addr, entry->text);
            return true;
        }
        case ESTA_TARGET_TABLE: {
            uint8_t row = entry->sub_addr / TABLE_MAX_COLS;
            uint8_t col = entry->sub_addr % TABLE_MAX_COLS;
            TABLE_UpdateText(ctx->target_inst, row, col, entry->text);
            TABLE_ReDraw(ctx->target_inst);
            return true;
        }
        case ESTA_TARGET_MENU: {
            MENU_UpdateItemLabel(ctx->target_inst, entry->sub_addr, entry->text);
            MENU_ReDraw(ctx->target_inst);
            return true;
        }
        default:
            return false;
    }
}

static bool action_sequence(const ESTA_Event *evt, void *user_data) {
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    App_MainState *s = (App_MainState *)ctx->app;
    if (s == NULL) return false;
    uint8_t seq_idx = ctx->param;
    const ESTA_ProfileSet_TypeDef *p = s->page_state.profiles;
    if (seq_idx >= p->sequence_count) return false;

    const ESTA_ActionSequence_TypeDef *seq = &p->sequences[seq_idx];
    bool any = false;
    for (uint8_t i = 0; i < seq->step_count; i++) {
        const ESTA_ActionStep_TypeDef *step = &seq->steps[i];
        ESTA_EventHandler h = App_ActionGetHandler((ESTA_ActionType)step->action);
        if (h == NULL) continue;
        App_BindingContext step_ctx = {
            .app = ctx->app,
            .target_type = step->target_type,
            .target_inst = step->target_inst,
            .param = step->param,
        };
        if (h(evt, &step_ctx)) any = true;
    }
    return any;
}

static ESTA_EventHandler g_custom_actions[ESTA_CUSTOM_ACTION_MAX];

static bool action_custom_dispatch(const ESTA_Event *evt, void *user_data) {
    App_BindingContext *ctx = (App_BindingContext *)user_data;
    uint8_t id = ctx->param;
    if (id >= ESTA_CUSTOM_ACTION_MAX || g_custom_actions[id] == NULL) return false;
    return g_custom_actions[id](evt, user_data);
}

void App_RegisterCustomAction(uint8_t custom_id, ESTA_EventHandler handler) {
    if (custom_id < ESTA_CUSTOM_ACTION_MAX) {
        g_custom_actions[custom_id] = handler;
    }
}

static const ESTA_EventHandler g_action_table[] = {
    [ESTA_ACTION_NONE]          = NULL,
    [ESTA_ACTION_PAGE_NEXT]     = action_page_next,
    [ESTA_ACTION_PAGE_PREV]     = action_page_prev,
    [ESTA_ACTION_THEME_TOGGLE]  = action_theme_toggle,
    [ESTA_ACTION_WAVE_REDRAW]   = action_wave_redraw,
    [ESTA_ACTION_MENU_UP]       = action_menu_up,
    [ESTA_ACTION_MENU_DOWN]     = action_menu_down,
    [ESTA_ACTION_MENU_ENTER]    = action_menu_enter,
    [ESTA_ACTION_MENU_BACK]     = action_menu_back,
    [ESTA_ACTION_FLAG_SET]      = action_flag_set,
    [ESTA_ACTION_TEXT_SET]      = action_text_set,
    [ESTA_ACTION_SEQUENCE]      = action_sequence,
};

#define ACTION_TABLE_SIZE (sizeof(g_action_table) / sizeof(g_action_table[0]))

ESTA_EventHandler App_ActionGetHandler(ESTA_ActionType action) {
    if (action == ESTA_ACTION_CUSTOM) {
        return action_custom_dispatch;
    }
    if ((unsigned)action >= ACTION_TABLE_SIZE) {
        return NULL;
    }
    return g_action_table[action];
}
