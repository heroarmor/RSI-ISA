/* MiBench office/stringsearch — naive and KMP string matching */
#include <stdint.h>

#define TEXT_LEN 512
#define PAT_LEN 8

static char text[TEXT_LEN];
static char pattern[PAT_LEN] = "ABCDABCD";
static int kmp_table[PAT_LEN];

static int naive_search(const char *text, int tlen, const char *pat, int plen) {
    int count = 0;
    for (int i = 0; i <= tlen - plen; i++) {
        int j;
        for (j = 0; j < plen; j++) {
            if (text[i + j] != pat[j])
                break;
        }
        if (j == plen)
            count++;
    }
    return count;
}

static void kmp_build(const char *pat, int plen) {
    kmp_table[0] = 0;
    int len = 0, i = 1;
    while (i < plen) {
        if (pat[i] == pat[len]) {
            len++;
            kmp_table[i] = len;
            i++;
        } else {
            if (len != 0)
                len = kmp_table[len - 1];
            else {
                kmp_table[i] = 0;
                i++;
            }
        }
    }
}

static int kmp_search(const char *text, int tlen, const char *pat, int plen) {
    int count = 0, i = 0, j = 0;
    while (i < tlen) {
        if (pat[j] == text[i]) {
            i++; j++;
        }
        if (j == plen) {
            count++;
            j = kmp_table[j - 1];
        } else if (i < tlen && pat[j] != text[i]) {
            if (j != 0)
                j = kmp_table[j - 1];
            else
                i++;
        }
    }
    return count;
}

int main(void) {
    /* Fill text with repeating pattern + noise */
    for (int i = 0; i < TEXT_LEN; i++)
        text[i] = 'A' + ((i * 3 + i / 7) % 8);

    kmp_build(pattern, PAT_LEN);

    int total = 0;
    for (int iter = 0; iter < 50; iter++) {
        total += naive_search(text, TEXT_LEN, pattern, PAT_LEN);
        total += kmp_search(text, TEXT_LEN, pattern, PAT_LEN);
        /* Mutate text slightly */
        text[iter % TEXT_LEN] = 'A' + (iter % 8);
    }
    return total & 0xFF;
}
