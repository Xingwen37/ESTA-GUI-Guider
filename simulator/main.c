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
  #include "osc_port_sdl2.h"
  
  /* 测试使用的一个周期的正弦波 */
  const uint16_t gOutputSignal[] = {2048, 2248, 2447, 2642, 2831, 3013,
      3185, 3347, 3496, 3631, 3750, 3854, 3940, 4007, 4056, 4086, 4095, 4086,
      4056, 4007, 3940, 3854, 3750, 3631, 3496, 3347, 3185, 3013, 2831, 2642,
      2447, 2248, 2048, 1847, 1648, 1453, 1264, 1082, 910, 748, 599, 464, 345,
      241, 155, 88, 39, 9, 0, 9, 39, 88, 155, 241, 345, 464, 599, 748, 910, 1082,
      1264, 1453, 1648, 1847};
  
  /**
    * @brief  The PC application entry point.
    * @note   SDL2 要求 main 函数带有 argc 和 argv 参数
    */
  int main(int argc, char *argv[])
  {
      (void)argc;
      (void)argv;
  
      OSC_SDL2_Init();
  
      /* 示波器配置 */
      OSC_Config_TypeDef OSC_Config;
  
      uint16_t ruler_y[5] = {1000, 2000, 3000, 4000, 0};
      uint16_t ruler_x[5] = {30, 50, 90, 0, 0};
  
      OSC_ConfigSetPositionAndSize(&OSC_Config, 10, 0, 200, 120);
      OSC_ConfigSetDisplayRange(&OSC_Config, 0, 4095);
      OSC_ConfigSetChannelNum(&OSC_Config, 2);
      OSC_ConfigSetChannelEnabled(&OSC_Config, CH0);
      OSC_ConfigSetChannelEnabled(&OSC_Config, CH1);
      OSC_ConfigSetRulerY(&OSC_Config, true, &ruler_y[0], 4, 4);
      OSC_ConfigSetRulerX(&OSC_Config, true, &ruler_x[0], 3, 0, 100, 8);
      OSC_ConfigSetTheme(&OSC_Config, OSC_THEME_DEFAULT);
      OSC_ConfigSetAutoClear(&OSC_Config, true);
  
      // 初始化实例 0
      OSC_Init(OSC_INST(0), &OSC_Config);
      OSC_ReDraw(OSC_INST(0));
  
      // 修改配置以初始化实例 1
      OSC_ConfigSetPositionAndSize(&OSC_Config, 0, 120, 200, 120);
      OSC_ConfigSetTheme(&OSC_Config, OSC_THEME_LIGHT);
  
      OSC_Init(OSC_INST(1), &OSC_Config);
      OSC_ReDraw(OSC_INST(1));
  
      int demo_signal_size = sizeof(gOutputSignal) / sizeof(gOutputSignal[0]);
      int num = 0;
      int num_1 = 32;
  
      /* PC 端的主循环与事件处理 */
      bool is_running = true;
      SDL_Event event;
  
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
  
          /* 查表输出到缓冲纹理 */
          OSC_CurveDraw(OSC_INST(0), gOutputSignal[num], gOutputSignal[num_1]);
          OSC_CurveDraw(OSC_INST(1), gOutputSignal[num_1], gOutputSignal[num]);
  
          /* 将缓冲数据刷新到计算机屏幕 */
          OSC_SDL2_Update();
          
          /* 调用在 Port 层封装的延时函数 */
          OSC_SDL2_Delay(10);
  
          num++; 
          num_1++;
          if(num == demo_signal_size) {
              num = 0;
          }
          if(num_1 == demo_signal_size) {
              num_1 = 0;
          } 
      }
  
      /* 4. 退出循环后安全释放资源 */
      OSC_SDL2_Quit();
  
      return 0;
  }