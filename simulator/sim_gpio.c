#include "sim_gpio.h"
#include <string.h>

static uint8_t     s_pin_state[SIM_GPIO_MAX_PINS];
static SimGPIO_ISR s_isr[SIM_GPIO_MAX_PINS];

void SimGPIO_Init(void) {
    memset(s_pin_state, 0, sizeof(s_pin_state));
    memset(s_isr, 0, sizeof(s_isr));
}

void SimGPIO_SetPin(uint8_t pin, uint8_t level) {
    if (pin >= SIM_GPIO_MAX_PINS) return;
    uint8_t prev = s_pin_state[pin];
    s_pin_state[pin] = level ? 1 : 0;

    if (prev == 0 && s_pin_state[pin] == 1 && s_isr[pin] != NULL) {
        s_isr[pin](pin);
    }
}

uint8_t SimGPIO_ReadPin(uint8_t pin) {
    if (pin >= SIM_GPIO_MAX_PINS) return 0;
    return s_pin_state[pin];
}

void SimGPIO_AttachInterrupt(uint8_t pin, SimGPIO_ISR isr) {
    if (pin >= SIM_GPIO_MAX_PINS) return;
    s_isr[pin] = isr;
}

void SimGPIO_DetachInterrupt(uint8_t pin) {
    if (pin >= SIM_GPIO_MAX_PINS) return;
    s_isr[pin] = NULL;
}
