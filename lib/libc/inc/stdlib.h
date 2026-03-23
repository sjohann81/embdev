#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H

#include <stddef.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

int atoi (const char *s);
long atol (const char *s);
long strtol (const char *s, char **endptr, int base);
unsigned long strtoul(const char *s, char **endptr, int base);
int abs (int n);
long labs(long n);
void *malloc(size_t size);
void free(void *ptr);

#ifdef STDLIB_EXTRA
void qsort(void *base, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));
void *bsearch(const void *key, const void *base,
	size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));
int rand(void);
void srand(unsigned int seed);
long random(void);
void srandom(unsigned int seed);
typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
#endif
#endif /* _LIBC_STDLIB_H */
