#include "esta_port_sdl2.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include "infra/font.h"

// 内部维护的 SDL 上下文
static SDL_Window* g_window   = NULL;
static SDL_Renderer* g_renderer = NULL;
static SDL_Texture* g_screen   = NULL; // 虚拟屏幕纹理

// RGB565 到 RGB888 的色彩转换宏
#define RGB565_R(color) (((color & 0xF800) >> 11) * 255 / 31)
#define RGB565_G(color) (((color & 0x07E0) >> 5)  * 255 / 63)
#define RGB565_B(color) (( color & 0x001F)        * 255 / 31)

// 内部辅助函数：设置渲染画笔颜色
static void set_draw_color(uint16_t color) {
    SDL_SetRenderDrawColor(g_renderer, RGB565_R(color), RGB565_G(color), RGB565_B(color), 255);
}

// 内部辅助函数：画单个像素
static void draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    set_draw_color(color);
    SDL_RenderDrawPoint(g_renderer, x, y);
}

static void draw_glyph_columns(uint16_t x, uint16_t y, char c, const unsigned char *font,
                               uint8_t width, uint8_t height, uint8_t bytes_per_col,
                               uint16_t color) {
    if (c < ' ' || c > '~') return;
    int offset = (c - ' ') * width * bytes_per_col;
    const unsigned char *glyph = font + offset;
    for (int col = 0; col < width; col++) {
        for (int byte_idx = 0; byte_idx < bytes_per_col; byte_idx++) {
            uint8_t line_data = glyph[col * bytes_per_col + byte_idx];
            for (int bit = 0; bit < 8; bit++) {
                uint8_t row = (uint8_t)(byte_idx * 8 + bit);
                if (row >= height) {
                    break;
                }
                if (line_data & (0x80 >> bit)) {
                    draw_pixel((uint16_t)(x + col), (uint16_t)(y + row), color);
                }
            }
        }
    }
}

/* ================== 系统级控制 ================== */

void ESTA_SDL2_Init(void) {
    SDL_Init(SDL_INIT_VIDEO);
    
    g_window = SDL_CreateWindow("TL-ESTA Simulator", 
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                SIMULATOR_SCREEN_WIDTH * 2,  // 放大两倍显示，不那么伤眼
                                SIMULATOR_SCREEN_HEIGHT * 2, 
                                SDL_WINDOW_SHOWN);
                                
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    
    // 强制按实际分辨率缩放
    SDL_RenderSetLogicalSize(g_renderer, SIMULATOR_SCREEN_WIDTH, SIMULATOR_SCREEN_HEIGHT);

    // 创建一张可以被作为“目标”绘制的纹理，作为物理屏幕的平替
    g_screen = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, 
                                 SDL_TEXTUREACCESS_TARGET, 
                                 SIMULATOR_SCREEN_WIDTH, SIMULATOR_SCREEN_HEIGHT);
                                 
    // 所有的画笔操作都重定向到这张虚拟屏幕上
    SDL_SetRenderTarget(g_renderer, g_screen);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
}

void ESTA_SDL2_Update(void) {
    // 1. 暂时把渲染目标切回电脑真实的屏幕窗口
    SDL_SetRenderTarget(g_renderer, NULL); 
    // 2. 把我们画好数据的虚拟屏幕纹理盖到窗口上
    SDL_RenderCopy(g_renderer, g_screen, NULL, NULL); 
    // 3. 推送显示
    SDL_RenderPresent(g_renderer); 
    // 4. 切回虚拟屏幕，准备下一次的数据写入
    SDL_SetRenderTarget(g_renderer, g_screen); 
}

void ESTA_SDL2_Quit(void) {
    SDL_DestroyTexture(g_screen);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

void ESTA_SDL2_Delay(uint32_t ms) {
    SDL_Delay(ms);
}

uint32_t ESTA_SDL2_GetWindowID(void) {
    return SDL_GetWindowID(g_window);
}

void ESTA_SDL2_SaveScreenshot(const char *path) {
    SDL_Surface *surface = SDL_CreateRGBSurface(0,
        SIMULATOR_SCREEN_WIDTH, SIMULATOR_SCREEN_HEIGHT, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (surface == NULL) return;
    SDL_SetRenderTarget(g_renderer, g_screen);
    SDL_RenderReadPixels(g_renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
        surface->pixels, surface->pitch);
    SDL_SaveBMP(surface, path);
    SDL_FreeSurface(surface);
}

/* ================== 绘图 API 映射 ================== */

void ESTA_SDL2_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    set_draw_color(color);
    SDL_RenderDrawLine(g_renderer, x1, y1, x2, y2);
}

void ESTA_SDL2_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    set_draw_color(color);
    SDL_Rect rect = {x1, y1, x2 - x1, y2 - y1};
    SDL_RenderDrawRect(g_renderer, &rect);
}

void ESTA_SDL2_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    set_draw_color(color);
    SDL_Rect rect = {x1, y1, x2 - x1, y2 - y1};
    SDL_RenderFillRect(g_renderer, &rect);
}

void ESTA_SDL2_DrawStringFont(uint16_t x, uint16_t y, const char *str, uint8_t len,
                              uint8_t font_size, uint16_t color) {
    if (str == NULL) return;
    uint8_t width = 8;
    for (int i = 0; i < len; i++) {
        char c = str[i];
        if (c == '\0') break;
        if (font_size == 0) {
            width = 6;
            draw_glyph_columns(x, y, c, (const unsigned char *)asc2_1206, 6, 12, 2, color);
        } else if (font_size == 2) {
            width = 12;
            draw_glyph_columns(x, y, c, (const unsigned char *)asc2_2412, 12, 24, 3, color);
        } else {
            width = 8;
            draw_glyph_columns(x, y, c, (const unsigned char *)asc2_1608, 8, 16, 2, color);
        }
        x += width; // 渲染完一个字符，X轴偏移一个字宽
    }
}

void ESTA_SDL2_DrawString(uint16_t x, uint16_t y, const char *str, uint8_t len, uint16_t color) {
    ESTA_SDL2_DrawStringFont(x, y, str, len, 1, color);
}

void ESTA_SDL2_DrawNumFont(uint16_t x, uint16_t y, uint32_t num, uint8_t len,
                           uint8_t font_size, uint16_t color) {
    char buf[16];
    // 格式化为带前导零的字符串
    snprintf(buf, sizeof(buf), "%0*u", len, num);
    ESTA_SDL2_DrawStringFont(x, y, buf, (uint8_t)strlen(buf), font_size, color);
}

void ESTA_SDL2_DrawNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint16_t color) {
    ESTA_SDL2_DrawNumFont(x, y, num, len, 1, color);
}
