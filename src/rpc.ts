import { JsonRpcProvider, Transaction, type TransactionReceipt, type TransactionResponse } from "ethers";
import { logger } from "./logger.js";
import { EXPECTED_CHAIN_ID } from "./safety.js";

export class RotatingProvider {
  private index = 0;
  private providers: JsonRpcProvider[];

  constructor(
    private readonly urls: string[],
    private readonly timeoutMs = 8000,
  ) {
    if (urls.length === 0) {
      throw new Error("No RPC URLs configured");
    }
    this.providers = urls.map((url) => this.createProvider(url));
  }

  current(): JsonRpcProvider {
    return this.providers[this.index];
  }

  currentUrlLabel(): string {
    const url = this.urls[this.index] ?? "";
    try {
      const parsed = new URL(url);
      return `${parsed.protocol}//${parsed.host}`;
    } catch {
      return `rpc[${this.index}]`;
    }
  }

  async withProvider<T>(operation: (provider: JsonRpcProvider) => Promise<T>): Promise<T> {
    let lastError: unknown;
    for (let attempt = 0; attempt < this.urls.length; attempt++) {
      const provider = this.current();
      try {
        const result = await this.withTimeout(operation(provider), `RPC operation timed out at ${this.currentUrlLabel()}`);
        await this.withTimeout(this.assertMainnet(provider), `RPC chain check timed out at ${this.currentUrlLabel()}`);
        return result;
      } catch (error) {
        if (isContractCallException(error)) {
          throw error;
        }
        lastError = error;
        logger.warn("RPC operation failed, rotating provider", {
          rpc: this.currentUrlLabel(),
          error: error instanceof Error ? error.message : String(error),
        });
        this.rotate();
      }
    }
    throw lastError instanceof Error ? lastError : new Error(String(lastError));
  }

  async broadcastSignedTransaction(signedTx: string): Promise<TransactionResponse> {
    const parsed = Transaction.from(signedTx);
    const txHash = parsed.hash;
    let lastError: unknown;

    for (let attempt = 0; attempt < this.urls.length; attempt++) {
      const provider = this.current();
      try {
        const response = await this.withTimeout(
          provider.broadcastTransaction(signedTx),
          `broadcast timed out at ${this.currentUrlLabel()}`,
        );
        logger.info("Signed transaction broadcast accepted", { rpc: this.currentUrlLabel(), txHash: response.hash });
        return response;
      } catch (error) {
        lastError = error;
        const message = error instanceof Error ? error.message : String(error);
        logger.warn("Broadcast failed, rotating provider", { rpc: this.currentUrlLabel(), txHash, error: message });

        const known = await this.lookupKnownTransaction(provider, txHash);
        if (known) {
          logger.info("Signed transaction is already known by RPC", { rpc: this.currentUrlLabel(), txHash });
          return known;
        }
        this.rotate();
      }
    }

    throw lastError instanceof Error ? lastError : new Error(String(lastError));
  }

  async waitForReceipt(txHash: string, confirmations: number, timeoutMs: number): Promise<TransactionReceipt | null> {
    const deadline = Date.now() + timeoutMs;
    while (Date.now() < deadline) {
      for (let i = 0; i < this.urls.length; i++) {
        const provider = this.providers[(this.index + i) % this.providers.length];
        try {
          const receipt = await this.withTimeout(
            provider.getTransactionReceipt(txHash),
            `receipt lookup timed out at rpc[${i}]`,
          );
          if (receipt) {
            const actualConfirmations = await this.withTimeout(
              receipt.confirmations(),
              `confirmation lookup timed out at rpc[${i}]`,
            );
            if (actualConfirmations >= confirmations) {
              return receipt;
            }
          }
        } catch (error) {
          logger.debug("Receipt lookup failed", {
            rpc: this.labelForIndex((this.index + i) % this.providers.length),
            txHash,
            error: error instanceof Error ? error.message : String(error),
          });
        }
      }
      await new Promise((resolve) => setTimeout(resolve, 10_000));
    }
    return null;
  }

  private rotate(): void {
    this.index = (this.index + 1) % this.urls.length;
    logger.info("Using RPC provider", { rpc: this.currentUrlLabel() });
  }

  private createProvider(url: string): JsonRpcProvider {
    return new JsonRpcProvider(url, Number(EXPECTED_CHAIN_ID), {
      staticNetwork: true,
    });
  }

  private async assertMainnet(provider: JsonRpcProvider): Promise<void> {
    const network = await provider.getNetwork();
    if (network.chainId !== EXPECTED_CHAIN_ID) {
      throw new Error(`RPC chainId mismatch: expected 1, got ${network.chainId.toString()}`);
    }
  }

  private async lookupKnownTransaction(
    provider: JsonRpcProvider,
    txHash: string | null,
  ): Promise<TransactionResponse | null> {
    if (!txHash) {
      return null;
    }
    try {
      return await this.withTimeout(provider.getTransaction(txHash), `known tx lookup timed out at ${this.currentUrlLabel()}`);
    } catch {
      return null;
    }
  }

  private labelForIndex(index: number): string {
    const url = this.urls[index] ?? "";
    try {
      const parsed = new URL(url);
      return `${parsed.protocol}//${parsed.host}`;
    } catch {
      return `rpc[${index}]`;
    }
  }

  private async withTimeout<T>(promise: Promise<T>, message: string): Promise<T> {
    let timer: NodeJS.Timeout | undefined;
    try {
      return await Promise.race([
        promise,
        new Promise<never>((_resolve, reject) => {
          timer = setTimeout(() => reject(new Error(message)), this.timeoutMs);
        }),
      ]);
    } finally {
      if (timer) {
        clearTimeout(timer);
      }
    }
  }
}

function isContractCallException(error: unknown): boolean {
  if (error === null || typeof error !== "object") {
    return false;
  }
  const record = error as Record<string, unknown>;
  return record.code === "CALL_EXCEPTION";
}
