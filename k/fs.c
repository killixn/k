#include "fs.h"

#include <k/atapi.h>
#include <k/iso9660.h>
#include <k/kstd.h>

#include "stdio.h"
#include "string.h"

#define MAX_FD 16

static fd_entry_t fd_table[MAX_FD];

static u8 cache_buf[ISO_BLOCK_SIZE];
static u32 cache_lba = (u32)-1;

static char *fs_strchr(const char *s, int c)
{
    for (; *s; s++)
    {
        if ((unsigned char)*s == (unsigned char)c)
        {
            return (char *)s;
        }
    }
    return NULL;
}

static const u8 *get_block(u32 lba)
{
    if (lba != cache_lba)
    {
        // atapi driver
        const u8 *p = (const u8 *)read_block(lba);
        memcpy(cache_buf, p, ISO_BLOCK_SIZE);
        cache_lba = lba;
    }

    return cache_buf;
}

static int alloc_fd(void)
{
    // 0,1,2 reserved for stdin/stdout/stderr
    for (int i = 3; i < MAX_FD; i++)
    {
        if (!fd_table[i].used)
        {
            return i;
        }
    }

    return -1;
}

// Compare an ISO directory identifier against a plain C string
static int iso_namecmp(const char *idf, u8 idf_len, const char *name)
{
    // strip ";1" version suffix if present
    u8 len = idf_len;
    if (len >= 2 && idf[len - 2] == ';')
        len -= 2;

    // strip trailing dot (empty extension)
    if (len > 0 && idf[len - 1] == '.')
        len--;

    if (strlen(name) != len)
        return 1;

    return strncasecmp(idf, name, len);
}

// Walk all directory records inside a directory whose first data block is at
// dir_lba and whose total size is dir_size bytes
static int find_in_dir(u32 dir_lba, u32 dir_size, const char *name,
                       struct iso_dir *out)
{
    u32 bytes_left = dir_size;
    u32 lba = dir_lba;

    while (bytes_left > 0)
    {
        const u8 *block = get_block(lba);
        u32 off = 0;

        while (off < ISO_BLOCK_SIZE && off < bytes_left)
        {
            struct iso_dir *d = (struct iso_dir *)(block + off);

            // rest is padding
            if (d->dir_size == 0)
            {
                break;
            }

            // skip "." and ".."
            if (d->idf_len == 1 && ((u8)d->idf[0] <= 1))
            {
                off += d->dir_size;
                continue;
            }

            if (iso_namecmp(d->idf, d->idf_len, name) == 0)
            {
                memcpy(out, d, sizeof(struct iso_dir));
                return 1;
            }

            off += d->dir_size;
        }

        bytes_left -=
            (bytes_left > ISO_BLOCK_SIZE ? ISO_BLOCK_SIZE : bytes_left);
        lba++;
    }
    return 0;
}

// Resolve a full absolute path starting from the root directory described in
// the primary volume descriptor
static int resolve_path(const char *pathname, u32 *out_lba, u32 *out_size)
{
    // read Primary Volume Descriptor (block 16)
    const u8 *pvd_block = get_block(ISO_PRIM_VOLDESC_BLOCK);
    const struct iso_prim_voldesc *pvd =
        (const struct iso_prim_voldesc *)pvd_block;

    if (pvd->vol_desc_type != 1)
    {
        puts("[!] fs: not a valid ISO9660 PVD\r\n");
        return -1;
    }

    // start walking from the root directory
    u32 dir_lba = pvd->root_dir.data_blk.le;
    u32 dir_size = pvd->root_dir.file_size.le;

    // need a local copy of the PVD's root dir entry
    struct iso_dir root_copy;
    memcpy(&root_copy, &pvd->root_dir, sizeof(root_copy));
    dir_lba = root_copy.data_blk.le;
    dir_size = root_copy.file_size.le;

    // tokenise the path on "/"
    char path_buf[256];
    strncpy(path_buf, pathname, sizeof(path_buf) - 1);
    path_buf[sizeof(path_buf) - 1] = '\0';

    char *p = path_buf;

    // skip leading "/"
    while (*p == '/')
    {
        p++;
    }

    while (*p)
    {
        // extract next component
        char *slash = fs_strchr(p, '/');
        if (slash)
        {
            *slash = '\0';
        }

        // find this component in the current directory
        struct iso_dir entry_copy;
        if (!find_in_dir(dir_lba, dir_size, p, &entry_copy))
        {
            printf("[!] fs: component '%s' not found\r\n", p);
            return -1;
        }

        dir_lba = entry_copy.data_blk.le;
        dir_size = entry_copy.file_size.le;

        if (!slash)
        {
            // last component, the file we want
            *out_lba = dir_lba;
            *out_size = dir_size;
            return 0;
        }

        // intermediate component must be a directory
        if (!(entry_copy.type & ISO_FILE_ISDIR))
        {
            puts("[!] fs: path component is not a directory\r\n");
            return -1;
        }

        p = slash + 1;
        while (*p == '/')
        {
            p++;
        }
    }

    // pathname was e.g. "/"
    // return root dir itself
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
    if (!pathname)
    {
        return -1;
    }

    if (flags != O_RDONLY)
    {
        puts("[!] fs: open() only supports O_RDONLY\r\n");
        return -1;
    }

    // Parse ISO 9660 to locate the file
    // from 17/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    u32 lba;
    u32 size;
    if (resolve_path(pathname, &lba, &size) != 0)
    {
        printf("[i] File \"%s\" not found\r\n", pathname);
        return -1;
    }

    // Find a free slot in the descriptor table
    // from 17/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    int fd = alloc_fd();
    if (fd < 0)
    {
        puts("[!] fs: no free file descriptors\r\n");
        return -1;
    }

    fd_table[fd].used = 1;
    fd_table[fd].lba = lba;
    fd_table[fd].size = size;
    fd_table[fd].cursor = 0;

    printf("[i] fs: open('%s') -> fd %d  (lba=%u size=%u)\r\n", pathname, fd,
           lba, size);
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
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].used || !buf)
    {
        return -1;
    }

    fd_entry_t *f = &fd_table[fd];

    // Clamp to what's left in the file
    if (f->cursor >= f->size)
    {
        return 0;
    }

    size_t available = f->size - f->cursor;
    if (count > available)
    {
        count = available;
    }

    size_t remaining = count;
    u8 *dst = (u8 *)buf;

    while (remaining > 0)
    {
        u32 lba = f->lba + (f->cursor / ISO_BLOCK_SIZE);
        u32 block_off = f->cursor % ISO_BLOCK_SIZE;
        u32 chunk = ISO_BLOCK_SIZE - block_off;
        if (chunk > remaining)
        {
            chunk = remaining;
        }

        const u8 *block = get_block(lba);
        memcpy(dst, block + block_off, chunk);

        dst += chunk;
        f->cursor += chunk;
        remaining -= chunk;
    }

    return (ssize_t)count;
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
        return (off_t)-1;
    }

    fd_entry_t *f = &fd_table[fd];
    off_t new_cursor;

    // Modify the file offset according to whence
    // from https://k.lre.epita.fr/assignments/file_system.html
    // from 18/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    switch (whence)
    {
    // Cursor is set to offset bytes
    case SEEK_SET:
        new_cursor = offset;
        break;
    // Cursor is set to its current location plus offset bytes
    case SEEK_CUR:
        new_cursor = (off_t)f->cursor + offset;
        break;
    // Cursor is set to the size of the file plus offset bytes
    case SEEK_END:
        new_cursor = (off_t)f->size + offset;
        break;
    default:
        // Unknown whence
        return (off_t)-1;
    }

    // Ensure the new offset stays within valid bounds
    if (new_cursor < 0)
    {
        return (off_t)-1;
    }

    f->cursor = (u32)new_cursor;
    return new_cursor;
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
    // from 17/19 https://k.lre.epita.fr/data/slides/kern-4.pdf
    fd_table[fd].used = 0;

    return 0;
}
