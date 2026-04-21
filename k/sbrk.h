#ifndef SBRK_H
#define SBRK_H

#include <k/kstd.h>

void init_sbrk(u8 *start, u8 *limit);
void *sbrk(ssize_t increment);

#endif /* SBRK_H */
