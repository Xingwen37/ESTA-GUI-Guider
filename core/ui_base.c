#include "ui_base.h"

uint16_t ui_coor_normal(uint16_t coor_width, uint16_t value_max_range, uint16_t value) {
    if (value_max_range == 0 || coor_width == 0) return 0;
    return (uint16_t)((((uint32_t)value * coor_width) + (value_max_range >> 1)) / value_max_range);
}

uint16_t ui_limit(uint16_t max, uint16_t min, uint16_t value) {
    if (value > max) return max;
    else if (value < min) return min;
    else return value;
}

bool ui_is_out_of_bound(uint16_t max, uint16_t min, uint16_t value) {
    return (value < min || value > max);
}

uint16_t ui_num_digits(uint16_t x) {
    int count = 0;
    while (x != 0) {
        x /= 10;
        count++;
    }
    return count;
}
