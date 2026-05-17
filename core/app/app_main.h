#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#include "app/app_page.h"
#include "app/app_event.h"

typedef void (*App_WaveRedrawFn)(int inst, void *ctx);

typedef struct {
    App_PageState page_state;
    int wave_inst_count;
    int bar_inst_count;
    int table_inst_count;
    int menu_inst_count;
    App_WaveRedrawFn wave_redraw_fn;
    void *wave_redraw_ctx;
} App_MainState;

void App_MainInit(App_MainState *state, uint16_t screen_w, uint16_t screen_h);
void App_MainTick(App_MainState *state);

App_PageState *App_MainGetPageState(App_MainState *state);

#endif
