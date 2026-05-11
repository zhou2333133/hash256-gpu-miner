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

// ── Direct state construction (no message buffer, no block array) ──
// Builds the 64-byte keccak256 input inline and applies padding,
// then runs keccakf and extracts the 32-byte digest.
__device__ void hash_challenge_nonce(uint64_t nonce, uint8_t output[32]) {
    uint64_t st[25] = {0};

    // Absorb challenge bytes 0–31 from constant memory into st[0..3]
    #pragma unroll
    for (int w = 0; w < 4; w++) {
        uint64_t lane = 0;
        #pragma unroll
        for (int j = 0; j < 8; j++) {
            lane |= ((uint64_t)c_challenge[w * 8 + j]) << (8 * j);
        }
        st[w] ^= lane;
    }

    // Absorb nonce as abi.encode(uint256): big-endian, zero-padded to 32 bytes.
    // Bytes 32–55 (high 24 bytes of uint256) are zero → st[4..6] are already 0.
    // Bytes 56–63 (low 8 bytes of uint256) are nonce in big-endian → st[7].
    // The byte-reversal below produces the same LE lane value as the old
    // block-absorb loop would.
    uint64_t lane = 0;
    #pragma unroll
    for (int j = 0; j < 8; j++) {
        lane |= ((nonce >> (8 * j)) & 0xff) << (8 * (7 - j));
    }
    st[7] ^= lane;

    // Keccak padding for rate = 136 bytes (pad10*1 rule)
    st[8]  ^= 0x01ULL;                       // byte 64  = 0x01 (start padding)
    st[16] ^= 0x8000000000000000ULL;         // byte 135 = 0x80 (end padding)

    keccakf(st);

    // Extract digest: first 32 bytes, little-endian lane order
    #pragma unroll
    for (int i = 0; i < 32; i++) {
        output[i] = (uint8_t)((st[i / 8] >> (8 * (i % 8))) & 0xff);
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
// Shared-memory early-exit flag replaces the old per-iteration global atomic.
// Constant-memory challenge/difficulty eliminates global-memory reads.
// Larger grid + chunk reduces kernel-launch overhead.
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
        // Volatile read from shared memory — no global traffic.
        if (sharedFound) return;

        hash_challenge_nonce(nonceStart + offset, hash);

        if (hash_less_than_difficulty(hash)) {
            int old = atomicCAS((int*)&sharedFound, 0, 1);
            if (old == 0) {                  // I'm the winner
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
//  Host helpers (unchanged from original)
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

    // Copy read-only parameters to __constant__ (per-SM constant cache)
    cuda_check(cudaMemcpyToSymbol(c_challenge, job.challenge, 32),
               "cudaMemcpyToSymbol challenge");
    cuda_check(cudaMemcpyToSymbol(c_difficulty, job.difficulty, 32),
               "cudaMemcpyToSymbol difficulty");

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
        cuda_check(cudaMemcpy(d_found, &zero, sizeof(int), cudaMemcpyHostToDevice),
                   "reset found");
        uint64_t remaining = total - processed;
        uint64_t thisChunk = remaining < chunk ? remaining : chunk;

        search_kernel<<<blocks, threads>>>(
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
    return 0;
}
