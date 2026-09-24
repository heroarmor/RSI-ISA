/* MiBench automotive/qsort — quicksort on integer array */
#include <stdint.h>

#define ARRAY_SIZE 256

static int32_t arr[ARRAY_SIZE];

static void swap(int32_t *a, int32_t *b) {
    int32_t t = *a;
    *a = *b;
    *b = t;
}

static int partition(int32_t *a, int lo, int hi) {
    int32_t pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        if (a[j] <= pivot) {
            i++;
            swap(&a[i], &a[j]);
        }
    }
    swap(&a[i + 1], &a[hi]);
    return i + 1;
}

static void quicksort(int32_t *a, int lo, int hi) {
    if (lo < hi) {
        int p = partition(a, lo, hi);
        quicksort(a, lo, p - 1);
        quicksort(a, p + 1, hi);
    }
}

int main(void) {
    int32_t total = 0;
    for (int iter = 0; iter < 500; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++)
            arr[i] = (int32_t)((i * 2654435761U + iter * 31) >> 16) & 0xFFFF;
        quicksort(arr, 0, ARRAY_SIZE - 1);
        total += arr[0] + arr[ARRAY_SIZE/2] + arr[ARRAY_SIZE-1];
    }
    return (int)total;
}
