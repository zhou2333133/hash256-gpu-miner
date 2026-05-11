import { existsSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { config as loadDotenv } from "dotenv";
import { formatUnits, getAddress, parseEther, parseUnits } from "ethers";
import { assertTargetContract, EXPECTED_CHAIN_ID } from "./safety.js";

const moduleRoot = dirname(dirname(fileURLToPath(import.meta.url)));
export const PROJECT_ROOT = existsSync(join(process.cwd(), "package.json")) ? process.cwd() : moduleRoot;
export const DATA_DIR = join(PROJECT_ROOT, "data");
export const LOG_DIR = join(PROJECT_ROOT, "logs");
const DEFAULT_PUBLIC_MAINNET_RPCS = [
  "https://ethereum.publicnode.com",
  "https://ethereum-rpc.publicnode.com",
  "https://eth-mainnet.public.blastapi.io",
];

const envPath = join(PROJECT_ROOT, ".env");
if (existsSync(envPath)) {
  loadDotenv({ path: envPath });
} else {
  loadDotenv();
}

export type AppConfig = {
  rpcUrls: string[];
  privateKey: string;
  contractAddress: string;
  chainId: bigint;
  dryRun: boolean;
  autoSend: boolean;
  maxFeeGweiCap: string;
  maxPriorityFeeGweiCap: string;
  maxFeePerGasWei: bigint;
  maxPriorityFeePerGasWei: bigint;
  gasMultiplier: number;
  maxGasLimit: bigint;
  maxEthPerTxWei: bigint;
  maxEthPerHourWei: bigint;
  epochRefreshMs: number;
  gasRefreshMs: number;
  txConfirmations: number;
  cudaDeviceId: number;
  nonceBatchSize: bigint;
  cudaWorkerPath: string;
  terminalDashboard: boolean;
  rpcTimeoutMs: number;
};

function readString(name: string, fallback?: string): string {
  const value = process.env[name]?.trim();
  if (value) {
    return value;
  }
  if (fallback !== undefined) {
    return fallback;
  }
  throw new Error(`Missing required env var ${name}`);
}

function readOptionalString(name: string): string | undefined {
  const value = process.env[name]?.trim();
  return value ? value : undefined;
}

function readBool(name: string, fallback: boolean): boolean {
  const value = process.env[name]?.trim().toLowerCase();
  if (!value) {
    return fallback;
  }
  if (value === "true") {
    return true;
  }
  if (value === "false") {
    return false;
  }
  throw new Error(`${name} must be true or false`);
}

function readNumber(name: string, fallback: number): number {
  const raw = process.env[name]?.trim();
  if (!raw) {
    return fallback;
  }
  const value = Number(raw);
  if (!Number.isFinite(value)) {
    throw new Error(`${name} must be a finite number`);
  }
  return value;
}

function readPositiveInt(name: string, fallback: number): number {
  const value = readNumber(name, fallback);
  if (!Number.isInteger(value) || value <= 0) {
    throw new Error(`${name} must be a positive integer`);
  }
  return value;
}

function readNonNegativeInt(name: string, fallback: number): number {
  const value = readNumber(name, fallback);
  if (!Number.isInteger(value) || value < 0) {
    throw new Error(`${name} must be a non-negative integer`);
  }
  return value;
}

function normalizePrivateKey(raw: string): string {
  const privateKey = raw.trim();
  if (!/^0x[0-9a-fA-F]{64}$/.test(privateKey)) {
    throw new Error("MINER_PRIVATE_KEY must be a 0x-prefixed 32-byte private key. Never use a seed phrase.");
  }
  return privateKey;
}

export function loadConfig(): AppConfig {
  const configuredRpcUrls = [
    readOptionalString("RPC_URL_PRIMARY"),
    readOptionalString("RPC_URL_BACKUP_1"),
    readOptionalString("RPC_URL_BACKUP_2"),
  ].filter((url): url is string => Boolean(url));
  const rpcUrls = dedupe(configuredRpcUrls.length > 0 ? [...configuredRpcUrls, ...DEFAULT_PUBLIC_MAINNET_RPCS] : DEFAULT_PUBLIC_MAINNET_RPCS);

  if (rpcUrls.length === 0) {
    throw new Error("At least one RPC_URL_* is required");
  }

  const contractAddress = assertTargetContract(
    getAddress(readString("CONTRACT_ADDRESS", "0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc")),
  );
  const chainId = BigInt(readString("CHAIN_ID", EXPECTED_CHAIN_ID.toString()));
  if (chainId !== EXPECTED_CHAIN_ID) {
    throw new Error(`CHAIN_ID must be ${EXPECTED_CHAIN_ID.toString()}`);
  }

  const dryRun = readBool("DRY_RUN", true);
  const autoSend = readBool("AUTO_SEND", false);
  if (autoSend && dryRun) {
    throw new Error("AUTO_SEND=true requires DRY_RUN=false. Refusing ambiguous live mode.");
  }

  const gasMultiplier = readNumber("GAS_MULTIPLIER", 1.15);
  if (gasMultiplier <= 0 || gasMultiplier > 10) {
    throw new Error("GAS_MULTIPLIER must be > 0 and <= 10");
  }

  const maxFeeGweiCap = readString("MAX_FEE_GWEI_CAP", "3");
  const maxPriorityFeeGweiCap = readString("MAX_PRIORITY_FEE_GWEI_CAP", "0.5");
  const maxGasLimit = BigInt(readPositiveInt("MAX_GAS_LIMIT", 150000));
  const nonceBatchSize = BigInt(readPositiveInt("NONCE_BATCH_SIZE", 1_000_000_000));
  const cudaWorkerPath = process.env.CUDA_WORKER_PATH?.trim() || defaultCudaWorkerPath();

  return {
    rpcUrls,
    privateKey: normalizePrivateKey(readString("MINER_PRIVATE_KEY")),
    contractAddress,
    chainId,
    dryRun,
    autoSend,
    maxFeeGweiCap,
    maxPriorityFeeGweiCap,
    maxFeePerGasWei: parseUnits(maxFeeGweiCap, "gwei"),
    maxPriorityFeePerGasWei: parseUnits(maxPriorityFeeGweiCap, "gwei"),
    gasMultiplier,
    maxGasLimit,
    maxEthPerTxWei: parseEther(readString("MAX_ETH_PER_TX", "0.0002")),
    maxEthPerHourWei: parseEther(readString("MAX_ETH_PER_HOUR", "0.002")),
    epochRefreshMs: readPositiveInt("EPOCH_REFRESH_SECONDS", 5) * 1000,
    gasRefreshMs: readPositiveInt("GAS_REFRESH_SECONDS", 10) * 1000,
    txConfirmations: readPositiveInt("TX_CONFIRMATIONS", 1),
    cudaDeviceId: readNonNegativeInt("CUDA_DEVICE_ID", 0),
    nonceBatchSize,
    cudaWorkerPath,
    terminalDashboard: readBool("TERMINAL_DASHBOARD", true),
    rpcTimeoutMs: readPositiveInt("RPC_TIMEOUT_MS", 8000),
  };
}

function defaultCudaWorkerPath(): string {
  const exe = process.platform === "win32" ? "cuda_miner.exe" : "cuda_miner";
  return join(PROJECT_ROOT, "gpu", exe);
}

export function formatEthWei(wei: bigint): string {
  return `${formatUnits(wei, 18)} ETH`;
}

function dedupe(values: string[]): string[] {
  const seen = new Set<string>();
  const output: string[] = [];
  for (const value of values) {
    if (!seen.has(value)) {
      seen.add(value);
      output.push(value);
    }
  }
  return output;
}
