#include "event/soft_timer.h"
#include "infra/ui_base.h"
#include <string.h>

static ESTA_SoftTimerConfig s_configs[ESTA_SOFT_TIMER_MAX];
static uint16_t s_elapsed[ESTA_SOFT_TIMER_MAX];
static bool s_registered[ESTA_SOFT_TIMER_MAX];

void ESTA_SoftTimerInit(void) {
    memset(s_configs, 0, sizeof(s_configs));
    memset(s_elapsed, 0, sizeof(s_elapsed));
    memset(s_registered, 0, sizeof(s_registered));
}

void ESTA_SoftTimerRegister(uint8_t timer_id, const ESTA_SoftTimerConfig *config) {
    if (timer_id >= ESTA_SOFT_TIMER_MAX || config == NULL) return;
    s_configs[timer_id] = *config;
    s_elapsed[timer_id] = 0;
    s_registered[timer_id] = (config->period_ms > 0);
}

void ESTA_SoftTimerTick(uint16_t delta_ms) {
    for (uint8_t i = 0; i < ESTA_SOFT_TIMER_MAX; i++) {
        if (!s_registered[i]) continue;
        s_elapsed[i] += delta_ms;
        if (s_elapsed[i] >= s_configs[i].period_ms) {
            s_elapsed[i] = 0;
            ESTA_Event evt;
            evt.type      = s_configs[i].event_type;
            evt.source    = s_configs[i].event_source;
            evt.id        = s_configs[i].event_id;
            evt.timestamp = ESTA_GET_TICK();
            ESTA_EventPush(&evt);
        }
    }
}
