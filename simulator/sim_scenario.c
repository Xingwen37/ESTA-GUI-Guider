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

ESTA_StatusTypeDef SimScenario_ApplyDefaultProfile(int ESTA_idx) {
    ESTA_Config_TypeDef config = {0};
    uint16_t ruler_y[5] = {1000, 2000, 3000, 4000, 0};
    uint16_t ruler_x[5] = {30, 50, 90, 0, 0};

    if (ESTA_idx == 0) {
        ESTA_ConfigSetPositionAndSize(&config, 10, 0, 200, 120);
        ESTA_ConfigSetTheme(&config, ESTA_THEME_DEFAULT);
        ESTA_ConfigSetChannelEnabled(&config, CH0 | CH1 | CH2 | CH3);
    } else if (ESTA_idx == 1) {
        ESTA_ConfigSetPositionAndSize(&config, 0, 120, 200, 120);
        ESTA_ConfigSetTheme(&config, ESTA_THEME_LIGHT);
        ESTA_ConfigSetChannelEnabled(&config, CH0 | CH2 | CH3);
    } else {
        return ESTA_ERROR;
    }

    ESTA_ConfigSetDisplayRange(&config, 0, 4095);
    ESTA_ConfigSetChannelNum(&config, 4);
    ESTA_ConfigSetRulerY(&config, true, ruler_y, 4, 4);
    ESTA_ConfigSetRulerX(&config, true, ruler_x, 3, 0, 100, 8);
    ESTA_ConfigSetAutoClear(&config, true);

    return ESTA_Init(ESTA_INST(ESTA_idx), &config);
}

bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int ESTA_idx, uint16_t data_ch[MAX_ESTA_CHANNEL]) {
    if (runtime == NULL || data_ch == NULL) return false;
    if (ESTA_idx < 0 || ESTA_idx >= SIM_SCENARIO_ESTA_COUNT) return false;
    if (runtime->signal_lut == NULL || runtime->signal_len == 0) return false;

    memset(data_ch, 0, sizeof(uint16_t) * MAX_ESTA_CHANNEL);
    size_t base = runtime->tick % runtime->signal_len;
    size_t shift_32 = (runtime->tick + 32U) % runtime->signal_len;

    if (ESTA_idx == 0) {
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
