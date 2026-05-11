import { ethers, type InterfaceAbi, type JsonRpcProvider, type Wallet } from "ethers";
import { EXPECTED_CHAIN_ID, TARGET_CONTRACT } from "./safety.js";

export const HASH_ABI = [
  "function mine(uint256 nonce) external",
  "function getChallenge(address miner) view returns (bytes32)",
  "function miningState() view returns (uint256 era,uint256 reward,uint256 difficulty,uint256 minted,uint256 remaining,uint256 epoch,uint256 epochBlocksLeft)",
  "function genesisState() view returns (uint256 minted,uint256 remaining,uint256 ethRaised,bool complete)",
  "function epochBlocksLeft() view returns (uint256)",
  "function currentDifficulty() view returns (uint256)",
  "function totalMints() view returns (uint256)",
  "function totalMiningMinted() view returns (uint256)",
  "function MINING_SUPPLY() view returns (uint256)",
  "function MAX_MINTS_PER_BLOCK() view returns (uint256)",
  "function mintsInBlock(uint256 blockNumber) view returns (uint256)",
  "function usedProofs(bytes32 proof) view returns (bool)",
  "event Mined(address indexed miner, uint256 nonce, uint256 reward, uint256 era)",
  "error GenesisNotComplete()",
  "error SupplyExhausted()",
  "error BlockCapReached()",
  "error InsufficientWork()",
  "error ProofAlreadyUsed()",
  "error ReentrancyGuardReentrantCall()",
] as const satisfies InterfaceAbi;

export const hashInterface = new ethers.Interface(HASH_ABI);

export type MiningState = {
  era: bigint;
  reward: bigint;
  difficulty: bigint;
  minted: bigint;
  remaining: bigint;
  epoch: bigint;
  epochBlocksLeft: bigint;
};

export type GenesisState = {
  minted: bigint;
  remaining: bigint;
  ethRaised: bigint;
  complete: boolean;
};

export type ChainSnapshot = {
  blockNumber: number;
  challenge: string;
  mining: MiningState;
  genesis: GenesisState;
};

export class HashContractClient {
  readonly contract: ethers.Contract;

  constructor(
    readonly provider: JsonRpcProvider,
    signerOrProvider?: Wallet | JsonRpcProvider,
  ) {
    this.contract = new ethers.Contract(TARGET_CONTRACT, HASH_ABI, signerOrProvider ?? provider);
  }

  async assertNetwork(): Promise<void> {
    const network = await this.provider.getNetwork();
    if (network.chainId !== EXPECTED_CHAIN_ID) {
      throw new Error(`RPC is not Ethereum mainnet: expected chainId 1, got ${network.chainId.toString()}`);
    }
  }

  async readSnapshot(minerAddress: string): Promise<ChainSnapshot> {
    await this.assertNetwork();
    const [blockNumber, challenge, miningRaw, genesisRaw] = await Promise.all([
      this.provider.getBlockNumber(),
      this.contract.getChallenge(minerAddress) as Promise<string>,
      this.contract.miningState() as Promise<bigint[]>,
      this.contract.genesisState() as Promise<[bigint, bigint, bigint, boolean]>,
    ]);

    return {
      blockNumber,
      challenge,
      mining: {
        era: miningRaw[0],
        reward: miningRaw[1],
        difficulty: miningRaw[2],
        minted: miningRaw[3],
        remaining: miningRaw[4],
        epoch: miningRaw[5],
        epochBlocksLeft: miningRaw[6],
      },
      genesis: {
        minted: genesisRaw[0],
        remaining: genesisRaw[1],
        ethRaised: genesisRaw[2],
        complete: genesisRaw[3],
      },
    };
  }

  encodeMine(nonce: bigint): string {
    return hashInterface.encodeFunctionData("mine", [nonce]);
  }

  computeChallenge(minerAddress: string, epoch: bigint): string {
    return ethers.keccak256(
      ethers.AbiCoder.defaultAbiCoder().encode(
        ["uint256", "address", "address", "uint256"],
        [EXPECTED_CHAIN_ID, TARGET_CONTRACT, minerAddress, epoch],
      ),
    );
  }

  computeResult(challenge: string, nonce: bigint): string {
    return ethers.keccak256(
      ethers.AbiCoder.defaultAbiCoder().encode(["bytes32", "uint256"], [challenge, nonce]),
    );
  }

  computeProofKey(minerAddress: string, nonce: bigint, epoch: bigint): string {
    return ethers.keccak256(
      ethers.AbiCoder.defaultAbiCoder().encode(["address", "uint256", "uint256"], [minerAddress, nonce, epoch]),
    );
  }

  isHashValid(result: string, difficulty: bigint): boolean {
    return BigInt(result) < difficulty;
  }

  async isProofUsed(minerAddress: string, nonce: bigint, epoch: bigint): Promise<boolean> {
    const key = this.computeProofKey(minerAddress, nonce, epoch);
    return this.contract.usedProofs(key) as Promise<boolean>;
  }
}

export function compactHash(value: string): string {
  return `${value.slice(0, 10)}...${value.slice(-6)}`;
}

export function decodeHashContractError(error: unknown): string | undefined {
  const data = findRevertData(error);
  if (!data) {
    return undefined;
  }
  try {
    const parsed = hashInterface.parseError(data);
    return parsed?.name;
  } catch {
    return `Unknown(${data.slice(0, 10)})`;
  }
}

function findRevertData(value: unknown, depth = 0): string | undefined {
  if (depth > 8 || value === null || typeof value !== "object") {
    return undefined;
  }
  const record = value as Record<string, unknown>;

  // Direct hex data fields in ethers v6 CallExceptionError
  for (const key of ["data", "revert", "body"]) {
    const candidate = record[key];
    if (typeof candidate === "string" && /^0x[0-9a-fA-F]{8,}$/.test(candidate)) {
      return candidate;
    }
  }

  // ethers v6 sometimes nests the actual error response under .error
  // or wraps it in .info, .payload, .reason
  // Check all known ethers v6 nesting paths
  for (const key of ["error", "info", "payload", "reason", "shortMessage"]) {
    const nested = findRevertData(record[key], depth + 1);
    if (nested) {
      return nested;
    }
  }

  // Also check raw response body for error data field
  const response = record.body ?? record.response;
  if (typeof response === "object" && response !== null) {
    const respRecord = response as Record<string, unknown>;
    const respError = respRecord.error;
    const errData = (typeof respError === "object" && respError !== null
        ? (respError as Record<string, unknown>).data
        : undefined) ?? respRecord.result;
    if (typeof errData === "string" && /^0x[0-9a-fA-F]{8,}$/.test(errData)) {
      return errData;
    }
  }

  return undefined;
}
