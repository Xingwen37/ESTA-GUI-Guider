#include "event/event.h"
#include "infra/ui_base.h"

#include <string.h>

static ESTA_Event g_event_queue[ESTA_EVENT_QUEUE_SIZE];
static uint8_t g_event_head = 0;
static uint8_t g_event_tail = 0;

void ESTA_EventInit(void) {
    memset(g_event_queue, 0, sizeof(g_event_queue));
    g_event_head = 0;
    g_event_tail = 0;
}

bool ESTA_EventPush(const ESTA_Event *event) {
    if (event == NULL) return false;
    uint8_t next = (g_event_head + 1) % ESTA_EVENT_QUEUE_SIZE;
    if (next == g_event_tail) {
        return false;
    }
    g_event_queue[g_event_head] = *event;
    g_event_head = next;
    return true;
}

bool ESTA_EventPoll(ESTA_Event *out_event) {
    if (out_event == NULL) return false;
    if (g_event_head == g_event_tail) {
        return false;
    }
    *out_event = g_event_queue[g_event_tail];
    g_event_tail = (g_event_tail + 1) % ESTA_EVENT_QUEUE_SIZE;
    return true;
}

uint8_t ESTA_EventAvailable(void) {
    if (g_event_head >= g_event_tail) {
        return g_event_head - g_event_tail;
    }
    return (uint8_t)(ESTA_EVENT_QUEUE_SIZE - g_event_tail + g_event_head);
}

bool ESTA_EventEmitButton(uint8_t button_id, ESTA_EventType type) {
    ESTA_Event evt;
    evt.type      = (uint8_t)type;
    evt.source    = button_id;
    evt.id        = 0;
    evt.timestamp = ESTA_GET_TICK();
    return ESTA_EventPush(&evt);
}

bool ESTA_EventEmitMenuSelect(uint8_t menu_inst, uint16_t event_id) {
    ESTA_Event evt;
    evt.type      = (uint8_t)ESTA_EVENT_MENU_SELECT;
    evt.source    = menu_inst;
    evt.id        = event_id;
    evt.timestamp = ESTA_GET_TICK();
    return ESTA_EventPush(&evt);
}
