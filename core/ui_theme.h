#ifndef __UI_THEME_H
#define __UI_THEME_H

#include <stdint.h>

#define UI_THEME_MAX        8
#define UI_COLOR_SLOT_MAX   16

extern uint16_t UI_themeColorTable[UI_THEME_MAX][UI_COLOR_SLOT_MAX];

uint16_t UI_GetThemeColor(int theme, int color_index);

#define ESTA_GetThemeColor(theme, color_index) \
    UI_GetThemeColor((int)(theme), (int)(color_index))

#endif
