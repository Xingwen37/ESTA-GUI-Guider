#ifndef HELPER_H
#define HELPER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t channel_to_mask(int channel);
void enable_channel(uint8_t *mask, int channel);
void disable_channel(uint8_t *mask, int channel);
bool is_channel_enabled(uint8_t mask, int channel);
char *decode_channels(uint8_t mask, char *buffer, size_t bufsize);
void print_enabled_channels(uint8_t mask);

#endif
