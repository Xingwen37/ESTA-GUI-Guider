#ifndef __APP_EVENT_H
#define __APP_EVENT_H

#include "event/event.h"

#define APP_EVENT_TYPE_MAX 6

typedef void (*ESTA_EventHandler)(const ESTA_Event *event, void *user_data);

void App_EventInit(void);
void App_RegisterHandler(ESTA_EventType type, ESTA_EventHandler handler, void *user_data);
void App_UnregisterHandler(ESTA_EventType type);
void App_DispatchEvents(void);

#endif
