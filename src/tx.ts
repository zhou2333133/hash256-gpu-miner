import {
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
import { RotatingProvider } from "./rpc.js";

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
    private readonly providerPool: RotatingProvider,
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
      estimatedGas = await this.providerPool.withProvider((provider) => provider.estimateGas(callRequest));
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
    const gasQuote = await this.providerPool.withProvider((provider) => this.gasManager.quote(provider, paddedGas));
    const affordability = this.gasManager.canAffordQuote(gasQuote);
    if (!affordability.ok) {
      logger.warn("Gas policy blocked transaction; continuing mining", { reason: affordability.reason });
      return { sent: false, reason: affordability.reason, simulated: true };
    }

    const tx: TransactionRequest = {
      to: TARGET_CONTRACT,
      data,
      value: 0n,
      chainId: EXPECTED_CHAIN_ID,
      type: 2,
      gasLimit: gasQuote.gasLimit,
      maxFeePerGas: gasQuote.maxFeePerGas,
      maxPriorityFeePerGas: gasQuote.maxPriorityFeePerGas,
    };

    assertSafeTransactionRequest(tx, {
      maxGasLimit: this.config.maxGasLimit,
      maxFeePerGas: this.config.maxFeePerGasWei,
      maxPriorityFeePerGas: this.config.maxPriorityFeePerGasWei,
    });

    logger.info("Sending mine(uint256) transaction", {
      nonce: nonce.toString(),
      epoch: epoch.toString(),
      simulationMode: simulation.mode,
      gasLimit: gasQuote.gasLimit.toString(),
      maxFeePerGasGwei: formatUnits(gasQuote.maxFeePerGas, "gwei"),
      maxPriorityFeePerGasGwei: formatUnits(gasQuote.maxPriorityFeePerGas, "gwei"),
      estimatedCostEth: formatUnits(gasQuote.estimatedCostWei, 18),
    });

    let response: TransactionResponse;
    try {
      const accountNonce = await this.providerPool.withProvider((provider) =>
        provider.getTransactionCount(this.wallet.address, "pending"),
      );
      const txWithNonce: TransactionRequest = {
        ...tx,
        nonce: accountNonce,
      };
      assertSafeTransactionRequest(txWithNonce, {
        maxGasLimit: this.config.maxGasLimit,
        maxFeePerGas: this.config.maxFeePerGasWei,
        maxPriorityFeePerGas: this.config.maxPriorityFeePerGasWei,
      });
      const signedTx = await this.wallet.signTransaction(txWithNonce);
      response = await this.providerPool.broadcastSignedTransaction(signedTx);
    } catch (error) {
      const reason = error instanceof Error ? error.message : String(error);
      logger.error("sendTransaction failed", { reason });
      return { sent: false, reason: `send failed: ${reason}`, simulated: true };
    }

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

    let receipt = await this.waitForReceipt(response);
    let finalTxHash = response.hash;

    if (!receipt) {
      const replacementQuote = this.gasManager.replacementQuote(gasQuote);
      if (replacementQuote) {
        const stillValid = await this.stillSimulates(data);
        if (stillValid) {
          const replacementTx: TransactionRequest = {
            ...tx,
            nonce: response.nonce,
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
            previousTxHash: response.hash,
            accountNonce: response.nonce,
            replacementMaxFeeGwei: formatUnits(replacementQuote.maxFeePerGas, "gwei"),
            replacementPriorityGwei: formatUnits(replacementQuote.maxPriorityFeePerGas, "gwei"),
          });
          const signedReplacement = await this.wallet.signTransaction(replacementTx);
          const replacement = await this.providerPool.broadcastSignedTransaction(signedReplacement);
          finalTxHash = replacement.hash;
          this.stateStore.appendSubmitted({
            timestamp: new Date().toISOString(),
            txHash: replacement.hash,
            nonce: nonce.toString(),
            epoch: epoch.toString(),
            gasLimit: replacementQuote.gasLimit.toString(),
            maxFeePerGas: replacementQuote.maxFeePerGas.toString(),
            maxPriorityFeePerGas: replacementQuote.maxPriorityFeePerGas.toString(),
            simulation: "success-replacement-sent",
          });
          receipt = await this.waitForReceipt(replacement);
        } else {
          logger.warn("Pending transaction not replaced because mine(nonce) no longer simulates successfully", {
            txHash: response.hash,
            nonce: nonce.toString(),
          });
        }
      } else {
        logger.warn("Pending transaction not replaced because bumped gas would exceed configured caps", {
          txHash: response.hash,
        });
      }
    }

    const gasCostWei =
      receipt?.gasUsed !== undefined && receipt?.fee !== undefined ? receipt.fee : undefined;
    if (gasCostWei !== undefined) {
      this.gasManager.recordSpend(finalTxHash, gasCostWei);
    }

    this.stateStore.appendSubmitted({
      timestamp: new Date().toISOString(),
      txHash: finalTxHash,
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
      txHash: finalTxHash,
      receiptStatus: receipt?.status ?? undefined,
      gasCostWei,
      gasQuote,
    };
  }

  private async simulateMine(
    callRequest: { to: string; from: string; data: string; value: bigint },
    nonce: bigint,
    epoch: bigint,
  ): Promise<
    | { ok: true; mode: "latest" | "pending" }
    | { ok: false; result: Extract<SubmissionResult, { sent: false }> }
  > {
    try {
      await this.providerPool.withProvider((provider) => provider.call(callRequest));
      logger.info("eth_call simulation succeeded", { nonce: nonce.toString(), epoch: epoch.toString() });
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
        nonce: nonce.toString(),
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
      await this.providerPool.withProvider((provider) =>
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
        nonce: nonce.toString(),
        epoch: epoch.toString(),
      });
      return true;
    } catch (error) {
      const reason = error instanceof Error ? error.message : String(error);
      logger.warn("pending eth_call simulation failed", {
        nonce: nonce.toString(),
        epoch: epoch.toString(),
        contractError: decodeHashContractError(error),
        reason,
      });
      return false;
    }
  }

  private async waitForReceipt(response: TransactionResponse): Promise<TransactionReceipt | null> {
    try {
      const receipt = await this.providerPool.waitForReceipt(response.hash, this.config.txConfirmations, 180_000);
      if (receipt) {
        return receipt;
      }
      logger.warn("Transaction still pending after wait window", { txHash: response.hash });
      return null;
    } catch (error) {
      logger.warn("Receipt wait failed or timed out", {
        txHash: response.hash,
        error: error instanceof Error ? error.message : String(error),
      });
      return null;
    }
  }

  private async stillSimulates(data: string): Promise<boolean> {
    try {
      await this.providerPool.withProvider((provider) =>
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
}
