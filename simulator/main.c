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
  #include "profile/ESTA_Profile.h"
  #include "event/event.h"
  #include "sim_scenario.h"
  #include "esta_port_sdl2.h"
  #include "btn_ui.h"

  #define SIM_BATCH_MAX_POINTS 320

  /**
    * @brief  The PC application entry point.
    * @note   SDL2 要求 main 函数带有 argc 和 argv 参数
    */
  int main(int argc, char *argv[])
  {
      (void)argc;
      (void)argv;

      ESTA_SDL2_Init();

      SimScenarioRuntime scenario;
      if (!SimScenario_LoadDefault(&scenario)) {
          printf("SimScenario_LoadDefault failed.\n");
          ESTA_SDL2_Quit();
          return 1;
      }

      const ESTA_ProfileSet_TypeDef *profiles = ESTA_Profile_GetDefault();
      if (profiles == NULL) {
          printf("ESTA_Profile_GetDefault failed.\n");
          ESTA_SDL2_Quit();
          return 1;
      }

      int inst_count = profiles->wave_inst_count;
      if (inst_count > SIM_SCENARIO_WAVE_COUNT) {
          inst_count = SIM_SCENARIO_WAVE_COUNT;
      }
      if (inst_count > MAX_WAVE_NUM) {
          inst_count = MAX_WAVE_NUM;
      }
      if (inst_count > ESTA_PROFILE_MAX_WAVE_INST) {
          inst_count = ESTA_PROFILE_MAX_WAVE_INST;
      }

      for (int i = 0; i < inst_count; i++) {
          if (ESTA_Profile_Apply(WAVE_INST(i), &profiles->wave_profiles[i]) != ESTA_OK) {
              printf("ESTA_Profile_Apply failed at inst=%d.\n", i);
              ESTA_SDL2_Quit();
              return 1;
          }
          if (WAVE_ReDraw(WAVE_INST(i)) != ESTA_OK) {
              printf("WAVE_ReDraw failed at inst=%d.\n", i);
              ESTA_SDL2_Quit();
              return 1;
          }
      }

      int bar_inst_count = profiles->bar_inst_count;
      if (bar_inst_count > SIM_SCENARIO_BARCHART_COUNT) {
          bar_inst_count = SIM_SCENARIO_BARCHART_COUNT;
      }
      if (bar_inst_count > BARCHART_MAX_NUM) {
          bar_inst_count = BARCHART_MAX_NUM;
      }
      if (bar_inst_count > ESTA_PROFILE_MAX_BARCHART_INST) {
          bar_inst_count = ESTA_PROFILE_MAX_BARCHART_INST;
      }

      /* BARCHART 初始化 */
      for (int i = 0; i < bar_inst_count; i++) {
          if (ESTA_Profile_ApplyBARCHART(BARCHART_INST(i), &profiles->bar_profiles[i]) != ESTA_OK) {
              printf("ESTA_Profile_ApplyBARCHART failed at inst=%d.\n", i);
              ESTA_SDL2_Quit();
              return 1;
          }
          if (BARCHART_ReDraw(BARCHART_INST(i)) != ESTA_OK) {
              printf("BARCHART_ReDraw failed at inst=%d.\n", i);
              ESTA_SDL2_Quit();
              return 1;
          }
      }

      /* 事件系统与仿真按键窗口初始化 */
      ESTA_Profile_ApplyEvents(profiles);
      BTN_UI_Init(profiles->button_count);

      /* PC 端的主循环与事件处理 */
      bool is_running = true;
      SDL_Event event;

      uint16_t data_ESTA[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL] = {0};
      uint16_t data_BARCHART[SIM_SCENARIO_BARCHART_COUNT][BARCHART_MAX_BARS] = {0};
      /* batch 模式：每实例每通道独立缓冲区 + 游标 */
      uint16_t ch_buf[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL][SIM_BATCH_MAX_POINTS];
      uint16_t batch_buf_idx[SIM_SCENARIO_WAVE_COUNT];
      bool     batch_mode_active[SIM_SCENARIO_WAVE_COUNT];
      uint16_t bar_count[SIM_SCENARIO_BARCHART_COUNT] = {0};

      for (int i = 0; i < SIM_SCENARIO_WAVE_COUNT; i++) {
          memset(ch_buf[i], 0, sizeof(ch_buf[i]));
          batch_buf_idx[i] = 0;
          batch_mode_active[i] = (i < inst_count) ?
              profiles->wave_profiles[i].is_use_batch_draw : false;
      }

      for (int i = 0; i < bar_inst_count; i++) {
          bar_count[i] = profiles->bar_profiles[i].bar_count;
          if (bar_count[i] > BARCHART_MAX_BARS) {
              bar_count[i] = BARCHART_MAX_BARS;
          }
      }

      while (is_running)
      {
          /* 抓取系统事件：处理窗口关闭等操作，防止界面卡死 */
          while (SDL_PollEvent(&event))
          {
              if (event.type == SDL_QUIT)
              {
                  is_running = false; /* 全局退出信号 */
              }
              else if (event.type == SDL_WINDOWEVENT &&
                       event.window.event == SDL_WINDOWEVENT_CLOSE)
              {
                  uint32_t main_id = ESTA_SDL2_GetWindowID();
                  if (event.window.windowID == main_id) {
                      is_running = false; /* 主窗口关闭 */
                  }
              }
              BTN_UI_ProcessEvent(&event);
          }
          /* 由场景层按统一时间基生成每个示波器每一帧的数据 */
          for (int i = 0; i < inst_count; i++) {
              if (!SimScenario_GetNextFrame(&scenario, i, data_ESTA[i])) {
                  is_running = false;
                  break;
              }
              /* 支持 batch / 增量两种绘制模式 */
              if (batch_mode_active[i]) {
                  uint16_t xw = WAVE_CONFIG_MEMBER(i, x_width);
                  uint16_t rndy = WAVE_CONFIG_MEMBER(i, ruler_num_digits_y);
                  volatile bool idry = WAVE_CONFIG_MEMBER(i, is_display_ruler_y);
                  uint16_t label_width = rndy * CHAR_PIXEL_WIDTH;
                  uint16_t xfw = xw;
                  if (idry) {
                      xfw = (xw > label_width) ? (xw - label_width) : 0;
                  }
                  if (xfw > SIM_BATCH_MAX_POINTS) {
                      xfw = SIM_BATCH_MAX_POINTS;
                  }
                  if (xfw == 0) {
                      continue;
                  }
                  uint8_t mask = WAVE_CONFIG_MEMBER(i, channel_mask);
                  for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
                      if (is_channel_enabled(mask, (uint8_t)(CH0 << ch))) {
                          ch_buf[i][ch][batch_buf_idx[i]] = data_ESTA[i][ch];
                      }
                  }
                  batch_buf_idx[i]++;
                  if (batch_buf_idx[i] >= xfw) {
                      WAVE_CurveClear(WAVE_INST(i));
                      for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
                          if (is_channel_enabled(mask, (uint8_t)(CH0 << ch))) {
                              WAVE_WRITE_PRIVATE(i, last_index, 0);
                              WAVE_WRITE_PRIVATE(i, x_coor_last,
                                  WAVE_CONFIG_MEMBER(i, x_origin));
                              WAVE_CurveDrawBatch(WAVE_INST(i), ch,
                                  ch_buf[i][ch], batch_buf_idx[i]);
                          }
                      }
                      batch_buf_idx[i] = 0;
                  }
              } else {
                  if (WAVE_CurveDraw(WAVE_INST(i), data_ESTA[i]) == ESTA_ERROR) {
                      is_running = false;
                      break;
                  }
              }
          }

          /* 更新柱状图数据 */
          for (int i = 0; i < bar_inst_count; i++) {
              if (SimScenario_BARCHART_GetData(&scenario, data_BARCHART[i], bar_count[i])) {
                  BARCHART_UpdateAll(BARCHART_INST(i), data_BARCHART[i], bar_count[i]);
              }
          }

          /* 消费事件队列：组件响应外部按键 */
          {
              ESTA_EventTypeDef evt;
              while (ESTA_EventPoll(&evt)) {
                  if (evt.event_type == ESTA_EVENT_BUTTON_PRESS && evt.button_id == 0 &&
                      inst_count > 0) {
                      WAVE_theme_type cur = WAVE_CONFIG_MEMBER(0, theme_type);
                      WAVE_theme_type next = (cur == WAVE_THEME_DEFAULT)
                                             ? WAVE_THEME_LIGHT : WAVE_THEME_DEFAULT;
                      WAVE_WRITE_CONFIG(0, theme_type, next);
                      WAVE_ReDraw(WAVE_INST(0));
                  }
                  if (evt.event_type == ESTA_EVENT_BUTTON_PRESS && evt.button_id == 1 &&
                      inst_count > 0) {
                      batch_mode_active[0] = !batch_mode_active[0];
                      WAVE_CurveClear(WAVE_INST(0));
                      batch_buf_idx[0] = 0;
                      printf("[BTN_1] WAVE0 batch draw: %s\n",
                             batch_mode_active[0] ? "ON" : "OFF");
                  }
                  if (evt.event_type == ESTA_EVENT_BUTTON_PRESS && evt.button_id == 2 &&
                      inst_count > 1) {
                      batch_mode_active[1] = !batch_mode_active[1];
                      WAVE_CurveClear(WAVE_INST(1));
                      batch_buf_idx[1] = 0;
                      printf("[BTN_2] WAVE1 batch draw: %s\n",
                             batch_mode_active[1] ? "ON" : "OFF");
                  }
              }
          }

          /* 渲染仿真按键窗口 */
          BTN_UI_Render();

          /* 将缓冲数据刷新到计算机屏幕 */
          ESTA_SDL2_Update();

          /* 调用在 Port 层封装的延时函数 */
          ESTA_SDL2_Delay(20);

          SimScenario_Tick(&scenario);
      }

      /* 4. 退出循环后安全释放资源 */
      BTN_UI_Destroy();
      ESTA_SDL2_Quit();

      return 0;
  }
