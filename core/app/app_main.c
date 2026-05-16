#include "app/app_main.h"
#include "ui/WAVE.h"
#include "ui/BARCHART.h"
#include "ui/TABLE.h"
#include "ui/MENU.h"

#include <stdio.h>

static bool on_menu_button(const ESTA_Event *evt, void *user_data) {
    App_MainState *s = (App_MainState *)user_data;
    const ESTA_ProfileSet_TypeDef *p = s->page_state.profiles;
    uint8_t active = App_GetActivePage(&s->page_state);
    for (int i = 0; i < s->menu_inst_count; i++) {
        if (p->menu_profiles[i].page != active) continue;
        if (MENU_HandleEvent(MENU_INST(i), evt)) return true;
    }
    return false;
}

static bool on_page_switch(const ESTA_Event *evt, void *user_data) {
    App_MainState *s = (App_MainState *)user_data;
    if (evt->source == 1 && s->page_state.page_count > 1) {
        App_PageNext(&s->page_state);
        printf("Switched to page %d\n", App_GetActivePage(&s->page_state));
        return true;
    }
    return false;
}

void App_MainInit(App_MainState *state, uint16_t screen_w, uint16_t screen_h) {
    if (state == NULL) return;

    const ESTA_ProfileSet_TypeDef *profiles = ESTA_Profile_GetDefault();
    if (profiles == NULL) return;

    state->wave_inst_count  = profiles->wave_inst_count;
    if (state->wave_inst_count > MAX_WAVE_NUM) state->wave_inst_count = MAX_WAVE_NUM;
    state->bar_inst_count   = profiles->bar_inst_count;
    if (state->bar_inst_count > BARCHART_MAX_NUM) state->bar_inst_count = BARCHART_MAX_NUM;
    state->table_inst_count = profiles->table_inst_count;
    if (state->table_inst_count > TABLE_MAX_NUM) state->table_inst_count = TABLE_MAX_NUM;
    state->menu_inst_count  = profiles->menu_inst_count;
    if (state->menu_inst_count > MENU_MAX_NUM) state->menu_inst_count = MENU_MAX_NUM;

    App_PageInit(&state->page_state, profiles, screen_w, screen_h);
    App_ApplyAndDrawPage(&state->page_state, 0);

    ESTA_Profile_ApplyEvents(profiles);
    App_EventInit();
    App_Subscribe(ESTA_EVENT_BUTTON_PRESS, 0, 3, on_menu_button, state);
    App_Subscribe(ESTA_EVENT_BUTTON_PRESS, 0, APP_SOURCE_ANY, on_page_switch, state);
}

void App_MainTick(App_MainState *state) {
    (void)state;
    App_DispatchEvents();
}

App_PageState *App_MainGetPageState(App_MainState *state) {
    if (state == NULL) return NULL;
    return &state->page_state;
}
