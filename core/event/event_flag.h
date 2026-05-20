#ifndef __ESTA_EVENT_FLAG_H
#define __ESTA_EVENT_FLAG_H

#include <stdint.h>
#include <stdbool.h>
#include "event/event.h"

#define ESTA_FLAG_MAX 8
#define ESTA_FLAG_WAVE_REDRAW 0

typedef enum {
    ESTA_FLAG_MODE_MANUAL = 0,
    ESTA_FLAG_MODE_AUTO_EVENT,
} ESTA_FlagMode;

typedef struct {
    ESTA_FlagMode mode;
    ESTA_EventType event_type;
    uint8_t event_source;
    uint16_t event_id;
} ESTA_FlagConfig;

void ESTA_FlagInit(void);
void ESTA_FlagRegister(uint8_t flag_id, const ESTA_FlagConfig *config);
void ESTA_FlagSet(uint8_t flag_id);
bool ESTA_FlagCheck(uint8_t flag_id);
bool ESTA_FlagPeek(uint8_t flag_id);
void ESTA_FlagClear(uint8_t flag_id);
void ESTA_FlagPoll(void);
uint8_t ESTA_FlagReadAll(void);

#endif
