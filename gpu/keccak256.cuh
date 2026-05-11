#pragma once

#include <stdint.h>

__device__ __forceinline__ uint64_t rotl64(uint64_t x, int s) {
    return (x << s) | (x >> (64 - s));
}

__device__ __constant__ uint64_t KECCAKF_RNDC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL,
    0x800000000000808aULL, 0x8000000080008000ULL,
    0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL,
    0x000000000000008aULL, 0x0000000000000088ULL,
    0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL,
    0x8000000000008089ULL, 0x8000000000008003ULL,
    0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL,
    0x0000000080000001ULL, 0x8000000080008008ULL
};

__device__ __constant__ int KECCAKF_ROTC[24] = {
    1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14,
    27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44
};

__device__ __constant__ int KECCAKF_PILN[24] = {
    10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1
};

__device__ void keccakf(uint64_t st[25]) {
    uint64_t bc[5];

    for (int round = 0; round < 24; round++) {
        for (int i = 0; i < 5; i++) {
            bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
        }

        for (int i = 0; i < 5; i++) {
            uint64_t t = bc[(i + 4) % 5] ^ rotl64(bc[(i + 1) % 5], 1);
            for (int j = 0; j < 25; j += 5) {
                st[j + i] ^= t;
            }
        }

        uint64_t t = st[1];
        for (int i = 0; i < 24; i++) {
            int j = KECCAKF_PILN[i];
            bc[0] = st[j];
            st[j] = rotl64(t, KECCAKF_ROTC[i]);
            t = bc[0];
        }

        for (int j = 0; j < 25; j += 5) {
            for (int i = 0; i < 5; i++) {
                bc[i] = st[j + i];
            }
            for (int i = 0; i < 5; i++) {
                st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
            }
        }

        st[0] ^= KECCAKF_RNDC[round];
    }
}

__device__ void keccak256_64(const uint8_t input[64], uint8_t output[32]) {
    uint64_t st[25];
    uint8_t block[136];

    for (int i = 0; i < 25; i++) {
        st[i] = 0;
    }
    for (int i = 0; i < 136; i++) {
        block[i] = 0;
    }
    for (int i = 0; i < 64; i++) {
        block[i] = input[i];
    }

    block[64] ^= 0x01;
    block[135] ^= 0x80;

    for (int i = 0; i < 17; i++) {
        uint64_t lane = 0;
        for (int j = 0; j < 8; j++) {
            lane |= ((uint64_t)block[i * 8 + j]) << (8 * j);
        }
        st[i] ^= lane;
    }

    keccakf(st);

    for (int i = 0; i < 32; i++) {
        output[i] = (uint8_t)((st[i / 8] >> (8 * (i % 8))) & 0xff);
    }
}

