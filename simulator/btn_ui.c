#include "btn_ui.h"
#include "event/event.h"

#include <stdio.h>

#define BTN_W  80
#define BTN_H  40
#define BTN_GAP 10
#define WIN_H  120

static SDL_Window   *g_btn_window   = NULL;
static SDL_Renderer *g_btn_renderer = NULL;
static uint8_t       g_button_count = 0;
static bool          g_alive        = false;
static bool          g_pressed[16];  /* track which buttons are held down */

/* button area rectangles, computed on init */
static SDL_Rect g_btn_rects[16];

bool BTN_UI_Init(uint8_t button_count) {
    if (button_count == 0 || button_count > 16) return false;

    g_button_count = button_count;

    int win_w = button_count * BTN_W + (button_count - 1) * BTN_GAP + 20;
    if (win_w < 200) win_w = 200;

    g_btn_window = SDL_CreateWindow("ESTA Button Panel",
        SDL_WINDOWPOS_CENTERED + 400, SDL_WINDOWPOS_CENTERED,
        win_w, WIN_H, SDL_WINDOW_SHOWN);
    if (g_btn_window == NULL) return false;

    g_btn_renderer = SDL_CreateRenderer(g_btn_window, -1, SDL_RENDERER_ACCELERATED);
    if (g_btn_renderer == NULL) {
        SDL_DestroyWindow(g_btn_window);
        g_btn_window = NULL;
        return false;
    }

    int total_w = button_count * BTN_W + (button_count - 1) * BTN_GAP;
    int start_x = (win_w - total_w) / 2;
    int start_y = (WIN_H - BTN_H) / 2;

    for (int i = 0; i < button_count; i++) {
        g_btn_rects[i].x = start_x + i * (BTN_W + BTN_GAP);
        g_btn_rects[i].y = start_y;
        g_btn_rects[i].w = BTN_W;
        g_btn_rects[i].h = BTN_H;
        g_pressed[i] = false;
    }

    g_alive = true;
    printf("[BTN_UI] Initialized with %d button(s)\n", button_count);
    for (int i = 0; i < button_count; i++) {
        printf("  BTN_%d: click leftmost → rightmost, button at (%d,%d)\n",
               i, g_btn_rects[i].x, g_btn_rects[i].y);
    }
    return true;
}

void BTN_UI_ProcessEvent(const SDL_Event *event) {
    if (!g_alive) return;

    uint32_t our_id = SDL_GetWindowID(g_btn_window);

    if (event->type == SDL_QUIT) {
        g_alive = false;
        return;
    }

    if (event->type == SDL_WINDOWEVENT
        && event->window.windowID == our_id
        && event->window.event == SDL_WINDOWEVENT_CLOSE) {
        printf("[BTN_UI] Button panel window closed\n");
        g_alive = false;
        return;
    }

    /* only handle mouse events in our window */
    if (event->button.windowID != our_id) return;

    int mx, my;
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        mx = event->button.x;
        my = event->button.y;
    } else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        mx = event->button.x;
        my = event->button.y;
    } else {
        return;
    }

    SDL_Point pt = { mx, my };
    for (int i = 0; i < g_button_count; i++) {
        if (SDL_PointInRect(&pt, &g_btn_rects[i])) {
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                g_pressed[i] = true;
                ESTA_EventPush((uint8_t)i, ESTA_EVENT_BUTTON_PRESS);
                printf("Button %d Pressed\n", i);
            } else if (event->type == SDL_MOUSEBUTTONUP) {
                g_pressed[i] = false;
                ESTA_EventPush((uint8_t)i, ESTA_EVENT_BUTTON_RELEASE);
                printf("Button %d Released\n", i);
            }
            break;
        }
    }
}

void BTN_UI_Render(void) {
    if (!g_alive) return;

    /* background */
    SDL_SetRenderDrawColor(g_btn_renderer, 30, 30, 30, 255);
    SDL_RenderClear(g_btn_renderer);

    for (int i = 0; i < g_button_count; i++) {
        if (g_pressed[i]) {
            SDL_SetRenderDrawColor(g_btn_renderer, 14, 99, 156, 255);
        } else {
            SDL_SetRenderDrawColor(g_btn_renderer, 60, 60, 60, 255);
        }
        SDL_RenderFillRect(g_btn_renderer, &g_btn_rects[i]);

        /* border */
        SDL_SetRenderDrawColor(g_btn_renderer, 212, 212, 212, 255);
        SDL_RenderDrawRect(g_btn_renderer, &g_btn_rects[i]);
    }

    SDL_RenderPresent(g_btn_renderer);
}

bool BTN_UI_IsAlive(void) {
    return g_alive;
}

void BTN_UI_Destroy(void) {
    if (g_btn_renderer != NULL) {
        SDL_DestroyRenderer(g_btn_renderer);
        g_btn_renderer = NULL;
    }
    if (g_btn_window != NULL) {
        SDL_DestroyWindow(g_btn_window);
        g_btn_window = NULL;
    }
    g_alive = false;
    g_button_count = 0;
}
