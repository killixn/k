#ifndef FS_H
#define FS_H

#include <k/kstd.h>

// Struct for an open file
typedef struct
{
    // 1 if this slot is occupied, 0 if free
    int used;
    // first logical block on the disc
    size_t lba;
    // file size (in bytes)
    size_t size;
    // current read position (in [0, size])
    off_t offset;
} file_descriptor_t;

int open(const char *pathname, int flags);
ssize_t read(int fd, void *buf, size_t count);
off_t seek(int fd, off_t offset, int whence);
int close(int fd);

#endif /* FS_H */
