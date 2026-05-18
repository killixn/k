#ifndef COMPAT_H
#define COMPAT_H

#include <stddef.h>
#include <ctype.h>

typedef int FILE;

FILE *fopen(const char *pathname, const char *mode);
int fprintf(FILE *stream, const char *format, ...);
int fseek(FILE *stream, long offset, int whence);
int fclose(FILE *stream);
int fscanf(FILE *stream, const char *format, ...);

int sscanf(const char *str, const char *format, ...);
void setbuf(FILE *stream, char *buf);

int mkdir(const char *path, ...);
int getchar(void);
int feof(FILE *stream);

int access(const char *path, int amode);

char *getenv(const char *name);

struct stat
{
	int st_size;
};

int fstat(int fildes, struct stat *buf);
void exit(int status);
int atoi(const char *nptr);

extern FILE *stdin;
extern FILE *stderr;
extern FILE *stdout;

#define R_OK		0
#define O_BINARY	0
#define O_WRONLY	1
#define O_CREAT		2
#define O_TRUNC		3
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#define O_RDONLY	0

#define alloca __builtin_alloca

#endif
