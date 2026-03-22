#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <k/kstd.h>

// key_event struct (already defined in k/kstd.h)
// from https://k.lre.epita.fr/assignments/keyboard.html
/*
struct key_event
{
    int state; // KEY_PRESSED or KEY_RELEASED
    int key; // scan code
} __attribute__((packed));
*/

#define FIFO_SIZE 64

struct key_fifo
{
    struct key_event buffer[FIFO_SIZE];
    volatile unsigned int head;
    volatile unsigned int tail;
};

void init_keyboard();
int readkey(struct key_event *event);
void keyboard_irq_handler();

#endif /* KEYBOARD_H */
