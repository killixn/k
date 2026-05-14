// https://k.lre.epita.fr/assignments/sound_driver.html
#include "sound.h"

#include <k/kstd.h>

#include "io.h"
#include "timer.h"

// Current melody playback state
static struct melody *current_melody = NULL;
static int current_note = 0;
static int current_repeat = 0;
static unsigned long note_end_tick = 0;

// Check if a note is the end-of-melody sentinel
// from load_sound: sentinel is {freq=0, duration=(unsigned long)-1}
static int is_sentinel(struct melody *note)
{
    return note->freq == 0 && note->duration == (unsigned long)-1;
}

// Stop the PC speaker
// from https://wiki.osdev.org/PC_Speaker#Sample_Code
void sound_stop(void)
{
    u8 tmp = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, tmp & ~0x03);
}

// Play a single frequency on the PC speaker
static void sound_play_freq(unsigned long freq)
{
    if (freq == 0)
    {
        sound_stop();
        return;
    }

    // Configure PIT channel 2: square wave mode, binary
    // read page 32/37 from https://k.lre.epita.fr/data/slides/kern-3.pdf
    outb(0x43, 0xB6);

    // Set frequency divisor
    u32 divisor = PIT_FREQUENCY / freq;
    outb(0x42, divisor & 0xFF);
    outb(0x42, (divisor >> 8) & 0xFF);

    // Enable PC speaker via port 0x61 bits 0-1
    u8 tmp = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, tmp | 0x03);
}

// Called every tick by pit_irq_handler to advance the melody
void sound_tick(void)
{
    if (!current_melody)
        return;

    // Wait until current note duration has elapsed
    if (gettick() < note_end_tick)
        return;

    // Advance to next note
    current_note++;

    // Check for end-of-melody sentinel
    if (is_sentinel(&current_melody[current_note]))
    {
        if (current_repeat == -1)
            current_note = 0;
        else
        {
            sound_stop();
            current_melody = NULL;
            return;
        }
    }

    // Play next note
    sound_play_freq(current_melody[current_note].freq);
    note_end_tick = gettick() + current_melody[current_note].duration;
}

/*
NAME
    playsound - play a melody
SYNOPSIS
    int playsound(struct melody *melody, int repeat);
DESCRIPTION
    playsound() launches the given melody, and repeats it, if the repeat
    flag is set.
RETURN VALUE
    playsound() returns 0 on success or -1 otherwise.
*/
// from https://wiki.osdev.org/PC_Speaker#Sample_Code
int playsound(struct melody *melody, int repeat)
{
    if (!melody || is_sentinel(melody))
        return -1;

    current_melody = melody;
    current_note = 0;
    current_repeat = repeat;

    // Play the first note immediately
    sound_play_freq(melody[0].freq);
    note_end_tick = gettick() + melody[0].duration;

    return 0;
}
