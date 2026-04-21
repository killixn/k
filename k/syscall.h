#ifndef SYSCALL_H
#define SYSCALL_H

#include "idt.h"

typedef int (*syscall_t)(struct regs *r);

void init_syscall();
void syscall_handler(struct regs *r);

#endif /* SYSCALL_H */
