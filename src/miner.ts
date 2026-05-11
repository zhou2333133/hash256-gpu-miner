import { existsSync } from "node:fs";
import { spawn, type ChildProcessWithoutNullStreams } from "node:child_process";
import { randomBytes } from "node:crypto";
import { performance } from "node:perf_hooks";
import { cpus } from "node:os";
import { HashContractClient } from "./contract.js";
import { type AppConfig } from "./config.js";
import { logger } from "./logger.js";

export type MinerJob = {
  challenge: string;
  difficulty: bigint;
  minerAddress: string;
  epoch: bigint;
  nonceStart: bigint;
  nonceEnd: bigint;
};

export type MinerHit = {
  nonce: bigint;
  hash: string;
  epoch: bigint;
  scanned: bigint;
  hashrate: number;
  worker: "cuda" | "cpu";
};

export type MinerProgress = {
  scanned: bigint;
  hashrate: number;
  gpuUtilization?: number;
};

type WorkerResponse =
  | { type: "progress"; scanned: string; hashrate: number; gpuUtilization?: number }
  | { type: "found"; nonce: string; hash: string; scanned: string; hashrate: number }
  | { type: "done"; scanned: string; hashrate: number }
  | { type: "error"; message: string };

export class GpuMiner {
  private process?: ChildProcessWithoutNullStreams;
  private stdoutBuffer = "";

  constructor(private readonly config: AppConfig) {}

  available(): boolean {
    return existsSync(this.config.cudaWorkerPath);
  }

  async search(job: MinerJob, onProgress: (progress: MinerProgress) => void): Promise<MinerHit | undefined> {
    if (!this.available()) {
      return undefined;
    }

    return new Promise<MinerHit | undefined>((resolve, reject) => {
      const child = spawn(this.config.cudaWorkerPath, [], {
        stdio: ["pipe", "pipe", "pipe"],
        env: {
          ...process.env,
          CUDA_VISIBLE_DEVICES: String(this.config.cudaDeviceId),
        },
      });
      this.process = child;

      let settled = false;
      const finish = (value: MinerHit | undefined) => {
        if (settled) {
          return;
        }
        settled = true;
        child.kill();
        resolve(value);
      };

      child.once("error", (error) => {
        if (!settled) {
          settled = true;
          reject(error);
        }
      });

      child.stderr.on("data", (chunk: Buffer) => {
        logger.warn("CUDA worker stderr", { message: chunk.toString("utf8").trim() });
      });

      child.stdout.on("data", (chunk: Buffer) => {
        this.stdoutBuffer += chunk.toString("utf8");
        let newline = this.stdoutBuffer.indexOf("\n");
        while (newline >= 0) {
          const line = this.stdoutBuffer.slice(0, newline).trim();
          this.stdoutBuffer = this.stdoutBuffer.slice(newline + 1);
          newline = this.stdoutBuffer.indexOf("\n");
          if (!line) {
            continue;
          }
          try {
            const message = JSON.parse(line) as WorkerResponse;
            if (message.type === "progress") {
              onProgress({
                scanned: BigInt(message.scanned),
                hashrate: message.hashrate,
                gpuUtilization: message.gpuUtilization,
              });
            } else if (message.type === "found") {
              finish({
                nonce: BigInt(message.nonce),
                hash: message.hash,
                epoch: job.epoch,
                scanned: BigInt(message.scanned),
                hashrate: message.hashrate,
                worker: "cuda",
              });
            } else if (message.type === "done") {
              finish(undefined);
            } else if (message.type === "error") {
              throw new Error(message.message);
            }
          } catch (error) {
            child.kill();
            reject(error);
          }
        }
      });

      child.once("exit", (code, signal) => {
        if (!settled) {
          if (code === 0) {
            finish(undefined);
          } else {
            const reason = signal
              ? `killed by signal ${signal} (${signalName(signal)})`
              : `exited with code ${String(code)}`;
            reject(new Error(`CUDA worker ${reason}`));
          }
        }
      });

      child.stdin.write(
        `${JSON.stringify({
          type: "search",
          challenge: job.challenge,
          difficulty: job.difficulty.toString(),
          minerAddress: job.minerAddress,
          epoch: job.epoch.toString(),
          nonceStart: job.nonceStart.toString(),
          nonceEnd: job.nonceEnd.toString(),
          deviceId: this.config.cudaDeviceId,
        })}\n`,
      );
      child.stdin.end();
    });
  }

  stop(): void {
    this.process?.kill();
  }
}

export class CpuFallbackMiner {
  private shouldStop = false;

  constructor(private readonly contractClient: HashContractClient) {}

  stop(): void {
    this.shouldStop = true;
  }

  async search(job: MinerJob, onProgress: (progress: MinerProgress) => void): Promise<MinerHit | undefined> {
    this.shouldStop = false;
    const start = performance.now();
    let scanned = 0n;
    let lastReport = performance.now();

    for (let nonce = job.nonceStart; nonce < job.nonceEnd; nonce++) {
      if (this.shouldStop) {
        return undefined;
      }
      const hash = this.contractClient.computeResult(job.challenge, nonce);
      scanned++;
      if (this.contractClient.isHashValid(hash, job.difficulty)) {
        const elapsedSeconds = Math.max((performance.now() - start) / 1000, 0.001);
        return {
          nonce,
          hash,
          epoch: job.epoch,
          scanned,
          hashrate: Number(scanned) / elapsedSeconds,
          worker: "cpu",
        };
      }

      if (scanned % 10_000n === 0n) {
        await new Promise((resolve) => setImmediate(resolve));
        const now = performance.now();
        if (now - lastReport >= 1000) {
          lastReport = now;
          const elapsedSeconds = Math.max((now - start) / 1000, 0.001);
          onProgress({
            scanned,
            hashrate: Number(scanned) / elapsedSeconds,
          });
        }
      }
    }

    const elapsedSeconds = Math.max((performance.now() - start) / 1000, 0.001);
    onProgress({
      scanned,
      hashrate: Number(scanned) / elapsedSeconds,
    });
    return undefined;
  }
}

export class MiningCoordinator {
  private readonly gpu: GpuMiner;
  private readonly cpu: CpuFallbackMiner;

  constructor(
    private readonly config: AppConfig,
    contractClient: HashContractClient,
  ) {
    this.gpu = new GpuMiner(config);
    this.cpu = new CpuFallbackMiner(contractClient);
  }

  async search(job: MinerJob, onProgress: (progress: MinerProgress) => void): Promise<MinerHit | undefined> {
    if (this.gpu.available()) {
      try {
        logger.info("Starting CUDA nonce search", {
          path: this.config.cudaWorkerPath,
          epoch: job.epoch.toString(),
          nonceStart: job.nonceStart.toString(),
          nonceEnd: job.nonceEnd.toString(),
        });
        return await this.gpu.search(job, onProgress);
      } catch (error) {
        logger.warn("CUDA worker failed; falling back to CPU verification worker", {
          error: error instanceof Error ? error.message : String(error),
        });
      }
    } else {
      logger.warn("CUDA worker not found; using CPU fallback", { path: this.config.cudaWorkerPath });
    }

    return this.cpu.search(job, onProgress);
  }

  stop(): void {
    this.gpu.stop();
    this.cpu.stop();
  }
}

export function initialNonceStart(saved?: string): bigint {
  if (saved !== undefined) {
    try {
      return BigInt(saved);
    } catch {
      return randomNonceStart();
    }
  }
  return randomNonceStart();
}

function randomNonceStart(): bigint {
  return BigInt(`0x${randomBytes(8).toString("hex")}`);
}

function signalName(signal: string): string {
  const names: Record<string, string> = {
    SIGSEGV: "segfault (invalid memory access)",
    SIGABRT: "abort (CUDA runtime assertion failed)",
    SIGKILL: "killed by OOM killer or system",
    SIGTERM: "termination request",
    SIGBUS: "bus error (hardware/memory issue)",
  };
  return names[signal] ?? `signal ${signal}`;
}

export function recommendedCpuThreads(): number {
  return Math.max(1, Math.min(8, cpus().length - 1));
}
