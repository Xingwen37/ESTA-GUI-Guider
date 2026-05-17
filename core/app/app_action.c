#include "app/app_action.h"
#include "app/app_main.h"
#include "ui/WAVE.h"
#include "ui/MENU.h"
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
    (void)user_data;
    ESTA_FlagSet(ESTA_FLAG_WAVE_REDRAW);
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
};

#define ACTION_TABLE_SIZE (sizeof(g_action_table) / sizeof(g_action_table[0]))

ESTA_EventHandler App_ActionGetHandler(ESTA_ActionType action) {
    if (action == ESTA_ACTION_CUSTOM || (unsigned)action >= ACTION_TABLE_SIZE) {
        return NULL;
    }
    return g_action_table[action];
}
