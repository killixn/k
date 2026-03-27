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

void gdt_prettyprint();
void init_stack();
void load_gdt();
void load_tss();

#endif /* GDT_H */
