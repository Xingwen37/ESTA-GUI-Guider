/**
  ******************************************************************************
  * @file           : main.c (Simulator Version)
  * @brief          : SDL2 Simulator Main program body for ESTA Library
  * @author         : TongLewis(yangyutong) HEU ESTA 2025
  ******************************************************************************
  */

  #include <stdio.h>
  #include <stdbool.h>
  #include <SDL2/SDL.h>

  #include "WAVE.h"
  #include "ESTA_Profile.h"
  #include "sim_scenario.h"
  #include "esta_port_sdl2.h"

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

      int inst_count = profiles->inst_count;
      if (inst_count > SIM_SCENARIO_WAVE_COUNT) {
          inst_count = SIM_SCENARIO_WAVE_COUNT;
      }
      if (inst_count > MAX_WAVE_NUM) {
          inst_count = MAX_WAVE_NUM;
      }

      for (int i = 0; i < inst_count; i++) {
          if (ESTA_Profile_Apply(WAVE_INST(i), &profiles->profiles[i]) != ESTA_OK) {
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

      /* PC 端的主循环与事件处理 */
      bool is_running = true;
      SDL_Event event;

      uint16_t data_ESTA[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL] = {0};

      while (is_running)
      {
          /* 抓取系统事件：处理窗口关闭等操作，防止界面卡死 */
          while (SDL_PollEvent(&event))
          {
              if (event.type == SDL_QUIT)
              {
                  is_running = false; // 用户点击了窗口的 'X' 号
              }
          }
          /* 由场景层按统一时间基生成每个示波器每一帧的数据 */
          for (int i = 0; i < inst_count; i++) {
              if (!SimScenario_GetNextFrame(&scenario, i, data_ESTA[i])) {
                  is_running = false;
                  break;
              }
              if (WAVE_CurveDraw(WAVE_INST(i), data_ESTA[i]) == ESTA_ERROR) {
                  is_running = false;
                  break;
              }
          }

          /* 将缓冲数据刷新到计算机屏幕 */
          ESTA_SDL2_Update();

          /* 调用在 Port 层封装的延时函数 */
          ESTA_SDL2_Delay(10);

          SimScenario_Tick(&scenario);
      }

      /* 4. 退出循环后安全释放资源 */
      ESTA_SDL2_Quit();

      return 0;
  }
