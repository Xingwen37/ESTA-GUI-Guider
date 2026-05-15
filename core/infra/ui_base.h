#ifndef __UI_BASE_H
#define __UI_BASE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ESTA_FONT_1206 = 0,
    ESTA_FONT_1608,
    ESTA_FONT_2412,
    ESTA_FONT_SIZE_COUNT
} ESTA_FontSize;

/* ================== 硬件抽象层 ================== */
// 选择底层屏幕驱动
// #define SCREEN_USE_ILI9341
#define SCREEN_USE_SDL2

#ifdef SCREEN_USE_ILI9341
#include "main.h"
#include "ili9341_driver.h"

#define SCREEN_DRAW_LINE(x1, y1, x2, y2, COLOR) \
            ILI9341_draw_line(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, COLOR) \
            ILI9341_draw_rectangle(x1, y1, x2, y2, COLOR)
#define SCREEN_FILL(x1, y1, x2, y2, COLOR) \
            ILI9341_fill(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_NUM(x, y, num, len, COLOR) \
            ILI9341_draw_num(x, y, num, len, COLOR)
#define SCREEN_DRAW_STRING(x, y, str, len, COLOR) \
            ILI9341_draw_string(x, y, str, len, COLOR)
#define SCREEN_DRAW_NUM_FONT(x, y, num, len, font, COLOR) \
            ILI9341_draw_num(x, y, num, len, COLOR)
#define SCREEN_DRAW_STRING_FONT(x, y, str, len, font, COLOR) \
            ILI9341_draw_string(x, y, str, len, COLOR)

#elif defined(SCREEN_USE_SDL2)
#include "esta_port_sdl2.h"

#define SCREEN_DRAW_LINE(x1, y1, x2, y2, COLOR) \
            ESTA_SDL2_DrawLine(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, COLOR) \
            ESTA_SDL2_DrawRectangle(x1, y1, x2, y2, COLOR)
#define SCREEN_FILL(x1, y1, x2, y2, COLOR) \
            ESTA_SDL2_Fill(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_NUM(x, y, num, len, COLOR) \
            ESTA_SDL2_DrawNum(x, y, num, len, COLOR)
#define SCREEN_DRAW_STRING(x, y, str, len, COLOR) \
            ESTA_SDL2_DrawString(x, y, str, len, COLOR)
#define SCREEN_DRAW_NUM_FONT(x, y, num, len, font, COLOR) \
            ESTA_SDL2_DrawNumFont(x, y, num, len, (uint8_t)(font), COLOR)
#define SCREEN_DRAW_STRING_FONT(x, y, str, len, font, COLOR) \
            ESTA_SDL2_DrawStringFont(x, y, str, len, (uint8_t)(font), COLOR)

#else
// 自定义屏幕驱动
#define SCREEN_DRAW_LINE(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, COLOR)
#define SCREEN_FILL(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_NUM(x, y, num, len, COLOR)
#define SCREEN_DRAW_STRING(x, y, str, len, COLOR)
#define SCREEN_DRAW_NUM_FONT(x, y, num, len, font, COLOR)
#define SCREEN_DRAW_STRING_FONT(x, y, str, len, font, COLOR)
#endif

/* ================== 字体度量 ================== */
#define CHAR_PIXEL_WIDTH    8
#define CHAR_PIXEL_HEIGHT   16

/* ================== 颜色定义 (RGB565) ================== */
#define RGB888_To_RGB565(R,G,B)  (uint16_t)((R & 0x1f)<<11|(G & 0x3f)<<5|(B & 0x1f))
#define __WHITE               0xFFFF
#define __BLACK               0x0000
#define __BLUE                0x001F
#define __DEEP_BLUE           0x101F
#define __BRED                0XF81F
#define __GRED                0XFFE0
#define __GBLUE               0x059F
#define __RED                 0xF800
#define __GREEN               0x07E0
#define __YELLOW              0xFFE0
#define __BROWN               0XBC40
#define __BRRED               0XFC07
#define __GRAY                0X8430
#define __ORANGE              0XFD20
#define __PURPLE              0X8010

/* ================== 通用状态码 ================== */
typedef enum
{
    ESTA_OK       = 0x00,
    ESTA_ERROR    = 0x01,
    ESTA_FULL     = 0x02
} ESTA_StatusTypeDef;

/* ================== 公共配置基类 ================== */
/* 所有可视化组件的 Config 结构体必须以此 4 字段开头，顺序一致 */
typedef struct {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t x_width;
    uint16_t y_width;
} ESTA_BaseConfig;

ESTA_StatusTypeDef ESTA_ConfigSetPositionAndSize(ESTA_BaseConfig *base,
    uint16_t x_origin, uint16_t y_origin, uint16_t x_width, uint16_t y_width);

/* ================== 错误传播宏 ================== */
#define ESTA_RETURN_IF_ERROR(expr) \
    do { if ((expr) != ESTA_OK) return ESTA_ERROR; } while(0)

/* ================== 通用工具函数 ================== */
uint16_t ui_coor_normal(uint16_t coor_width, uint16_t value_max_range, uint16_t value);
uint16_t ui_limit(uint16_t max, uint16_t min, uint16_t value);
bool ui_is_out_of_bound(uint16_t max, uint16_t min, uint16_t value);
uint16_t ui_num_digits(uint16_t x);
uint16_t ui_font_width(ESTA_FontSize font_size);
uint16_t ui_font_height(ESTA_FontSize font_size);

#endif
