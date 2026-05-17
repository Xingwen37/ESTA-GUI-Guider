#ifndef __ESTA_EVENT_LIB
#define __ESTA_EVENT_LIB

#include <stdint.h>
#include <stdbool.h>

#define ESTA_EVENT_QUEUE_SIZE 16

typedef enum {
    ESTA_EVENT_NONE           = 0,
    ESTA_EVENT_BUTTON_PRESS   = 1,
    ESTA_EVENT_BUTTON_RELEASE = 2,
    ESTA_EVENT_MENU_SELECT    = 3,
    ESTA_EVENT_ENCODER_ROTATE = 4,
    ESTA_EVENT_TIMER          = 5,
    ESTA_EVENT_FLAG           = 6,
    ESTA_EVENT_CUSTOM         = 0xFF
} ESTA_EventType;

typedef struct {
    uint8_t  type;       /* ESTA_EventType */
    uint8_t  source;     /* physical source: button index, encoder index, component inst */
    uint16_t id;         /* semantic ID: menu item event_id, timer_id, etc. */
    uint32_t timestamp;  /* system tick in ms, filled by ESTA_GET_TICK() */
} ESTA_Event;

void        ESTA_EventInit(void);
bool        ESTA_EventPush(const ESTA_Event *event);
bool        ESTA_EventPoll(ESTA_Event *out_event);
uint8_t     ESTA_EventAvailable(void);

bool        ESTA_EventEmitButton(uint8_t button_id, ESTA_EventType type);
bool        ESTA_EventEmitMenuSelect(uint8_t menu_inst, uint16_t event_id);

#endif
