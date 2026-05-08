#ifndef __ESTA_PORT_SDL2_H
#define __ESTA_PORT_SDL2_H

#include <stdint.h>
#include <stdbool.h>

/* 模拟屏幕的分辨率定义 */
#define SIMULATOR_SCREEN_WIDTH  320
#define SIMULATOR_SCREEN_HEIGHT 240

/* 供 main() 调用的系统级函数 */
void ESTA_SDL2_Init(void);
void ESTA_SDL2_Update(void);
void ESTA_SDL2_Quit(void);

/* 供 ESTA.h 中的 SCREEN_DRAW_* 宏调用的底层绘图函数 */
void ESTA_SDL2_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ESTA_SDL2_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ESTA_SDL2_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ESTA_SDL2_DrawNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint16_t color);
void ESTA_SDL2_Delay(uint32_t ms);
uint32_t ESTA_SDL2_GetWindowID(void);

#endif /* __ESTA_PORT_SDL2_H */