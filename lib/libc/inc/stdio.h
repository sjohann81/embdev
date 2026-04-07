#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#include <stdarg.h>
#include <stddef.h>

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);
int putchar(int c);
int puts(const char *s);
char *gets(char *str);

#ifdef STDIO_EXTRA
#define EOF	(-1)
int vsscanf(const char *str, const char *fmt, va_list args);
int sscanf(const char *str, const char *fmt, ...);
int vscanf(const char *fmt, va_list args);
int scanf(const char *fmt, ...);
int getchar(void);
#endif

#endif /* _LIBC_STDIO_H */
