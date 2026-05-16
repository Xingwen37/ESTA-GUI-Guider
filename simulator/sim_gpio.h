#ifndef SIM_GPIO_H
#define SIM_GPIO_H

#include <stdint.h>
#include <stdbool.h>

#define SIM_GPIO_MAX_PINS  16

typedef void (*SimGPIO_ISR)(uint8_t pin);

void    SimGPIO_Init(void);
void    SimGPIO_SetPin(uint8_t pin, uint8_t level);
uint8_t SimGPIO_ReadPin(uint8_t pin);
void    SimGPIO_AttachInterrupt(uint8_t pin, SimGPIO_ISR isr);
void    SimGPIO_DetachInterrupt(uint8_t pin);

#endif
