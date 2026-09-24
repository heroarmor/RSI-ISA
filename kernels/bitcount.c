/* MiBench automotive/bitcount — counts bits using multiple algorithms */
#include <stdint.h>

int bitcount_naive(uint32_t x) {
    int count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

int bitcount_kernighan(uint32_t x) {
    int count = 0;
    while (x) {
        x &= (x - 1);
        count++;
    }
    return count;
}

/* Lookup table based */
static const uint8_t bits_in_byte[256] = {
    0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
};

int bitcount_lookup(uint32_t x) {
    return bits_in_byte[x & 0xff] + bits_in_byte[(x >> 8) & 0xff] +
           bits_in_byte[(x >> 16) & 0xff] + bits_in_byte[(x >> 24) & 0xff];
}

int bitcount_parallel(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    return (x * 0x01010101) >> 24;
}

volatile uint32_t test_data[] = {
    0, 1, 0xFF, 0xFFFF, 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555,
    0x12345678, 0xDEADBEEF, 0xCAFEBABE
};

int main(void) {
    int total = 0;
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < 10; i++) {
            uint32_t v = test_data[i];
            total += bitcount_naive(v);
            total += bitcount_kernighan(v);
            total += bitcount_lookup(v);
            total += bitcount_parallel(v);
        }
    }
    return total;
}
