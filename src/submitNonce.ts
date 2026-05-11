import { Wallet, formatEther, formatUnits } from "ethers";
import { loadConfig, formatEthWei } from "./config.js";
import { HashContractClient, compactHash } from "./contract.js";
import { GasManager } from "./gas.js";
import { logger } from "./logger.js";
import { RotatingProvider } from "./rpc.js";
import { MINE_SELECTOR, TARGET_CONTRACT } from "./safety.js";
import { MinerStateStore } from "./state.js";
import { TransactionSubmitter } from "./tx.js";

async function main(): Promise<void> {
  const nonceArg = process.argv[2];
  if (!nonceArg || !/^\d+$/.test(nonceArg)) {
    throw new Error("Usage: npm run submit-nonce -- <decimal_nonce>");
  }

  const nonce = BigInt(nonceArg);
  const config = loadConfig();
  const readPool = new RotatingProvider(config.readRpcUrls, config.rpcTimeoutMs);
  const txPool = new RotatingProvider(config.txRpcUrls, config.rpcTimeoutMs);
  const provider = readPool.current();
  const wallet = new Wallet(config.privateKey, provider);
  const stateStore = new MinerStateStore();
  const gasManager = new GasManager(config);
  const contractClient = new HashContractClient(provider, wallet);
  const submitter = new TransactionSubmitter(config, wallet, contractClient, gasManager, stateStore, readPool, txPool);

  const [balance, snapshot] = await Promise.all([
    readPool.withProvider((activeProvider) => activeProvider.getBalance(wallet.address)),
    readPool.withProvider((activeProvider) => {
      const activeClient = new HashContractClient(activeProvider, wallet.connect(activeProvider));
      return activeClient.readSnapshot(wallet.address);
    }),
  ]);

  logger.info("Manual nonce rescue check", {
    wallet: wallet.address,
    ethBalance: formatEther(balance),
    targetContract: TARGET_CONTRACT,
    selector: MINE_SELECTOR,
    dryRun: config.dryRun,
    autoSend: config.autoSend,
    nonce: nonce.toString(),
    currentEpoch: snapshot.mining.epoch.toString(),
    currentDifficulty: snapshot.mining.difficulty.toString(),
    challenge: compactHash(snapshot.challenge),
  });

  const result = contractClient.computeResult(snapshot.challenge, nonce);
  const valid = contractClient.isHashValid(result, snapshot.mining.difficulty);
  logger.info("Manual nonce local hash check", {
    nonce: nonce.toString(),
    result,
    difficulty: snapshot.mining.difficulty.toString(),
    valid,
    rewardHash: formatUnits(snapshot.mining.reward, 18),
    epochBlocksLeft: snapshot.mining.epochBlocksLeft.toString(),
    maxEthPerTx: formatEthWei(config.maxEthPerTxWei),
  });

  if (!valid) {
    throw new Error(
      "This nonce is not valid for the current wallet/challenge/epoch. If it was found in an older epoch or by another wallet, it cannot be submitted now.",
    );
  }

  const used = await contractClient.isProofUsed(wallet.address, nonce, snapshot.mining.epoch);
  if (used) {
    throw new Error("This proof is already marked used on-chain for the current wallet and epoch.");
  }

  const submission = await submitter.simulateAndMaybeSubmit(nonce, snapshot.mining.epoch);
  logger.info("Manual nonce submission result", submission);
}

main().catch((error) => {
  logger.error("Manual nonce submission failed", {
    error: error instanceof Error ? error.message : String(error),
    stack: error instanceof Error ? error.stack : undefined,
  });
  process.exitCode = 1;
});
