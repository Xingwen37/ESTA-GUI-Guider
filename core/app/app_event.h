#ifndef __APP_EVENT_H
#define __APP_EVENT_H

#include "event/event.h"

#define APP_MAX_SUBSCRIPTIONS 8
#define APP_SOURCE_ANY        0xFF

typedef bool (*ESTA_EventHandler)(const ESTA_Event *event, void *user_data);

void App_EventInit(void);
int  App_Subscribe(ESTA_EventType type, uint8_t source_min, uint8_t source_max,
                   ESTA_EventHandler handler, void *user_data);
void App_Unsubscribe(int subscription_id);
void App_DispatchEvents(void);

#endif
