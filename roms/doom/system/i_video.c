#include "../i_video.h"

#include <graphic.h>
#include <stdint.h>

#include "../d_event.h"
#include "../d_main.h"
#include "../doomdef.h"

extern byte *screens[5];
static uint8_t framebuffer[FB_SIZE];

void I_InitGraphics(void)
{
    switch_graphic();
    screens[0] = framebuffer;
}

void I_ShutdownGraphics(void)
{
    switch_text();
}

void I_SetPalette(byte *palette)
{
    static byte *prev = NULL;
    if (prev == palette)
        return;
    prev = palette;

    unsigned int colours[256];

    for (size_t i = 0; i < 256; ++i)
    {
        uint8_t r = palette[i * 3];
        uint8_t g = palette[i * 3 + 1];
        uint8_t b = palette[i * 3 + 2];

        colours[i] = (r << 16) | (g << 8) | b;
    }

    set_palette(colours, 256);
}

unsigned char kbdus[] = {
    [KEY_ESC] = KEY_ESCAPE,
    [KEY_1] = '1',
    [KEY_2] = '2',
    [KEY_3] = '3',
    [KEY_4] = '4',
    [KEY_5] = '5',
    [KEY_6] = '6',
    [KEY_7] = '7',
    [KEY_8] = '8',
    [KEY_9] = '9',
    [KEY_0] = '0',
    '-',
    '=',
    '\b',
    '\t',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\r',
    [KEY_CTRL] = KEY_RCTRL,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    [KEY_LSHIFT] = KEY_RSHIFT,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    [KEY_RSHIFT] = KEY_RSHIFT,
    '*',
    [KEY_ALT] = KEY_RALT,
    [KEY_SPACE] = ' ',
    [KEY_MAJLOCK] = '\0',
    [KEY_UP] = KEY_UPARROW,
    '\0',
    '-',
    [KEY_LEFT] = KEY_LEFTARROW,
    [KEY_RIGHT] = KEY_RIGHTARROW,
    '+',
    '\0',
    [KEY_DOWN] = KEY_DOWNARROW,
    '\0',
    [KEY_INSER] = '\0',
    '\0',
};

static void send_key(int evtype, int key)
{
    event_t ev;

    if ((unsigned)key > (sizeof(kbdus) / sizeof(*kbdus)) || kbdus[key] == 0)
        return;
    ev.data1 = kbdus[key];
    ev.type = evtype;

    D_PostEvent(&ev);
}

static void handle_keyboard(void)
{
    struct key_event ev;
    while (!readkey(&ev))
    {
        send_key(ev.state == KEY_PRESSED ? ev_keydown : ev_keyup, ev.key);
    }
}

void I_FinishUpdate(void)
{
    handle_keyboard();
    swap_frontbuffer(framebuffer);
}

void I_WaitVBL(int count)
{
    unsigned long msecs = count * (1000 / 70);
    unsigned long start = gettick();

    while (msecs > gettick() - start)
        continue;
}

void I_ReadScreen(byte *scr)
{
    memcpy(scr, framebuffer, sizeof(framebuffer));
}

void I_BeginRead(void)
{}
void I_EndRead(void)
{}
void I_UpdateNoBlit(void)
{}
