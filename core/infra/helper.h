#ifndef HELPER_H
#define HELPER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* 通道掩码常量（独热码） */
#define CH0  (1U << 0)
#define CH1  (1U << 1)
#define CH2  (1U << 2)
#define CH3  (1U << 3)
#define CH4  (1U << 4)
#define CH5  (1U << 5)
#define CH6  (1U << 6)
#define CH7  (1U << 7)

uint8_t channel_to_mask(int channel);
void enable_channel(uint8_t *mask, uint8_t channel_mask);
void disable_channel(uint8_t *mask, uint8_t channel_mask);
bool is_channel_enabled(uint8_t mask, uint8_t channel_mask);
char *decode_channels(uint8_t mask, char *buffer, size_t bufsize);
void print_enabled_channels(uint8_t mask);

#endif
