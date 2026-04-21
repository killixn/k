// https://k.lre.epita.fr/assignments/syscalls.html

#include "syscall.h"

#include <k/kstd.h>

#include "syscall_wrapper.h"

#define SYSCALL_TEST 0
#define MAX_SYSCALLS 32

static syscall_t syscall_table[MAX_SYSCALLS];

// Register a handler to the table depending on the syscall identifier
static int register_syscall(int id, syscall_t handler)
{
    if (id < 0 || id >= MAX_SYSCALLS || handler == NULL)
    {
        return -1;
    }

    syscall_table[id] = handler;
    return 0;
}

void syscall_handler(struct regs *r)
{
    // recover return value
    u32 syscall_id = r->eax;
    int ret = syscall_table[syscall_id](r);

    // set to eax
    r->eax = ret;

    // printf("syscall %u called (arg1=%u), ret=%d\n", syscall_id, r->ebx, ret);
}

void init_syscall()
{
    // initialize the syscall dispatch table
    for (int i = 0; i < MAX_SYSCALLS; i++)
    {
        syscall_table[i] = sys_unknown;
    }

    // fill the syscall dispatch table with the developed syscalls
    register_syscall(SYSCALL_WRITE, sys_write);
    register_syscall(SYSCALL_SBRK, sys_sbrk);
    register_syscall(SYSCALL_GETKEY, sys_getkey);
    register_syscall(SYSCALL_GETTICK, sys_gettick);
    register_syscall(SYSCALL_OPEN, sys_open);
    register_syscall(SYSCALL_READ, sys_read);
    register_syscall(SYSCALL_SEEK, sys_seek);
    register_syscall(SYSCALL_CLOSE, sys_close);
    register_syscall(SYSCALL_SETVIDEO, sys_setvideo);
    register_syscall(SYSCALL_SWAP_FRONTBUFFER, sys_swap_frontbuffer);
    register_syscall(SYSCALL_PLAYSOUND, sys_playsound);
    register_syscall(SYSCALL_SETPALETTE, sys_setpalette);
    register_syscall(SYSCALL_GETMOUSE, sys_getmouse);
    register_syscall(SYSCALL_READKEY, sys_readkey);
}
