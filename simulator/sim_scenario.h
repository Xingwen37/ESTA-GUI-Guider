#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include <stddef.h>
#include <stdint.h>

#include "ui/WAVE.h"
#include "ui/BARCHART.h"

#define SIM_SCENARIO_WAVE_COUNT 2
#define SIM_SCENARIO_BARCHART_COUNT 1

typedef struct {
    const uint16_t *signal_lut;
    size_t signal_len;
    size_t tick;
} SimScenarioRuntime;

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime);

bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int inst_idx, uint16_t data_ch[MAX_WAVE_CHANNEL]);
bool SimScenario_GetBatchData(const SimScenarioRuntime *runtime, uint16_t *buf, uint16_t count);
void SimScenario_Tick(SimScenarioRuntime *runtime);

bool SimScenario_BARCHART_GetData(const SimScenarioRuntime *runtime, uint16_t bar_data[BARCHART_MAX_BARS], uint16_t bar_count);

#endif
