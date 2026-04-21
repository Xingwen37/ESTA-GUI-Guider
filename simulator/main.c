/**
  ******************************************************************************
  * @file           : main.c (Simulator Version)
  * @brief          : SDL2 Simulator Main program body for OSC Library
  * @author         : TongLewis(yangyutong) HEU ESTA 2025
  ******************************************************************************
  */

  #include <stdio.h>
  #include <stdbool.h>
  #include <SDL2/SDL.h>
  
  #include "OSC.h"
  #include "sim_scenario.h"
  #include "osc_port_sdl2.h"

  #ifndef USE_OSC_PROFILE_ENV
  #define USE_OSC_PROFILE_ENV 1
  #endif

  #if USE_OSC_PROFILE_ENV
  #include "OSC_Profile.h"
  #endif

  /**
    * @brief  The PC application entry point.
    * @note   SDL2 要求 main 函数带有 argc 和 argv 参数
    */
  int main(int argc, char *argv[])
  {
      (void)argc;
      (void)argv;
  
      OSC_SDL2_Init();
  
      SimScenarioRuntime scenario;
      if (!SimScenario_LoadDefault(&scenario)) {
          printf("SimScenario_LoadDefault failed.\n");
          OSC_SDL2_Quit();
          return 1;
      }

      int osc_count = 0;
  #if USE_OSC_PROFILE_ENV
      const OSC_ProfileSet_TypeDef *profiles = OSC_Profile_GetDefault();
      if (profiles == NULL) {
          printf("OSC_Profile_GetDefault failed.\n");
          OSC_SDL2_Quit();
          return 1;
      }

      osc_count = profiles->osc_count;
      if (osc_count > SIM_SCENARIO_OSC_COUNT) {
          osc_count = SIM_SCENARIO_OSC_COUNT;
      }
      if (osc_count > MAX_OSC_NUM) {
          osc_count = MAX_OSC_NUM;
      }

      for (int i = 0; i < osc_count; i++) {
          if (OSC_Profile_Apply(OSC_INST(i), &profiles->profiles[i]) != OSC_OK) {
              printf("OSC_Profile_Apply failed at osc=%d.\n", i);
              OSC_SDL2_Quit();
              return 1;
          }
  #else
      osc_count = SIM_SCENARIO_OSC_COUNT;
      if (osc_count > MAX_OSC_NUM) {
          osc_count = MAX_OSC_NUM;
      }

      for (int i = 0; i < osc_count; i++) {
          if (SimScenario_ApplyDefaultProfile(i) != OSC_OK) {
              printf("SimScenario_ApplyDefaultProfile failed at osc=%d.\n", i);
              OSC_SDL2_Quit();
              return 1;
          }
  #endif
          if (OSC_ReDraw(OSC_INST(i)) != OSC_OK) {
              printf("OSC_ReDraw failed at osc=%d.\n", i);
              OSC_SDL2_Quit();
              return 1;
          }
      }
  
      /* PC 端的主循环与事件处理 */
      bool is_running = true;
      SDL_Event event;

      uint16_t data_OSC[SIM_SCENARIO_OSC_COUNT][MAX_OSC_CHANNEL] = {0};
  
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
          for (int i = 0; i < osc_count; i++) {
              if (!SimScenario_GetNextFrame(&scenario, i, data_OSC[i])) {
                  is_running = false;
                  break;
              }
              if (OSC_CurveDraw(OSC_INST(i), data_OSC[i]) == OSC_ERROR) {
                  is_running = false;
                  break;
              }
          }
  
          /* 将缓冲数据刷新到计算机屏幕 */
          OSC_SDL2_Update();
          
          /* 调用在 Port 层封装的延时函数 */
          OSC_SDL2_Delay(10);
  
          SimScenario_Tick(&scenario);
      }
  
      /* 4. 退出循环后安全释放资源 */
      OSC_SDL2_Quit();
  
      return 0;
  }