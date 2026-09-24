/* 32-tap FIR filter — classic DSP benchmark for MAC chains.
 * y[n] = sum(coeff[k] * x[n-k]) for k = 0..31
 * This is a pure dot product, ideal for MAC compound instruction.
 */
#include <stdint.h>

#define TAPS 32
#define SAMPLES 1024
#define ITERATIONS 500

static int32_t coeffs[TAPS] = {
    1, -3, 5, -7, 11, -13, 17, -19,
    23, -29, 31, -37, 41, -43, 47, -53,
    53, -47, 43, -41, 37, -31, 29, -23,
    19, -17, 13, -11, 7, -5, 3, -1
};

static int32_t input[SAMPLES + TAPS];
static int32_t output[SAMPLES];

static void fir_filter(const int32_t *in, int32_t *out, const int32_t *h, int n, int taps) {
    for (int i = 0; i < n; i++) {
        int32_t sum = 0;
        for (int k = 0; k < taps; k++) {
            sum += h[k] * in[i + taps - 1 - k];  /* MAC: sum = h[k]*in[...] + sum */
        }
        out[i] = sum;
    }
}

int main(void) {
    /* Initialize input with pseudo-random data */
    for (int i = 0; i < SAMPLES + TAPS; i++) {
        input[i] = (int32_t)((i * 2654435761u) >> 16) - 32768;
    }

    int32_t total = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        fir_filter(input, output, coeffs, SAMPLES, TAPS);
        total += output[0] + output[SAMPLES/2];
    }
    return (int)(total & 0xFF);
}
