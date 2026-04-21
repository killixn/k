// https://k.lre.epita.fr/assignments/keyboard.html

#include "keyboard.h"

#include <k/kstd.h>

#include "io.h"
#include "keyboard.h"
#include "pic.h"

// Track modifier state
static int shift_pressed = 0;
static int caps_lock = 0;

struct key_fifo keyboard_fifo;

// Init FIFO
static void fifo_init(struct key_fifo *f)
{
    f->head = 0;
    f->tail = 0;
}

// Push a key in the FIFO
static int fifo_push(struct key_fifo *f, struct key_event ev)
{
    unsigned int next = (f->head + 1) % FIFO_SIZE;

    if (next == f->tail)
    {
        return -1;
    }

    f->buffer[f->head] = ev;
    f->head = next;

    return 0;
}

// Pop a key from the FIFO
static int fifo_pop(struct key_fifo *f, struct key_event *ev)
{
    if (f->head == f->tail)
    {
        return -1;
    }

    *ev = f->buffer[f->tail];
    f->tail = (f->tail + 1) % FIFO_SIZE;

    return 0;
}

void init_keyboard()
{
    fifo_init(&keyboard_fifo);
    // Update the PIC to unmask the IRQ line 1
    irq_clear_mask(1);
}

/*
DESCRIPTION
    readkey() returns the key events as a FIFO.
RETURN VALUE
    readkey() returns 0 in case of success.
ERRORS
    -EAGAIN no key event since last call.
*/
int readkey(struct key_event *event)
{
    if (fifo_pop(&keyboard_fifo, event) < 0)
    {
        return -EAGAIN;
    }

    return 0;
}

/*
DESCRIPTION
    getkey() blocks until a key press event is available and returns it.
RETURN VALUE
    getkey() returns the key code of the pressed key.
*/
int getkey(void)
{
    struct key_event event;

    // Block until a KEY_PRESSED event is available
    while (1)
    {
        if (readkey(&event) == 0 && event.state == KEY_PRESSED)
            return event.key;
    }
}

void keyboard_irq_handler()
{
    // Read scancodes from I/O port 0x60
    int scancode = inb(0x60);
    // Read status register from port 0x64
    // int status_register = inb(0x64);

    int is_release = (scancode & 0x80);
    int key = scancode & ~0x80; // strip the release bit

    // Track shift keys
    if (key == KEY_LSHIFT || key == KEY_RSHIFT)
    {
        shift_pressed = !is_release;
        return;
    }

    // Toggle caps lock on press only
    if (key == KEY_MAJLOCK && !is_release)
    {
        caps_lock = !caps_lock;
        return;
    }

    // Recover state
    // read page 26/37 https://k.lre.epita.fr/data/slides/kern-3.pdf
    int curr_state = (scancode & 0x80) ? KEY_RELEASED : KEY_PRESSED;

    // Translate scancode to ASCII
    char c = 0;
    int use_shift = shift_pressed ^ caps_lock;

    if (key < (int)sizeof(keymap_normal))
    {
        c = use_shift ? keymap_shift[key] : keymap_normal[key];
    }

    struct key_event event = { .state = curr_state, .key = c ? c : key };
    fifo_push(&keyboard_fifo, event);
}
