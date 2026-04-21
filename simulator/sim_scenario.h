#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "OSC.h"

#define SIM_SCENARIO_OSC_COUNT 2

typedef struct {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t x_width;
    uint16_t y_width;

    uint16_t display_num_min;
    uint16_t display_num_max;

    uint16_t channel_num;
    uint8_t channel_mask;

    bool is_display_ruler_y;
    uint16_t ruler_y[OSC_MAX_RULER_Y_NUM];
    uint16_t ruler_count_y;
    uint16_t ruler_num_digits_y;

    bool is_display_ruler_x;
    uint16_t ruler_x[OSC_MAX_RULER_X_NUM];
    uint16_t ruler_count_x;
    uint16_t ruler_zero_value_x;
    uint16_t ruler_full_value_x;
    uint16_t ruler_num_digits_x;

    OSC_theme_type theme_type;
    bool is_auto_clear;

    bool use_wave[MAX_OSC_CHANNEL];
    uint16_t dc_value[MAX_OSC_CHANNEL];
    uint8_t phase_offset[MAX_OSC_CHANNEL];
} SimScenarioOscProfile;

typedef struct {
    SimScenarioOscProfile osc_profiles[SIM_SCENARIO_OSC_COUNT];
    const uint16_t *signal_lut;
    size_t signal_len;
    size_t tick;
} SimScenarioRuntime;

bool SimScenario_LoadDefault(SimScenarioRuntime *runtime);
bool SimScenario_FillOscConfig(const SimScenarioRuntime *runtime, int osc_idx, OSC_Config_TypeDef *config);
bool SimScenario_GetNextFrame(const SimScenarioRuntime *runtime, int osc_idx, uint16_t data_ch[MAX_OSC_CHANNEL]);
void SimScenario_Tick(SimScenarioRuntime *runtime);

#endif
