#ifndef SIM_INPUT_H
#define SIM_INPUT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool quit_requested;
} SimInputResult;

SimInputResult SimInput_Poll(uint8_t button_count);

#endif
