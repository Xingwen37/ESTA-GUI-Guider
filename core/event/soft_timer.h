#ifndef __ESTA_SOFT_TIMER_H
#define __ESTA_SOFT_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "event/event.h"

#define ESTA_SOFT_TIMER_MAX 4

typedef struct {
    uint16_t period_ms;       /* 触发周期（ms），0 = 禁用 */
    uint8_t  event_type;      /* ESTA_EventType */
    uint8_t  event_source;
    uint16_t event_id;
} ESTA_SoftTimerConfig;

void ESTA_SoftTimerInit(void);
void ESTA_SoftTimerRegister(uint8_t timer_id, const ESTA_SoftTimerConfig *config);
void ESTA_SoftTimerTick(uint16_t delta_ms);

#endif
