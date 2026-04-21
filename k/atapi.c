// https://k.lre.epita.fr/assignments/file_system.html

#include <k/atapi.h>
#include <k/types.h>

#include "io.h"
#include "stdio.h"

// Control register defines
#define CONTROL 0x206
#define ALTERNATE_STATUS 0

static u16 cur_bus = SECONDARY_REG;
static u16 cur_drive = ATA_PORT_MASTER;
u8 atapi_buffer[CD_BLOCK_SZ] = { 0 };

static void busy_wait(u8 reg)
{
    while (1)
    {
        u8 status = inb(ATA_REG_STATUS(reg));
        if (!(status & BSY))
            break;
        if (status & ERR || status & DF)
            return;
    }
}

// Wait for a packet to be requested
// from diapo 10/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
static void wait_packet_request(u16 bus)
{
    while (1)
    {
        u8 status = inb(ATA_REG_STATUS(bus));
        if (status & ERR || status & DF)
            return;
        if (!(status & BSY) && (status & DRQ))
            break;
    }
}

// Wait 400 ns -> approx. 4 * inb()
// from https://wiki.osdev.org/ATAPI#x86_Directions
static void ata_io_wait(u16 reg)
{
    inb(reg + CONTROL + ALTERNATE_STATUS);
    inb(reg + CONTROL + ALTERNATE_STATUS);
    inb(reg + CONTROL + ALTERNATE_STATUS);
    inb(reg + CONTROL + ALTERNATE_STATUS);
}

// Look for ATAPI signature
// from http://wiki.osdev.org/ATAPI#Detecting
static int is_atapi_drive(u16 bus)
{
    u8 sc = inb(ATA_REG_SECTOR_COUNT(bus));
    u8 lo = inb(ATA_REG_LBA_LO(bus));
    u8 mi = inb(ATA_REG_LBA_MI(bus));
    u8 hi = inb(ATA_REG_LBA_HI(bus));

    return (sc == ATAPI_SIG_SC && lo == ATAPI_SIG_LBA_LO
            && mi == ATAPI_SIG_LBA_MI && hi == ATAPI_SIG_LBA_HI);
}

// Discover ATAPI drive
// from diapo 8/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
void discover_atapi_drive()
{
    u16 buses[2] = { PRIMARY_REG, SECONDARY_REG };
    u8 drives[2] = { ATA_PORT_MASTER, ATA_PORT_SLAVE };

    // Send ‘Software Reset’ to the controller’s DCR
    // Send ‘Disable IRQ’ to the controller’s DCR
    // from diapo 7/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    outb(PRIMARY_DCR, SRST);
    outb(PRIMARY_DCR, INTERRUPT_DISABLE);
    ata_io_wait((u8)PRIMARY_REG);

    outb(SECONDARY_DCR, SRST);
    outb(SECONDARY_DCR, INTERRUPT_DISABLE);
    ata_io_wait((u8)PRIMARY_REG);

    for (u8 b = 0; b < 2; b++)
    {
        u16 bus = buses[b];

        // from diapo 8/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
        for (u8 d = 0; d < 2; d++)
        {
            u16 drive = drives[d];

            // Select current drive
            // from http://wiki.osdev.org/ATAPI#x86_Directions
            outb(ATA_REG_DRIVE(bus), drive);
            // Approx. 4 * inb()
            ata_io_wait(bus);
            // Look for ATAPI signature
            // If it matches: saves (reg, drive)
            if (is_atapi_drive(bus))
            {
                cur_bus = bus;
                cur_drive = drive;
                return;
            }
        }
    }
}

// Send a packet to the drive
// from diapo 9/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
void send_packet(struct SCSI_packet *pkt, u16 bus)
{
    // Ensure drive is selected
    outb(ATA_REG_DRIVE(bus), cur_drive);
    ata_io_wait(bus);

    // Wait while busy
    busy_wait(bus);

    // No overlap / no DMA
    outb(ATA_REG_FEATURES(bus), 0);
    // No queuing
    outb(ATA_REG_SECTOR_COUNT(bus), 0);
    // Byte count limit = CD_BLOCK_SZ
    outb(ATA_REG_LBA_MI(bus), CD_BLOCK_SZ & 0xFF);
    outb(ATA_REG_LBA_HI(bus), (CD_BLOCK_SZ >> 8) & 0xFF);
    // Issue PACKET command
    outb(ATA_REG_COMMAND(bus), PACKET);

    wait_packet_request(bus);

    // Write the 12-byte SCSI packet as 6 words to the Data Register
    // from diapo 10/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    u16 *data = (u16 *)pkt;
    for (int i = 0; i < PACKET_SZ / 2; i++)
    {
        printf("%.04X ", data[i]);
        outw(ATA_REG_DATA(bus), data[i]);
    }
    printf("\r\n");

    // Wait until the drive is no longer signalling DATA_TRANSMIT
    // from diapo 10/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    wait_packet_request(bus);
}

void *read_block(size_t lba)
{
    // Build a READ 12 SCSI packet
    // from diapo 9/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    struct SCSI_packet pkt = { 0 };
    pkt.op_code = READ_12;
    pkt.lba_hi = (lba >> 24) & 0xFF;
    pkt.lba_mihi = (lba >> 16) & 0xFF;
    pkt.lba_milo = (lba >> 8) & 0xFF;
    pkt.lba_lo = lba & 0xFF;
    // read 1 block
    pkt.transfer_length_lo = 1;

    send_packet(&pkt, cur_bus);

    // Read CD_BLOCK_SZ bytes word by word
    // from diapo 11/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    for (int i = 0; i < CD_BLOCK_SZ; i += 2)
    {
        u16 tmp = inw(ATA_REG_DATA(cur_bus));
        atapi_buffer[i] = tmp & 0xFF;
        atapi_buffer[i + 1] = (tmp >> 8) & 0xFF;
    }

    // Wait for PACKET_COMMAND_COMPLETE
    // from diapo 11/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    while ((inb(ATA_REG_SECTOR_COUNT(cur_bus)) & 0x3)
           != PACKET_COMMAND_COMPLETE)
        ;

    return atapi_buffer;
}
