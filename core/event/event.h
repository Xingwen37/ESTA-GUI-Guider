#ifndef __ESTA_EVENT_LIB
#define __ESTA_EVENT_LIB

#include <stdint.h>
#include <stdbool.h>

#define ESTA_EVENT_QUEUE_SIZE 16

typedef enum {
    ESTA_EVENT_BUTTON_PRESS   = 0,
    ESTA_EVENT_BUTTON_RELEASE = 1,
    ESTA_EVENT_MENU_SELECT    = 2
} ESTA_EventType;

typedef struct {
    uint8_t button_id;
    uint8_t event_type;
} ESTA_EventTypeDef;

void        ESTA_EventInit(void);
bool        ESTA_EventPush(uint8_t button_id, uint8_t event_type);
bool        ESTA_EventPoll(ESTA_EventTypeDef *out_event);
uint8_t     ESTA_EventAvailable(void);

#endif
