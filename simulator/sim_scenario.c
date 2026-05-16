#include "sim_scenario.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint16_t SimSignal_Generate(const SimSignalGen *gen, size_t tick) {
    if (gen->period == 0) return gen->offset;

    double t = fmod((double)(tick + gen->phase), (double)gen->period) / (double)gen->period;
    double value = 0.0;

    switch (gen->type) {
        case SIM_SIGNAL_SINE:
            value = sin(2.0 * M_PI * t);
            break;
        case SIM_SIGNAL_SQUARE:
            value = (t < 0.5) ? 1.0 : -1.0;
            break;
        case SIM_SIGNAL_TRIANGLE:
            value = (t < 0.5) ? (4.0 * t - 1.0) : (3.0 - 4.0 * t);
            break;
        case SIM_SIGNAL_SAWTOOTH:
            value = 2.0 * t - 1.0;
            break;
        case SIM_SIGNAL_NOISE:
            value = ((double)(rand() % 2001) - 1000.0) / 1000.0;
            break;
        default:
            value = 0.0;
            break;
    }

    int32_t result = (int32_t)gen->offset + (int32_t)(value * (double)gen->amplitude);
    if (result < 0) result = 0;
    if (result > SIM_SIGNAL_MAX) result = SIM_SIGNAL_MAX;
    return (uint16_t)result;
}

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime) {
    if (runtime == NULL) return false;
    memset(runtime, 0, sizeof(*runtime));

    /* Instance 0: CH0=sine, CH1=square, CH2=triangle, CH3=const */
    runtime->wave_signals[0][0] = (SimSignalGen){ SIM_SIGNAL_SINE,     SIM_SIGNAL_AMP_DEFAULT, SIM_SIGNAL_MID, 200, 0 };
    runtime->wave_signals[0][1] = (SimSignalGen){ SIM_SIGNAL_SQUARE,   (SIM_SIGNAL_MAX * 37) / 100, SIM_SIGNAL_MID, 150, 0 };
    runtime->wave_signals[0][2] = (SimSignalGen){ SIM_SIGNAL_TRIANGLE, (SIM_SIGNAL_MAX * 29) / 100, SIM_SIGNAL_MID, 100, 0 };
    runtime->wave_signals[0][3] = (SimSignalGen){ SIM_SIGNAL_CONST,       0, SIM_SIGNAL_MID,   0, 0 };

    /* Instance 1: all sine, different frequencies/phases */
    runtime->wave_signals[1][0] = (SimSignalGen){ SIM_SIGNAL_SINE, (SIM_SIGNAL_MAX * 39) / 100, SIM_SIGNAL_MID, 180, 0 };
    runtime->wave_signals[1][1] = (SimSignalGen){ SIM_SIGNAL_SINE, (SIM_SIGNAL_MAX * 34) / 100, SIM_SIGNAL_MID, 120, 30 };
    runtime->wave_signals[1][2] = (SimSignalGen){ SIM_SIGNAL_SAWTOOTH, (SIM_SIGNAL_MAX * 24) / 100, SIM_SIGNAL_MID, 160, 0 };
    runtime->wave_signals[1][3] = (SimSignalGen){ SIM_SIGNAL_CONST, 0, SIM_SIGNAL_MID / 2, 0, 0 };

    /* Instance 2-3: sine with varying phase */
    for (int inst = 2; inst < SIM_SCENARIO_WAVE_COUNT; inst++) {
        for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
            runtime->wave_signals[inst][ch] = (SimSignalGen){
                SIM_SIGNAL_SINE, (SIM_SIGNAL_MAX * 37) / 100, SIM_SIGNAL_MID,
                (uint16_t)(200 + inst * 50), (uint16_t)(ch * 25)
            };
        }
    }

    /* BARCHART: sine waves with different phases, output 0-100 range */
    for (int inst = 0; inst < SIM_SCENARIO_BARCHART_COUNT; inst++) {
        for (int i = 0; i < BARCHART_MAX_BARS; i++) {
            runtime->bar_signals[inst][i] = (SimSignalGen){
                SIM_SIGNAL_SINE, 45, 50, 200, (uint16_t)(i * 20)
            };
        }
    }

    runtime->tick = 0;
    return true;
}

bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int inst_idx,
                              uint16_t data_ch[MAX_WAVE_CHANNEL]) {
    if (runtime == NULL || data_ch == NULL) return false;
    if (inst_idx < 0 || inst_idx >= SIM_SCENARIO_WAVE_COUNT) return false;

    for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
        data_ch[ch] = SimSignal_Generate(&runtime->wave_signals[inst_idx][ch], runtime->tick);
    }
    return true;
}

void SimScenario_Tick(SimScenarioRuntime *runtime) {
    if (runtime == NULL) return;
    runtime->tick++;
}

bool SimScenario_BARCHART_GetData(const SimScenarioRuntime *runtime,
                                  uint16_t bar_data[BARCHART_MAX_BARS], uint16_t bar_count) {
    if (runtime == NULL || bar_data == NULL) return false;
    if (bar_count > BARCHART_MAX_BARS) bar_count = BARCHART_MAX_BARS;

    for (int i = 0; i < bar_count; i++) {
        bar_data[i] = SimSignal_Generate(&runtime->bar_signals[0][i], runtime->tick);
    }
    return true;
}
