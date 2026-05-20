#include "event/event_flag.h"
#include "infra/ui_base.h"
#include <string.h>

static volatile uint8_t s_flags[ESTA_FLAG_MAX];
static ESTA_FlagConfig s_config[ESTA_FLAG_MAX];
static bool s_registered[ESTA_FLAG_MAX];

void ESTA_FlagInit(void) {
    memset((void *)s_flags, 0, sizeof(s_flags));
    memset(s_config, 0, sizeof(s_config));
    memset(s_registered, 0, sizeof(s_registered));
}

void ESTA_FlagRegister(uint8_t flag_id, const ESTA_FlagConfig *config) {
    if (flag_id >= ESTA_FLAG_MAX || config == NULL) return;
    s_config[flag_id] = *config;
    s_registered[flag_id] = true;
    s_flags[flag_id] = 0;
}

void ESTA_FlagSet(uint8_t flag_id) {
    if (flag_id >= ESTA_FLAG_MAX) return;
    s_flags[flag_id] = 1;

    ESTA_Event evt;
    evt.type = (uint8_t)ESTA_EVENT_FLAG;
    evt.source = flag_id;
    evt.id = 0;
    evt.timestamp = ESTA_GET_TICK();
    ESTA_EventPush(&evt);
}

bool ESTA_FlagCheck(uint8_t flag_id) {
    if (flag_id >= ESTA_FLAG_MAX) return false;
    if (s_flags[flag_id]) {
        s_flags[flag_id] = 0;
        return true;
    }
    return false;
}

bool ESTA_FlagPeek(uint8_t flag_id) {
    if (flag_id >= ESTA_FLAG_MAX) return false;
    return s_flags[flag_id] != 0;
}

void ESTA_FlagClear(uint8_t flag_id) {
    if (flag_id < ESTA_FLAG_MAX) {
        s_flags[flag_id] = 0;
    }
}

void ESTA_FlagPoll(void) {
    for (uint8_t i = 0; i < ESTA_FLAG_MAX; i++) {
        if (!s_registered[i]) continue;
        if (!s_flags[i]) continue;

        bool is_latch = (s_config[i].mode == ESTA_FLAG_MODE_LATCH);
        if (s_config[i].mode != ESTA_FLAG_MODE_AUTO_EVENT && !is_latch) continue;

        ESTA_Event evt;
        evt.type = (uint8_t)s_config[i].event_type;
        evt.source = s_config[i].event_source;
        evt.id = s_config[i].event_id;
        evt.timestamp = ESTA_GET_TICK();
        ESTA_EventPush(&evt);

        if (!is_latch) {
            s_flags[i] = 0;
        }
    }
}

uint8_t ESTA_FlagReadAll(void) {
    uint8_t result = 0;
    for (uint8_t i = 0; i < ESTA_FLAG_MAX; i++) {
        if (s_flags[i]) result |= (uint8_t)(1u << i);
    }
    return result;
}
