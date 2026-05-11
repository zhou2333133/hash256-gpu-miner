import { getAddress, id, toBeHex, type TransactionRequest } from "ethers";

export const TARGET_CONTRACT = getAddress("0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc");
export const EXPECTED_CHAIN_ID = 1n;
export const MINE_SELECTOR = id("mine(uint256)").slice(0, 10);

const FORBIDDEN_SIGNATURES = [
  "approve(address,uint256)",
  "permit(address,address,uint256,uint256,uint8,bytes32,bytes32)",
  "transfer(address,uint256)",
  "transferFrom(address,address,uint256)",
  "setApprovalForAll(address,bool)",
  "mintGenesis(uint256)",
  "swap(address,bool,int256,uint160,bytes)",
  "addLiquidity(address,uint256,uint256)",
  "multicall(bytes[])",
];

export const FORBIDDEN_SELECTORS = new Set(
  FORBIDDEN_SIGNATURES.map((signature) => id(signature).slice(0, 10).toLowerCase()),
);

export function assertTargetContract(configuredAddress: string): string {
  const normalized = getAddress(configuredAddress);
  if (normalized !== TARGET_CONTRACT) {
    throw new Error(`CONTRACT_ADDRESS mismatch: expected ${TARGET_CONTRACT}, got ${normalized}`);
  }
  return normalized;
}

export function assertMainnet(chainId: bigint | number): void {
  const actual = BigInt(chainId);
  if (actual !== EXPECTED_CHAIN_ID) {
    throw new Error(`Unsafe chainId: expected ${EXPECTED_CHAIN_ID.toString()}, got ${actual.toString()}`);
  }
}

export function assertAllowedCalldata(data: string): void {
  if (!data || !data.startsWith("0x") || data.length < 10) {
    throw new Error("Transaction data is missing or malformed");
  }
  const selector = data.slice(0, 10).toLowerCase();
  if (selector !== MINE_SELECTOR.toLowerCase()) {
    throw new Error(`Unsafe method selector: expected ${MINE_SELECTOR}, got ${selector}`);
  }
  if (FORBIDDEN_SELECTORS.has(selector)) {
    throw new Error(`Forbidden method selector attempted: ${selector}`);
  }
}

export function assertZeroValue(value: unknown): void {
  if (value === undefined || value === null) {
    return;
  }
  const valueBigInt = BigInt(value.toString());
  if (valueBigInt !== 0n) {
    throw new Error(`Unsafe transaction value: expected 0, got ${valueBigInt.toString()}`);
  }
}

export type SafetyLimits = {
  maxGasLimit: bigint;
  maxFeePerGas: bigint;
  maxPriorityFeePerGas: bigint;
};

export function assertSafeTransactionRequest(
  tx: TransactionRequest,
  limits: SafetyLimits,
  expectedChainId = EXPECTED_CHAIN_ID,
): void {
  if (!tx.to) {
    throw new Error("Unsafe transaction: missing to");
  }
  const to = getAddress(tx.to.toString());
  if (to !== TARGET_CONTRACT) {
    throw new Error(`Unsafe transaction target: expected ${TARGET_CONTRACT}, got ${to}`);
  }

  const data = typeof tx.data === "string" ? tx.data : toBeHex(tx.data ?? "0x");
  assertAllowedCalldata(data);
  assertZeroValue(tx.value);

  if (tx.chainId === undefined || tx.chainId === null) {
    throw new Error("Unsafe transaction: missing chainId");
  }
  const chainId = BigInt(tx.chainId.toString());
  assertMainnet(chainId);
  if (chainId !== expectedChainId) {
    throw new Error(`Unsafe transaction: unexpected chainId ${chainId.toString()}`);
  }

  if (tx.type !== 2) {
    throw new Error(`Unsafe transaction type: expected EIP-1559 type 2, got ${String(tx.type)}`);
  }
  if (tx.gasLimit === undefined || tx.gasLimit === null) {
    throw new Error("Unsafe transaction: missing gasLimit");
  }
  const gasLimit = BigInt(tx.gasLimit.toString());
  if (gasLimit > limits.maxGasLimit) {
    throw new Error(`gasLimit exceeds cap: ${gasLimit.toString()} > ${limits.maxGasLimit.toString()}`);
  }

  if (
    tx.maxFeePerGas === undefined ||
    tx.maxFeePerGas === null ||
    tx.maxPriorityFeePerGas === undefined ||
    tx.maxPriorityFeePerGas === null
  ) {
    throw new Error("Unsafe transaction: missing EIP-1559 fee fields");
  }
  const maxFeePerGas = BigInt(tx.maxFeePerGas.toString());
  const maxPriorityFeePerGas = BigInt(tx.maxPriorityFeePerGas.toString());
  if (maxFeePerGas > limits.maxFeePerGas) {
    throw new Error(`maxFeePerGas exceeds cap: ${maxFeePerGas.toString()} > ${limits.maxFeePerGas.toString()}`);
  }
  if (maxPriorityFeePerGas > limits.maxPriorityFeePerGas) {
    throw new Error(
      `maxPriorityFeePerGas exceeds cap: ${maxPriorityFeePerGas.toString()} > ${limits.maxPriorityFeePerGas.toString()}`,
    );
  }
}
