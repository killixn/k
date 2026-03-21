#ifndef IDT_H
#define IDT_H

#include <k/types.h>

// extern function from interrupt_stub.S
extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_15();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_20();
extern void isr_21();
extern void isr_22();
extern void isr_23();
extern void isr_24();
extern void isr_25();
extern void isr_26();
extern void isr_27();
extern void isr_28();
extern void isr_29();
extern void isr_30();
extern void isr_31();

extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_15();

// idt_entry struct
// from https://wiki.osdev.org/Interrupts_Tutorial#Entries
struct idt_entry_t
{
    u16 offset_1; // offset bits 0..15
    u16 selector; // a code segment selector in GDT or LDT
    u8 zero; // unused, set to 0
    u8 type_attributes; // gate type, dpl, and p fields
    u16 offset_2; // offset bits 16..31} __attribute__((packed));
} __attribute__((packed));

// idt_r struct
// from https://wiki.osdev.org/Interrupts_Tutorial#Table
struct idt_r
{
    u16 limit;
    u32 base;
} __attribute__((packed));

struct trap_frame
{
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 vector;
    u32 error;
    u32 eip;
    u32 cs;
    u32 eflags;
} __attribute__((packed));

void load_idt();

#endif /* IDT_H */
