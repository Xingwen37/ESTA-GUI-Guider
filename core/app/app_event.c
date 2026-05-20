#include "app/app_event.h"
#include "app/app_action.h"
#include "event/event_flag.h"
#include <stddef.h>

typedef struct {
    uint8_t type;
    uint8_t source_id;
    uint16_t event_id;
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

int App_Subscribe(ESTA_EventType type, uint8_t source_id, uint16_t event_id,
                  ESTA_EventHandler handler, void *user_data) {
    if (handler == NULL) return -1;
    for (int i = 0; i < APP_MAX_SUBSCRIPTIONS; i++) {
        if (!g_subs[i].active) {
            g_subs[i].type = (uint8_t)type;
            g_subs[i].source_id = source_id;
            g_subs[i].event_id = event_id;
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
            if (g_subs[i].source_id == APP_SOURCE_ANY) continue;
            if (g_subs[i].source_id != evt.source) continue;
            if (g_subs[i].event_id != APP_TRIGGER_ID_ANY && g_subs[i].event_id != evt.id) continue;
            {
                const App_BindingContext *ctx = (const App_BindingContext *)g_subs[i].user_data;
                if (ctx != NULL) {
                    uint8_t flags = ESTA_FlagReadAll();
                    if (ctx->guard_and_mask && (flags & ctx->guard_and_mask) != ctx->guard_and_mask) continue;
                    if (ctx->guard_or_mask  && (flags & ctx->guard_or_mask)  == 0)                   continue;
                    if (ctx->guard_inv_mask && (flags & ctx->guard_inv_mask) != 0)                   continue;
                }
            }
            if (g_subs[i].handler(&evt, g_subs[i].user_data)) {
                consumed = true;
                break;
            }
        }

        if (!consumed) {
            for (int i = 0; i < APP_MAX_SUBSCRIPTIONS; i++) {
                if (!g_subs[i].active) continue;
                if (g_subs[i].type != evt.type) continue;
                if (g_subs[i].source_id != APP_SOURCE_ANY) continue;
                if (g_subs[i].event_id != APP_TRIGGER_ID_ANY && g_subs[i].event_id != evt.id) continue;
                {
                    const App_BindingContext *ctx = (const App_BindingContext *)g_subs[i].user_data;
                    if (ctx != NULL) {
                        uint8_t flags = ESTA_FlagReadAll();
                        if (ctx->guard_and_mask && (flags & ctx->guard_and_mask) != ctx->guard_and_mask) continue;
                        if (ctx->guard_or_mask  && (flags & ctx->guard_or_mask)  == 0)                   continue;
                        if (ctx->guard_inv_mask && (flags & ctx->guard_inv_mask) != 0)                   continue;
                    }
                }
                g_subs[i].handler(&evt, g_subs[i].user_data);
                break;
            }
        }
    }
}
