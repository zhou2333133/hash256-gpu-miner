import { existsSync, mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { DATA_DIR } from "./config.js";
import { logger } from "./logger.js";

export type LastState = {
  updatedAt: string;
  epoch: string;
  nextNonceStart: string;
  scanned: string;
};

export type SubmittedNonceRecord = {
  timestamp: string;
  txHash?: string;
  nonce: string;
  epoch: string;
  gasLimit?: string;
  maxFeePerGas?: string;
  maxPriorityFeePerGas?: string;
  simulation: string;
  receiptStatus?: number;
  actualGasCostWei?: string;
};

const lastStatePath = join(DATA_DIR, "last_state.json");
const submittedPath = join(DATA_DIR, "submitted_nonces.jsonl");

export class MinerStateStore {
  private submitted = new Set<string>();

  constructor() {
    mkdirSync(dirname(lastStatePath), { recursive: true });
    this.loadSubmitted();
  }

  loadLastState(): LastState | undefined {
    if (!existsSync(lastStatePath)) {
      return undefined;
    }
    try {
      return JSON.parse(readFileSync(lastStatePath, "utf8")) as LastState;
    } catch (error) {
      logger.warn("Could not read last_state.json, starting fresh", { error: String(error) });
      return undefined;
    }
  }

  saveLastState(state: Omit<LastState, "updatedAt">): void {
    const payload: LastState = {
      ...state,
      updatedAt: new Date().toISOString(),
    };
    writeFileSync(lastStatePath, `${JSON.stringify(payload, null, 2)}\n`, { encoding: "utf8" });
  }

  hasSubmitted(epoch: bigint, nonce: bigint): boolean {
    return this.submitted.has(this.key(epoch, nonce));
  }

  appendSubmitted(record: SubmittedNonceRecord): void {
    writeFileSync(submittedPath, `${JSON.stringify(record)}\n`, { encoding: "utf8", flag: "a" });
    this.submitted.add(this.key(BigInt(record.epoch), BigInt(record.nonce)));
  }

  private loadSubmitted(): void {
    if (!existsSync(submittedPath)) {
      return;
    }
    const lines = readFileSync(submittedPath, "utf8").split(/\r?\n/).filter(Boolean);
    for (const line of lines) {
      try {
        const record = JSON.parse(line) as SubmittedNonceRecord;
        this.submitted.add(this.key(BigInt(record.epoch), BigInt(record.nonce)));
      } catch (error) {
        logger.warn("Ignoring malformed submitted nonce entry", { line, error: String(error) });
      }
    }
  }

  private key(epoch: bigint, nonce: bigint): string {
    return `${epoch.toString()}:${nonce.toString()}`;
  }
}
