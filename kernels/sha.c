/* MiBench security/sha — SHA-1 hash (simplified) */
#include <stdint.h>

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static uint32_t h0, h1, h2, h3, h4;
static uint32_t w[80];

static void sha1_init(void) {
    h0 = 0x67452301; h1 = 0xEFCDAB89;
    h2 = 0x98BADCFE; h3 = 0x10325476;
    h4 = 0xC3D2E1F0;
}

static void sha1_transform(const uint32_t *block) {
    for (int i = 0; i < 16; i++)
        w[i] = block[i];
    for (int i = 16; i < 80; i++)
        w[i] = ROTL(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

    uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

    for (int i = 0; i < 80; i++) {
        uint32_t f, k;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        uint32_t temp = ROTL(a, 5) + f + e + k + w[i];
        e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
    }

    h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
}

static uint32_t test_block[16];

int main(void) {
    uint32_t total = 0;
    for (int iter = 0; iter < 200; iter++) {
        sha1_init();
        for (int i = 0; i < 16; i++)
            test_block[i] = (uint32_t)(i * 0x9E3779B9 + iter);
        sha1_transform(test_block);
        total ^= h0 ^ h1 ^ h2 ^ h3 ^ h4;
    }
    return (int)total;
}
