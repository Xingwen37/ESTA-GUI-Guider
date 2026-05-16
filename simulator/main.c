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
  #include "sim_scenario.h"
  #include "esta_port_sdl2.h"
  #include "btn_ui.h"

  #define SIM_BATCH_MAX_POINTS 320

  static const ESTA_ProfileSet_TypeDef *g_profiles;
  static uint8_t g_active_page = 0;

  static void ApplyAndDrawPage(uint8_t page) {
      SCREEN_FILL(0, 0, 400, 320, 0x0000);
      ESTA_Profile_SetActivePage(page);

      for (int i = 0; i < g_profiles->wave_inst_count; i++) {
          if (g_profiles->wave_profiles[i].page == page) {
              ESTA_Profile_Apply(WAVE_INST(i), &g_profiles->wave_profiles[i]);
              WAVE_ReDraw(WAVE_INST(i));
          }
      }
      for (int i = 0; i < g_profiles->bar_inst_count; i++) {
          if (g_profiles->bar_profiles[i].page == page) {
              ESTA_Profile_ApplyBARCHART(BARCHART_INST(i), &g_profiles->bar_profiles[i]);
              BARCHART_ReDraw(BARCHART_INST(i));
          }
      }
      for (int i = 0; i < g_profiles->table_inst_count; i++) {
          if (g_profiles->table_profiles[i].page == page) {
              ESTA_Profile_ApplyTABLE(TABLE_INST(i), &g_profiles->table_profiles[i]);
              TABLE_ReDraw(TABLE_INST(i));
          }
      }
      for (int i = 0; i < g_profiles->menu_inst_count; i++) {
          if (g_profiles->menu_profiles[i].page == page) {
              ESTA_Profile_ApplyMENU(MENU_INST(i), &g_profiles->menu_profiles[i]);
              MENU_ReDraw(MENU_INST(i));
          }
      }
  }

  int main(int argc, char *argv[])
  {
      const char *screenshot_path = NULL;
      for (int i = 1; i < argc; i++) {
          if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
              screenshot_path = argv[++i];
          }
      }

      ESTA_SDL2_Init();

      SimScenarioRuntime scenario;
      if (!SimScenario_LoadDefault(&scenario)) {
          printf("SimScenario_LoadDefault failed.\n");
          ESTA_SDL2_Quit();
          return 1;
      }

      g_profiles = ESTA_Profile_GetDefault();
      if (g_profiles == NULL) {
          printf("ESTA_Profile_GetDefault failed.\n");
          ESTA_SDL2_Quit();
          return 1;
      }

      const ESTA_ProfileSet_TypeDef *profiles = g_profiles;

      int inst_count = profiles->wave_inst_count;
      if (inst_count > SIM_SCENARIO_WAVE_COUNT) inst_count = SIM_SCENARIO_WAVE_COUNT;
      if (inst_count > MAX_WAVE_NUM) inst_count = MAX_WAVE_NUM;

      int bar_inst_count = profiles->bar_inst_count;
      if (bar_inst_count > SIM_SCENARIO_BARCHART_COUNT) bar_inst_count = SIM_SCENARIO_BARCHART_COUNT;
      if (bar_inst_count > BARCHART_MAX_NUM) bar_inst_count = BARCHART_MAX_NUM;

      int table_inst_count = profiles->table_inst_count;
      if (table_inst_count > TABLE_MAX_NUM) table_inst_count = TABLE_MAX_NUM;

      int menu_inst_count = profiles->menu_inst_count;
      if (menu_inst_count > MENU_MAX_NUM) menu_inst_count = MENU_MAX_NUM;

      uint8_t page_count = profiles->page_count;
      if (page_count == 0) page_count = 1;

      ApplyAndDrawPage(0);

      if (screenshot_path) {
          for (uint8_t p = 0; p < page_count; p++) {
              ApplyAndDrawPage(p);
              char filename[256];
              snprintf(filename, sizeof(filename), "%s_%d.bmp", screenshot_path, p);
              ESTA_SDL2_SaveScreenshot(filename);
          }
          ESTA_SDL2_Quit();
          return 0;
      }

      ESTA_Profile_ApplyEvents(profiles);
      BTN_UI_Init(profiles->button_count);

      bool is_running = true;
      SDL_Event event;

      uint16_t data_ESTA[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL] = {0};
      uint16_t data_BARCHART[SIM_SCENARIO_BARCHART_COUNT][BARCHART_MAX_BARS] = {0};
      uint16_t ch_buf[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL][SIM_BATCH_MAX_POINTS];
      uint16_t batch_window_len[SIM_SCENARIO_WAVE_COUNT];
      uint16_t bar_count[SIM_SCENARIO_BARCHART_COUNT] = {0};

      for (int i = 0; i < SIM_SCENARIO_WAVE_COUNT; i++) {
          memset(ch_buf[i], 0, sizeof(ch_buf[i]));
          batch_window_len[i] = 0;
      }

      for (int i = 0; i < inst_count; i++) {
          uint16_t xfw = WAVE_GetSampleCapacity(WAVE_INST(i));
          if (xfw > SIM_BATCH_MAX_POINTS) xfw = SIM_BATCH_MAX_POINTS;
          batch_window_len[i] = xfw;
      }

      for (int i = 0; i < bar_inst_count; i++) {
          bar_count[i] = profiles->bar_profiles[i].bar_count;
          if (bar_count[i] > BARCHART_MAX_BARS) bar_count[i] = BARCHART_MAX_BARS;
      }

      while (is_running)
      {
          while (SDL_PollEvent(&event))
          {
              if (event.type == SDL_QUIT)
              {
                  is_running = false;
              }
              else if (event.type == SDL_WINDOWEVENT &&
                       event.window.event == SDL_WINDOWEVENT_CLOSE)
              {
                  uint32_t main_id = ESTA_SDL2_GetWindowID();
                  if (event.window.windowID == main_id) {
                      is_running = false;
                  }
              }
              BTN_UI_ProcessEvent(&event);
          }

          for (int i = 0; i < inst_count; i++) {
              if (profiles->wave_profiles[i].page != g_active_page) continue;
              if (!SimScenario_GetNextFrame(&scenario, i, data_ESTA[i])) {
                  is_running = false;
                  break;
              }
              uint16_t xfw = batch_window_len[i];
              if (xfw == 0) continue;

              uint8_t mask = WAVE_CONFIG_MEMBER(i, channel_mask);
              for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
                  if (is_channel_enabled(mask, (uint8_t)(CH0 << ch))) {
                      memmove(&ch_buf[i][ch][0], &ch_buf[i][ch][1],
                              (xfw - 1U) * sizeof(ch_buf[i][ch][0]));
                      ch_buf[i][ch][xfw - 1U] = data_ESTA[i][ch];
                  }
              }

              WAVE_CurveClear(WAVE_INST(i));
              for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
                  if (is_channel_enabled(mask, (uint8_t)(CH0 << ch))) {
                      WAVE_WRITE_PRIVATE(i, last_index, 0);
                      WAVE_WRITE_PRIVATE(i, x_coor_last,
                          WAVE_CONFIG_MEMBER(i, x_origin));
                      WAVE_CurveDrawBatch(WAVE_INST(i), ch, ch_buf[i][ch], xfw);
                  }
              }
          }

          for (int i = 0; i < bar_inst_count; i++) {
              if (profiles->bar_profiles[i].page != g_active_page) continue;
              if (SimScenario_BARCHART_GetData(&scenario, data_BARCHART[i], bar_count[i])) {
                  BARCHART_UpdateAll(BARCHART_INST(i), data_BARCHART[i], bar_count[i]);
              }
          }

          for (int i = 0; i < table_inst_count; i++) {
              if (profiles->table_profiles[i].page != g_active_page) continue;
              TABLE_UpdateUInt32(TABLE_INST(i), 0, 1, data_ESTA[0][0]);
              TABLE_UpdateUInt32(TABLE_INST(i), 1, 1, (uint32_t)(1000U + scenario.tick * 10U));
          }

          {
              for (int i = 0; i < menu_inst_count; i++) {
                  if (profiles->menu_profiles[i].page != g_active_page) continue;
                  MENU_ProcessInput(MENU_INST(i));
              }

              ESTA_EventTypeDef evt;
              while (ESTA_EventPoll(&evt)) {
                  if (evt.event_type == ESTA_EVENT_MENU_SELECT) {
                      printf("MENU SELECT: event_id=%d\n", evt.button_id);
                  } else if (evt.event_type == ESTA_EVENT_BUTTON_PRESS && evt.button_id == 0 &&
                      inst_count > 0 && profiles->wave_profiles[0].page == g_active_page) {
                      WAVE_theme_type cur = WAVE_CONFIG_MEMBER(0, theme_type);
                      WAVE_theme_type next = (cur == WAVE_THEME_DEFAULT)
                                             ? WAVE_THEME_LIGHT : WAVE_THEME_DEFAULT;
                      WAVE_WRITE_CONFIG(0, theme_type, next);
                      WAVE_ReDraw(WAVE_INST(0));
                  } else if (evt.event_type == ESTA_EVENT_BUTTON_PRESS && evt.button_id == 1 &&
                      page_count > 1) {
                      g_active_page = (uint8_t)((g_active_page + 1) % page_count);
                      ApplyAndDrawPage(g_active_page);
                      printf("Switched to page %d\n", g_active_page);
                  }
              }
          }

          BTN_UI_Render();
          ESTA_SDL2_Update();
          ESTA_SDL2_Delay(20);
          SimScenario_Tick(&scenario);
      }

      BTN_UI_Destroy();
      ESTA_SDL2_Quit();

      return 0;
  }
