#include "app/app_main.h"
#include "ui/WAVE.h"
#include "ui/BARCHART.h"
#include "ui/TABLE.h"
#include "ui/MENU.h"

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

    ESTA_Profile_ApplyEvents(profiles, state);
}

void App_MainTick(App_MainState *state) {
    (void)state;
    App_DispatchEvents();
}

App_PageState *App_MainGetPageState(App_MainState *state) {
    if (state == NULL) return NULL;
    return &state->page_state;
}
