// https://k.lre.epita.fr/assignments/memory_manager.html

#include "gdt.h"

#include <k/kstd.h>

#include "memory.h"
#include "stdio.h"

static struct gdt_desc gdt[6];

void gdt_prettyprint()
{
    for (size_t i = 0; i < sizeof(gdt) / sizeof(gdt[0]); i++)
    {
        printf("GDT %lu:\r\n", i);
        printf("\tlimit:\t0x%.08x\r\n", gdt[i].limit_15_00);
        printf("\tbase_1:\t0x%.08x\r\n", gdt[i].base_15_00);
        printf("\tbase_2:\t0x%.08x\r\n", gdt[i].base_23_16);
        printf("\taccess:\t0x%.02x\r\n", gdt[i].access);
        printf("\tflags:\t0x%.02x\r\n", gdt[i].flags_and_limit_19_16);
        printf("\tbase_3:\t0x%.08x\r\n", gdt[i].base_31_24);
    }
}

// from https://wiki.osdev.org/GDT_Tutorial#Filling_the_Table
static void init_gdt_entry(struct gdt_desc *target, u32 base, u32 limit,
                           u8 access, u8 flags)
{
    target->base_15_00 = (base) & 0xFFFF; // first 16 bits
    target->base_23_16 = (base >> 16) & 0xFF; // next 8 bits
    target->base_31_24 = (base >> 24) & 0xFF; // last 8 bits

    target->access = access;

    target->limit_15_00 = (limit) & 0xFFFF; // first 16 bits
    // (first 4 bits of the flags | first 4 bits of the last 8 bits of limit)
    target->flags_and_limit_19_16 = (flags & 0xF0) | ((limit >> 16) & 0x0F);
}

void load_gdt()
{
    /* Define all segments */

    // for args infos:
    // https://wiki.osdev.org/GDT_Tutorial#Flat_/_Long_Mode_Setup null segment
    init_gdt_entry(&gdt[0], 0, 0x00000, 0x00, 0x00);
    // kernel code segment
    init_gdt_entry(&gdt[1], 0, 0xFFFFF, 0x9A, 0xCF);
    // kernel data segment
    init_gdt_entry(&gdt[2], 0, 0xFFFFF, 0x92, 0xCF);
    // userland code segment
    init_gdt_entry(&gdt[3], 0, 0xFFFFF, 0xFA, 0xCF);
    // userland data segment
    init_gdt_entry(&gdt[4], 0, 0xFFFFF, 0xF2, 0xCF);
    // TODO : TSS (Task State Segment)
    // read https://wiki.osdev.org/Getting_to_Ring_3
    init_gdt_entry(&gdt[5], 0, 0x00000, 0x00, 0x00);

    // Disable interrupt
    __asm__ volatile("cli");

    // from https://k.lre.epita.fr/data/slides/kern-2.pdf
    struct gdt_r gdtr;
    gdtr.base = (u32)gdt; /* gdt base address */
    gdtr.limit = sizeof(gdt) - 1; /* gdt size - 1 */
    __asm__ volatile("lgdt %0\n"
                     : /* no output */
                     : "m"(gdtr)
                     : "memory");

    /* Switching to Protected Mode */

    // init cr0 to switch from 16 to 32 bits (not necessary here, already in 32)
    // from https://wiki.osdev.org/Protected_Mode#Entering_Protected_Mode
    /*
    __asm__ volatile("movl %eax, %cr0\n"
                     "or al, 1\n"
                     "movl %cr0, %eax\n");
    */

    // Reload %ds, %es %fs, %gs, %ss
    // from https://wiki.osdev.org/GDT_Tutorial#Protected_Mode,_Flat_Model
    __asm__ volatile("movw $0x10, %ax\n" // 0x10 -> stand-in for data segment
                     "movw %ax, %ds\n" // data segment
                     "movw %ax, %es\n" // extra segment unused
                     "movw %ax, %fs\n" // extra segment unused
                     "movw %ax, %gs\n" // extra segment unused
                     "movw %ax, %ss\n" // extra segment unused
    );

    // Init %cs
    // from https://wiki.osdev.org/GDT_Tutorial#Protected_Mode,_Flat_Model
    __asm__ volatile("ljmp $0x08, $1f\n" // 0x08 -> stand-in for code segment
                     "1:\n");

    // Enable interrupt
    __asm__ volatile("sti");
}

// TODO : load_tss()
// read https://wiki.osdev.org/Getting_to_Ring_3
void load_tss()
{}

// Init a Stack
void init_stack()
{
    void *stack = memory_reserve(0x4000);
    u32 stack_top = (u32)stack + 0x4000;
    __asm__ volatile("mov %0, %%esp\n"
                     "mov %0, %%ebp\n"
                     : /* no output */
                     : "r"(stack_top));
}
