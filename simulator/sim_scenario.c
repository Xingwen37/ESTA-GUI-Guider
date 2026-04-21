#include "sim_scenario.h"

#include <string.h>

static const uint16_t g_default_signal[] = {
    2048, 2248, 2447, 2642, 2831, 3013, 3185, 3347, 3496, 3631, 3750, 3854,
    3940, 4007, 4056, 4086, 4095, 4086, 4056, 4007, 3940, 3854, 3750, 3631,
    3496, 3347, 3185, 3013, 2831, 2642, 2447, 2248, 2048, 1847, 1648, 1453,
    1264, 1082, 910, 748, 599, 464, 345, 241, 155, 88, 39, 9, 0, 9, 39, 88,
    155, 241, 345, 464, 599, 748, 910, 1082, 1264, 1453, 1648, 1847
};

static void sim_scenario_set_default_profile(SimScenarioOscProfile *profile, int osc_idx) {
    if (profile == NULL) return;
    memset(profile, 0, sizeof(*profile));

    profile->x_origin = (osc_idx == 0) ? 10 : 0;
    profile->y_origin = (osc_idx == 0) ? 0 : 120;
    profile->x_width = 200;
    profile->y_width = 120;

    profile->display_num_min = 0;
    profile->display_num_max = 4095;

    profile->channel_num = 4;
    profile->channel_mask = CH0 | CH1 | CH2 | CH3;
    if (osc_idx == 1) {
        profile->channel_mask &= (uint8_t)~CH1;
    }

    profile->is_display_ruler_y = true;
    profile->ruler_y[0] = 1000;
    profile->ruler_y[1] = 2000;
    profile->ruler_y[2] = 3000;
    profile->ruler_y[3] = 4000;
    profile->ruler_count_y = 4;
    profile->ruler_num_digits_y = 4;

    profile->is_display_ruler_x = true;
    profile->ruler_x[0] = 30;
    profile->ruler_x[1] = 50;
    profile->ruler_x[2] = 90;
    profile->ruler_count_x = 3;
    profile->ruler_zero_value_x = 0;
    profile->ruler_full_value_x = 100;
    profile->ruler_num_digits_x = 8;

    profile->theme_type = (osc_idx == 0) ? OSC_THEME_DEFAULT : OSC_THEME_LIGHT;
    profile->is_auto_clear = true;

    if (osc_idx == 0) {
        profile->use_wave[0] = true;
        profile->use_wave[1] = true;
        profile->use_wave[2] = false;
        profile->use_wave[3] = false;
        profile->dc_value[2] = 1000;
        profile->dc_value[3] = 1600;
        profile->phase_offset[0] = 0;
        profile->phase_offset[1] = 32;
    } else {
        profile->use_wave[0] = true;
        profile->use_wave[1] = true;
        profile->use_wave[2] = false;
        profile->use_wave[3] = false;
        profile->dc_value[2] = 1400;
        profile->dc_value[3] = 2400;
        profile->phase_offset[0] = 32;
        profile->phase_offset[1] = 0;
    }
}

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime) {
    if (runtime == NULL) return false;
    memset(runtime, 0, sizeof(*runtime));

    runtime->signal_lut = g_default_signal;
    runtime->signal_len = sizeof(g_default_signal) / sizeof(g_default_signal[0]);
    runtime->tick = 0;

    for (int i = 0; i < SIM_SCENARIO_OSC_COUNT; i++) {
        sim_scenario_set_default_profile(&runtime->osc_profiles[i], i);
    }
    return true;
}

bool SimScenario_FillOscConfig(const SimScenarioRuntime *runtime, int osc_idx, OSC_Config_TypeDef *config) {
    if (runtime == NULL || config == NULL) return false;
    if (osc_idx < 0 || osc_idx >= SIM_SCENARIO_OSC_COUNT) return false;

    const SimScenarioOscProfile *profile = &runtime->osc_profiles[osc_idx];
    memset(config, 0, sizeof(*config));

    OSC_ConfigSetPositionAndSize(config, profile->x_origin, profile->y_origin,
                                 profile->x_width, profile->y_width);
    OSC_ConfigSetDisplayRange(config, profile->display_num_min, profile->display_num_max);
    OSC_ConfigSetChannelNum(config, profile->channel_num);
    OSC_ConfigSetChannelEnabled(config, profile->channel_mask);
    OSC_ConfigSetRulerY(config, profile->is_display_ruler_y, (uint16_t *)profile->ruler_y,
                        profile->ruler_count_y, profile->ruler_num_digits_y);
    OSC_ConfigSetRulerX(config, profile->is_display_ruler_x, (uint16_t *)profile->ruler_x,
                        profile->ruler_count_x, profile->ruler_zero_value_x,
                        profile->ruler_full_value_x, profile->ruler_num_digits_x);
    OSC_ConfigSetTheme(config, profile->theme_type);
    OSC_ConfigSetAutoClear(config, profile->is_auto_clear);
    return true;
}

bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int osc_idx, uint16_t data_ch[MAX_OSC_CHANNEL]) {
    if (runtime == NULL || data_ch == NULL) return false;
    if (osc_idx < 0 || osc_idx >= SIM_SCENARIO_OSC_COUNT) return false;
    if (runtime->signal_lut == NULL || runtime->signal_len == 0) return false;

    memset(data_ch, 0, sizeof(uint16_t) * MAX_OSC_CHANNEL);

    const SimScenarioOscProfile *profile = &runtime->osc_profiles[osc_idx];
    uint16_t active_channel_num = profile->channel_num;
    if (active_channel_num > MAX_OSC_CHANNEL) {
        active_channel_num = MAX_OSC_CHANNEL;
    }

    for (uint16_t ch = 0; ch < active_channel_num; ch++) {
        uint8_t ch_mask = (uint8_t)(CH0 << ch);
        if ((profile->channel_mask & ch_mask) == 0) continue;

        if (profile->use_wave[ch]) {
            size_t idx = (runtime->tick + profile->phase_offset[ch]) % runtime->signal_len;
            data_ch[ch] = runtime->signal_lut[idx];
        } else {
            data_ch[ch] = profile->dc_value[ch];
        }
    }
    return true;
}

void SimScenario_Tick(SimScenarioRuntime *runtime) {
    if (runtime == NULL || runtime->signal_len == 0) return;
    runtime->tick = (runtime->tick + 1U) % runtime->signal_len;
}
