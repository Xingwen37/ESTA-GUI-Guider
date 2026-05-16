#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include <stddef.h>
#include <stdint.h>

#include "ui/WAVE.h"
#include "ui/BARCHART.h"

#define SIM_SCENARIO_WAVE_COUNT 4
#define SIM_SCENARIO_BARCHART_COUNT 4

#define SIM_SIGNAL_MAX        255   /* output clamp (12-bit ADC) */
#define SIM_SIGNAL_MID        (SIM_SIGNAL_MAX / 2)  /* DC midpoint */
#define SIM_SIGNAL_AMP_DEFAULT ((SIM_SIGNAL_MAX * 44) / 100)  /* ~44% of max */

typedef enum {
    SIM_SIGNAL_SINE = 0,
    SIM_SIGNAL_SQUARE,
    SIM_SIGNAL_TRIANGLE,
    SIM_SIGNAL_SAWTOOTH,
    SIM_SIGNAL_NOISE,
    SIM_SIGNAL_CONST,
    SIM_SIGNAL_COUNT
} SimSignalType;

typedef struct {
    SimSignalType type;
    uint16_t amplitude;
    uint16_t offset;
    uint16_t period;
    uint16_t phase;
} SimSignalGen;

typedef struct {
    SimSignalGen wave_signals[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL];
    SimSignalGen bar_signals[SIM_SCENARIO_BARCHART_COUNT][BARCHART_MAX_BARS];
    size_t tick;
} SimScenarioRuntime;

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime);

bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int inst_idx, uint16_t data_ch[MAX_WAVE_CHANNEL]);
void SimScenario_Tick(SimScenarioRuntime *runtime);

bool SimScenario_BARCHART_GetData(const SimScenarioRuntime *runtime, uint16_t bar_data[BARCHART_MAX_BARS], uint16_t bar_count);

#endif
