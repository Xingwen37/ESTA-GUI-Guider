#include "sim_feed.h"

#include <string.h>
#include "ui/WAVE.h"
#include "ui/BARCHART.h"
#include "ui/TABLE.h"

#define SIM_BATCH_MAX_POINTS 320

static uint16_t s_data_wave[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL];
static uint16_t s_data_bar[SIM_SCENARIO_BARCHART_COUNT][BARCHART_MAX_BARS];
static uint16_t s_ch_buf[SIM_SCENARIO_WAVE_COUNT][MAX_WAVE_CHANNEL][SIM_BATCH_MAX_POINTS];
static uint16_t s_batch_window_len[SIM_SCENARIO_WAVE_COUNT];
static uint16_t s_bar_count[SIM_SCENARIO_BARCHART_COUNT];

static bool sim_feed_wave(App_MainState *app, SimScenarioRuntime *scenario) {
    if (!app->wave_trigger) return true;
    app->wave_trigger = false;

    uint8_t active = App_GetActivePage(&app->page_state);
    for (int i = 0; i < app->wave_inst_count; i++) {
        if (app->page_state.profiles->wave_profiles[i].page != active) continue;
        if (!SimScenario_GetNextFrame(scenario, i, s_data_wave[i])) {
            return false;
        }
        uint16_t xfw = s_batch_window_len[i];
        if (xfw == 0) continue;

        uint8_t mask = WAVE_CONFIG_MEMBER(i, channel_mask);
        for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
            if (is_channel_enabled(mask, (uint8_t)(CH0 << ch))) {
                memmove(&s_ch_buf[i][ch][0], &s_ch_buf[i][ch][1],
                        (xfw - 1U) * sizeof(s_ch_buf[i][ch][0]));
                s_ch_buf[i][ch][xfw - 1U] = s_data_wave[i][ch];
            }
        }

        WAVE_CurveClear(WAVE_INST(i));
        for (int ch = 0; ch < MAX_WAVE_CHANNEL; ch++) {
            if (is_channel_enabled(mask, (uint8_t)(CH0 << ch))) {
                WAVE_WRITE_PRIVATE(i, last_index, 0);
                WAVE_WRITE_PRIVATE(i, x_coor_last,
                    WAVE_CONFIG_MEMBER(i, x_origin));
                WAVE_CurveDrawBatch(WAVE_INST(i), ch, s_ch_buf[i][ch], xfw);
            }
        }
    }
    return true;
}

static void sim_feed_barchart(App_MainState *app, SimScenarioRuntime *scenario) {
    uint8_t active = App_GetActivePage(&app->page_state);
    for (int i = 0; i < app->bar_inst_count; i++) {
        if (app->page_state.profiles->bar_profiles[i].page != active) continue;
        if (SimScenario_BARCHART_GetData(scenario, s_data_bar[i], s_bar_count[i])) {
            BARCHART_UpdateAll(BARCHART_INST(i), s_data_bar[i], s_bar_count[i]);
        }
    }
}

static void sim_feed_table(App_MainState *app, SimScenarioRuntime *scenario) {
    uint8_t active = App_GetActivePage(&app->page_state);
    for (int i = 0; i < app->table_inst_count; i++) {
        if (app->page_state.profiles->table_profiles[i].page != active) continue;
        TABLE_UpdateUInt32(TABLE_INST(i), 0, 1, s_data_wave[0][0]);
        TABLE_UpdateUInt32(TABLE_INST(i), 1, 1, (uint32_t)(1000U + scenario->tick * 10U));
    }
}

void SimFeed_Init(App_MainState *app, SimScenarioRuntime *scenario) {
    (void)scenario;
    const ESTA_ProfileSet_TypeDef *profiles = app->page_state.profiles;

    memset(s_ch_buf, 0, sizeof(s_ch_buf));
    memset(s_batch_window_len, 0, sizeof(s_batch_window_len));
    memset(s_bar_count, 0, sizeof(s_bar_count));

    for (int i = 0; i < app->wave_inst_count; i++) {
        uint16_t xfw = WAVE_GetSampleCapacity(WAVE_INST(i));
        if (xfw > SIM_BATCH_MAX_POINTS) xfw = SIM_BATCH_MAX_POINTS;
        s_batch_window_len[i] = xfw;
    }
    for (int i = 0; i < app->bar_inst_count; i++) {
        s_bar_count[i] = profiles->bar_profiles[i].bar_count;
        if (s_bar_count[i] > BARCHART_MAX_BARS) s_bar_count[i] = BARCHART_MAX_BARS;
    }
}

bool SimFeed_Update(App_MainState *app, SimScenarioRuntime *scenario) {
    if (!sim_feed_wave(app, scenario)) return false;
    sim_feed_barchart(app, scenario);
    sim_feed_table(app, scenario);
    return true;
}

void SimFeed_Tick(SimScenarioRuntime *scenario) {
    SimScenario_Tick(scenario);
}
