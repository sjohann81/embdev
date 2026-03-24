#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <limits.h>

static void buf_putc(char *buf, size_t size, size_t *pos, char c)
{
	if (buf && *pos + 1 < size)
		buf[*pos] = c;
	(*pos)++;
}

static void buf_puts(char *buf, size_t size, size_t *pos,
	const char *s, int width, int left_align, char pad)
{
	int len = (int)strlen(s);
	int padding = (width > len) ? (width - len) : 0;

	if (!left_align)
		while (padding--) buf_putc(buf, size, pos, pad);

	while (*s) buf_putc(buf, size, pos, *s++);

	if (left_align)
		while (padding--) buf_putc(buf, size, pos, ' ');
}

static void format_uint(char *buf, size_t size, size_t *pos,
	unsigned long value, int base, int uppercase,
	int width, int left_align, char pad)
{
	const char *digits_lo = "0123456789abcdef";
	const char *digits_hi = "0123456789ABCDEF";
	const char *digits = uppercase ? digits_hi : digits_lo;
	char tmp[32];
	int  len = 0;

	if (value == 0) {
		tmp[len++] = '0';
	} else {
		while (value) {
			tmp[len++] = digits[value % base];
			value /= base;
		}
	}

	int padding = (width > len) ? (width - len) : 0;

	if (!left_align)
		while (padding--) buf_putc(buf, size, pos, pad);
		
	while (len--) buf_putc(buf, size, pos, tmp[len]);
	
	if (left_align)
		while (padding--) buf_putc(buf, size, pos, ' ');
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	size_t pos = 0;

	while (*fmt) {
		if (*fmt != '%') {
			buf_putc(buf, size, &pos, *fmt++);
			continue;
		}
		fmt++;

		int left_align = 0;
		char pad = ' ';
		while (*fmt == '-' || *fmt == '0') {
			if (*fmt == '-') left_align = 1;
			if (*fmt == '0') pad = '0';
			fmt++;
		}

		int width = 0;
		while (*fmt >= '0' && *fmt <= '9')
			width = width * 10 + (*fmt++ - '0');

		switch (*fmt) {
		case 'c': {
			char c = (char)va_arg(args, int);
			char s[2] = { c, '\0' };
			buf_puts(buf, size, &pos, s, width, left_align, pad);
			break;
		}
		case 's': {
			const char *s = va_arg(args, const char *);
			if (!s) s = "(null)";
			buf_puts(buf, size, &pos, s, width, left_align, pad);
			break;
		}
		case 'd':
		case 'i': {
			long val = va_arg(args, int);
			if (val < 0) {
				buf_putc(buf, size, &pos, '-');
				val = -val;
				if (width > 0) width--;
			}
			format_uint(buf, size, &pos, (unsigned long)val,
				10, 0, width, left_align, pad);
			break;
		}
		case 'u':
			format_uint(buf, size, &pos,
				(unsigned long)va_arg(args, unsigned int),
				10, 0, width, left_align, pad);
			break;
		case 'x':
			format_uint(buf, size, &pos,
				(unsigned long)va_arg(args, unsigned int),
				16, 0, width, left_align, pad);
			break;
		case 'X':
			format_uint(buf, size, &pos,
				(unsigned long)va_arg(args, unsigned int),
				16, 1, width, left_align, pad);
			break;
		case 'o':
			format_uint(buf, size, &pos,
				(unsigned long)va_arg(args, unsigned int),
				8, 0, width, left_align, pad);
			break;
		case 'p':
			buf_puts(buf, size, &pos, "0x", 0, 0, ' ');
			format_uint(buf, size, &pos,
				(unsigned long)va_arg(args, void *),
				16, 0, width, left_align, '0');
			break;
		case '%':
			buf_putc(buf, size, &pos, '%');
			break;
		default:
			buf_putc(buf, size, &pos, '%');
			buf_putc(buf, size, &pos, *fmt);
			break;
		}
		fmt++;
	}

	if (buf && size > 0)
		buf[pos < size ? pos : size - 1] = '\0';

	return (int)pos;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	int ret = vsnprintf(buf, size, fmt, args);
	va_end(args);

	return ret;
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	int ret = vsnprintf(buf, (size_t)-1, fmt, args);
	va_end(args);

	return ret;
}

__attribute__((weak)) int putchar(int c)
{
	(void)c;
	
	return c;
}

__attribute__((weak)) int puts(const char *s)
{
	while (*s) putchar((unsigned char)*s++);
	putchar('\n');
	
	return 0;
}

#define PRINTF_BUFFER_SIZE 256

int printf(const char *fmt, ...)
{
	char buf[PRINTF_BUFFER_SIZE];
	va_list args;

	va_start(args, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	for (int i = 0; i < len && buf[i]; i++)
		putchar((unsigned char)buf[i]);

	return len;
}


#ifdef STDIO_EXTRA

static int is_space(int c)
{
	return c == ' ' || c == '\t' || c == '\n' ||
		c == '\r' || c == '\f' || c == '\v';
}

static int is_digit(int c)
{
	return c >= '0' && c <= '9';
}

static int match_char_set(int c, const char *set, int invert)
{
	while (*set) {
		if (*set == c)
			return !invert;
		set++;
	}

	return invert;
}

int vsscanf(const char *str, const char *fmt, va_list args)
{
	int matched = 0;
	const char *s = str;

	while (*fmt) {
		if (is_space(*fmt)) {
			while (is_space(*s))
				s++;
			fmt++;
			continue;
		}

		if (*fmt != '%') {
			if (*s != *fmt)
				return matched;
			s++;
			fmt++;
			continue;
		}

		fmt++;
		
		int suppress = 0;
		if (*fmt == '*') {
			suppress = 1;
			fmt++;
		}

		int width = INT_MAX;
		if (is_digit(*fmt)) {
			width = 0;
			while (is_digit(*fmt))
				width = width * 10 + (*fmt++ - '0');
		}

		int length_mod = 0;
		if (*fmt == 'h') {
			length_mod = 'h';
			fmt++;
			if (*fmt == 'h') {
				length_mod = 'H';
				fmt++;
			}
		} else if (*fmt == 'l') {
			length_mod = 'l';
			fmt++;
		}

		switch (*fmt) {
		case 'c': {
			if (width == INT_MAX)
				width = 1;
			if (suppress) {
				s += width;
			} else {
				char *dest = va_arg(args, char *);
				for (int i = 0; i < width && *s; i++)
					*dest++ = *s++;
				matched++;
			}
			break;
		}

		case 's': {
			while (is_space(*s)) s++;
			
			if (suppress) {
				int count = 0;
				while (*s && !is_space(*s) && count < width) {
					s++;
					count++;
				}
			} else {
				char *dest = va_arg(args, char *);
				int count = 0;
				while (*s && !is_space(*s) && count < width)
					dest[count++] = *s++;
				dest[count] = '\0';
				matched++;
			}
			break;
		}

		case '[': {
			fmt++;
			int invert = 0;
			if (*fmt == '^') {
				invert = 1;
				fmt++;
			}
			const char *set_start = fmt;
			while (*fmt && *fmt != ']') fmt++;
			if (*fmt != ']')
				return matched;

			char set[256];
			size_t set_len = (size_t)(fmt - set_start);
			if (set_len >= sizeof(set)) set_len = sizeof(set) - 1;
			memcpy(set, set_start, set_len);
			set[set_len] = '\0';

			if (suppress) {
				int count = 0;
				while (*s && match_char_set(*s, set, invert) && count < width) {
					s++;
					count++;
				}
			} else {
				char *dest = va_arg(args, char *);
				int count = 0;
				while (*s && match_char_set(*s, set, invert) && count < width)
					dest[count++] = *s++;
				dest[count] = '\0';
				matched++;
			}
			break;
		}

		case 'd':
		case 'i':
		case 'u':
		case 'x':
		case 'X':
		case 'o':
		case 'p': {
			while (is_space(*s))
				s++;

			int base = 10;
			if (*fmt == 'x' || *fmt == 'X' || *fmt == 'p') base = 16;
			if (*fmt == 'o') base = 8;
			if (*fmt == 'i') base = 0;

			char *endptr;
			unsigned long val;
			if (*fmt == 'd' || *fmt == 'i') {
				long signed_val = strtol(s, &endptr, base);
				val = (unsigned long)signed_val;
			} else {
				val = strtoul(s, &endptr, base);
			}

			if (endptr == s)
				return matched;

			s = endptr;

			if (!suppress) {
				if (length_mod == 'H') {
					if (*fmt == 'd' || *fmt == 'i')
						*va_arg(args, signed char *) = (signed char)val;
					else
						*va_arg(args, unsigned char *) = (unsigned char)val;
				} else if (length_mod == 'h') {
					if (*fmt == 'd' || *fmt == 'i')
						*va_arg(args, short *) = (short)val;
					else
						*va_arg(args, unsigned short *) = (unsigned short)val;
				} else if (length_mod == 'l') {
					if (*fmt == 'd' || *fmt == 'i')
						*va_arg(args, long *) = (long)val;
					else
						*va_arg(args, unsigned long *) = val;
				} else {
					if (*fmt == 'd' || *fmt == 'i')
						*va_arg(args, int *) = (int)val;
					else
						*va_arg(args, unsigned int *) = (unsigned int)val;
				}
				matched++;
			}
			break;
		}

		case '%':
			if (*s != '%')
				return matched;
			s++;
			break;

		default:
			return matched;
		}

		fmt++;
	}

	return matched;
}

int sscanf(const char *str, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	int ret = vsscanf(str, fmt, args);
	va_end(args);

	return ret;
}

#define SCANF_BUFFER_SIZE 256

int vscanf(const char *fmt, va_list args)
{
	char buf[SCANF_BUFFER_SIZE];
	int i = 0;
	int c;

	while (i < SCANF_BUFFER_SIZE - 1 && (c = getchar()) != EOF && c != '\n')
		buf[i++] = c;
	buf[i] = '\0';

	return vsscanf(buf, fmt, args);
}

int scanf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = vscanf(fmt, args);
	va_end(args);
	return ret;
}

__attribute__((weak)) int getchar(void)
{
	return EOF;
}

#endif
