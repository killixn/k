#ifndef SYSCALL_WRAPPER_H
#define SYSCALL_WRAPPER_H

#include "idt.h"

int sys_unknown(struct regs *r);
int sys_test(struct regs *r);

int sys_write(struct regs *r);
int sys_sbrk(struct regs *r);
int sys_getkey(struct regs *r);
int sys_gettick(struct regs *r);
int sys_open(struct regs *r);
int sys_read(struct regs *r);
int sys_seek(struct regs *r);
int sys_close(struct regs *r);
int sys_setvideo(struct regs *r);
int sys_swap_frontbuffer(struct regs *r);
int sys_playsound(struct regs *r);
int sys_setpalette(struct regs *r);
int sys_getmouse(struct regs *r);
int sys_readkey(struct regs *r);

#endif /* SYSCALL_WRAPPER_H */
