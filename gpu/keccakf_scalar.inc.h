// π forward mapping: 0,10,20,5,15,16,1,11,21,6,7,17,2,12,22,23,8,18,3,13,14,24,9,19,4
// Fully scalar register-based keccakf — 25 individual uint64_t lanes,
// all 24 rounds fully unrolled. Produced by gen_keccak_scalar.mjs
__device__ __forceinline__ void keccakf_scalar(
    uint64_t &s00,
    uint64_t &s10,
    uint64_t &s20,
    uint64_t &s30,
    uint64_t &s40,
    uint64_t &s01,
    uint64_t &s11,
    uint64_t &s21,
    uint64_t &s31,
    uint64_t &s41,
    uint64_t &s02,
    uint64_t &s12,
    uint64_t &s22,
    uint64_t &s32,
    uint64_t &s42,
    uint64_t &s03,
    uint64_t &s13,
    uint64_t &s23,
    uint64_t &s33,
    uint64_t &s43,
    uint64_t &s04,
    uint64_t &s14,
    uint64_t &s24,
    uint64_t &s34,
    uint64_t &s44
) {

    { // ─── Round 0 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 0 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 0 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 0 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x0000000000000001ULL;
    }

    { // ─── Round 1 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 1 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 1 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 1 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x0000000000008082ULL;
    }

    { // ─── Round 2 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 2 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 2 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 2 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x800000000000808aULL;
    }

    { // ─── Round 3 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 3 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 3 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 3 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000080008000ULL;
    }

    { // ─── Round 4 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 4 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 4 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 4 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x000000000000808bULL;
    }

    { // ─── Round 5 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 5 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 5 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 5 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x0000000080000001ULL;
    }

    { // ─── Round 6 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 6 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 6 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 6 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000080008081ULL;
    }

    { // ─── Round 7 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 7 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 7 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 7 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000000008009ULL;
    }

    { // ─── Round 8 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 8 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 8 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 8 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x000000000000008aULL;
    }

    { // ─── Round 9 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 9 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 9 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 9 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x0000000000000088ULL;
    }

    { // ─── Round 10 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 10 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 10 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 10 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x0000000080008009ULL;
    }

    { // ─── Round 11 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 11 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 11 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 11 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x000000008000000aULL;
    }

    { // ─── Round 12 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 12 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 12 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 12 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x000000008000808bULL;
    }

    { // ─── Round 13 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 13 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 13 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 13 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x800000000000008bULL;
    }

    { // ─── Round 14 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 14 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 14 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 14 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000000008089ULL;
    }

    { // ─── Round 15 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 15 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 15 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 15 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000000008003ULL;
    }

    { // ─── Round 16 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 16 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 16 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 16 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000000008002ULL;
    }

    { // ─── Round 17 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 17 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 17 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 17 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000000000080ULL;
    }

    { // ─── Round 18 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 18 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 18 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 18 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x000000000000800aULL;
    }

    { // ─── Round 19 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 19 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 19 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 19 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x800000008000000aULL;
    }

    { // ─── Round 20 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 20 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 20 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 20 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000080008081ULL;
    }

    { // ─── Round 21 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 21 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 21 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 21 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000000008080ULL;
    }

    { // ─── Round 22 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 22 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 22 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 22 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x0000000080000001ULL;
    }

    { // ─── Round 23 ───
    uint64_t c0 = s00 ^ s01 ^ s02 ^ s03 ^ s04;
    uint64_t c1 = s10 ^ s11 ^ s12 ^ s13 ^ s14;
    uint64_t c2 = s20 ^ s21 ^ s22 ^ s23 ^ s24;
    uint64_t c3 = s30 ^ s31 ^ s32 ^ s33 ^ s34;
    uint64_t c4 = s40 ^ s41 ^ s42 ^ s43 ^ s44;
    uint64_t d0 = c4 ^ rotl64(c1, 1);
    uint64_t d1 = c0 ^ rotl64(c2, 1);
    uint64_t d2 = c1 ^ rotl64(c3, 1);
    uint64_t d3 = c2 ^ rotl64(c4, 1);
    uint64_t d4 = c3 ^ rotl64(c0, 1);
    s00 ^= d0;
    s10 ^= d1;
    s20 ^= d2;
    s30 ^= d3;
    s40 ^= d4;
    s01 ^= d0;
    s11 ^= d1;
    s21 ^= d2;
    s31 ^= d3;
    s41 ^= d4;
    s02 ^= d0;
    s12 ^= d1;
    s22 ^= d2;
    s32 ^= d3;
    s42 ^= d4;
    s03 ^= d0;
    s13 ^= d1;
    s23 ^= d2;
    s33 ^= d3;
    s43 ^= d4;
    s04 ^= d0;
    s14 ^= d1;
    s24 ^= d2;
    s34 ^= d3;
    s44 ^= d4;

    // ─── Round 23 (ρ) ───
    s10 = rotl64(s10, 1);
    s20 = rotl64(s20, 62);
    s30 = rotl64(s30, 28);
    s40 = rotl64(s40, 27);
    s01 = rotl64(s01, 36);
    s11 = rotl64(s11, 44);
    s21 = rotl64(s21, 6);
    s31 = rotl64(s31, 55);
    s41 = rotl64(s41, 20);
    s02 = rotl64(s02, 3);
    s12 = rotl64(s12, 10);
    s22 = rotl64(s22, 43);
    s32 = rotl64(s32, 25);
    s42 = rotl64(s42, 39);
    s03 = rotl64(s03, 41);
    s13 = rotl64(s13, 45);
    s23 = rotl64(s23, 15);
    s33 = rotl64(s33, 21);
    s43 = rotl64(s43, 8);
    s04 = rotl64(s04, 18);
    s14 = rotl64(s14, 2);
    s24 = rotl64(s24, 61);
    s34 = rotl64(s34, 56);
    s44 = rotl64(s44, 14);

    // ─── Round 23 (π) ───
    uint64_t p0 = s00;
    uint64_t p1 = s10;
    uint64_t p2 = s20;
    uint64_t p3 = s30;
    uint64_t p4 = s40;
    uint64_t p5 = s01;
    uint64_t p6 = s11;
    uint64_t p7 = s21;
    uint64_t p8 = s31;
    uint64_t p9 = s41;
    uint64_t p10 = s02;
    uint64_t p11 = s12;
    uint64_t p12 = s22;
    uint64_t p13 = s32;
    uint64_t p14 = s42;
    uint64_t p15 = s03;
    uint64_t p16 = s13;
    uint64_t p17 = s23;
    uint64_t p18 = s33;
    uint64_t p19 = s43;
    uint64_t p20 = s04;
    uint64_t p21 = s14;
    uint64_t p22 = s24;
    uint64_t p23 = s34;
    uint64_t p24 = s44;

    s02 = p1;
    s04 = p2;
    s01 = p3;
    s03 = p4;
    s13 = p5;
    s10 = p6;
    s12 = p7;
    s14 = p8;
    s11 = p9;
    s21 = p10;
    s23 = p11;
    s20 = p12;
    s22 = p13;
    s24 = p14;
    s34 = p15;
    s31 = p16;
    s33 = p17;
    s30 = p18;
    s32 = p19;
    s42 = p20;
    s44 = p21;
    s41 = p22;
    s43 = p23;
    s40 = p24;

    // ─── Round 23 (χ) ───
    {
    uint64_t bc0 = s00 ^ ((~s10) & s20);
    uint64_t bc1 = s10 ^ ((~s20) & s30);
    uint64_t bc2 = s20 ^ ((~s30) & s40);
    uint64_t bc3 = s30 ^ ((~s40) & s00);
    uint64_t bc4 = s40 ^ ((~s00) & s10);
    s00 = bc0; s10 = bc1; s20 = bc2; s30 = bc3; s40 = bc4;
    }
    {
    uint64_t bc0 = s01 ^ ((~s11) & s21);
    uint64_t bc1 = s11 ^ ((~s21) & s31);
    uint64_t bc2 = s21 ^ ((~s31) & s41);
    uint64_t bc3 = s31 ^ ((~s41) & s01);
    uint64_t bc4 = s41 ^ ((~s01) & s11);
    s01 = bc0; s11 = bc1; s21 = bc2; s31 = bc3; s41 = bc4;
    }
    {
    uint64_t bc0 = s02 ^ ((~s12) & s22);
    uint64_t bc1 = s12 ^ ((~s22) & s32);
    uint64_t bc2 = s22 ^ ((~s32) & s42);
    uint64_t bc3 = s32 ^ ((~s42) & s02);
    uint64_t bc4 = s42 ^ ((~s02) & s12);
    s02 = bc0; s12 = bc1; s22 = bc2; s32 = bc3; s42 = bc4;
    }
    {
    uint64_t bc0 = s03 ^ ((~s13) & s23);
    uint64_t bc1 = s13 ^ ((~s23) & s33);
    uint64_t bc2 = s23 ^ ((~s33) & s43);
    uint64_t bc3 = s33 ^ ((~s43) & s03);
    uint64_t bc4 = s43 ^ ((~s03) & s13);
    s03 = bc0; s13 = bc1; s23 = bc2; s33 = bc3; s43 = bc4;
    }
    {
    uint64_t bc0 = s04 ^ ((~s14) & s24);
    uint64_t bc1 = s14 ^ ((~s24) & s34);
    uint64_t bc2 = s24 ^ ((~s34) & s44);
    uint64_t bc3 = s34 ^ ((~s44) & s04);
    uint64_t bc4 = s44 ^ ((~s04) & s14);
    s04 = bc0; s14 = bc1; s24 = bc2; s34 = bc3; s44 = bc4;
    }

    s00 ^= 0x8000000080008008ULL;
    }

}
