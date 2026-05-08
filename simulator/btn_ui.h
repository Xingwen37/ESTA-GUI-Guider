#ifndef __BTN_UI_H
#define __BTN_UI_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool BTN_UI_Init(uint8_t button_count);
void BTN_UI_ProcessEvent(const SDL_Event *event);
void BTN_UI_Render(void);
bool BTN_UI_IsAlive(void);
void BTN_UI_Destroy(void);

#endif
