#include "sim_scenario.h"

#include <string.h>

static const uint16_t g_default_signal[] = {
    2048, 2248, 2447, 2642, 2831, 3013, 3185, 3347, 3496, 3631, 3750, 3854,
    3940, 4007, 4056, 4086, 4095, 4086, 4056, 4007, 3940, 3854, 3750, 3631,
    3496, 3347, 3185, 3013, 2831, 2642, 2447, 2248, 2048, 1847, 1648, 1453,
    1264, 1082, 910, 748, 599, 464, 345, 241, 155, 88, 39, 9, 0, 9, 39, 88,
    155, 241, 345, 464, 599, 748, 910, 1082, 1264, 1453, 1648, 1847
};

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime) {
    if (runtime == NULL) return false;
    memset(runtime, 0, sizeof(*runtime));

    runtime->signal_lut = g_default_signal;
    runtime->signal_len = sizeof(g_default_signal) / sizeof(g_default_signal[0]);
    runtime->tick = 0;

    return true;
}

bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int inst_idx, uint16_t data_ch[MAX_WAVE_CHANNEL]) {
    if (runtime == NULL || data_ch == NULL) return false;
    if (inst_idx < 0 || inst_idx >= SIM_SCENARIO_WAVE_COUNT) return false;
    if (runtime->signal_lut == NULL || runtime->signal_len == 0) return false;

    memset(data_ch, 0, sizeof(uint16_t) * MAX_WAVE_CHANNEL);
    size_t base = runtime->tick % runtime->signal_len;
    size_t shift_32 = (runtime->tick + 32U) % runtime->signal_len;

    if (inst_idx == 0) {
        data_ch[0] = runtime->signal_lut[base];
        data_ch[1] = runtime->signal_lut[shift_32];
        data_ch[2] = 1000;
        data_ch[3] = 1600;
    } else {
        data_ch[0] = runtime->signal_lut[shift_32];
        data_ch[1] = runtime->signal_lut[base];
        data_ch[2] = 1400;
        data_ch[3] = 2400;
    }

    return true;
}

void SimScenario_Tick(SimScenarioRuntime *runtime) {
    if (runtime == NULL || runtime->signal_len == 0) return;
    runtime->tick = (runtime->tick + 1U) % runtime->signal_len;
}

bool SimScenario_BARCHART_GetData(const SimScenarioRuntime *runtime, uint16_t bar_data[BARCHART_MAX_BARS], uint16_t bar_count) {
    if (runtime == NULL || bar_data == NULL) return false;
    if (bar_count > BARCHART_MAX_BARS) bar_count = BARCHART_MAX_BARS;

    /* 用正弦波的多相位采样产生动态柱状图数据 */
    size_t t = runtime->tick;
    size_t len = runtime->signal_len;
    for (int i = 0; i < bar_count; i++) {
        size_t phase = (t + i * 10U) % len;
        uint16_t raw = runtime->signal_lut[phase];
        /* 将 12-bit 范围 0-4095 映射到 0-100 */
        bar_data[i] = (uint16_t)(((uint32_t)raw * 100U) / 4095U);
    }

    return true;
}
