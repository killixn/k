#ifndef SERIAL_H
#define SERIAL_H

#include <k/types.h>

void write_serial(char c);
int write(const char *buf, size_t count);

#endif /* SERIAL_H */
