#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include <stddef.h>
#include <stdint.h>

#include "ESTA.h"

#define SIM_SCENARIO_ESTA_COUNT 2

typedef struct {
    const uint16_t *signal_lut;
    size_t signal_len;
    size_t tick;
} SimScenarioRuntime;

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime);
ESTA_StatusTypeDef SimScenario_ApplyDefaultProfile(int ESTA_idx);
bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int ESTA_idx, uint16_t data_ch[MAX_ESTA_CHANNEL]);
void SimScenario_Tick(SimScenarioRuntime *runtime);

#endif
