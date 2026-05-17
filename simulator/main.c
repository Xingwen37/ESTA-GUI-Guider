#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "ui/WAVE.h"
#include "app/app_main.h"
#include "event/event_flag.h"
#include "sim_scenario.h"
#include "sim_feed.h"
#include "sim_input.h"
#include "esta_port_sdl2.h"

#define SIM_TARGET_FRAME_MS  20  /* 50 FPS */

static App_MainState g_app;
static SimScenarioRuntime g_scenario;

/*
 * MCU 移植最小模板：
 *
 *   App_MainState app;
 *   App_MainInit(&app, LCD_WIDTH, LCD_HEIGHT);
 *
 *   while (1) {
 *       采集数据 → WAVE_CurveDrawBatch() / BARCHART_UpdateAll()
 *       检测按键 → ESTA_EventEmitButton(btn_id, ESTA_EVENT_BUTTON_PRESS)
. *       HAL_Delay(20);
 *   }
 */

int main(int argc, char *argv[])
{
    const char *screenshot_path = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            screenshot_path = argv[++i];
        }
    }

    ESTA_SDL2_Init();

    if (!SimScenario_LoadDefault(&g_scenario)) {
        printf("SimScenario_LoadDefault failed.\n");
        ESTA_SDL2_Quit();
        return 1;
    }

    App_MainInit(&g_app, SIMULATOR_SCREEN_WIDTH / 2, SIMULATOR_SCREEN_HEIGHT / 2);
    if (g_app.page_state.profiles == NULL) {
        printf("ESTA_Profile_GetDefault failed.\n");
        ESTA_SDL2_Quit();
        return 1;
    }

    const ESTA_ProfileSet_TypeDef *profiles = g_app.page_state.profiles;

    if (screenshot_path) {
        for (uint8_t p = 0; p < g_app.page_state.page_count; p++) {
            App_ApplyAndDrawPage(&g_app.page_state, p);
            char filename[256];
            snprintf(filename, sizeof(filename), "%s_%d.bmp", screenshot_path, p);
            ESTA_SDL2_SaveScreenshot(filename);
        }
        ESTA_SDL2_Quit();
        return 0;
    }

    SimFeed_Init(&g_app, &g_scenario);

    while (true) {
        uint32_t frame_start = SDL_GetTicks();

        SimInputResult input = SimInput_Poll(profiles->button_count);
        if (input.quit_requested) break;

        ESTA_FlagPoll();
        App_MainTick(&g_app);
        if (!SimFeed_Update(&g_app, &g_scenario)) break;

        ESTA_SDL2_Update();

        uint32_t elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < SIM_TARGET_FRAME_MS) {
            ESTA_SDL2_Delay(SIM_TARGET_FRAME_MS - elapsed);
        }
        SimFeed_Tick(&g_scenario);
    }

    ESTA_SDL2_Quit();
    return 0;
}
