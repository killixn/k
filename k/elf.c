#include "elf.h"

#include <string.h>

#include "fs.h"
#include "memory.h"
#include "multiboot.h"
#include "sbrk.h"
#include "stdio.h"

#define SEG_UDATA 0x23
#define SEG_UCODE 0x1B

#define USER_STACK_SIZE 0x4000
#define USER_HEAP_SIZE 0x800000

// Ensure ELF Validity
// from https://wiki.osdev.org/ELF_Tutorial#Checking_the_ELF_Header
static int elf_check_file(Elf32_Ehdr *hdr)
{
    if (!hdr)
        return 0;
    if (hdr->e_ident[EI_MAG0] != ELFMAG0)
    {
        printf("[i] ELF Header EI_MAG0 incorrect\r\n");
        return 0;
    }
    if (hdr->e_ident[EI_MAG1] != ELFMAG1)
    {
        printf("[i] ELF Header EI_MAG1 incorrect\r\n");
        return 0;
    }
    if (hdr->e_ident[EI_MAG2] != ELFMAG2)
    {
        printf("[i] ELF Header EI_MAG2 incorrect\r\n");
        return 0;
    }
    if (hdr->e_ident[EI_MAG3] != ELFMAG3)
    {
        printf("[i] ELF Header EI_MAG3 incorrect\r\n");
        return 0;
    }
    return 1;
}

// from https://wiki.osdev.org/ELF_Tutorial#Checking_the_ELF_Header
static int elf_check_supported(Elf32_Ehdr *hdr)
{
    if (!elf_check_file(hdr))
    {
        printf("[i] Invalid ELF File\r\n");
        return 0;
    }
    if (hdr->e_ident[EI_CLASS] != ELFCLASS32)
    {
        printf("[i] Unsupported ELF File Class\r\n");
        return 0;
    }
    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        printf("[i] Unsupported ELF File byte order\r\n");
        return 0;
    }
    if (hdr->e_machine != EM_386)
    {
        printf("[i] Unsupported ELF File target\r\n");
        return 0;
    }
    if (hdr->e_ident[EI_VERSION] != EV_CURRENT)
    {
        printf("[i] Unsupported ELF File version\r\n");
        return 0;
    }
    if (hdr->e_type != ET_REL && hdr->e_type != ET_EXEC)
    {
        printf("[i] Unsupported ELF File type\r\n");
        return 0;
    }
    return 1;
}

static u32 elf_load_file(void *file)
{
    Elf32_Ehdr *hdr = (Elf32_Ehdr *)file;
    if (!elf_check_supported(hdr))
    {
        printf("[i] ELF File cannot be loaded\r\n");
        return -1;
    }

    if (hdr->e_type == ET_EXEC)
    {
        // Get the program header table
        Elf32_Phdr *phdr = (Elf32_Phdr *)(file + hdr->e_phoff);

        // Iterate over each segment
        for (int i = 0; i < hdr->e_phnum; i++)
        {
            // Skip non-loadable segments
            if (phdr[i].p_type != PT_LOAD)
            {
                continue;
            }

            // Target virtual address and source data in the file
            u8 *dst = (u8 *)phdr[i].p_vaddr;
            u8 *src = (u8 *)file + phdr[i].p_offset;

            // Copy p_filesz bytes from the file into memory at p_vaddr
            memcpy(dst, src, phdr[i].p_filesz);

            // Zero out the remaining bytes
            // BSS region: p_memsz - p_filesz
            memset(dst + phdr[i].p_filesz, 0,
                   phdr[i].p_memsz - phdr[i].p_filesz);
        }

        // Return the entry point
        return hdr->e_entry;
    }
    return -1;
}

// Enter user mode with iret method
// from https://wiki.osdev.org/Getting_to_Ring_3#iret_method
static void jump_usermode(u32 entry, u32 user_esp)
{
    printf("[i] Jumping to usermode: entry=0x%x esp=0x%x\r\n", entry, user_esp);

    // set data segment registers to user data (GDT[4] | RPL3 = 0x23)
    __asm__ volatile("mov $0x23, %ax\n");
    __asm__ volatile("mov %ax, %ds\n");
    __asm__ volatile("mov %ax, %es\n");
    __asm__ volatile("mov %ax, %fs\n");
    __asm__ volatile("mov %ax, %gs\n");

    // Build iret frame
    __asm__ volatile("pushl $0x23\n"); // ss
    __asm__ volatile("push %0\n" ::"r"(user_esp)); // esp
    __asm__ volatile("pushfl\n");
    __asm__ volatile("pushl $0x1B\n"); // cs (GDT[3] | RPL3)
    __asm__ volatile("pushl %0\n" ::"r"(entry)); // eip
    __asm__ volatile("iret\n");
}

void init_elf(multiboot_info_t *info)
{
    // Allocate a heap
    u8 *user_heap = memory_reserve(USER_HEAP_SIZE);
    if (!user_heap)
    {
        printf("[i] Cannot allocate user heap\r\n");
        return;
    }

    u8 *user_heap_top = user_heap + USER_HEAP_SIZE;
    init_sbrk(user_heap, user_heap_top);
    printf("[i] User heap: %p -> %p\r\n", user_heap, user_heap_top);

    // Open the ROM selected
    int fd = open((char *)info->cmdline, O_RDONLY);
    if (fd < 0)
    {
        printf("[i] Cannot open %s\r\n", (char *)info->cmdline);
        return;
    }

    // Seek to end to get file size
    off_t size = seek(fd, 0, SEEK_END);
    seek(fd, 0, SEEK_SET);

    // Allocate a buffer for the whole file
    void *buf = memory_reserve(size);
    if (!buf)
    {
        printf("[i] Cannot allocate %d bytes for the file buf\r\n", size);
        close(fd);
        return;
    }

    // Read the entire ELF file into the buffer
    ssize_t bytes_read = read(fd, buf, size);
    close(fd);

    if (bytes_read < (ssize_t)size)
    {
        printf("[i] Short read (%d/%d bytes)\r\n", bytes_read, size);
        return;
    }

    // Load ELF segments
    u32 entry = elf_load_file(buf);
    if (!entry)
    {
        printf("[i] elf_load_file failed\r\n");
        return;
    }

    // Allocate a user-mode stack
    void *user_stack_base = memory_reserve(USER_STACK_SIZE);
    if (!user_stack_base)
    {
        printf("[i] Cannot allocate user stack\r\n");
        return;
    }
    u32 user_stack_top = (u32)user_stack_base + USER_STACK_SIZE;

    // Switch to ring 3
    jump_usermode(entry, user_stack_top);
}
