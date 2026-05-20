#include "event/event_flag.h"
#include "infra/ui_base.h"
#include <string.h>

static volatile uint8_t s_flags[ESTA_FLAG_MAX];

void ESTA_FlagInit(void) {
    memset((void *)s_flags, 0, sizeof(s_flags));
}

void ESTA_FlagRegister(uint8_t flag_id, const ESTA_FlagConfig *config) {
    if (flag_id >= ESTA_FLAG_MAX || config == NULL) return;
    s_flags[flag_id] = 0;
}

void ESTA_FlagSignal(uint8_t flag_id) {
    if (flag_id >= ESTA_FLAG_MAX) return;
    s_flags[flag_id] = 1;

    ESTA_Event evt;
    evt.type = (uint8_t)ESTA_EVENT_FLAG;
    evt.source = flag_id;
    evt.id = 0;
    evt.timestamp = ESTA_GET_TICK();
    ESTA_EventPush(&evt);
}

void ESTA_FlagSet(uint8_t flag_id) {
    if (flag_id >= ESTA_FLAG_MAX) return;
    s_flags[flag_id] = 1;
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

uint8_t ESTA_FlagReadAll(void) {
    uint8_t result = 0;
    for (uint8_t i = 0; i < ESTA_FLAG_MAX; i++) {
        if (s_flags[i]) result |= (uint8_t)(1u << i);
    }
    return result;
}
