#ifndef __UI_THEME_H
#define __UI_THEME_H

#include <stdint.h>

#define ESTA_THEME_COLOR(table, theme, idx) \
    ((table)[(int)(theme)][(int)(idx)])

#endif
