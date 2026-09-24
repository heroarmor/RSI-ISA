/* MiBench telecomm/CRC32 — cyclic redundancy check */
#include <stdint.h>

static uint32_t crc_table[256];

static void crc32_init(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
        crc_table[i] = crc;
    }
}

static uint32_t crc32(const uint8_t *data, int len) {
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        uint8_t idx = (crc ^ data[i]) & 0xFF;
        crc = crc_table[idx] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

/* Test data */
static uint8_t test_buf[256];

int main(void) {
    crc32_init();

    /* Fill test buffer */
    for (int i = 0; i < 256; i++)
        test_buf[i] = (uint8_t)(i * 7 + 13);

    uint32_t total = 0;
    for (int iter = 0; iter < 100; iter++) {
        total ^= crc32(test_buf, 256);
        test_buf[iter & 0xFF] ^= (uint8_t)(iter + 1);  /* perturb the buffer every iteration */
    }
    return (int)total;
}
