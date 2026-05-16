#ifndef __APP_PAGE_H
#define __APP_PAGE_H

#include <stdint.h>
#include "profile/ESTA_Profile.h"

typedef struct {
    const ESTA_ProfileSet_TypeDef *profiles;
    uint8_t active_page;
    uint8_t page_count;
    uint16_t screen_width;
    uint16_t screen_height;
} App_PageState;

void    App_PageInit(App_PageState *state, const ESTA_ProfileSet_TypeDef *profiles,
                     uint16_t screen_width, uint16_t screen_height);
void    App_ApplyAndDrawPage(App_PageState *state, uint8_t page);
void    App_PageNext(App_PageState *state);
uint8_t App_GetActivePage(const App_PageState *state);

#endif
