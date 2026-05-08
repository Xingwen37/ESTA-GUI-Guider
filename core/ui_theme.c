#include "ui_theme.h"
#include "WAVE.h"

uint16_t UI_themeColorTable[UI_THEME_MAX][UI_COLOR_SLOT_MAX] = {
    [WAVE_THEME_DEFAULT] = {
        [WAVE_THEME_FRAME_INDEX]      = __WHITE,
        [WAVE_THEME_RULER_INDEX]      = __GRAY,
        [WAVE_THEME_WAVE_CH0_INDEX]   = __GREEN,
        [WAVE_THEME_WAVE_CH1_INDEX]   = __GBLUE,
        [WAVE_THEME_WAVE_CH2_INDEX]   = __YELLOW,
        [WAVE_THEME_WAVE_CH3_INDEX]   = __RED,
        [WAVE_THEME_BACKGROUND_INDEX] = __BLACK
    },
    [WAVE_THEME_LIGHT] = {
        [WAVE_THEME_FRAME_INDEX]      = __BLACK,
        [WAVE_THEME_RULER_INDEX]      = __GRAY,
        [WAVE_THEME_WAVE_CH0_INDEX]   = __ORANGE,
        [WAVE_THEME_WAVE_CH1_INDEX]   = __DEEP_BLUE,
        [WAVE_THEME_WAVE_CH2_INDEX]   = __RED,
        [WAVE_THEME_WAVE_CH3_INDEX]   = __BLUE,
        [WAVE_THEME_BACKGROUND_INDEX] = __WHITE
    }
};

uint16_t UI_GetThemeColor(int theme, int color_index) {
    if (theme >= UI_THEME_MAX || theme < 0) theme = 0;
    if (color_index >= UI_COLOR_SLOT_MAX || color_index < 0) color_index = 0;
    return UI_themeColorTable[theme][color_index];
}
