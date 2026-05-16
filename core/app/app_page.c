#include "app/app_page.h"
#include "infra/ui_base.h"
#include "ui/WAVE.h"
#include "ui/BARCHART.h"
#include "ui/TABLE.h"
#include "ui/MENU.h"

void App_PageInit(App_PageState *state, const ESTA_ProfileSet_TypeDef *profiles,
                  uint16_t screen_width, uint16_t screen_height) {
    if (state == NULL || profiles == NULL) return;
    state->profiles = profiles;
    state->active_page = 0;
    state->page_count = profiles->page_count;
    if (state->page_count == 0) state->page_count = 1;
    state->screen_width = screen_width;
    state->screen_height = screen_height;
}

void App_ApplyAndDrawPage(App_PageState *state, uint8_t page) {
    if (state == NULL || state->profiles == NULL) return;

    const ESTA_ProfileSet_TypeDef *p = state->profiles;
    state->active_page = page;

    SCREEN_FILL(0, 0, state->screen_width, state->screen_height, 0x0000);
    ESTA_Profile_SetActivePage(page);

    for (int i = 0; i < p->wave_inst_count; i++) {
        if (p->wave_profiles[i].page == page) {
            ESTA_Profile_Apply(WAVE_INST(i), &p->wave_profiles[i]);
            WAVE_ReDraw(WAVE_INST(i));
        }
    }
    for (int i = 0; i < p->bar_inst_count; i++) {
        if (p->bar_profiles[i].page == page) {
            ESTA_Profile_ApplyBARCHART(BARCHART_INST(i), &p->bar_profiles[i]);
            BARCHART_ReDraw(BARCHART_INST(i));
        }
    }
    for (int i = 0; i < p->table_inst_count; i++) {
        if (p->table_profiles[i].page == page) {
            ESTA_Profile_ApplyTABLE(TABLE_INST(i), &p->table_profiles[i]);
            TABLE_ReDraw(TABLE_INST(i));
        }
    }
    for (int i = 0; i < p->menu_inst_count; i++) {
        if (p->menu_profiles[i].page == page) {
            ESTA_Profile_ApplyMENU(MENU_INST(i), &p->menu_profiles[i]);
            MENU_ReDraw(MENU_INST(i));
        }
    }
}

void App_PageNext(App_PageState *state) {
    if (state == NULL || state->page_count <= 1) return;
    uint8_t next = (uint8_t)((state->active_page + 1) % state->page_count);
    App_ApplyAndDrawPage(state, next);
}

uint8_t App_GetActivePage(const App_PageState *state) {
    if (state == NULL) return 0;
    return state->active_page;
}
