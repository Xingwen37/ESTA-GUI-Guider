#include "event.h"

#include <string.h>

static ESTA_EventTypeDef g_event_queue[ESTA_EVENT_QUEUE_SIZE];
static uint8_t g_event_head = 0;
static uint8_t g_event_tail = 0;

void ESTA_EventInit(void) {
    memset(g_event_queue, 0, sizeof(g_event_queue));
    g_event_head = 0;
    g_event_tail = 0;
}

bool ESTA_EventPush(uint8_t button_id, uint8_t event_type) {
    uint8_t next = (g_event_head + 1) % ESTA_EVENT_QUEUE_SIZE;
    if (next == g_event_tail) {
        return false; /* queue full */
    }
    g_event_queue[g_event_head].button_id  = button_id;
    g_event_queue[g_event_head].event_type = event_type;
    g_event_head = next;
    return true;
}

bool ESTA_EventPoll(ESTA_EventTypeDef *out_event) {
    if (g_event_head == g_event_tail) {
        return false; /* queue empty */
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
