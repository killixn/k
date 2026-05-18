#include <kstd.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE in = -2;
FILE out = -3;
FILE err = -4;

FILE *stdin = &in;
FILE *stdout = &out;
FILE *stderr = &err;


FILE *fopen(const char *pathname, const char *mode)
{
	if (mode[0] != 'r' || mode[1] != '\0')
		return NULL;
	FILE *f = malloc(sizeof (FILE));
	if (!f)
		return NULL;
	if ((*f = open(pathname, O_RDONLY)) < 0) {
		free(f);
		return NULL;
	}
	return f;
}

int fprintf(FILE *stream, const char *format, ...)
{
	if (*stream >= -1)
		return -1;

	char buf[1024];
	va_list args;
	va_start(args, format);

	vsprintf(buf, format, args);
	int r = printf("%s\n", buf);

	va_end(args);
	return r;
}

int fseek(FILE *stream, long offset, int whence)
{
	if (*stream < 0)
		return -1;

	return lseek(*stream, offset, whence);
}

int fclose(FILE *stream)
{
	if (*stream < 0)
		return 0;

	int r = close(*stream);
	free(stream);
	return r;
}

int fscanf(FILE *stream, const char *format, ...)
{
	(void) stream;
	(void) format;

	return -1;
}

long ftell(FILE *stream)
{
	return lseek(*stream, 0, SEEK_CUR);
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if (*stream < 0)
		return 0;

	ssize_t r = read(*stream, ptr, size * nmemb);
	if (r < 0)
		return 0;
	return r / size;
}

int mkdir(const char *path, ...)
{
	(void) path;

	return -1;
}

int getchar(void)
{
	while (getkey() == -1)
		continue;
	return 0;
}

int feof(FILE *stream)
{
	struct stat s;
	fstat(*stream, &s);
	return lseek(*stream, 0, SEEK_CUR) == s.st_size;
}

int access(const char *path, int amode)
{
	(void) amode;
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	close(fd);
	return 0;
}

void exit(int status)
{
	(void) status;

	printf("exit(%d)\n", status);
	for(;;);
}

int atoi(const char *nptr)
{
	int ret = 0;
	while (*nptr >= '0' && *nptr <= '9') {
		ret *= 10;
		ret += *nptr++ - '0';
	}
	return ret;
}

int fstat(int fildes, struct stat *buf)
{
	off_t old = lseek(fildes, 0, SEEK_CUR);
	buf->st_size = lseek(fildes, 0, SEEK_END);
	lseek(fildes, old, SEEK_SET);
	return buf->st_size > 0;
}

void setbuf(FILE *stream, char *buf)
{
	(void) stream;
	(void) buf;
}

int sscanf(const char *str, const char *format, ...)
{
	(void) str;
	(void) format;

	return -1;
}

char *getenv(const char *name)
{
	if (!strcmp(name, "HOME"))
		return "/";

	if (!strcmp(name, "DOOMWADDIR"))
		return RES_PATH "/roms";

	return NULL;
}
