import { existsSync, mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { formatUnits, parseUnits, type FeeData, type JsonRpcProvider } from "ethers";
import { DATA_DIR, type AppConfig } from "./config.js";
import { logger } from "./logger.js";

export type GasQuote = {
  gasLimit: bigint;
  baseFeePerGas: bigint;
  maxFeePerGas: bigint;
  maxPriorityFeePerGas: bigint;
  estimatedCostWei: bigint;
};

type GasSpendEntry = {
  timestamp: string;
  txHash: string;
  gasCostWei: string;
};

const spendPath = join(DATA_DIR, "gas_spend.jsonl");

export class GasManager {
  constructor(private readonly config: AppConfig) {
    mkdirSync(dirname(spendPath), { recursive: true });
  }

  async quote(provider: JsonRpcProvider, estimatedGas: bigint): Promise<GasQuote> {
    const [feeData, block] = await Promise.all([provider.getFeeData(), provider.getBlock("latest")]);
    const baseFeePerGas = block?.baseFeePerGas ?? feeData.gasPrice ?? parseUnits("1", "gwei");
    const priority = this.scalePriorityFee(feeData);
    const maxPriorityFeePerGas =
      priority > this.config.maxPriorityFeePerGasWei ? this.config.maxPriorityFeePerGasWei : priority;
    const computedMaxFee = baseFeePerGas * 2n + maxPriorityFeePerGas;
    const maxFeePerGas = computedMaxFee > this.config.maxFeePerGasWei ? this.config.maxFeePerGasWei : computedMaxFee;
    const gasLimit = estimatedGas > this.config.maxGasLimit ? this.config.maxGasLimit : estimatedGas;
    const estimatedCostWei = gasLimit * maxFeePerGas;

    return {
      gasLimit,
      baseFeePerGas,
      maxFeePerGas,
      maxPriorityFeePerGas,
      estimatedCostWei,
    };
  }

  canAffordQuote(quote: GasQuote): { ok: true } | { ok: false; reason: string } {
    if (quote.gasLimit > this.config.maxGasLimit) {
      return { ok: false, reason: `gasLimit ${quote.gasLimit.toString()} exceeds MAX_GAS_LIMIT` };
    }
    if (quote.maxFeePerGas > this.config.maxFeePerGasWei) {
      return { ok: false, reason: "maxFeePerGas exceeds MAX_FEE_GWEI_CAP" };
    }
    if (quote.maxPriorityFeePerGas > this.config.maxPriorityFeePerGasWei) {
      return { ok: false, reason: "maxPriorityFeePerGas exceeds MAX_PRIORITY_FEE_GWEI_CAP" };
    }
    if (quote.estimatedCostWei > this.config.maxEthPerTxWei) {
      return {
        ok: false,
        reason: `estimated cost ${formatUnits(quote.estimatedCostWei, 18)} ETH exceeds MAX_ETH_PER_TX`,
      };
    }

    const recent = this.recentHourlySpendWei();
    if (recent + quote.estimatedCostWei > this.config.maxEthPerHourWei) {
      return {
        ok: false,
        reason: `hourly gas budget would be exceeded: ${formatUnits(recent + quote.estimatedCostWei, 18)} ETH`,
      };
    }

    return { ok: true };
  }

  recordSpend(txHash: string, gasCostWei: bigint): void {
    const entry: GasSpendEntry = {
      timestamp: new Date().toISOString(),
      txHash,
      gasCostWei: gasCostWei.toString(),
    };
    writeFileSync(spendPath, `${JSON.stringify(entry)}\n`, { encoding: "utf8", flag: "a" });
  }

  recentHourlySpendWei(): bigint {
    if (!existsSync(spendPath)) {
      return 0n;
    }
    const cutoffMs = Date.now() - 60 * 60 * 1000;
    return readFileSync(spendPath, "utf8")
      .split(/\r?\n/)
      .filter(Boolean)
      .reduce((sum, line) => {
        try {
          const entry = JSON.parse(line) as GasSpendEntry;
          if (Date.parse(entry.timestamp) >= cutoffMs) {
            return sum + BigInt(entry.gasCostWei);
          }
        } catch (error) {
          logger.warn("Ignoring malformed gas spend entry", { line, error: String(error) });
        }
        return sum;
      }, 0n);
  }

  replacementQuote(previous: GasQuote): GasQuote | undefined {
    const bump = (value: bigint) => (value * 1125n + 999n) / 1000n;
    const maxFeePerGas = bump(previous.maxFeePerGas);
    const maxPriorityFeePerGas = bump(previous.maxPriorityFeePerGas);
    if (
      maxFeePerGas > this.config.maxFeePerGasWei ||
      maxPriorityFeePerGas > this.config.maxPriorityFeePerGasWei
    ) {
      return undefined;
    }
    const estimatedCostWei = previous.gasLimit * maxFeePerGas;
    if (estimatedCostWei > this.config.maxEthPerTxWei) {
      return undefined;
    }
    return {
      ...previous,
      maxFeePerGas,
      maxPriorityFeePerGas,
      estimatedCostWei,
    };
  }

  private scalePriorityFee(feeData: FeeData): bigint {
    const suggested = feeData.maxPriorityFeePerGas ?? feeData.gasPrice ?? parseUnits("0.1", "gwei");
    const scaled = Math.ceil(Number(suggested) * this.config.gasMultiplier);
    if (!Number.isSafeInteger(scaled)) {
      const numerator = BigInt(Math.round(this.config.gasMultiplier * 1000));
      return (suggested * numerator) / 1000n;
    }
    return BigInt(scaled);
  }
}
