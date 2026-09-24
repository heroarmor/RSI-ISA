/* MiBench telecomm/FFT — fixed-point FFT (simplified) */
#include <stdint.h>

#define FFT_SIZE 256
#define FIXED_SHIFT 14

/* Fixed-point sin/cos table (pre-computed, Q14 format) */
static int16_t sin_table[FFT_SIZE];
static int16_t cos_table[FFT_SIZE];

static int32_t real[FFT_SIZE];
static int32_t imag[FFT_SIZE];

static void init_tables(void) {
    /* Approximate sin/cos with integer math */
    for (int i = 0; i < FFT_SIZE; i++) {
        /* Crude approximation: sin ≈ polynomial */
        int32_t x = (i * 25736) / FFT_SIZE; /* 0 to 2*pi in Q13 */
        int32_t x2 = (x * x) >> 14;
        int32_t x3 = (x2 * x) >> 14;
        int32_t x5 = (x3 * x2) >> 14;
        sin_table[i] = (int16_t)((x - x3 / 6 + x5 / 120) >> 0);
        /* cos = sin(x + pi/2) approximation */
        int32_t y = x - 8192; /* shift by pi/2 */
        int32_t y2 = (y * y) >> 14;
        int32_t y3 = (y2 * y) >> 14;
        int32_t y5 = (y3 * y2) >> 14;
        cos_table[i] = (int16_t)((16384 - y2 / 2 + y2 * y2 / 24) >> 0);
    }
}

/* Bit-reverse permutation */
static int bit_reverse(int x, int bits) {
    int result = 0;
    for (int i = 0; i < bits; i++) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

static void fft_fixed(void) {
    int n = FFT_SIZE;
    int bits = 8; /* log2(256) */

    /* Bit-reverse permutation */
    for (int i = 0; i < n; i++) {
        int j = bit_reverse(i, bits);
        if (j > i) {
            int32_t tr = real[i]; real[i] = real[j]; real[j] = tr;
            int32_t ti = imag[i]; imag[i] = imag[j]; imag[j] = ti;
        }
    }

    /* Butterfly stages */
    for (int stage = 1; stage <= bits; stage++) {
        int m = 1 << stage;
        int half = m >> 1;
        int step = n / m;

        for (int k = 0; k < n; k += m) {
            for (int j = 0; j < half; j++) {
                int idx = (j * step) & (FFT_SIZE - 1);
                int32_t wr = cos_table[idx];
                int32_t wi = -sin_table[idx];

                int32_t tr = (wr * real[k+j+half] - wi * imag[k+j+half]) >> FIXED_SHIFT;
                int32_t ti = (wr * imag[k+j+half] + wi * real[k+j+half]) >> FIXED_SHIFT;

                real[k+j+half] = real[k+j] - tr;
                imag[k+j+half] = imag[k+j] - ti;
                real[k+j] = real[k+j] + tr;
                imag[k+j] = imag[k+j] + ti;
            }
        }
    }
}

int main(void) {
    init_tables();

    int32_t total = 0;
    for (int iter = 0; iter < 200; iter++) {
        /* Initialize with test signal */
        for (int i = 0; i < FFT_SIZE; i++) {
            real[i] = ((i * 127 + iter) % 1000) - 500;
            imag[i] = 0;
        }
        fft_fixed();
        total += real[1] + imag[1];
    }
    return (int)(total & 0xFF);
}
