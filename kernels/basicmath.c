/* MiBench automotive/basicmath — fixed-point math kernels */
#include <stdint.h>

#define FP_SHIFT 16
#define FP_ONE (1 << FP_SHIFT)

/* Fixed-point multiply */
static int32_t fp_mul(int32_t a, int32_t b) {
    return (int32_t)(((int64_t)a * b) >> FP_SHIFT);
}

/* Fixed-point sqrt via Newton-Raphson */
static int32_t fp_sqrt(int32_t x) {
    if (x <= 0) return 0;
    int32_t guess = x >> 1;
    if (guess == 0) guess = 1;
    for (int i = 0; i < 15; i++) {
        int32_t div = ((int64_t)x << FP_SHIFT) / guess;
        guess = (guess + div) >> 1;
    }
    return guess;
}

/* Fixed-point sin via Taylor series */
static int32_t fp_sin(int32_t x) {
    /* x in Q16, range [-pi, pi] mapped to [-205887, 205887] */
    int32_t x2 = fp_mul(x, x);
    int32_t x3 = fp_mul(x2, x);
    int32_t x5 = fp_mul(x3, x2);
    int32_t x7 = fp_mul(x5, x2);
    /* sin(x) ≈ x - x^3/6 + x^5/120 - x^7/5040 */
    return x - x3 / 6 + x5 / 120 - x7 / 5040;
}

/* Cubic root via Newton-Raphson */
static int32_t fp_cbrt(int32_t x) {
    if (x == 0) return 0;
    int32_t neg = 0;
    if (x < 0) { neg = 1; x = -x; }
    int32_t guess = x >> 2;
    if (guess == 0) guess = FP_ONE;
    for (int i = 0; i < 20; i++) {
        int32_t g2 = fp_mul(guess, guess);
        if (g2 == 0) break;
        int32_t g3 = fp_mul(g2, guess);
        /* Newton: g_new = g - (g^3 - x) / (3*g^2) */
        int32_t num = g3 - x;
        int32_t den = 3 * (g2 >> FP_SHIFT);
        if (den == 0) break;
        guess = guess - num / den;
    }
    return neg ? -guess : guess;
}

/* Matrix multiply 4x4 (fixed-point) */
static int32_t mat_a[4][4], mat_b[4][4], mat_c[4][4];

static void mat_mul_4x4(void) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            int32_t sum = 0;
            for (int k = 0; k < 4; k++)
                sum += fp_mul(mat_a[i][k], mat_b[k][j]);
            mat_c[i][j] = sum;
        }
}

int main(void) {
    int32_t total = 0;

    /* Test sqrt and cbrt */
    for (int i = 1; i < 500; i++) {
        int32_t x = i * FP_ONE;
        total += fp_sqrt(x);
        total += fp_cbrt(x);
    }

    /* Test sin */
    for (int i = -100; i < 100; i++) {
        int32_t x = i * (FP_ONE / 50); /* range roughly [-2, 2] */
        total += fp_sin(x);
    }

    /* Test matmul */
    for (int iter = 0; iter < 500; iter++) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) {
                mat_a[i][j] = (i * 3 + j * 7 + iter) * (FP_ONE / 10);
                mat_b[i][j] = (i * 11 + j * 5 + iter * 3) * (FP_ONE / 10);
            }
        mat_mul_4x4();
        total += mat_c[0][0] + mat_c[3][3];
    }

    return (int)(total & 0xFF);
}
