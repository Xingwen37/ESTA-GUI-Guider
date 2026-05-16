#include "app/app_event.h"
#include <stddef.h>

typedef struct {
    uint8_t type;
    uint8_t source_min;
    uint8_t source_max;
    ESTA_EventHandler handler;
    void *user_data;
    bool active;
} App_Subscription;

static App_Subscription g_subs[APP_MAX_SUBSCRIPTIONS];

void App_EventInit(void) {
    for (int i = 0; i < APP_MAX_SUBSCRIPTIONS; i++) {
        g_subs[i].active = false;
    }
}

int App_Subscribe(ESTA_EventType type, uint8_t source_min, uint8_t source_max,
                  ESTA_EventHandler handler, void *user_data) {
    if (handler == NULL) return -1;
    for (int i = 0; i < APP_MAX_SUBSCRIPTIONS; i++) {
        if (!g_subs[i].active) {
            g_subs[i].type = (uint8_t)type;
            g_subs[i].source_min = source_min;
            g_subs[i].source_max = source_max;
            g_subs[i].handler = handler;
            g_subs[i].user_data = user_data;
            g_subs[i].active = true;
            return i;
        }
    }
    return -1;
}

void App_Unsubscribe(int subscription_id) {
    if (subscription_id < 0 || subscription_id >= APP_MAX_SUBSCRIPTIONS) return;
    g_subs[subscription_id].active = false;
}

void App_DispatchEvents(void) {
    ESTA_Event evt;
    while (ESTA_EventPoll(&evt)) {
        bool consumed = false;

        for (int i = 0; i < APP_MAX_SUBSCRIPTIONS; i++) {
            if (!g_subs[i].active) continue;
            if (g_subs[i].type != evt.type) continue;
            if (g_subs[i].source_max == APP_SOURCE_ANY) continue;
            if (evt.source >= g_subs[i].source_min &&
                evt.source <= g_subs[i].source_max) {
                if (g_subs[i].handler(&evt, g_subs[i].user_data)) {
                    consumed = true;
                    break;
                }
            }
        }

        if (!consumed) {
            for (int i = 0; i < APP_MAX_SUBSCRIPTIONS; i++) {
                if (!g_subs[i].active) continue;
                if (g_subs[i].type != evt.type) continue;
                if (g_subs[i].source_max != APP_SOURCE_ANY) continue;
                g_subs[i].handler(&evt, g_subs[i].user_data);
                break;
            }
        }
    }
}
