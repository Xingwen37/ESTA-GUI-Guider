#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include <stddef.h>
#include <stdint.h>

#include "WAVE.h"

#define SIM_SCENARIO_WAVE_COUNT 2

typedef struct {
    const uint16_t *signal_lut;
    size_t signal_len;
    size_t tick;
} SimScenarioRuntime;

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime);

bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int inst_idx, uint16_t data_ch[MAX_WAVE_CHANNEL]);
void SimScenario_Tick(SimScenarioRuntime *runtime);

#endif
