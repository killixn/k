/*
 * Copyright (c) LSE
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY LSE AS IS AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL LSE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <k/atapi.h>
#include <k/kstd.h>
#include <k/types.h>

#include "elf.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "memory.h"
#include "multiboot.h"
#include "stdio.h"
#include "syscall.h"
#include "timer.h"

#define STACK_SIZE 0x4000

static void k_main_post_stack(multiboot_info_t *info, u32 stack_top)
{
    load_gdt(stack_top);
    printf("[i] gdt loaded\r\n");
    load_idt();
    printf("[i] idt loaded\r\n");
    init_keyboard();
    printf("[i] keyboard initialized\r\n");
    init_timer();
    printf("[i] timer initialized\r\n");
    init_syscall();
    printf("[i] syscall table initialized\r\n");
    discover_atapi_drive();
    printf("[i] atapi drive discovered\r\n");
    init_elf(info);
    printf("[i] elf initialized\r\n");

    /* UNREACHABLE */

    // System call calling for debug
    int res;
    __asm__ volatile("mov $0, %%eax\n"
                     "mov $0, %%ebx\n"
                     "int $0x80\n"
                     "mov %%eax, %0\n"
                     : "=r"(res)
                     :
                     : "eax", "ebx");
    printf("[d] eax=%d\r\n", res);

    // Frame Buffer
    char *fb = (void *)0xb8000;

    // Keys printer
    for (;;)
    {
        struct key_event event;

        // Keep looping if no event
        if (readkey(&event) != 0)
            continue;

        // Only handle key presses
        if (event.state != KEY_PRESSED)
            continue;

        // Write the scancode as a char
        *fb = (char)event.key;
        // VGA -> char + attribute byte interleaved
        fb += 2;
    }

    // Flipping star
    char star[5] = "|/-\\";
    for (unsigned i = 0;;)
        *fb = star[i++ % 4];

    for (;;)
        asm volatile("hlt");
}

// Kernel Entry Function
void k_main(unsigned long magic, multiboot_info_t *info)
{
    (void)magic;

    // Init memory
    memory_init(info);

    // Reserve a new stack
    // from https://k.lre.epita.fr/assignments/memory_manager.html
    void *stack = memory_reserve(STACK_SIZE);
    u32 stack_top = (u32)stack + STACK_SIZE;
    __asm__ volatile("mov %0, %%esp\n"
                     "mov %0, %%ebp\n"
                     : /* no output */
                     : "r"(stack_top));

    printf("[i] stack initialized\r\n");

    // Stack alignment
    k_main_post_stack(info, stack_top);
}
