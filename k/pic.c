#include "pic.h"

#include "io.h"

// Disable the PIC by masking every single interrupt
// from https://wiki.osdev.org/8259_PIC#Disabling
void pic_disable(void)
{
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

// from https://wiki.osdev.org/8259_PIC#Masking
void irq_set_mask(u8 irq_line)
{
    u16 port;
    u8 value;

    if (irq_line < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    value = inb(port) | (1 << irq_line);
    outb(port, value);
}

void irq_clear_mask(u8 irq_line)
{
    u16 port;
    u8 value;

    if (irq_line < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    value = inb(port) & ~(1 << irq_line);
    outb(port, value);
}

/*
arguments:
    offset1 - vector offset for master PIC
        vectors on the master become offset1..offset1+7
    offset2 - same for slave PIC: offset2..offset2+7
*/
// from https://wiki.osdev.org/8259_PIC#Initialisation
void pic_remap(int offset1, int offset2)
{
    // starts the initialization sequence (in cascade mode)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    // ICW2: Master PIC vector offset
    outb(PIC1_DATA, offset1);
    // ICW2: Slave PIC vector offset
    outb(PIC2_DATA, offset2);
    // ICW3: tell Master PIC that there is a slave PIC at IRQ2
    outb(PIC1_DATA, 1 << CASCADE_IRQ);
    // ICW3: tell Slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 2);

    // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Unmask both PICs
    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

// from https://wiki.osdev.org/8259_PIC#End_of_Interrupt
void pic_send_eoi(u8 irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}
