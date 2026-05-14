#ifndef SOUND_H
#define SOUND_H

#include <k/kstd.h>

// from https://wiki.osdev.org/PC_Speaker#Sample_Code
#define PIT_FREQUENCY 1193182
#define PC_SPEAKER_PORT 0x61

void sound_stop(void);
void sound_tick(void);
int playsound(struct melody *melody, int repeat);

#endif /* SOUND_H */
