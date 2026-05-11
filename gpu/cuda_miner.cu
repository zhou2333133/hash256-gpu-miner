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

__device__ bool hash_less_than_difficulty(const uint8_t hash[32], const uint8_t difficulty[32]) {
    for (int i = 0; i < 32; i++) {
        if (hash[i] < difficulty[i]) return true;
        if (hash[i] > difficulty[i]) return false;
    }
    return false;
}

__device__ void build_abi_message(const uint8_t challenge[32], uint64_t nonce, uint8_t message[64]) {
    for (int i = 0; i < 32; i++) {
        message[i] = challenge[i];
    }
    for (int i = 32; i < 64; i++) {
        message[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        message[63 - i] = (uint8_t)((nonce >> (8 * i)) & 0xff);
    }
}

__global__ void search_kernel(
    const uint8_t *challenge,
    const uint8_t *difficulty,
    uint64_t nonceStart,
    uint64_t total,
    int *found,
    uint64_t *foundNonce,
    uint8_t *foundHash
) {
    uint64_t idx = (uint64_t)blockIdx.x * blockDim.x + threadIdx.x;
    uint64_t stride = (uint64_t)gridDim.x * blockDim.x;

    for (uint64_t offset = idx; offset < total; offset += stride) {
        if (atomicAdd(found, 0) != 0) return;

        uint64_t nonce = nonceStart + offset;
        uint8_t message[64];
        uint8_t hash[32];
        build_abi_message(challenge, nonce, message);
        keccak256_64(message, hash);

        if (hash_less_than_difficulty(hash, difficulty)) {
            if (atomicCAS(found, 0, 1) == 0) {
                *foundNonce = nonce;
                for (int i = 0; i < 32; i++) {
                    foundHash[i] = hash[i];
                }
            }
            return;
        }
    }
}

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

int main() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cerr << "{\"type\":\"error\",\"message\":\"missing search job on stdin\"}" << std::endl;
        return 2;
    }

    Job job = parse_job(line);
    cuda_check(cudaSetDevice(job.deviceId), "cudaSetDevice");

    uint8_t *d_challenge = nullptr;
    uint8_t *d_difficulty = nullptr;
    uint8_t *d_hash = nullptr;
    int *d_found = nullptr;
    uint64_t *d_nonce = nullptr;

    cuda_check(cudaMalloc(&d_challenge, 32), "cudaMalloc challenge");
    cuda_check(cudaMalloc(&d_difficulty, 32), "cudaMalloc difficulty");
    cuda_check(cudaMalloc(&d_hash, 32), "cudaMalloc hash");
    cuda_check(cudaMalloc(&d_found, sizeof(int)), "cudaMalloc found");
    cuda_check(cudaMalloc(&d_nonce, sizeof(uint64_t)), "cudaMalloc nonce");

    cuda_check(cudaMemcpy(d_challenge, job.challenge, 32, cudaMemcpyHostToDevice), "copy challenge");
    cuda_check(cudaMemcpy(d_difficulty, job.difficulty, 32, cudaMemcpyHostToDevice), "copy difficulty");

    const uint64_t total = job.nonceEnd - job.nonceStart;
    const uint64_t chunk = 8ULL * 1024ULL * 1024ULL;
    const int threads = 256;
    const int blocks = 4096;
    uint64_t processed = 0;
    auto started = std::chrono::steady_clock::now();

    while (processed < total) {
        int zero = 0;
        cuda_check(cudaMemcpy(d_found, &zero, sizeof(int), cudaMemcpyHostToDevice), "reset found");
        uint64_t remaining = total - processed;
        uint64_t thisChunk = remaining < chunk ? remaining : chunk;

        search_kernel<<<blocks, threads>>>(
            d_challenge,
            d_difficulty,
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
        cuda_check(cudaMemcpy(&found, d_found, sizeof(int), cudaMemcpyDeviceToHost), "copy found");
        if (found) {
            uint64_t nonce = 0;
            uint8_t hash[32];
            cuda_check(cudaMemcpy(&nonce, d_nonce, sizeof(uint64_t), cudaMemcpyDeviceToHost), "copy nonce");
            cuda_check(cudaMemcpy(hash, d_hash, 32, cudaMemcpyDeviceToHost), "copy hash");
            std::cout
                << "{\"type\":\"found\",\"nonce\":\"" << nonce
                << "\",\"hash\":\"" << hex32(hash)
                << "\",\"scanned\":\"" << processed
                << "\",\"hashrate\":" << (uint64_t)hashrate
                << "}" << std::endl;
            cudaFree(d_challenge);
            cudaFree(d_difficulty);
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

    cudaFree(d_challenge);
    cudaFree(d_difficulty);
    cudaFree(d_hash);
    cudaFree(d_found);
    cudaFree(d_nonce);
    return 0;
}

