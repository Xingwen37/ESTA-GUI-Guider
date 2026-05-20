#ifndef __ESTA_EVENT_FLAG_H
#define __ESTA_EVENT_FLAG_H

#include <stdint.h>
#include <stdbool.h>
#include "event/event.h"

#define ESTA_FLAG_MAX 8
#define ESTA_FLAG_WAVE_REDRAW 0

typedef enum {
    ESTA_FLAG_MODE_DISABLED = 0,
    ESTA_FLAG_MODE_MANUAL = 1,
} ESTA_FlagMode;

typedef struct {
    ESTA_FlagMode mode;
} ESTA_FlagConfig;

void ESTA_FlagInit(void);
void ESTA_FlagRegister(uint8_t flag_id, const ESTA_FlagConfig *config);
void ESTA_FlagSignal(uint8_t flag_id);  /* 置位状态位 + 推送 FLAG 事件 */
void ESTA_FlagSet(uint8_t flag_id);     /* 仅置位状态位，不推送事件 */
bool ESTA_FlagCheck(uint8_t flag_id);
bool ESTA_FlagPeek(uint8_t flag_id);
void ESTA_FlagClear(uint8_t flag_id);
uint8_t ESTA_FlagReadAll(void);

#endif
