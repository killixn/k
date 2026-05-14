// https://k.lre.epita.fr/assignments/syscalls.html

#include <k/kstd.h>

#include "fs.h"
#include "idt.h"
#include "keyboard.h"
#include "sbrk.h"
#include "serial.h"
#include "sound.h"
#include "stdio.h"
#include "timer.h"
#include "vga.h"

// Fallback syscall
// from syscall.c
int sys_unknown(struct regs *r)
{
    printf("[i] Unknown syscall: %u\r\n", r->eax);
    return -1;
}

// from serial.c
int sys_write(struct regs *r)
{
    write_serial((char)r->ecx);
    return 0;
}

// from sbrk.c
int sys_sbrk(struct regs *r)
{
    return (int)sbrk((ssize_t)r->ebx);
}

/* Legacy system call, do not implement */
int sys_getkey(struct regs *r)
{
    (void)r;
    return getkey();
}

// from timer.c
int sys_gettick(struct regs *r)
{
    (void)r;
    return gettick();
}

// from fs.c
int sys_open(struct regs *r)
{
    return open((char *)r->ebx, (int)r->ecx);
}

// from fs.c
int sys_read(struct regs *r)
{
    return read((int)r->ebx, (void *)r->ecx, (size_t)r->edx);
}

// from fs.c
int sys_seek(struct regs *r)
{
    return seek((int)r->ebx, (off_t)r->ecx, (int)r->edx);
}

// from fs.c
int sys_close(struct regs *r)
{
    return close((int)r->ebx);
}

// from vga.c
int sys_setvideo(struct regs *r)
{
    return setvideo((int)r->ebx);
}

// from vga.c
int sys_swap_frontbuffer(struct regs *r)
{
    swap_frontbuffer((void *)r->ebx);
    return 0;
}

// from sound.c
int sys_playsound(struct regs *r)
{
    return playsound((struct melody *)r->ebx, (int)r->ecx);
}

// from vga.c
int sys_setpalette(struct regs *r)
{
    set_palette((u32 *)r->ebx, (size_t)r->ecx);
    return 0;
}

// TODO : implement mouse syscall
int sys_getmouse(struct regs *r)
{
    (void)r;
    return 0;
}

// from keyboard.c
int sys_readkey(struct regs *r)
{
    struct key_event *event = (struct key_event *)r->ebx;
    return readkey(event);
}
