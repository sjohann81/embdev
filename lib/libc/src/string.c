#include <string.h>
#include <stdlib.h>

void *memcpy(void *dest, const void *src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	while (n--)
		*d++ = *s++;
	
	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	if (d == s || n == 0)
		return dest;

	if (d < s) {
		while (n--) *d++ = *s++;
	} else {
		d += n;
		s += n;
		while (n--)
			*--d = *--s;
	}
	
	return dest;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char *p = s;

	while (n--)
		*p++ = (unsigned char)c;
		
	return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *a = s1, *b = s2;

	while (n--) {
		if (*a != *b)
			return *a - *b;
		a++; b++;
	}

	return 0;
}

void *memchr(const void *s, int c, size_t n)
{
	const unsigned char *p = s;

	while (n--) {
		if (*p == (unsigned char)c)
			return (void *)p;
		p++;
	}
	
	return NULL;
}


size_t strlen(const char *s)
{
	const char *p = s;

	while (*p)
		p++;
		
	return (size_t)(p - s);
}

size_t strnlen(const char *s, size_t maxlen)
{
	size_t len = 0;

	while (len < maxlen && s[len])
		len++;

	return len;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++; s2++;
	}
		
	return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	while (n && *s1 && (*s1 == *s2)) {
		s1++; s2++; n--;
	}
	if (n == 0)
		return 0;
		
	return (unsigned char)*s1 - (unsigned char)*s2;
}

char *strcpy(char *dest, const char *src)
{
	char *d = dest;

	while ((*d++ = *src++));

	return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *d = dest;

	while (n && (*d++ = *src++))
		n--;
	while (n--)
		*d++ = '\0';

	return dest;
}

char *strcat(char *dest, const char *src)
{
	char *d = dest + strlen(dest);

	while ((*d++ = *src++));

	return dest;
}

char *strncat(char *dest, const char *src, size_t n)
{
	char *d = dest + strlen(dest);

	while (n && (*d++ = *src++))
		n--;
	*d = '\0';

	return dest;
}

char *strchr(const char *s, int c)
{
	do {
		if (*s == (char)c)
			return (char *)s;
	} while (*s++);
	
	return NULL;
}

char *strrchr(const char *s, int c)
{
	const char *last = NULL;
	
	do {
		if (*s == (char)c)
			last = s;
	} while (*s++);
	
	return (char *)last;
}

char *strstr(const char *haystack, const char *needle)
{
	if (!*needle)
		return (char *)haystack;
		
	size_t nlen = strlen(needle);
	
	while (*haystack) {
		if (*haystack == *needle && memcmp(haystack, needle, nlen) == 0)
			return (char *)haystack;
		haystack++;
	}
	
	return NULL;
}


#ifdef STRING_EXTRA

char *strtok_r(char *str, const char *delim, char **saveptr)
{
	if (str == NULL)
		str = *saveptr;

	if (str == NULL)
		return NULL;

	str += strspn(str, delim);
	if (*str == '\0') {
		*saveptr = NULL;
		return NULL;
	}

	char *end = str + strcspn(str, delim);
	if (*end == '\0') {
		*saveptr = NULL;
		return str;
	}

	*end = '\0';
	*saveptr = end + 1;
	return str;
}

char *strsep(char **stringp, const char *delim)
{
	char *start = *stringp;
	
	if (start == NULL)
		return NULL;

	char *end = strpbrk(start, delim);
	if (end) {
		*end = '\0';
		*stringp = end + 1;
	} else {
		*stringp = NULL;
	}

	return start;
}

static int to_lower(int c)
{
	return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

int strcasecmp(const char *s1, const char *s2)
{
	while (*s1 && (to_lower((unsigned char)*s1) == to_lower((unsigned char)*s2))) {
		s1++;
		s2++;
	}

	return to_lower((unsigned char)*s1) - to_lower((unsigned char)*s2);
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	while (n && *s1 && (to_lower((unsigned char)*s1) == to_lower((unsigned char)*s2))) {
		s1++;
		s2++;
		n--;
	}
	if (n == 0)
		return 0;
		
	return to_lower((unsigned char)*s1) - to_lower((unsigned char)*s2);
}

size_t strspn(const char *s, const char *accept)
{
	const char *p = s;
	while (*p) {
		const char *a = accept;
		while (*a && *a != *p)
			a++;
		if (*a == '\0') break;
		p++;
	}
	
	return p - s;
}

size_t strcspn(const char *s, const char *reject)
{
	const char *p = s;

	while (*p) {
		const char *r = reject;
		while (*r && *r != *p)
			r++;
		if (*r != '\0') break;
		p++;
	}
	
	return p - s;
}

char *strpbrk(const char *s, const char *accept)
{
	while (*s) {
		const char *a = accept;
		while (*a) {
			if (*s == *a)
				return (char *)s;
			a++;
		}
		s++;
	}
	
	return NULL;
}

char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	char *dup = malloc(len);

	if (dup)
		memcpy(dup, s, len);
		
	return dup;
}

char *strndup(const char *s, size_t n)
{
	size_t len = strnlen(s, n);
	char *dup = malloc(len + 1);

	if (dup) {
		memcpy(dup, s, len);
		dup[len] = '\0';
	}
	
	return dup;
}

static int is_space_char(char c)
{
	return c == ' ' || c == '\t' || c == '\n' ||
		c == '\r' || c == '\f' || c == '\v';
}

char *strtrim(char *s)
{
	if (!s) return NULL;

	while (*s && is_space_char(*s))
		s++;

	if (*s == '\0')
		return s;

	char *end = s + strlen(s) - 1;
	while (end > s && is_space_char(*end))
		end--;
	end[1] = '\0';

	return s;
}

#endif
