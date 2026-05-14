// https://k.lre.epita.fr/assignments/timer.html

#include "timer.h"

#include "io.h"
#include "sound.h"

volatile unsigned long tick;

// read page 31/37 from https://k.lre.epita.fr/data/slides/kern-3.pdf
// from https://k.lre.epita.fr/internals/8254_controller.html
void init_timer()
{
    // Init tick to 0
    tick = 0;

    // Write into control register
    int bin_counter = 0b0;
    int mode = 0b010;
    int io = 0b11;
    int counter = 0b00;
    u8 control = (counter << 6) | (io << 4) | (mode << 1) | bin_counter;
    // control_register on port 0x43;
    // read page 32/37 from https://k.lre.epita.fr/data/slides/kern-3.pdf
    outb(0x43, control);

    // Write the divider on the counter register
    int internal_frequency = 1193182;
    int desired_frequency = 100; // in Hz
    int divider = internal_frequency / desired_frequency;
    // counter 0 on port 0x40
    // read page 32/37 from https://k.lre.epita.fr/data/slides/kern-3.pdf
    // io = 0b11 ->  LSB first, then MSB
    outb(0x40, divider & 0xFF); // LSB
    outb(0x40, (divider >> 8) & 0xFF); // MSB
}

/*
DESCRIPTION
    gettick() returns the number of milliseconds elapsed since the machine
started
RETURN VALUE
    gettick() returns the time in milliseconds.
*/
// from https://k.lre.epita.fr/assignments/timer.html
unsigned long gettick(void)
{
    // 1 tick = 100 Hz = 10 tick / sec = 10ms
    // tick * 10 = 1ms
    return tick * 10;
}

void pit_irq_handler()
{
    tick++;
    sound_tick();
}
