#ifndef GDT_H
#define GDT_H

#include <k/types.h>

// gdt_r struct
struct gdt_r
{
    u16 limit;
    u32 base;
} __attribute__((packed));

// gdt struct
struct gdt_desc
{
    u16 limit_15_00;
    u16 base_15_00;

    u8 base_23_16;
    u8 access;
    u8 flags_and_limit_19_16;
    u8 base_31_24;
} __attribute__((packed));

// tss_entry_struct struct
struct tss_entry_struct
{
    // The previous TSS
    // with hardware task switching these form a kind of backward linked list.
    u32 prev_tss;
    // The stack pointer to load when changing to kernel mode.
    u32 esp0;
    // The stack segment to load when changing to kernel mode.
    u32 ss0;
    // Everything below here is unused.
    // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldt;
    u16 trap;
    u16 iomap_base;
} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t;

void gdt_prettyprint();
void load_gdt(u32 stack_top);

#endif /* GDT_H */
