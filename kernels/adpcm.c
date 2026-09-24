/* MiBench telecomm/adpcm — Adaptive Differential Pulse Code Modulation */
#include <stdint.h>

/* Step size index table */
static const int8_t index_table[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

/* Step size table */
static const int16_t step_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767,
};

typedef struct {
    int16_t prev_sample;
    int8_t index;
} AdpcmState;

static int16_t adpcm_decode_sample(uint8_t code, AdpcmState *state) {
    int step = step_table[state->index];
    int diff = step >> 3;
    if (code & 4) diff += step;
    if (code & 2) diff += step >> 1;
    if (code & 1) diff += step >> 2;
    if (code & 8) diff = -diff;

    int32_t sample = state->prev_sample + diff;
    if (sample > 32767) sample = 32767;
    else if (sample < -32768) sample = -32768;

    state->prev_sample = (int16_t)sample;
    state->index += index_table[code & 0xF];
    if (state->index < 0) state->index = 0;
    if (state->index > 88) state->index = 88;

    return (int16_t)sample;
}

static uint8_t adpcm_encode_sample(int16_t sample, AdpcmState *state) {
    int diff = sample - state->prev_sample;
    uint8_t code = 0;
    int step = step_table[state->index];

    if (diff < 0) { code = 8; diff = -diff; }
    if (diff >= step) { code |= 4; diff -= step; }
    if (diff >= (step >> 1)) { code |= 2; diff -= step >> 1; }
    if (diff >= (step >> 2)) { code |= 1; }

    /* Decode to update state (same as decoder) */
    adpcm_decode_sample(code, state);
    return code;
}

/* Test data: sawtooth wave */
static int16_t pcm_data[512];
static uint8_t adpcm_data[512];
static int16_t decoded_data[512];

int main(void) {
    int32_t total = 0;

    for (int iter = 0; iter < 20; iter++) {
        /* Generate test signal */
        for (int i = 0; i < 512; i++)
            pcm_data[i] = (int16_t)((i * 127 + iter * 31) % 65536 - 32768);

        /* Encode */
        AdpcmState enc_state = {0, 0};
        for (int i = 0; i < 512; i++)
            adpcm_data[i] = adpcm_encode_sample(pcm_data[i], &enc_state);

        /* Decode */
        AdpcmState dec_state = {0, 0};
        for (int i = 0; i < 512; i++)
            decoded_data[i] = adpcm_decode_sample(adpcm_data[i], &dec_state);

        /* Compute error */
        int32_t err = 0;
        for (int i = 0; i < 512; i++)
            err += (pcm_data[i] - decoded_data[i]) * (pcm_data[i] - decoded_data[i]) >> 16;
        total += err;
    }

    return (int)(total & 0xFF);
}
