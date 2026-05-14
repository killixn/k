#ifndef VGA_H
#define VGA_H

#include "k/types.h"

int setvideo(int mode);
void swap_frontbuffer(const void *buffer);
void set_palette(u32 *palette, size_t size);

#endif /* VGA_H */
