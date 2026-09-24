/* 8×8 Matrix Multiply — dot product / MAC chain benchmark.
 * C[i][j] = sum(A[i][k] * B[k][j]) for k = 0..7
 * Inner loop is an 8-element dot product → chain of MAC operations.
 */
#include <stdint.h>

#define N 8
#define ITERATIONS 10000

static int32_t A[N][N], B[N][N], C[N][N];

static void matmul(int32_t a[N][N], int32_t b[N][N], int32_t c[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int32_t sum = 0;
            for (int k = 0; k < N; k++) {
                sum += a[i][k] * b[k][j];  /* MAC chain */
            }
            c[i][j] = sum;
        }
    }
}

int main(void) {
    /* Initialize matrices */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            A[i][j] = (int32_t)(i * 7 + j * 13 - 50);
            B[i][j] = (int32_t)(i * 11 - j * 3 + 20);
        }

    int32_t total = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        matmul(A, B, C);
        total += C[0][0] + C[N-1][N-1];
        /* Prevent dead code elimination */
        A[0][0] += (iter & 1);
    }
    return (int)(total & 0xFF);
}
