#include "fs.h"

#include <k/atapi.h>
#include <k/iso9660.h>
#include <k/kstd.h>

#define MAX_FD 16

static file_descriptor_t fd_table[MAX_FD];

// Compare an ISO9660 filename (e.g. "FOO.TXT;1") with a plain name ("foo.txt")
// (ISO9660 names are uppercase and may have a ";1" version suffix)
// from http://wiki.osdev.org/ISO_9660#Locating_Data_on_the_CD
static int iso_name_match(const char *iso_name, u8 iso_len, const char *name)
{
    // Make a clean copy of the ISO name (without the ";1" version suffix)
    char clean[256];
    int len = 0;

    for (int i = 0; i < iso_len; i++)
    {
        if (iso_name[i] == ';')
        {
            break;
        }
        // Case-insensitive: lowercase the ISO name character
        clean[len++] = (iso_name[i] >= 'A' && iso_name[i] <= 'Z')
            ? iso_name[i] + 32
            : iso_name[i];
    }
    clean[len] = '\0';

    // Compare with the requested name (also lowercased)
    for (int i = 0; i <= len; i++)
    {
        char c = name[i];
        if (c >= 'A' && c <= 'Z')
        {
            c += 32;
        }
        if (clean[i] != c)
        {
            return 0;
        }
    }
    return 1;
}

// Walk one directory block looking for a given name
// from http://wiki.osdev.org/ISO_9660#Locating_Data_on_the_CD
static struct iso_dir *find_in_dir(u8 *block, size_t block_size,
                                   const char *name)
{
    size_t pos = 0;

    while (pos < block_size)
    {
        struct iso_dir *entry = (struct iso_dir *)(block + pos);

        // If end of sector reached
        if (entry->dir_size == 0)
        {
            break;
        }

        // Skip special entries "." and ".."
        if (entry->idf_len > 0 && entry->idf[0] != 0x00
            && entry->idf[0] != 0x01)
        {
            if (iso_name_match(entry->idf, entry->idf_len, name))
            {
                return entry;
            }
        }

        pos += entry->dir_size;
    }
    return NULL;
}

// Lookup a file by absolute path (e.g. "/boot/stage2.bin")
// from http://wiki.osdev.org/ISO_9660#Locating_Data_on_the_CD
static int iso9660_lookup(const char *pathname, size_t *out_lba,
                          size_t *out_size)
{
    // Read the Primary Volume Descriptor (always at LBA 16)
    u8 *pvd_block = read_block(ISO_PRIM_VOLDESC_BLOCK);
    struct iso_prim_voldesc *pvd = (struct iso_prim_voldesc *)pvd_block;

    // Start from the root directory entry embedded in the PVD
    size_t dir_lba = pvd->root_dir.data_blk.le;
    size_t dir_size = pvd->root_dir.file_size.le;

    // Walk each path component
    // e.g. "/boot/stage2.bin" -> ["boot", "stage2.bin"]
    const char *p = pathname;
    while (*p == '/')
    {
        // Skip leading slash
        p++;
    }

    while (*p)
    {
        // Extract the next path component into a local buffer
        char component[256];
        int clen = 0;
        while (*p && *p != '/')
        {
            component[clen++] = *p++;
        }
        component[clen] = '\0';
        if (*p == '/')
        {
            // Skip the separator
            p++;
        }

        // Load the current directory block + search for the component
        struct iso_dir *found = NULL;
        size_t bytes_scanned = 0;

        while (bytes_scanned < dir_size)
        {
            u8 *block = read_block(dir_lba + bytes_scanned / ISO_BLOCK_SIZE);
            found = find_in_dir(block, ISO_BLOCK_SIZE, component);
            if (found)
            {
                break;
            }
            bytes_scanned += ISO_BLOCK_SIZE;
        }

        if (!found)
        {
            // Component not found
            return -1;
        }

        // Move into the found entry (directory or final file)
        dir_lba = found->data_blk.le;
        dir_size = found->file_size.le;
    }

    // Full path consumed -> dir_lba/dir_size now point to the file
    *out_lba = dir_lba;
    *out_size = dir_size;
    return 0;
}

/*
NAME
    open - manipulate the filesystem
DESCRIPTION
    Given a pathname for a file, open() returns a file descriptor for use in
    subsequent system calls (read, seek, close). The parameter flags must be
    set to O RDONLY to open the file in read-only mode (the only one mode
    supported).
RETURN VALUE
    open() returns the new unique file descriptor, or -1 if an error
    occured.
NOTES
    There are no standard file descriptors like STDIN FILENO.
*/
int open(const char *pathname, int flags)
{
    if (!pathname || flags != O_RDONLY)
    {
        return -1;
    }

    // Find a free slot in the descriptor table
    // from 28/30 https://k.lre.epita.fr/data/slides/kern-4.pdf
    int fd = -1;
    for (int i = 0; i < MAX_FD; i++)
    {
        if (!fd_table[i].used)
        {
            // Register an fd in fd_table
            fd = i;
            break;
        }
    }
    // No free slots available
    if (fd == -1)
    {
        return -1;
    }

    // Parse ISO 9660 to locate the file
    // from 28/30 https://k.lre.epita.fr/data/slides/kern-4.pdf
    size_t lba;
    size_t size;
    if (iso9660_lookup(pathname, &lba, &size) < 0)
    {
        // File not found
        return -1;
    }

    // Fill in the descriptor
    fd_table[fd].used = 1;
    fd_table[fd].lba = lba;
    fd_table[fd].size = size;
    fd_table[fd].offset = 0;

    return fd;
}

/*
NAME
    read - manipulate the filesystem
DESCRIPTION
    read() attempts to read up to count bytes from file descriptor fd into
    the buffer starting at buf.
RETURN VALUE
    read() returns the number of bytes read, or -1 if an error occured.
*/
ssize_t read(int fd, void *buf, size_t count)
{
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].used)
    {
        return -1;
    }

    file_descriptor_t *f = &fd_table[fd];

    // Count to the remaining bytes in the file
    if ((size_t)f->offset >= f->size)
    {
        // EOF
        return 0;
    }
    // Adjust the offset if necessary
    if (f->offset + count > f->size)
    {
        count = f->size - f->offset;
    }

    size_t bytes_read = 0;
    u8 *dst = (u8 *)buf;

    while (bytes_read < count)
    {
        // Determine which block to read
        // Determine where to start
        size_t block_index = (f->offset + bytes_read) / CD_BLOCK_SZ;
        size_t block_offset = (f->offset + bytes_read) % CD_BLOCK_SZ;

        u8 *block = read_block(f->lba + block_index);
        if (!block)
            return bytes_read > 0 ? (ssize_t)bytes_read : -1;

        // Find how many available bytes remain in this block
        size_t available = CD_BLOCK_SZ - block_offset;
        size_t to_copy = count - bytes_read;
        if (to_copy > available)
        {
            to_copy = available;
        }

        for (size_t i = 0; i < to_copy; i++)
        {
            dst[bytes_read + i] = block[block_offset + i];
        }

        bytes_read += to_copy;
    }

    f->offset += bytes_read;
    return (ssize_t)bytes_read;
}

/*
NAME
    seek - manipulate the filesystem
DESCRIPTION
    seek() repositions the reading cursor of the open file associated with
    the file descriptor filedes to the argument offset.
RETURN VALUE
    seek() returns the new resulting offset location as measured in bytes
    from the beginning of the file. Otherwise, a value of (off t)-1 on
    error.
*/
off_t seek(int fd, off_t offset, int whence)
{
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].used)
    {
        return -1;
    }

    file_descriptor_t *f = &fd_table[fd];
    off_t new_offset;

    // Modify the file offset according to whence
    // from https://k.lre.epita.fr/assignments/file_system.html
    // from 29/30 https://k.lre.epita.fr/data/slides/kern-4.pdf
    switch (whence)
    {
    // Cursor is set to offset bytes
    case SEEK_SET:
        new_offset = offset;
        break;
    // Cursor is set to its current location plus offset bytes
    case SEEK_CUR:
        new_offset = f->offset + offset;
        break;
    // Cursor is set to the size of the file plus offset bytes
    case SEEK_END:
        new_offset = f->size + offset;
        break;
    default:
        // Unknown whence
        return -1;
    }

    // Ensure the new offset stays within valid bounds
    if (new_offset < 0 || (size_t)new_offset > f->size)
    {
        return -1;
    }

    f->offset = new_offset;
    return new_offset;
}

/*
NAME
    close - manipulate the filesystem
DESCRIPTION
    close() closes the file descriptor fd, so that it no longer refers to
    any file.
RETURN VALUE
    close() returns 0 on success, or -1 on error.
*/
int close(int fd)
{
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].used)
    {
        return -1;
    }

    // Free the fd
    // from 28/30 https://k.lre.epita.fr/data/slides/kern-4.pdf
    fd_table[fd].used = 0;
    fd_table[fd].lba = 0;
    fd_table[fd].size = 0;
    fd_table[fd].offset = 0;

    return 0;
}
