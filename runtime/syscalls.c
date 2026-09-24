// Minimal syscall stubs for bare-metal benchmarking
// Provides printf via HTIF putchar and rdcycle for timing

#include <stdint.h>
#include <stddef.h>

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

// HTIF putchar: write character via tohost device 1
static void htif_putchar(char c) {
    // HTIF command: dev=1 (console), cmd=1 (putchar), payload=c
    while (tohost != 0) {
        // Wait for previous command to be consumed
        // Clear fromhost if set
        if (fromhost != 0) fromhost = 0;
    }
    tohost = ((uint64_t)1 << 56) | ((uint64_t)1 << 48) | (unsigned char)c;
    // Wait for ack
    while (fromhost == 0);
    fromhost = 0;
}

// Minimal puts
void _puts(const char *s) {
    while (*s) htif_putchar(*s++);
}

// Minimal print unsigned decimal
void _print_uint(uint64_t val) {
    char buf[20];
    int i = 0;
    if (val == 0) { htif_putchar('0'); return; }
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i > 0) htif_putchar(buf[--i]);
}

// Read cycle counter
static inline uint64_t rdcycle(void) {
    uint64_t val;
    __asm__ volatile ("rdcycle %0" : "=r"(val));
    return val;
}

// Benchmark timing helpers
static uint64_t _bench_start;

void bench_start(void) {
    _bench_start = rdcycle();
}

uint64_t bench_end(void) {
    return rdcycle() - _bench_start;
}

// Stub out standard library functions that benchmarks may call
void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1, *b = s2;
    while (n--) {
        if (*a != *b) return *a - *b;
        a++; b++;
    }
    return 0;
}

size_t strlen(const char *s) {
    size_t n = 0;
    while (*s++) n++;
    return n;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// printf stub — benchmarks use printf for results, redirect to htif
#include <stdarg.h>

int printf(const char *fmt, ...) {
    // Minimal: just print the format string, ignore args
    // This is enough for benchmarks that only use printf for status
    _puts(fmt);
    return 0;
}

// Stubs for functions benchmarks might need
void exit(int code) {
    tohost = ((uint64_t)code << 1) | 1;
    while(1);
}

void abort(void) { exit(1); }

int puts(const char *s) {
    _puts(s);
    htif_putchar('\n');
    return 0;
}

// Wrap benchmark main: capture return value, always exit with 0
// The benchmark's return value is used for verification, not as exit code
extern int benchmark_main(void);

int __wrap_main(void) __attribute__((unused));

