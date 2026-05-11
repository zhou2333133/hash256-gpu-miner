// The miner uses a line-delimited JSON process protocol instead of an in-process
// Node native addon. This keeps private key handling in TypeScript and keeps the
// CUDA worker isolated to public mining parameters only.
//
// Build the worker with:
//   nvcc -O3 -std=c++17 -arch=sm_75 -o cuda_miner cuda_miner.cu
