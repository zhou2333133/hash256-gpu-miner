#include "keccak256.cuh"

#include <cuda_runtime.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Job {
    uint8_t challenge[32];
    uint8_t difficulty[32];
    uint64_t nonceStart;
    uint64_t nonceEnd;
    uint64_t epoch;
    int deviceId;
};

// ── Constant memory: per-SM cache, replaces global device pointers ──
__constant__ uint8_t c_challenge[32];
__constant__ uint8_t c_difficulty[32];

// ── Scalar register-based hash: no arrays, no stack frame ──
// Builds the 64-byte keccak256 input inline and applies padding,
// then runs keccakf_scalar (25 individual uint64_t lanes, fully unrolled).
__device__ void hash_challenge_nonce(uint64_t nonce, uint8_t output[32]) {
    // 25 individual lanes — zero initialized
    uint64_t s00=0, s10=0, s20=0, s30=0, s40=0;
    uint64_t s01=0, s11=0, s21=0, s31=0, s41=0;
    uint64_t s02=0, s12=0, s22=0, s32=0, s42=0;
    uint64_t s03=0, s13=0, s23=0, s33=0, s43=0;
    uint64_t s04=0, s14=0, s24=0, s34=0, s44=0;

    // Absorb challenge bytes 0–31 from constant memory into s00, s10, s20, s30
    #pragma unroll
    for (int j = 0; j < 8; j++) {
        s00 |= ((uint64_t)c_challenge[j]) << (8 * j);
    }
    #pragma unroll
    for (int j = 0; j < 8; j++) {
        s10 |= ((uint64_t)c_challenge[8 + j]) << (8 * j);
    }
    #pragma unroll
    for (int j = 0; j < 8; j++) {
        s20 |= ((uint64_t)c_challenge[16 + j]) << (8 * j);
    }
    #pragma unroll
    for (int j = 0; j < 8; j++) {
        s30 |= ((uint64_t)c_challenge[24 + j]) << (8 * j);
    }

    // Absorb nonce as abi.encode(uint256): big-endian, zero-padded to 32 bytes.
    // Bytes 32–55 (high 24 bytes of uint256) are zero → s40, s01, s11 are already 0.
    // Bytes 56–63 (low 8 bytes of uint256) are nonce in big-endian → s21 (st[7]).
    uint64_t lane = 0;
    #pragma unroll
    for (int j = 0; j < 8; j++) {
        lane |= ((nonce >> (8 * j)) & 0xff) << (8 * (7 - j));
    }
    s21 ^= lane;

    // Keccak padding for rate = 136 bytes (pad10*1 rule)
    s31 ^= 0x01ULL;                       // st[8]  = byte 64  = 0x01
    s13 ^= 0x8000000000000000ULL;         // st[16] = byte 135 = 0x80

    // ── Full scalar keccakf ──
    keccakf_scalar(
        s00, s10, s20, s30, s40,
        s01, s11, s21, s31, s41,
        s02, s12, s22, s32, s42,
        s03, s13, s23, s33, s43,
        s04, s14, s24, s34, s44
    );

    // Extract digest: first 32 bytes from s00, s10, s20, s30
    #pragma unroll
    for (int i = 0; i < 8; i++) {
        output[i]      = (uint8_t)((s00 >> (8 * i)) & 0xff);
        output[8 + i]  = (uint8_t)((s10 >> (8 * i)) & 0xff);
        output[16 + i] = (uint8_t)((s20 >> (8 * i)) & 0xff);
        output[24 + i] = (uint8_t)((s30 >> (8 * i)) & 0xff);
    }
}

// ── Big-endian uint256 comparison ──
__device__ __forceinline__ bool hash_less_than_difficulty(const uint8_t hash[32]) {
    #pragma unroll
    for (int i = 0; i < 32; i++) {
        if (hash[i] < c_difficulty[i]) return true;
        if (hash[i] > c_difficulty[i]) return false;
    }
    return false;
}

// ── Search kernel ──
__global__ void search_kernel(
    uint64_t  nonceStart,
    uint64_t  total,
    int      *foundFlag,
    uint64_t *foundNonce,
    uint8_t  *foundHash)
{
    __shared__ volatile int sharedFound;
    if (threadIdx.x == 0) sharedFound = 0;
    __syncthreads();

    uint64_t idx    = (uint64_t)blockIdx.x * blockDim.x + threadIdx.x;
    uint64_t stride = (uint64_t)gridDim.x * blockDim.x;
    uint8_t  hash[32];

    for (uint64_t offset = idx; offset < total; offset += stride) {
        if (sharedFound) return;

        hash_challenge_nonce(nonceStart + offset, hash);

        if (hash_less_than_difficulty(hash)) {
            int old = atomicCAS((int*)&sharedFound, 0, 1);
            if (old == 0) {
                *foundFlag  = 1;
                *foundNonce = nonceStart + offset;
                #pragma unroll
                for (int i = 0; i < 32; i++) foundHash[i] = hash[i];
            }
            return;
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Selftest — verifies scalar keccakf against baseline array keccakf
// ══════════════════════════════════════════════════════════════════════════

__global__ void selftest_kernel(
    uint64_t *testNonces,
    int       numTests,
    int      *failCount
) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= numTests) return;

    uint64_t nonce = testNonces[tid];

    // Compute reference hash via baseline array keccakf
    // Build 64-byte input: challenge || nonce (abi.encode)
    uint8_t input[64];
    for (int i = 0; i < 32; i++) input[i] = c_challenge[i];
    // Big-endian uint256 padding with zero, nonce in low 8 bytes
    for (int i = 0; i < 24; i++) input[32 + i] = 0;
    input[56] = (uint8_t)(nonce >> 56) & 0xff;
    input[57] = (uint8_t)(nonce >> 48) & 0xff;
    input[58] = (uint8_t)(nonce >> 40) & 0xff;
    input[59] = (uint8_t)(nonce >> 32) & 0xff;
    input[60] = (uint8_t)(nonce >> 24) & 0xff;
    input[61] = (uint8_t)(nonce >> 16) & 0xff;
    input[62] = (uint8_t)(nonce >>  8) & 0xff;
    input[63] = (uint8_t)(nonce >>  0) & 0xff;

    uint8_t expected[32];
    keccak256_64(input, expected);

    // Compute hash via scalar path (same as search_kernel uses)
    uint8_t actual[32];
    hash_challenge_nonce(nonce, actual);

    for (int i = 0; i < 32; i++) {
        if (expected[i] != actual[i]) {
            atomicAdd(failCount, 1);
            return;
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Host helpers
// ══════════════════════════════════════════════════════════════════════════

static std::string extract_string(const std::string &json, const std::string &key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    size_t end = json.find('"', pos + 1);
    if (end == std::string::npos) return "";
    return json.substr(pos + 1, end - pos - 1);
}

static uint64_t parse_u64_dec(const std::string &value) {
    uint64_t out = 0;
    for (char c : value) {
        if (c < '0' || c > '9') continue;
        uint64_t digit = (uint64_t)(c - '0');
        if (out > (UINT64_MAX - digit) / 10ULL) {
            std::cerr << "{\"type\":\"error\",\"message\":\"uint64 overflow while parsing " << value << "\"}" << std::endl;
            exit(2);
        }
        out = out * 10ULL + digit;
    }
    return out;
}

static void parse_u256_dec_to_be32(const std::string &value, uint8_t out[32]) {
    memset(out, 0, 32);
    for (char c : value) {
        if (c < '0' || c > '9') continue;
        int carry = c - '0';
        for (int i = 31; i >= 0; i--) {
            int v = out[i] * 10 + carry;
            out[i] = (uint8_t)(v & 0xff);
            carry = v >> 8;
        }
        if (carry != 0) {
            std::cerr << "{\"type\":\"error\",\"message\":\"uint256 overflow while parsing difficulty\"}" << std::endl;
            exit(2);
        }
    }
}

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool parse_hex32(const std::string &hex, uint8_t out[32]) {
    std::string s = hex;
    if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
        s = s.substr(2);
    }
    if (s.size() != 64) return false;
    for (int i = 0; i < 32; i++) {
        int hi = hex_nibble(s[i * 2]);
        int lo = hex_nibble(s[i * 2 + 1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return true;
}

static std::string hex32(const uint8_t bytes[32]) {
    static const char *digits = "0123456789abcdef";
    std::string out = "0x";
    out.reserve(66);
    for (int i = 0; i < 32; i++) {
        out.push_back(digits[bytes[i] >> 4]);
        out.push_back(digits[bytes[i] & 0x0f]);
    }
    return out;
}

static Job parse_job(const std::string &line) {
    Job job{};
    if (!parse_hex32(extract_string(line, "challenge"), job.challenge)) {
        std::cerr << "{\"type\":\"error\",\"message\":\"invalid challenge\"}" << std::endl;
        exit(2);
    }
    parse_u256_dec_to_be32(extract_string(line, "difficulty"), job.difficulty);
    job.nonceStart = parse_u64_dec(extract_string(line, "nonceStart"));
    job.nonceEnd = parse_u64_dec(extract_string(line, "nonceEnd"));
    job.epoch = parse_u64_dec(extract_string(line, "epoch"));
    std::string device = extract_string(line, "deviceId");
    job.deviceId = device.empty() ? 0 : (int)parse_u64_dec(device);
    if (job.nonceEnd <= job.nonceStart) {
        std::cerr << "{\"type\":\"error\",\"message\":\"nonceEnd must be greater than nonceStart\"}" << std::endl;
        exit(2);
    }
    return job;
}

static void cuda_check(cudaError_t err, const char *where) {
    if (err != cudaSuccess) {
        std::cerr << "{\"type\":\"error\",\"message\":\"CUDA error at " << where << ": " << cudaGetErrorString(err) << "\"}" << std::endl;
        exit(3);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Selftest runner (host)
// ══════════════════════════════════════════════════════════════════════════

static bool run_selftest(cudaStream_t stream) {
    std::cerr << "{\"type\":\"progress\",\"scanned\":\"0\",\"hashrate\":0,\"gpuUtilization\":-1}" << std::endl;

    const uint64_t fixed_tests[] = {
        0ULL,
        1ULL,
        255ULL,
        256ULL,
        0xFFFFFFFFULL,
        0xFFFFFFFFFFFFFFFFULL
    };
    const int num_fixed = sizeof(fixed_tests) / sizeof(fixed_tests[0]);

    // Generate 1000 random nonces on host
    const int num_random = 1000;
    const int num_total = num_fixed + num_random;

    std::vector<uint64_t> test_nonces(num_total);
    for (int i = 0; i < num_fixed; i++) test_nonces[i] = fixed_tests[i];
    for (int i = 0; i < num_random; i++) {
        test_nonces[num_fixed + i] = ((uint64_t)rand() << 32) ^ (uint64_t)rand();
    }

    uint64_t *d_nonces = nullptr;
    int *d_fail = nullptr;
    int h_fail = 0;

    cuda_check(cudaMalloc(&d_nonces, num_total * sizeof(uint64_t)), "selftest malloc nonces");
    cuda_check(cudaMalloc(&d_fail, sizeof(int)), "selftest malloc fail");
    cuda_check(cudaMemcpy(d_nonces, test_nonces.data(), num_total * sizeof(uint64_t), cudaMemcpyHostToDevice), "selftest copy nonces");
    cuda_check(cudaMemcpy(d_fail, &h_fail, sizeof(int), cudaMemcpyHostToDevice), "selftest copy fail");

    int threads = 256;
    int blocks = (num_total + threads - 1) / threads;
    selftest_kernel<<<blocks, threads, 0, stream>>>(d_nonces, num_total, d_fail);
    cuda_check(cudaGetLastError(), "selftest launch");
    cuda_check(cudaDeviceSynchronize(), "selftest sync");

    cuda_check(cudaMemcpy(&h_fail, d_fail, sizeof(int), cudaMemcpyDeviceToHost), "selftest copy fail back");

    cudaFree(d_nonces);
    cudaFree(d_fail);

    if (h_fail == 0) {
        std::cerr << "selftest: ALL " << num_total << " nonces PASSED (scalar matches baseline)" << std::endl;
        return true;
    } else {
        std::cerr << "selftest: " << h_fail << "/" << num_total << " nonces FAILED" << std::endl;
        return false;
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Main
// ══════════════════════════════════════════════════════════════════════════

int main() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cerr << "{\"type\":\"error\",\"message\":\"missing search job on stdin\"}" << std::endl;
        return 2;
    }

    Job job = parse_job(line);
    cuda_check(cudaSetDevice(job.deviceId), "cudaSetDevice");

    cudaStream_t stream;
    cuda_check(cudaStreamCreate(&stream), "cudaStreamCreate");

    // Copy read-only parameters to __constant__ (per-SM constant cache)
    cuda_check(cudaMemcpyToSymbolAsync(c_challenge, job.challenge, 32, 0, cudaMemcpyHostToDevice, stream),
               "cudaMemcpyToSymbol challenge");
    cuda_check(cudaMemcpyToSymbolAsync(c_difficulty, job.difficulty, 32, 0, cudaMemcpyHostToDevice, stream),
               "cudaMemcpyToSymbol difficulty");

    // ── Selftest before mining ──
    if (!run_selftest(stream)) {
        std::cerr << "{\"type\":\"error\",\"message\":\"selftest failed — aborting\"}" << std::endl;
        return 4;
    }

    uint8_t  *d_hash  = nullptr;
    int      *d_found = nullptr;
    uint64_t *d_nonce = nullptr;

    cuda_check(cudaMalloc(&d_hash,  32),              "cudaMalloc hash");
    cuda_check(cudaMalloc(&d_found, sizeof(int)),      "cudaMalloc found");
    cuda_check(cudaMalloc(&d_nonce, sizeof(uint64_t)), "cudaMalloc nonce");

    const uint64_t total   = job.nonceEnd - job.nonceStart;
    const uint64_t chunk   = 64ULL * 1024ULL * 1024ULL;  // 64M per launch
    const int      threads = 256;
    const int      blocks  = 4096;
    uint64_t processed = 0;
    auto started = std::chrono::steady_clock::now();

    while (processed < total) {
        int zero = 0;
        cuda_check(cudaMemcpyAsync(d_found, &zero, sizeof(int), cudaMemcpyHostToDevice, stream),
                   "reset found");
        uint64_t remaining = total - processed;
        uint64_t thisChunk = remaining < chunk ? remaining : chunk;

        search_kernel<<<blocks, threads, 0, stream>>>(
            job.nonceStart + processed,
            thisChunk,
            d_found,
            d_nonce,
            d_hash
        );
        cuda_check(cudaGetLastError(), "launch search_kernel");
        cuda_check(cudaDeviceSynchronize(), "synchronize search_kernel");

        processed += thisChunk;
        auto now = std::chrono::steady_clock::now();
        double seconds = std::chrono::duration<double>(now - started).count();
        double hashrate = processed / (seconds > 0 ? seconds : 0.001);

        int found = 0;
        cuda_check(cudaMemcpy(&found, d_found, sizeof(int), cudaMemcpyDeviceToHost),
                   "copy found");
        if (found) {
            uint64_t nonce = 0;
            uint8_t hash[32];
            cuda_check(cudaMemcpy(&nonce, d_nonce, sizeof(uint64_t), cudaMemcpyDeviceToHost),
                       "copy nonce");
            cuda_check(cudaMemcpy(hash, d_hash, 32, cudaMemcpyDeviceToHost),
                       "copy hash");
            std::cout
                << "{\"type\":\"found\",\"nonce\":\"" << nonce
                << "\",\"hash\":\"" << hex32(hash)
                << "\",\"scanned\":\"" << processed
                << "\",\"hashrate\":" << (uint64_t)hashrate
                << "}" << std::endl;
            cudaFree(d_hash);
            cudaFree(d_found);
            cudaFree(d_nonce);
            cudaStreamDestroy(stream);
            return 0;
        }

        std::cout
            << "{\"type\":\"progress\",\"scanned\":\"" << processed
            << "\",\"hashrate\":" << (uint64_t)hashrate
            << ",\"gpuUtilization\":-1}" << std::endl;
    }

    auto finished = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(finished - started).count();
    double hashrate = processed / (seconds > 0 ? seconds : 0.001);
    std::cout
        << "{\"type\":\"done\",\"scanned\":\"" << processed
        << "\",\"hashrate\":" << (uint64_t)hashrate
        << "}" << std::endl;

    cudaFree(d_hash);
    cudaFree(d_found);
    cudaFree(d_nonce);
    cudaStreamDestroy(stream);
    return 0;
}
