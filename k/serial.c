// https://k.lre.epita.fr/assignments/serial_driver.html

#include "serial.h"

#include "io.h"

#define PORT_COM1 0x3f8 // COM1
#define PORT_COM2 0x2f8 // COM2
#define PORT_COM3 0x3e8 // COM3
#define PORT_COM4 0x2e8 // COM4
#define PORT_COM5 0x5f8 // COM5
#define PORT_COM6 0x4f8 // COM6
#define PORT_COM7 0x5e8 // COM7
#define PORT_COM8 0x4e8 // COM8

// Global var to check if serial already initialized
static int serial_init = 0;

// from https://wiki.osdev.org/Serial_Ports#Example_Code
static int init_serial()
{
    outb(PORT_COM1 + 1, 0x00); // Disable all interrupts
    outb(PORT_COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT_COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT_COM1 + 1, 0x00); //                  (hi byte)
    outb(PORT_COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT_COM1 + 2,
         0xC7); // Enable FIFO, clear them, with 14-byte threshold

    /* OPTIONAL */
    outb(PORT_COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(PORT_COM1 + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT_COM1 + 0, 0xAE); // Test serial chip (send byte 0xAE and check if
                               // serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT_COM1 + 0) != 0xAE)
    {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT_COM1 + 4, 0x0F);

    serial_init = 1;

    return 0;
}

// from https://wiki.osdev.org/Serial_Ports#Receiving_data
// Not necessary yet, but in case..
int serial_received()
{
    return inb(PORT_COM1 + 5) & 1;
}

char read_serial()
{
    while (serial_received() == 0)
        ;

    return inb(PORT_COM1);
}

// from https://wiki.osdev.org/Serial_Ports#Sending_data
int is_transmit_empty()
{
    return inb(PORT_COM1 + 5) & 0x20;
}

void write_serial(char c)
{
    while (is_transmit_empty() == 0)
        ;

    outb(PORT_COM1, c);
}

// write sys_call
int write(const char *buf, size_t count)
{
    // init serial if not already initialized
    if (!serial_init)
    {
        if (init_serial() != 0)
        {
            return -1;
        }
    }

    // write logic
    size_t i = 0;
    while (i < count)
    {
        write_serial(buf[i++]);
    }
    return i;
}
