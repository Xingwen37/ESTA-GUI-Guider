/**
  ******************************************************************************
  * @file           : main.c (Simulator Version)
  * @brief          : SDL2 Simulator Main program body for ESTA Library
  * @author         : TongLewis(yangyutong) HEU ESTA 2025
  ******************************************************************************
  */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "ui/WAVE.h"
#include "ui/BARCHART.h"
#include "ui/MENU.h"
#include "profile/ESTA_Profile.h"
#include "event/event.h"
#include "app/app_event.h"
#include "app/app_page.h"
#include "sim_scenario.h"
#include "esta_port_sdl2.h"
#include "btn_ui.h"

#define SIM_BATCH_MAX_POINTS 320

/* ==================== Simulation state ==================== */

typedef struct {
    SimScenarioRuntime scenario;
    const ESTA_ProfileSet_TypeDef *profiles;
    App_PageState page_state;

    int wave_inst_count;
    int bar_inst_count;
    int table_inst_count;
    int menu_inst_count;

    uint16_t data_wave[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL];
    uint16_t data_bar[SIM_SCENARIO_BARCHART_COUNT][BARCHART_MAX_BARS];
    uint16_t ch_buf[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL][SIM_BATCH_MAX_POINTS];
    uint16_t batch_window_len[SIM_SCENARIO_WAVE_COUNT];
    uint16_t bar_count[SIM_SCENARIO_BARCHART_COUNT];
} SimState;

/* ==================== Event handlers ==================== */

static SimState g_sim;

static bool on_menu_button(const ESTA_Event *evt, void *user_data) {
    SimState *sim = (SimState *)user_data;
    uint8_t active = App_GetActivePage(&sim->page_state);
    for (int i = 0; i < sim->menu_inst_count; i++) {
        if (sim->profiles->menu_profiles[i].page != active) continue;
        if (MENU_HandleEvent(MENU_INST(i), evt)) return true;
    }
    return false;
}

static bool on_button_press(const ESTA_Event *evt, void *user_data) {
    SimState *sim = (SimState *)user_data;
    if (evt->source == 0 &&
        sim->wave_inst_count > 0 &&
        sim->profiles->wave_profiles[0].page == App_GetActivePage(&sim->page_state)) {
        WAVE_theme_type cur = WAVE_CONFIG_MEMBER(0, theme_type);
        WAVE_theme_type next = (cur == WAVE_THEME_DEFAULT)
                               ? WAVE_THEME_LIGHT : WAVE_THEME_DEFAULT;
        WAVE_WRITE_CONFIG(0, theme_type, next);
        WAVE_ReDraw(WAVE_INST(0));
        return true;
    } else if (evt->source == 1 && sim->page_state.page_count > 1) {
        App_PageNext(&sim->page_state);
        printf("Switched to page %d\n", App_GetActivePage(&sim->page_state));
        return true;
    }
    return false;
}

static bool on_menu_select(const ESTA_Event *evt, void *user_data) {
    (void)user_data;
    printf("MENU SELECT: event_id=%d\n", evt->id);
    return true;
}

/* ==================== Sim data feed helpers ==================== */

static bool sim_feed_wave(SimState *sim) {
    uint8_t active = App_GetActivePage(&sim->page_state);
    for (int i = 0; i < sim->wave_inst_count; i++) {
        if (sim->profiles->wave_profiles[i].page != active) continue;
        if (!SimScenario_GetNextFrame(&sim->scenario, i, sim->data_wave[i])) {
            return false;
        }
        uint16_t xfw = sim->batch_window_len[i];
        if (xfw == 0) continue;

        uint8_t mask = WAVE_CONFIG_MEMBER(i, channel_mask);
        for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
            if (is_channel_enabled(mask, (uint8_t)(CH0 << ch))) {
                memmove(&sim->ch_buf[i][ch][0], &sim->ch_buf[i][ch][1],
                        (xfw - 1U) * sizeof(sim->ch_buf[i][ch][0]));
                sim->ch_buf[i][ch][xfw - 1U] = sim->data_wave[i][ch];
            }
        }

        WAVE_CurveClear(WAVE_INST(i));
        for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
            if (is_channel_enabled(mask, (uint8_t)(CH0 << ch))) {
                WAVE_WRITE_PRIVATE(i, last_index, 0);
                WAVE_WRITE_PRIVATE(i, x_coor_last,
                    WAVE_CONFIG_MEMBER(i, x_origin));
                WAVE_CurveDrawBatch(WAVE_INST(i), ch, sim->ch_buf[i][ch], xfw);
            }
        }
    }
    return true;
}

static void sim_feed_barchart(SimState *sim) {
    uint8_t active = App_GetActivePage(&sim->page_state);
    for (int i = 0; i < sim->bar_inst_count; i++) {
        if (sim->profiles->bar_profiles[i].page != active) continue;
        if (SimScenario_BARCHART_GetData(&sim->scenario, sim->data_bar[i], sim->bar_count[i])) {
            BARCHART_UpdateAll(BARCHART_INST(i), sim->data_bar[i], sim->bar_count[i]);
        }
    }
}

static void sim_feed_table(SimState *sim) {
    uint8_t active = App_GetActivePage(&sim->page_state);
    for (int i = 0; i < sim->table_inst_count; i++) {
        if (sim->profiles->table_profiles[i].page != active) continue;
        TABLE_UpdateUInt32(TABLE_INST(i), 0, 1, sim->data_wave[0][0]);
        TABLE_UpdateUInt32(TABLE_INST(i), 1, 1, (uint32_t)(1000U + sim->scenario.tick * 10U));
    }
}

/* ==================== Main ==================== */

int main(int argc, char *argv[])
{
    const char *screenshot_path = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            screenshot_path = argv[++i];
        }
    }

    ESTA_SDL2_Init();

    if (!SimScenario_LoadDefault(&g_sim.scenario)) {
        printf("SimScenario_LoadDefault failed.\n");
        ESTA_SDL2_Quit();
        return 1;
    }

    g_sim.profiles = ESTA_Profile_GetDefault();
    if (g_sim.profiles == NULL) {
        printf("ESTA_Profile_GetDefault failed.\n");
        ESTA_SDL2_Quit();
        return 1;
    }

    const ESTA_ProfileSet_TypeDef *profiles = g_sim.profiles;

    g_sim.wave_inst_count = profiles->wave_inst_count;
    if (g_sim.wave_inst_count > SIM_SCENARIO_WAVE_COUNT) g_sim.wave_inst_count = SIM_SCENARIO_WAVE_COUNT;
    if (g_sim.wave_inst_count > MAX_WAVE_NUM) g_sim.wave_inst_count = MAX_WAVE_NUM;

    g_sim.bar_inst_count = profiles->bar_inst_count;
    if (g_sim.bar_inst_count > SIM_SCENARIO_BARCHART_COUNT) g_sim.bar_inst_count = SIM_SCENARIO_BARCHART_COUNT;
    if (g_sim.bar_inst_count > BARCHART_MAX_NUM) g_sim.bar_inst_count = BARCHART_MAX_NUM;

    g_sim.table_inst_count = profiles->table_inst_count;
    if (g_sim.table_inst_count > TABLE_MAX_NUM) g_sim.table_inst_count = TABLE_MAX_NUM;

    g_sim.menu_inst_count = profiles->menu_inst_count;
    if (g_sim.menu_inst_count > MENU_MAX_NUM) g_sim.menu_inst_count = MENU_MAX_NUM;

    App_PageInit(&g_sim.page_state, profiles,
                 SIMULATOR_SCREEN_WIDTH / 2, SIMULATOR_SCREEN_HEIGHT / 2);
    App_ApplyAndDrawPage(&g_sim.page_state, 0);

    if (screenshot_path) {
        for (uint8_t p = 0; p < g_sim.page_state.page_count; p++) {
            App_ApplyAndDrawPage(&g_sim.page_state, p);
            char filename[256];
            snprintf(filename, sizeof(filename), "%s_%d.bmp", screenshot_path, p);
            ESTA_SDL2_SaveScreenshot(filename);
        }
        ESTA_SDL2_Quit();
        return 0;
    }

    ESTA_Profile_ApplyEvents(profiles);
    App_EventInit();
    App_Subscribe(ESTA_EVENT_BUTTON_PRESS, 0, 3, on_menu_button, &g_sim);
    App_Subscribe(ESTA_EVENT_BUTTON_PRESS, 0, APP_SOURCE_ANY, on_button_press, &g_sim);
    App_Subscribe(ESTA_EVENT_MENU_SELECT, 0, APP_SOURCE_ANY, on_menu_select, NULL);
    BTN_UI_Init(profiles->button_count);

    memset(g_sim.ch_buf, 0, sizeof(g_sim.ch_buf));
    memset(g_sim.batch_window_len, 0, sizeof(g_sim.batch_window_len));
    memset(g_sim.bar_count, 0, sizeof(g_sim.bar_count));

    for (int i = 0; i < g_sim.wave_inst_count; i++) {
        uint16_t xfw = WAVE_GetSampleCapacity(WAVE_INST(i));
        if (xfw > SIM_BATCH_MAX_POINTS) xfw = SIM_BATCH_MAX_POINTS;
        g_sim.batch_window_len[i] = xfw;
    }
    for (int i = 0; i < g_sim.bar_inst_count; i++) {
        g_sim.bar_count[i] = profiles->bar_profiles[i].bar_count;
        if (g_sim.bar_count[i] > BARCHART_MAX_BARS) g_sim.bar_count[i] = BARCHART_MAX_BARS;
    }

    bool is_running = true;
    SDL_Event event;

    while (is_running)
    {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            } else if (event.type == SDL_WINDOWEVENT &&
                       event.window.event == SDL_WINDOWEVENT_CLOSE) {
                uint32_t main_id = ESTA_SDL2_GetWindowID();
                if (event.window.windowID == main_id) {
                    is_running = false;
                }
            }
            BTN_UI_ProcessEvent(&event);
        }

        if (!sim_feed_wave(&g_sim)) { is_running = false; break; }
        sim_feed_barchart(&g_sim);
        sim_feed_table(&g_sim);

        App_DispatchEvents();

        BTN_UI_Render();
        ESTA_SDL2_Update();
        ESTA_SDL2_Delay(20);
        SimScenario_Tick(&g_sim.scenario);
    }

    BTN_UI_Destroy();
    ESTA_SDL2_Quit();

    return 0;
}
