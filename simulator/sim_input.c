#include "sim_input.h"
#include <SDL2/SDL.h>
#include "event/event.h"
#include "esta_port_sdl2.h"

SimInputResult SimInput_Poll(uint8_t button_count) {
    SimInputResult result = { .quit_requested = false };
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            result.quit_requested = true;
        } else if (event.type == SDL_WINDOWEVENT &&
                   event.window.event == SDL_WINDOWEVENT_CLOSE) {
            uint32_t main_id = ESTA_SDL2_GetWindowID();
            if (event.window.windowID == main_id) {
                result.quit_requested = true;
            }
        } else if (event.type == SDL_KEYDOWN) {
            uint32_t main_id = ESTA_SDL2_GetWindowID();
            if (event.key.windowID == main_id &&
                event.key.keysym.sym >= SDLK_0 &&
                event.key.keysym.sym <= SDLK_9) {
                uint8_t btn_id = (uint8_t)(event.key.keysym.sym - SDLK_0);
                if (btn_id < button_count) {
                    ESTA_EventEmitButton(btn_id, ESTA_EVENT_BUTTON_PRESS);
                }
            }
        } else if (event.type == SDL_KEYUP) {
            uint32_t main_id = ESTA_SDL2_GetWindowID();
            if (event.key.windowID == main_id &&
                event.key.keysym.sym >= SDLK_0 &&
                event.key.keysym.sym <= SDLK_9) {
                uint8_t btn_id = (uint8_t)(event.key.keysym.sym - SDLK_0);
                if (btn_id < button_count) {
                    ESTA_EventEmitButton(btn_id, ESTA_EVENT_BUTTON_RELEASE);
                }
            }
        }
    }

    return result;
}
