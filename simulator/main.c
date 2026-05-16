#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "ui/WAVE.h"
#include "app/app_main.h"
#include "sim_scenario.h"
#include "sim_feed.h"
#include "sim_input.h"
#include "sim_gpio.h"
#include "esta_port_sdl2.h"
#include "btn_ui.h"

#define SIM_TARGET_FRAME_MS  20  /* 50 FPS */

static App_MainState g_app;
static SimScenarioRuntime g_scenario;

/* ---- GPIO 中断示例（模拟 MCU ISR 编程范式） ---- */

static volatile uint8_t g_wave_refresh_flag = 0;

static void btn0_rising_isr(uint8_t pin) {
    (void)pin;
    g_wave_refresh_flag = 1;
}

/* ---- 仿真器演示回调（MCU 替换为业务逻辑） ---- */

static bool on_theme_toggle(const ESTA_Event *evt, void *user_data) {
    (void)user_data;
    if (evt->source == 0 &&
        g_app.wave_inst_count > 0 &&
        g_app.page_state.profiles->wave_profiles[0].page ==
            App_GetActivePage(&g_app.page_state)) {
        WAVE_theme_type cur = WAVE_CONFIG_MEMBER(0, theme_type);
        WAVE_theme_type next = (cur == WAVE_THEME_DEFAULT)
                               ? WAVE_THEME_LIGHT : WAVE_THEME_DEFAULT;
        WAVE_WRITE_CONFIG(0, theme_type, next);
        WAVE_ReDraw(WAVE_INST(0));
        return true;
    }
    return false;
}

static bool on_menu_select(const ESTA_Event *evt, void *user_data) {
    (void)user_data;
    printf("MENU SELECT: event_id=%d\n", evt->id);
    return true;
}

/*
 * MCU 移植最小模板：
 *
 *   App_MainState app;
 *   App_MainInit(&app, LCD_WIDTH, LCD_HEIGHT);
 *   App_Subscribe(..., my_handler, ...);
 *
 *   while (1) {
 *       采集数据 → WAVE_CurveDrawBatch() / BARCHART_UpdateAll()
 *       检测按键 → ESTA_EventEmitButton(btn_id, ESTA_EVENT_BUTTON_PRESS)
 *       App_MainTick(&app);
 *       HAL_Delay(20);
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
    SimGPIO_Init();
    SimGPIO_AttachInterrupt(0, btn0_rising_isr);
    App_Subscribe(ESTA_EVENT_BUTTON_PRESS, 0, APP_SOURCE_ANY, on_theme_toggle, NULL);
    App_Subscribe(ESTA_EVENT_MENU_SELECT, 0, APP_SOURCE_ANY, on_menu_select, NULL);
    BTN_UI_Init(profiles->button_count);

    while (true) {
        uint32_t frame_start = SDL_GetTicks();

        SimInputResult input = SimInput_Poll(profiles->button_count);
        if (input.quit_requested) break;

        bool wave_trigger = false;
        if (g_wave_refresh_flag) {
            g_wave_refresh_flag = 0;
            wave_trigger = true;
        }

        if (!SimFeed_Update(&g_app, &g_scenario, wave_trigger)) break;
        App_MainTick(&g_app);

        BTN_UI_Render();
        ESTA_SDL2_Update();

        uint32_t elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < SIM_TARGET_FRAME_MS) {
            ESTA_SDL2_Delay(SIM_TARGET_FRAME_MS - elapsed);
        }
        SimFeed_Tick(&g_scenario);
    }

    BTN_UI_Destroy();
    ESTA_SDL2_Quit();
    return 0;
}
