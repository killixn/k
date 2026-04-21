#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <k/kstd.h>

static const char keymap_normal[] = {
    0,    0,   '1', '2',  '3',  '4',  '5', '6', '7',  '8', // 0-9
    '9',  '0', '-', '=',  '\b', '\t', 'q', 'w', 'e',  'r', // 10-19
    't',  'y', 'u', 'i',  'o',  'p',  '[', ']', '\n', 0, // 20-29
    'a',  's', 'd', 'f',  'g',  'h',  'j', 'k', 'l',  ';', // 30-39
    '\'', '`', 0,   '\\', 'z',  'x',  'c', 'v', 'b',  'n', // 40-49
    'm',  ',', '.', '/',  0,    '*',  0,   ' ' // 50-57
};

static const char keymap_shift[] = {
    0,   0,   '!', '@', '#',  '$',  '%', '^', '&',  '*', // 0-9
    '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E',  'R', // 10-19
    'T', 'Y', 'U', 'I', 'O',  'P',  '{', '}', '\n', 0, // 20-29
    'A', 'S', 'D', 'F', 'G',  'H',  'J', 'K', 'L',  ':', // 30-39
    '"', '~', 0,   '|', 'Z',  'X',  'C', 'V', 'B',  'N', // 40-49
    'M', '<', '>', '?', 0,    '*',  0,   ' ' // 50-57
};

#define FIFO_SIZE 64

struct key_fifo
{
    struct key_event buffer[FIFO_SIZE];
    volatile unsigned int head;
    volatile unsigned int tail;
};

void init_keyboard();
int readkey(struct key_event *event);
int getkey(void);
void keyboard_irq_handler();

#endif /* KEYBOARD_H */
