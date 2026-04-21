// https://k.lre.epita.fr/assignments/events.html

#include "idt.h"

#include <k/kstd.h>

#include "keyboard.h"
#include "pic.h"
#include "stdio.h"
#include "syscall.h"
#include "timer.h"

static struct idt_entry_t idt[256];

// Init all idt_entry's to 0
static void init_idt()
{
    for (size_t i = 0; i < 256; i++)
    {
        idt[i] = (struct idt_entry_t){ .offset_1 = 0,
                                       .selector = 0,
                                       .zero = 0,
                                       .type_attributes = 0,
                                       .offset_2 = 0 };
    }
}

// Assign the IDT entries with the correct values
// from https://wiki.osdev.org/Interrupts_Tutorial#Assembling
void idt_set_descriptor(u8 vector, void *isr, u8 flags)
{
    struct idt_entry_t *descriptor = &idt[vector];

    descriptor->offset_1 = (u32)isr & 0xFFFF;
    descriptor->selector = 0x08;
    descriptor->zero = 0;
    descriptor->type_attributes = flags;
    descriptor->offset_2 = (u32)isr >> 16;
}

void irq_handler(struct regs *r)
{
    // Exceptions CPU
    // from https://k.lre.epita.fr/data/slides/kern-3.pdf
    switch (r->vector)
    {
    case 0:
        printf("[i] IRQ Handler: Divide by 0\r\n");
        break;
    case 1:
        printf("[i] IRQ Handler: Debug\r\n");
        break;
    case 2:
        printf("[i] IRQ Handler: NMI\r\n");
        break;
    case 3:
        printf("[i] IRQ Handler: Breakpoint\r\n");
        break;
    case 4:
        printf("[i] IRQ Handler: Overflow\r\n");
        break;
    case 5:
        printf("[i] IRQ Handler: Bound Range Exceeded\r\n");
        break;
    case 6:
        printf("[i] IRQ Handler: Invalid Opcode\r\n");
        break;
    case 7:
        printf("[i] IRQ Handler: Device Not Available\r\n");
        break;
    case 8:
        printf("[i] IRQ Handler: Double Fault\r\n");
        break;
    case 9:
        printf("[i] IRQ Handler: Coprocessor\r\n");
        break;
    case 10:
        printf("[i] IRQ Handler: Invalid TSS\r\n");
        break;
    case 11:
        printf("[i] IRQ Handler: Segment not present\r\n");
        break;
    case 12:
        printf("[i] IRQ Handler: Stack Segment Fault\r\n");
        break;
    case 13:
        printf("[i] IRQ Handler: General Protection\r\n");
        break;
    case 14:
        printf("[i] IRQ Handler: Page Fault\r\n");
        break;
    case 15:
        printf("[i] IRQ Handler: (Intel reserved)\r\n");
        break;
    case 16:
        printf("[i] IRQ Handler: x87 FPU (Math Fault)\r\n");
        break;
    case 17:
        printf("[i] IRQ Handler: Alignement Check\r\n");
        break;
    case 18:
        printf("[i] IRQ Handler: Machine Check\r\n");
        break;
    case 19:
        printf("[i] IRQ Handler: SMD FLoating Point Ex.\r\n");
        break;
    case 20:
        printf("[i] IRQ Handler: Virtualization Ex.\r\n");
        break;
    case 21:
        printf("[i] IRQ Handler: (Intel reserved)\r\n");
        break;

    // Typical wiring of the PIC (Master)
    // from https://k.lre.epita.fr/data/slides/kern-3.pdf
    case 32:
        pit_irq_handler();
        break;
    case 33:
        keyboard_irq_handler();
        break;
    case 34:
        printf("[i] IRQ Handler: Not assigned in PC/XT; cascaded to slave "
               "8256\r\n");
        break;
    case 35:
        printf("[i] IRQ Handler: UART (COM2 and COM4)\r\n");
        break;
    case 36:
        printf("[i] IRQ Handler: UART (COM1 and COM3)\r\n");
        break;
    case 37:
        printf("[i] IRQ Handler: Hard disk in PC/XT; Parallel port LPT2 in "
               "PC/AT\r\n");
        break;
    case 38:
        printf("[i] IRQ Handler: Floppy disk controller\r\n");
        break;

    // Typical wiring of the PIC (Slave)
    // from https://k.lre.epita.fr/data/slides/kern-3.pdf
    case 39:
        printf("[i] IRQ Handler: Parallel port LPT1\r\n");
        break;
    case 40:
        printf("[i] IRQ Handler: RTC\r\n");
        break;
    case 41:
        printf("[i] IRQ Handler: IRQ9\r\n");
        break;
    case 42:
        printf("[i] IRQ Handler: IRQ10\r\n");
        break;
    case 43:
        printf("[i] IRQ Handler: IRQ11\r\n");
        break;
    case 44:
        printf("[i] IRQ Handler: PS/2 mouse controller\r\n");
        break;
    case 45:
        printf("[i] IRQ Handler: Math coprocessor\r\n");
        break;
    case 46:
        printf("[i] IRQ Handler: Hard disk controller 1\r\n");
        break;
    case 47:
        printf("[i] IRQ Handler: Hard disk controller 2\r\n");
        break;
    case 128:
        // printf("[i] IRQ Handler: Syscall intercepted\r\n");
        syscall_handler(r);
        break;
    default:
        printf("[i] IRQ Handler: Unhandled interrupt: %d\r\n", r->vector);
        break;
    }

    // IRQ[0] = vector - 32
    if (r->vector >= 32 && r->vector <= 47)
    {
        pic_send_eoi(r->vector - 32);
    }
}

void load_idt()
{
    // Init idt_entries
    init_idt();

    // from https://k.lre.epita.fr/data/slides/kern-3.pdf
    struct idt_r idtr;
    idtr.base = (u32)idt; /* idt base address */
    idtr.limit = sizeof(idt) - 1; /* idt size - 1 */

    // PIC remap
    pic_remap(32, 40);

    // Set the ISR entries at last
    // from https://wiki.osdev.org/Interrupts_Tutorial#Assembling
    static void *isr_table[32] = {
        isr_0,  isr_1,  isr_2,  isr_3,  isr_4,  isr_5,  isr_6,  isr_7,
        isr_8,  isr_9,  isr_10, isr_11, isr_12, isr_13, isr_14, isr_15,
        isr_16, isr_17, isr_18, isr_19, isr_20, isr_21, isr_22, isr_23,
        isr_24, isr_25, isr_26, isr_27, isr_28, isr_29, isr_30, isr_31
    };

    for (u8 vector = 0; vector < 32; vector++)
    {
        idt_set_descriptor(vector, isr_table[vector], 0x8E);
    }

    // Set the IRQ entries at last
    static void *irq_table[16] = { irq_0,  irq_1,  irq_2,  irq_3,
                                   irq_4,  irq_5,  irq_6,  irq_7,
                                   irq_8,  irq_9,  irq_10, irq_11,
                                   irq_12, irq_13, irq_14, irq_15 };

    for (u8 i = 0; i < 16; i++)
    {
        idt_set_descriptor(32 + i, irq_table[i], 0x8E);
    }

    idt_set_descriptor(128, isr_128, 0xEE);

    // Load the new IDT
    // from https://k.lre.epita.fr/data/slides/kern-3.pdf
    __asm__ volatile("lidt %0\n"
                     : /* no output */
                     : "m"(idtr)
                     : "memory");

    // Set interrupt flag
    __asm__ volatile("sti");
}
