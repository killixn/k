#include <k/kstd.h>

#include "fs.h"
#include "idt.h"
#include "keyboard.h"
#include "sbrk.h"
#include "serial.h"
#include "stdio.h"
#include "timer.h"
#include "vga.h"

int sys_unknown(struct regs *r)
{
    printf("unknown syscall: %u\r\n", r->eax);
    return -1;
}

int sys_test(struct regs *r)
{
    printf("syscall %u called (arg1=%u)\n", r->eax, r->ebx);
    return 0;
}

int sys_write(struct regs *r)
{
    return write((char *)r->ebx, (size_t)r->ecx);
}

int sys_sbrk(struct regs *r)
{
    return (int)sbrk((ssize_t)r->ebx);
}

/* Legacy system call, do not implement */
int sys_getkey(struct regs *r)
{
    (void)r;
    return 0;
}

int sys_gettick(struct regs *r)
{
    (void)r;
    return gettick();
}

int sys_open(struct regs *r)
{
    return open((char *)r->ebx, (int)r->ecx);
}

int sys_read(struct regs *r)
{
    return read((int)r->ebx, (void *)r->ecx, (size_t)r->edx);
}

int sys_seek(struct regs *r)
{
    return seek((int)r->ebx, (off_t)r->ecx, (int)r->edx);
}

int sys_close(struct regs *r)
{
    return close((int)r->ebx);
}

int sys_setvideo(struct regs *r)
{
    return setvideo((int)r->ebx);
}

int sys_swap_frontbuffer(struct regs *r)
{
    swap_frontbuffer((void *)r->ebx);
    return 0;
}

int sys_playsound(struct regs *r)
{
    (void)r;
    return 0;
}

int sys_setpalette(struct regs *r)
{
    (void)r;
    return 0;
}

int sys_getmouse(struct regs *r)
{
    (void)r;
    return 0;
}

int sys_readkey(struct regs *r)
{
    struct key_event event = { .state = (int)r->ebx, .key = (int)r->ecx };

    return readkey(&event);
}
