#include <stdlib.h>
#include <string.h>

static int is_space(char c)
{
	return c == ' ' || c == '\t' || c == '\n' ||
		c == '\r' || c == '\f' || c == '\v';
}

static int to_digit(char c, int base)
{
	int d;

	if (c >= '0' && c <= '9')
		d = c - '0';
	else if (c >= 'a' && c <= 'z')
		d = c - 'a' + 10;
	else if (c >= 'A' && c <= 'Z')
		d = c - 'A' + 10;
	else
		return -1;
		
	return d < base ? d : -1;
}

long strtol(const char *s, char **endptr, int base)
{
	int neg = 0;
	long result = 0;
	int any    = 0;
	int d;

	while (is_space(*s)) s++;

	if (*s == '-') {
		neg = 1;
		s++;
	} else {
		if (*s == '+') s++;
	}

	if (base == 0) {
		if (*s == '0' && (s[1] == 'x' || s[1] == 'X')) {
			base = 16; s += 2;
		} else {
			if (*s == '0') base = 8;
			else base = 10;
		}
	} else {
		if (base == 16 && *s == '0' && (s[1] == 'x' || s[1] == 'X'))
			s += 2;
	}


	while ((d = to_digit(*s, base)) >= 0) {
		result = result * base + d;
		any = 1;
		s++;
	}

	if (endptr)
		*endptr = any ? (char *)s : (char *)(s - (neg ? 1 : 0));
		
	return neg ? -result : result;
}

unsigned long strtoul(const char *s, char **endptr, int base)
{
	unsigned long result = 0;
	int d;

	while (is_space(*s)) s++;
	if (*s == '+') s++;

	if (base == 0) {
		if (*s == '0' && (s[1] == 'x' || s[1] == 'X')) {
			base = 16; s += 2;
		} else {
			if (*s == '0') base = 8;
			else base = 10;
		}
	} else {
		if (base == 16 && *s == '0' && (s[1] == 'x' || s[1] == 'X'))
			s += 2;
	}

	while ((d = to_digit(*s, base)) >= 0) {
		result = result * base + d;
		s++;
	}

	if (endptr)
		*endptr = (char *)s;
		
	return result;
}

int atoi(const char *s)
{
	return (int)strtol(s, NULL, 10);
}

long atol(const char *s)
{
	return strtol(s, NULL, 10);
}


int abs(int n)
{
	return n < 0 ? -n : n;
}

long labs(long n)
{
	return n < 0 ? -n : n;
}


#ifdef STDLIB_EXTRA

static void swap_bytes(void *a, void *b, size_t size)
{
	unsigned char *x = a, *y = b;
	
	while (size--) {
		unsigned char t = *x;
		*x++ = *y;
		*y++ = t;
	}
}

#define QSORT_MAX_DEPTH 64

void qsort(void *base, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *))
{
	if (nmemb < 2)
	return;

	long lo_stack[QSORT_MAX_DEPTH];
	long hi_stack[QSORT_MAX_DEPTH];
	int sp = 0;

	lo_stack[sp] = 0;
	hi_stack[sp] = (long)nmemb - 1;
	sp++;

	while (sp > 0) {
		sp--;
		long lo = lo_stack[sp];
		long hi = hi_stack[sp];
		if (lo >= hi) continue;

		/* Median-of-three pivot */
		long mid = lo + (hi - lo) / 2;
		unsigned char *a = (unsigned char *)base;
		
		if (compar(a + lo * size, a + mid * size) > 0)
			swap_bytes(a + lo * size, a + mid * size, size);
		if (compar(a + lo * size, a + hi  * size) > 0)
			swap_bytes(a + lo * size, a + hi  * size, size);
		if (compar(a + mid * size, a + hi * size) > 0)
			swap_bytes(a + mid * size, a + hi * size, size);

		/* Pivot is now at mid; move to hi-1 */
		swap_bytes(a + mid * size, a + (hi - 1) * size, size);
		void *pivot = a + (hi - 1) * size;

		long i = lo, j = hi - 1;
		while (1) {
			while (compar(a + (++i) * size, pivot) < 0);
			while (j > lo && compar(a + (--j) * size, pivot) > 0);
			if (i >= j) break;
			swap_bytes(a + i * size, a + j * size, size);
		}
		swap_bytes(a + i * size, a + (hi - 1) * size, size);

		/* Push sub-partitions */
		if (sp + 2 <= QSORT_MAX_DEPTH) {
			lo_stack[sp] = lo;  hi_stack[sp] = i - 1; sp++;
			lo_stack[sp] = i + 1; hi_stack[sp] = hi;  sp++;
		}
	}
}

void *bsearch(const void *key, const void *base,
	size_t nmemb, size_t size,
	int (*compar)(const void *, const void *))
{
	const unsigned char *lo = base;
	const unsigned char *hi = lo + nmemb * size;

	while (lo < hi) {
		const unsigned char *mid = lo + ((hi - lo) / size / 2) * size;
		int cmp = compar(key, mid);
		
		if (cmp < 0) {
			hi = mid;
		} else {
			if (cmp > 0) {
				lo = mid + size;
			} else {
				return (void *)mid;
			}
		}
	}
	
	return NULL;
}

#define RAND_MAX 0x7FFFFFFF

static unsigned long rand_state = 1;
static unsigned long random_state = 1;

void srand(unsigned int seed)
{
	rand_state = seed;
}

int rand(void)
{
	/* Park-Miller LCG: state = (state * 48271) % 2147483647 */
	rand_state = (rand_state * 48271UL) % 2147483647UL;

	return (int)(rand_state & RAND_MAX);
}

void srandom(unsigned int seed)
{
	random_state = seed;
}

long random(void)
{
	/* Better LCG with full 32-bit state */
	random_state = random_state * 1103515245UL + 12345UL;
	return (long)((random_state >> 16) & 0x7FFFFFFF);
}

#endif

extern char *_heap_start;
extern char *_heap_end;

#ifdef MALLOC_FULL

#define ALIGN_SIZE  sizeof(void *)
#define ALIGN(n)    (((n) + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))

typedef struct block_header {
	size_t size; /* Payload size  */
	int free;
	struct block_header *next;
} block_header_t;

static block_header_t *heap_head = NULL;

static void heap_init(void *heap_start, size_t heap_size)
{
	if (heap_size <= sizeof(block_header_t))
		return;

	heap_head = (block_header_t *)heap_start;
	
	heap_head->size = heap_size - sizeof(block_header_t);
	heap_head->free = 1;
	heap_head->next = NULL;
}

static void coalesce(void)
{
	block_header_t *cur = heap_head;

	while (cur && cur->next) {
		if (cur->free && cur->next->free) {
			cur->size += sizeof(block_header_t) + cur->next->size;
			cur->next  = cur->next->next;
		} else {
			cur = cur->next;
		}
	}
}

void *malloc(size_t size)
{
	/* Initialize heap on first call */
	if (!heap_head)
		heap_init(_heap_start, (size_t)(_heap_end - _heap_start));

	if (!size || !heap_head)
		return NULL;

	size = ALIGN(size);
	block_header_t *cur = heap_head;

	while (cur) {
		if (cur->free && cur->size >= size) {
			/* Split block if there's enough room for a new header + payload */
			if (cur->size >= size + sizeof(block_header_t) + ALIGN_SIZE) {
				block_header_t *new_block =
					(block_header_t *)((unsigned char *)(cur + 1) + size);
				new_block->size = cur->size - size - sizeof(block_header_t);
				new_block->free = 1;
				new_block->next = cur->next;
				cur->next = new_block;
				cur->size = size;
			}
			cur->free = 0;
		
			return (void *)(cur + 1);
		}
		cur = cur->next;
	}
	
	return NULL; /* Out of memory */
}

void free(void *ptr)
{
	if (!ptr) return;

	block_header_t *hdr = (block_header_t *)ptr - 1;

	hdr->free = 1;
	coalesce();
}

#else

/* Simple bump allocator */
static char *heap_ptr = 0;

void *malloc(size_t size)
{
	void *ptr;

	/* Initialize heap on first call */
	if (!heap_ptr)
		heap_ptr = _heap_start;

	/* Align to 16 bytes */
	size = (size + 15) & ~15;

	if (heap_ptr + size >= _heap_end)
		return 0;

	ptr = heap_ptr;
	heap_ptr += size;
	
	return ptr;
}

void free(void *ptr)
{
}

#endif
