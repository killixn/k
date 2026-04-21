// https://k.lre.epita.fr/assignments/vga_driver.html

#include "vga.h"

#include "k/kstd.h"
#include "libvga.h"

/*
name
    setvideo - graphic mode management
synopsis
    int setvideo(int mode);
description
    setvideo() sets the current video mode to mode. available modes are:
        video text text mode (80x25 characters).
        video graphic graphic mode (320*200, 8bpp color).
return value
    setvideo() returns 0 on success, or -1 on error.
*/
int setvideo(int mode)
{
    if (mode == VIDEO_GRAPHIC)
    {
        libvga_switch_mode13h();
        return 0;
    }
    else if (mode == VIDEO_TEXT)
    {
        libvga_switch_mode3h();
        return 0;
    }

    return -1;
}

/*
name
    swap_frontbuffer - graphic mode management
synopsis
    void swap_frontbuffer(const void *buffer);
description
    swap_frontbuffer() swap the content of the front buffer with the
    data given by the user.
notes
    swap_frontbuffer() must be called in video mode.
*/
void swap_frontbuffer(const void *buffer)
{
    char *fb = libvga_get_framebuffer();

    // Must be in graphic mode (0xA0000)
    if (fb != (char *)0xA0000)
    {
        return;
    }

    const unsigned char *src = (const unsigned char *)buffer;

    for (int i = 0; i < 320 * 200; i++)
    {
        fb[i] = src[i];
    }
}
