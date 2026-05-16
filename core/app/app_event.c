#include "app/app_event.h"
#include <stddef.h>

typedef struct {
    ESTA_EventHandler handler;
    void *user_data;
} App_HandlerEntry;

static App_HandlerEntry g_handlers[APP_EVENT_TYPE_MAX];

void App_EventInit(void) {
    for (int i = 0; i < APP_EVENT_TYPE_MAX; i++) {
        g_handlers[i].handler = NULL;
        g_handlers[i].user_data = NULL;
    }
}

void App_RegisterHandler(ESTA_EventType type, ESTA_EventHandler handler, void *user_data) {
    if ((uint8_t)type >= APP_EVENT_TYPE_MAX) return;
    g_handlers[(uint8_t)type].handler = handler;
    g_handlers[(uint8_t)type].user_data = user_data;
}

void App_UnregisterHandler(ESTA_EventType type) {
    if ((uint8_t)type >= APP_EVENT_TYPE_MAX) return;
    g_handlers[(uint8_t)type].handler = NULL;
    g_handlers[(uint8_t)type].user_data = NULL;
}

void App_DispatchEvents(void) {
    ESTA_Event evt;
    while (ESTA_EventPoll(&evt)) {
        uint8_t t = evt.type;
        if (t < APP_EVENT_TYPE_MAX && g_handlers[t].handler != NULL) {
            g_handlers[t].handler(&evt, g_handlers[t].user_data);
        }
    }
}
