#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include <stddef.h>
#include <stdint.h>

#include "OSC.h"

#define SIM_SCENARIO_OSC_COUNT 2

typedef struct {
    const uint16_t *signal_lut;
    size_t signal_len;
    size_t tick;
} SimScenarioRuntime;

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime);
OSC_StatusTypeDef SimScenario_ApplyDefaultProfile(int osc_idx);
bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int osc_idx, uint16_t data_ch[MAX_OSC_CHANNEL]);
void SimScenario_Tick(SimScenarioRuntime *runtime);

#endif
