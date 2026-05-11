import {
  Transaction,
  TransactionReceipt,
  TransactionResponse,
  Wallet,
  formatUnits,
  toQuantity,
  type JsonRpcProvider,
  type TransactionRequest,
} from "ethers";
import { type AppConfig } from "./config.js";
import { HashContractClient, decodeHashContractError } from "./contract.js";
import { GasManager, type GasQuote } from "./gas.js";
import { logger } from "./logger.js";
import {
  EXPECTED_CHAIN_ID,
  TARGET_CONTRACT,
  assertAllowedCalldata,
  assertSafeTransactionRequest,
} from "./safety.js";
import { MinerStateStore } from "./state.js";
import { RotatingProvider, classifyBroadcastError } from "./rpc.js";

export type SubmissionResult =
  | {
      sent: false;
      reason: string;
      simulated: boolean;
      retryable?: boolean;
      contractError?: string;
    }
  | {
      sent: true;
      txHash: string;
      receiptStatus?: number;
      gasCostWei?: bigint;
      gasQuote: GasQuote;
    };

export class TransactionSubmitter {
  constructor(
    private readonly config: AppConfig,
    private readonly wallet: Wallet,
    private readonly contractClient: HashContractClient,
    private readonly gasManager: GasManager,
    private readonly stateStore: MinerStateStore,
    private readonly readPool: RotatingProvider,
    private readonly txPool: RotatingProvider,
  ) {}

  async simulateAndMaybeSubmit(nonce: bigint, epoch: bigint): Promise<SubmissionResult> {
    if (this.stateStore.hasSubmitted(epoch, nonce)) {
      return { sent: false, reason: "nonce already recorded as submitted", simulated: false };
    }

    const data = this.contractClient.encodeMine(nonce);
    assertAllowedCalldata(data);

    const callRequest = {
      to: TARGET_CONTRACT,
      from: this.wallet.address,
      data,
      value: 0n,
    };

    const simulation = await this.simulateMine(callRequest, nonce, epoch);
    if (!simulation.ok) {
      return simulation.result;
    }

    if (this.config.dryRun || !this.config.autoSend) {
      return { sent: false, reason: "DRY_RUN=true or AUTO_SEND=false", simulated: true };
    }

    let estimatedGas: bigint;
    try {
      estimatedGas = await this.readPool.withProvider((provider) => provider.estimateGas(callRequest));
    } catch (error) {
      const contractError = decodeHashContractError(error);
      if (simulation.mode === "pending" && contractError === "BlockCapReached") {
        estimatedGas = this.config.maxGasLimit;
        logger.warn("Gas estimation hit latest block cap; using MAX_GAS_LIMIT after pending simulation succeeded", {
          nonce: nonce.toString(),
          gasLimit: estimatedGas.toString(),
        });
      } else {
        const reason = error instanceof Error ? error.message : String(error);
        logger.warn("Gas estimation failed; not sending", { reason, contractError });
        return { sent: false, reason: `gas estimation failed: ${contractError ?? reason}`, simulated: true };
      }
    }

    const paddedGas = (estimatedGas * 120n) / 100n;
    const gasQuote = await this.readPool.withProvider((provider) => this.gasManager.quote(provider, paddedGas));
    const affordability = this.gasManager.canAffordQuote(gasQuote);
    if (!affordability.ok) {
      logger.warn("Gas policy blocked transaction; continuing mining", { reason: affordability.reason });
      return { sent: false, reason: affordability.reason, simulated: true };
    }

    const baseTx: TransactionRequest = {
      to: TARGET_CONTRACT,
      data,
      value: 0n,
      chainId: EXPECTED_CHAIN_ID,
      type: 2,
      gasLimit: gasQuote.gasLimit,
      maxFeePerGas: gasQuote.maxFeePerGas,
      maxPriorityFeePerGas: gasQuote.maxPriorityFeePerGas,
    };

    assertSafeTransactionRequest(baseTx, {
      maxGasLimit: this.config.maxGasLimit,
      maxFeePerGas: this.config.maxFeePerGasWei,
      maxPriorityFeePerGas: this.config.maxPriorityFeePerGasWei,
    });

    logger.info("Sending mine(uint256) transaction", {
      epoch: epoch.toString(),
      simulationMode: simulation.mode,
      gasLimit: gasQuote.gasLimit.toString(),
      maxFeePerGasGwei: formatUnits(gasQuote.maxFeePerGas, "gwei"),
      maxPriorityFeePerGasGwei: formatUnits(gasQuote.maxPriorityFeePerGas, "gwei"),
      estimatedCostEth: formatUnits(gasQuote.estimatedCostWei, 18),
    });

    const accountNonce = await this.readPool.withProvider((provider) =>
      provider.getTransactionCount(this.wallet.address, "pending"),
    );
    const txWithNonce: TransactionRequest = {
      ...baseTx,
      nonce: accountNonce,
    };
    assertSafeTransactionRequest(txWithNonce, {
      maxGasLimit: this.config.maxGasLimit,
      maxFeePerGas: this.config.maxFeePerGasWei,
      maxPriorityFeePerGas: this.config.maxPriorityFeePerGasWei,
    });

    const signedTx = await this.wallet.signTransaction(txWithNonce);

    // ── Broadcast via txPool with fallback ──
    let response: TransactionResponse;
    let receipt: TransactionReceipt | null = null;
    let finalTxHash: string | undefined;

    try {
      response = await this.broadcastTx(signedTx);
    } catch (error) {
      if (error instanceof UnderpricedError) {
        const replaced = await this.attemptReplacement(baseTx, accountNonce, data, gasQuote, "pending", nonce, epoch);
        if (replaced) {
          return this.buildSubmissionResult(replaced.txHash, replaced.receipt, gasQuote, nonce, epoch);
        }
        logger.warn("Replacement failed after underpriced broadcast error", {
          nonce: nonce.toString(),
        });
        return { sent: false, reason: "replacement failed after underpriced error", simulated: true };
      }
      const reason = error instanceof Error ? error.message : String(error);
      logger.error("sendTransaction failed", { reason });
      return { sent: false, reason: `send failed: ${reason}`, simulated: true };
    }

    // Broadcast succeeded
    this.stateStore.appendSubmitted({
      timestamp: new Date().toISOString(),
      txHash: response.hash,
      nonce: nonce.toString(),
      epoch: epoch.toString(),
      gasLimit: gasQuote.gasLimit.toString(),
      maxFeePerGas: gasQuote.maxFeePerGas.toString(),
      maxPriorityFeePerGas: gasQuote.maxPriorityFeePerGas.toString(),
      simulation: `success-${simulation.mode}-sent`,
    });

    receipt = await this.waitForReceipt(response.hash);

    if (receipt) {
      finalTxHash = response.hash;
    } else {
      // ── Attempt replacement if no receipt ──
      const replaced = await this.attemptReplacement(baseTx, response.nonce, data, gasQuote, response.hash, nonce, epoch);
      if (replaced) {
        finalTxHash = replaced.txHash;
        receipt = replaced.receipt;
      } else {
        finalTxHash = response.hash;
      }
    }

    return this.buildSubmissionResult(finalTxHash, receipt, gasQuote, nonce, epoch);
  }

  // ── Broadcast with error classification ──

  private async broadcastTx(signedTx: string): Promise<TransactionResponse> {
    const parsed = Transaction.from(signedTx);
    const txHash = parsed.hash;
    if (!txHash) throw new Error("Signed transaction has no hash");

    // Phase 1: try each tx pool provider
    for (let i = 0; i < this.txPool.providers.length; i++) {
      const provider = this.txPool.providers[i];
      const label = this.rpcLabel(this.config.txRpcUrls[i]);

      try {
        const response = await provider.broadcastTransaction(signedTx);
        logger.info("Broadcast accepted by tx RPC", { rpc: label, txHash });
        return response;
      } catch (error) {
        const classified = classifyBroadcastError(error);
        logger.warn("Broadcast failed via tx RPC", {
          rpc: label,
          action: classified.action,
          reason: classified.reason,
        });

        switch (classified.action) {
          case "known": {
            const existing = await provider.getTransaction(txHash).catch(() => null);
            if (existing) return existing;
            const fromRead = await this.readPool.withProvider((p) => p.getTransaction(txHash)).catch(() => null);
            if (fromRead) return fromRead;
            throw new Error(`Transaction ${txHash} is known but unretrievable`);
          }
          case "nonce-too-low": {
            const confirmed = await this.readPool.withProvider((p) => p.getTransaction(txHash)).catch(() => null);
            if (confirmed) return confirmed;
            continue;
          }
          case "underpriced":
            throw new UnderpricedError(classified.reason);
          case "reverted":
          case "insufficient-funds":
          case "chain-id":
            throw new FatalBroadcastError(classified.action, classified.reason);
          case "network":
          case "unknown":
            continue;
        }
      }
    }

    // Phase 2: public fallback if all tx RPCs failed
    if (this.config.allowPublicFallback) {
      logger.warn("WARNING: public fallback exposes mine transaction to public mempool", { txHash });
      try {
        return await this.readPool.withProvider((p) => p.broadcastTransaction(signedTx));
      } catch (fallbackError) {
        const classified = classifyBroadcastError(fallbackError);
        if (classified.action === "known" || classified.action === "nonce-too-low") {
          const existing = await this.readPool.withProvider((p) => p.getTransaction(txHash)).catch(() => null);
          if (existing) return existing;
        }
        throw new Error(`Fallback broadcast failed: ${classified.reason}`);
      }
    }

    throw new Error("All tx RPCs exhausted (set ALLOW_PUBLIC_FALLBACK=true to use read RPCs as fallback)");
  }

  // ── Replacement tx (same nonce, bumped gas) ──

  private async attemptReplacement(
    baseTx: TransactionRequest,
    nonce: number,
    data: string,
    gasQuote: GasQuote,
    originalTxHash: string,
    candidateNonce: bigint,
    epoch: bigint,
  ): Promise<{ txHash: string; receipt: TransactionReceipt | null } | null> {
    const replacementQuote = this.gasManager.replacementQuote(gasQuote);
    if (!replacementQuote) {
      logger.warn("Cannot replace tx: bumped gas would exceed configured caps", { txHash: originalTxHash });
      return null;
    }

    const stillValid = await this.stillSimulates(data);
    if (!stillValid) {
      logger.warn("Cannot replace tx: mine(nonce) no longer simulates successfully", {
        txHash: originalTxHash,
        nonce: candidateNonce.toString(),
      });
      return null;
    }

    const replacementTx: TransactionRequest = {
      ...baseTx,
      nonce,
      gasLimit: replacementQuote.gasLimit,
      maxFeePerGas: replacementQuote.maxFeePerGas,
      maxPriorityFeePerGas: replacementQuote.maxPriorityFeePerGas,
    };
    assertSafeTransactionRequest(replacementTx, {
      maxGasLimit: this.config.maxGasLimit,
      maxFeePerGas: this.config.maxFeePerGasWei,
      maxPriorityFeePerGas: this.config.maxPriorityFeePerGasWei,
    });

    logger.warn("Replacing pending mine transaction with bumped EIP-1559 fees", {
      previousTxHash: originalTxHash,
      accountNonce: nonce,
      replacementMaxFeeGwei: formatUnits(replacementQuote.maxFeePerGas, "gwei"),
      replacementPriorityGwei: formatUnits(replacementQuote.maxPriorityFeePerGas, "gwei"),
    });

    try {
      const signedReplacement = await this.wallet.signTransaction(replacementTx);
      const replacement = await this.broadcastTx(signedReplacement);
      this.stateStore.appendSubmitted({
        timestamp: new Date().toISOString(),
        txHash: replacement.hash,
        nonce: candidateNonce.toString(),
        epoch: epoch.toString(),
        gasLimit: replacementQuote.gasLimit.toString(),
        maxFeePerGas: replacementQuote.maxFeePerGas.toString(),
        maxPriorityFeePerGas: replacementQuote.maxPriorityFeePerGas.toString(),
        simulation: "success-replacement-sent",
      });
      const receipt = await this.waitForReceipt(replacement.hash);
      return { txHash: replacement.hash, receipt };
    } catch (error) {
      logger.warn("Replacement broadcast also failed", {
        originalTxHash,
        error: error instanceof Error ? error.message : String(error),
      });
      return null;
    }
  }

  // ── Receipt waiting (readPool first, then txPool) ──

  private async waitForReceipt(txHash: string): Promise<TransactionReceipt | null> {
    try {
      const receipt = await this.readPool.waitForReceipt(txHash, this.config.txConfirmations, 180_000);
      if (receipt) {
        logger.info("Transaction confirmed", { txHash });
        return receipt;
      }
      const txReceipt = await this.txPool.waitForReceipt(txHash, this.config.txConfirmations, 60_000);
      if (txReceipt) {
        logger.info("Transaction confirmed via tx RPC", { txHash });
        return txReceipt;
      }
      logger.warn("Transaction still pending after wait window", { txHash });
      return null;
    } catch (error) {
      logger.warn("Receipt wait failed or timed out", {
        txHash,
        error: error instanceof Error ? error.message : String(error),
      });
      return null;
    }
  }

  // ── Simulation helpers (all readPool) ──

  private async simulateMine(
    callRequest: { to: string; from: string; data: string; value: bigint },
    nonce: bigint,
    epoch: bigint,
  ): Promise<
    | { ok: true; mode: "latest" | "pending" }
    | { ok: false; result: Extract<SubmissionResult, { sent: false }> }
  > {
    try {
      await this.readPool.withProvider((provider) => provider.call(callRequest));
      logger.info("eth_call simulation succeeded", { epoch: epoch.toString() });
      return { ok: true, mode: "latest" };
    } catch (error) {
      const reason = error instanceof Error ? error.message : String(error);
      const contractError = decodeHashContractError(error);
      if (contractError === "BlockCapReached") {
        const pendingOk = await this.simulatePending(callRequest, nonce, epoch);
        if (pendingOk) {
          return { ok: true, mode: "pending" };
        }
      }
      const retryable = contractError === "BlockCapReached";
      logger.warn("eth_call simulation failed; not sending", {
        reason,
        contractError,
        retryable,
      });
      return {
        ok: false,
        result: {
          sent: false,
          reason: contractError ? `simulation failed: ${contractError}` : `simulation failed: ${reason}`,
          simulated: false,
          retryable,
          contractError,
        },
      };
    }
  }

  private async simulatePending(
    callRequest: { to: string; from: string; data: string; value: bigint },
    nonce: bigint,
    epoch: bigint,
  ): Promise<boolean> {
    try {
      await this.readPool.withProvider((provider) =>
        provider.send("eth_call", [
          {
            to: callRequest.to,
            from: callRequest.from,
            data: callRequest.data,
            value: toQuantity(callRequest.value),
          },
          "pending",
        ]) as Promise<string>,
      );
      logger.info("pending eth_call simulation succeeded after latest block cap", {
        epoch: epoch.toString(),
      });
      return true;
    } catch (error) {
      const reason = error instanceof Error ? error.message : String(error);
      logger.warn("pending eth_call simulation failed", {
        epoch: epoch.toString(),
        contractError: decodeHashContractError(error),
        reason,
      });
      return false;
    }
  }

  private async stillSimulates(data: string): Promise<boolean> {
    try {
      await this.readPool.withProvider((provider) =>
        provider.call({
          to: TARGET_CONTRACT,
          from: this.wallet.address,
          data,
          value: 0n,
        }),
      );
      return true;
    } catch {
      return false;
    }
  }

  private buildSubmissionResult(
    txHash: string,
    receipt: TransactionReceipt | null,
    gasQuote: GasQuote,
    nonce: bigint,
    epoch: bigint,
  ): SubmissionResult {
    const gasCostWei =
      receipt?.gasUsed !== undefined && receipt?.fee !== undefined ? receipt.fee : undefined;
    if (gasCostWei !== undefined) {
      this.gasManager.recordSpend(txHash, gasCostWei);
    }

    this.stateStore.appendSubmitted({
      timestamp: new Date().toISOString(),
      txHash,
      nonce: nonce.toString(),
      epoch: epoch.toString(),
      gasLimit: gasQuote.gasLimit.toString(),
      maxFeePerGas: gasQuote.maxFeePerGas.toString(),
      maxPriorityFeePerGas: gasQuote.maxPriorityFeePerGas.toString(),
      simulation: "success-receipt",
      receiptStatus: receipt?.status ?? undefined,
      actualGasCostWei: gasCostWei?.toString(),
    });

    return {
      sent: true,
      txHash,
      receiptStatus: receipt?.status ?? undefined,
      gasCostWei,
      gasQuote,
    };
  }

  private rpcLabel(url: string): string {
    try {
      const parsed = new URL(url);
      return `${parsed.protocol}//${parsed.host}`;
    } catch {
      return url;
    }
  }
}

// ── Broadcast error classes ──

class UnderpricedError extends Error {
  constructor(message: string) {
    super(message);
    this.name = "UnderpricedError";
  }
}

class FatalBroadcastError extends Error {
  constructor(
    public readonly action: string,
    message: string,
  ) {
    super(message);
    this.name = "FatalBroadcastError";
  }
}
