#include "sim_scenario.h"

#include <string.h>

static const uint16_t g_default_signal[] = {
128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 
168, 172, 175, 179, 183, 186, 190, 193, 197, 200, 
204, 207, 210, 213, 216, 219, 222, 224, 227, 229, 
232, 234, 236, 239, 240, 242, 244, 246, 247, 249, 
250, 251, 252, 253, 254, 255, 255, 256, 256, 256, 
256, 256, 256, 255, 255, 254, 254, 253, 252, 251, 
249, 248, 247, 245, 243, 241, 240, 237, 235, 233, 
231, 228, 226, 223, 220, 217, 214, 211, 208, 205, 
202, 199, 195, 192, 188, 185, 181, 177, 173, 170, 
166, 162, 158, 154, 150, 146, 142, 138, 134, 130, 
126, 122, 118, 114, 110, 106, 102,  98,  94,  90, 
 86,  83,  79,  75,  71,  68,  64,  61,  57,  54, 
 51,  48,  45,  42,  39,  36,  33,  30,  28,  25, 
 23,  21,  19,  16,  15,  13,  11,   9,   8,   7, 
  5,   4,   3,   2,   2,   1,   1,   0,   0,   0, 
  0,   0,   0,   1,   1,   2,   3,   4,   5,   6, 
  7,   9,  10,  12,  14,  16,  17,  20,  22,  24, 
 27,  29,  32,  34,  37,  40,  43,  46,  49,  52, 
 56,  59,  63,  66,  70,  73,  77,  81,  84,  88, 
 92,  96, 100, 104, 108, 112, 116, 120, 124, 128 
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

bool SimScenario_GetBatchData(const SimScenarioRuntime *runtime, uint16_t *buf, uint16_t count) {
    if (runtime == NULL || buf == NULL || count == 0) return false;
    if (runtime->signal_lut == NULL || runtime->signal_len == 0) return false;

    for (uint16_t i = 0; i < count; i++) {
        buf[i] = runtime->signal_lut[(runtime->tick + i) % runtime->signal_len];
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
