import { Wallet, formatEther, formatUnits } from "ethers";
import { loadConfig, formatEthWei } from "./config.js";
import { HashContractClient, compactHash } from "./contract.js";
import { GasManager } from "./gas.js";
import { logger, setConsoleLogging } from "./logger.js";
import { currentEpochHitProbabilityLabel } from "./miningMath.js";
import { MiningCoordinator, initialNonceStart } from "./miner.js";
import { RotatingProvider } from "./rpc.js";
import { EXPECTED_CHAIN_ID, MINE_SELECTOR, TARGET_CONTRACT, assertMainnet } from "./safety.js";
import { MinerStateStore } from "./state.js";
import { TerminalDashboard } from "./terminal.js";
import { TransactionSubmitter } from "./tx.js";

type PendingCandidate = {
  nonce: bigint;
  hash: string;
  epoch: bigint;
  worker: "cuda" | "cpu";
};

async function main(): Promise<void> {
  const config = loadConfig();
  const readPool = new RotatingProvider(config.readRpcUrls, config.rpcTimeoutMs);
  const txPool = new RotatingProvider(config.txRpcUrls, config.rpcTimeoutMs);
  const provider = readPool.current();
  const wallet = new Wallet(config.privateKey, provider);
  const stateStore = new MinerStateStore();
  const gasManager = new GasManager(config);
  const contractClient = new HashContractClient(provider, wallet);
  const coordinator = new MiningCoordinator(config, contractClient);
  let submitter = new TransactionSubmitter(config, wallet, contractClient, gasManager, stateStore, readPool, txPool);
  const dashboard = new TerminalDashboard(config);
  if (dashboard.enabled) {
    setConsoleLogging(false);
    dashboard.start();
  }

  let stopping = false;
  const saveAndStop = () => {
    stopping = true;
    coordinator.stop();
    dashboard.event("Shutdown requested");
    logger.info("Graceful shutdown requested");
  };
  process.once("SIGINT", saveAndStop);
  process.once("SIGTERM", saveAndStop);

  await readPool.withProvider((activeProvider) => new HashContractClient(activeProvider).assertNetwork());
  const network = await readPool.withProvider((activeProvider) => activeProvider.getNetwork());
  assertMainnet(network.chainId);
  const balance = await readPool.withProvider((activeProvider) => activeProvider.getBalance(wallet.address));
  dashboard.update({
    wallet: wallet.address,
    ethBalance: formatEther(balance),
    rpc: readPool.currentUrlLabel(),
    status: config.autoSend && !config.dryRun ? "自动发送已就绪" : "只模拟",
  });
  dashboard.event("挖矿器已启动");

  logger.info("Startup safety summary", {
    wallet: wallet.address,
    ethBalance: formatEther(balance),
    targetContract: TARGET_CONTRACT,
    chainId: network.chainId.toString(),
    mineSelector: MINE_SELECTOR,
    dryRun: config.dryRun,
    autoSend: config.autoSend,
    maxGasLimit: config.maxGasLimit.toString(),
    maxFeeGweiCap: config.maxFeeGweiCap,
    maxPriorityFeeGweiCap: config.maxPriorityFeeGweiCap,
    maxEthPerTx: formatEthWei(config.maxEthPerTxWei),
    maxEthPerHour: formatEthWei(config.maxEthPerHourWei),
  });

  if (!config.autoSend || config.dryRun) {
    logger.warn("Live sending is disabled. Set DRY_RUN=false and AUTO_SEND=true to allow guarded real transactions.");
    dashboard.event("真实交易发送未开启");
  } else {
    dashboard.event("AUTO_SEND 已开启，找到有效 nonce 后会按安全上限自动提交");
  }

  let saved = stateStore.loadLastState();
  let nonceStart = initialNonceStart(saved?.nextNonceStart);
  let currentEpoch = saved?.epoch ? BigInt(saved.epoch) : undefined;
  let totalScanned = saved?.scanned ? BigInt(saved.scanned) : 0n;
  let lastHashrateHps: number | undefined;
  let lastGasLog = 0;
  let pendingCandidate: PendingCandidate | undefined;

  while (!stopping) {
    const snapshot = await readPool.withProvider((activeProvider) => {
      const activeClient = new HashContractClient(activeProvider, wallet.connect(activeProvider));
      return activeClient.readSnapshot(wallet.address);
    });
    const activeProvider = readPool.current();
    const activeWallet = wallet.connect(activeProvider);
    const activeContractClient = new HashContractClient(activeProvider, activeWallet);
    submitter = new TransactionSubmitter(config, activeWallet, activeContractClient, gasManager, stateStore, readPool, txPool);
    dashboard.update({ rpc: readPool.currentUrlLabel() });

    if (currentEpoch === undefined || currentEpoch !== snapshot.mining.epoch) {
      logger.info("Epoch changed; discarding old nonce range", {
        oldEpoch: currentEpoch?.toString(),
        newEpoch: snapshot.mining.epoch.toString(),
        blockNumber: snapshot.blockNumber,
      });
      currentEpoch = snapshot.mining.epoch;
      nonceStart = initialNonceStart();
      pendingCandidate = undefined;
      dashboard.event(`Epoch 已切换为 ${snapshot.mining.epoch.toString()}`);
    }

    const locallyComputedChallenge = activeContractClient.computeChallenge(wallet.address, snapshot.mining.epoch);
    if (locallyComputedChallenge.toLowerCase() !== snapshot.challenge.toLowerCase()) {
      throw new Error(
        `Challenge mismatch. local=${locallyComputedChallenge}, chain=${snapshot.challenge}. Refusing to mine.`,
      );
    }

    logger.info("Mining state", {
      blockNumber: snapshot.blockNumber,
      genesisComplete: snapshot.genesis.complete,
      epoch: snapshot.mining.epoch.toString(),
      epochBlocksLeft: snapshot.mining.epochBlocksLeft.toString(),
      difficulty: snapshot.mining.difficulty.toString(),
      challenge: compactHash(snapshot.challenge),
      rewardHash: formatUnits(snapshot.mining.reward, 18),
      remainingHash: formatUnits(snapshot.mining.remaining, 18),
      nonceStart: nonceStart.toString(),
    });
    dashboard.update({
      blockNumber: snapshot.blockNumber,
      genesisComplete: snapshot.genesis.complete,
      epoch: snapshot.mining.epoch.toString(),
      epochBlocksLeft: snapshot.mining.epochBlocksLeft.toString(),
      epochProbability: currentEpochHitProbabilityLabel(
        lastHashrateHps,
        snapshot.mining.difficulty,
        snapshot.mining.epochBlocksLeft,
      ),
      difficulty: snapshot.mining.difficulty.toString(),
      challenge: compactHash(snapshot.challenge),
      rewardHash: formatUnits(snapshot.mining.reward, 18),
      remainingHash: formatUnits(snapshot.mining.remaining, 18),
      nonceStart: nonceStart.toString(),
      nonceEnd: (nonceStart + config.nonceBatchSize).toString(),
      status: "挖矿中",
    });

    if (!snapshot.genesis.complete) {
      logger.warn("Mining is not open: genesisComplete=false. Waiting before next read.");
      dashboard.update({ status: "等待创世完成", epochProbability: "-" });
      dashboard.event("挖矿尚未开放");
      await sleep(config.epochRefreshMs);
      continue;
    }

    if (snapshot.mining.remaining <= 0n || snapshot.mining.reward <= 0n) {
      logger.warn("Mining supply or reward exhausted. Exiting.");
      dashboard.update({ status: "挖矿供应或奖励已耗尽", epochProbability: "-" });
      dashboard.event("挖矿供应或奖励已耗尽");
      break;
    }

    if (pendingCandidate) {
      logger.info("Retrying pending candidate nonce", {
        nonce: pendingCandidate.nonce.toString(),
        epoch: pendingCandidate.epoch.toString(),
        blockNumber: snapshot.blockNumber,
        epochBlocksLeft: snapshot.mining.epochBlocksLeft.toString(),
      });
      dashboard.update({
        lastCandidate: `${pendingCandidate.nonce.toString()} ${compactHash(pendingCandidate.hash)}`,
        status: "候选等待新区块重试",
      });
      dashboard.event(`重试候选 nonce：${pendingCandidate.nonce.toString()}`);

      const retryHandled = await handleCandidate(
        pendingCandidate,
        snapshot,
        activeContractClient,
        submitter,
        wallet.address,
        dashboard,
        stateStore,
        totalScanned,
      );
      if (retryHandled.action === "sent-or-final") {
        pendingCandidate = undefined;
        nonceStart = pendingCandidateNextNonce(retryHandled.nonce);
        stateStore.saveLastState({
          epoch: retryHandled.epoch.toString(),
          nextNonceStart: nonceStart.toString(),
          scanned: totalScanned.toString(),
        });
      } else if (retryHandled.action === "retry-later") {
        await sleepUntilNextBlock(readPool, snapshot.blockNumber, config.epochRefreshMs);
        continue;
      } else {
        pendingCandidate = undefined;
        currentEpoch = retryHandled.currentEpoch;
        nonceStart = initialNonceStart();
        continue;
      }
    }

    const nonceEnd = nonceStart + config.nonceBatchSize;
    const job = {
      challenge: snapshot.challenge,
      difficulty: snapshot.mining.difficulty,
      minerAddress: wallet.address,
      epoch: snapshot.mining.epoch,
      nonceStart,
      nonceEnd,
    };

    let batchScanned = 0n;
    const hit = await coordinator.search(job, (progress) => {
      batchScanned = progress.scanned;
      lastHashrateHps = progress.hashrate;
      const displayedTotalScanned = totalScanned + batchScanned;
      const epochProbability = currentEpochHitProbabilityLabel(
        progress.hashrate,
        snapshot.mining.difficulty,
        snapshot.mining.epochBlocksLeft,
      );
      dashboard.update({
        worker: progress.gpuUtilization === undefined ? "cpu" : "cuda",
        hashrateHps: Math.round(progress.hashrate),
        gpuUtilization: progress.gpuUtilization ?? "n/a",
        scannedBatch: progress.scanned.toString(),
        scannedTotal: displayedTotalScanned.toString(),
        epochProbability,
      });
      const now = Date.now();
      if (now - lastGasLog >= config.gasRefreshMs) {
        lastGasLog = now;
        logger.info("Mining progress", {
          hashrateHps: Math.round(progress.hashrate),
          hashrateMHps: (progress.hashrate / 1_000_000).toFixed(4),
          gpuUtilization: progress.gpuUtilization ?? "n/a",
          scannedBatch: progress.scanned.toString(),
          scannedTotal: displayedTotalScanned.toString(),
          epoch: snapshot.mining.epoch.toString(),
          difficulty: snapshot.mining.difficulty.toString(),
          epochProbability,
        });
      }
    });

    if (stopping) {
      break;
    }

    if (!hit) {
      nonceStart = nonceEnd;
      totalScanned += config.nonceBatchSize;
      stateStore.saveLastState({
        epoch: snapshot.mining.epoch.toString(),
        nextNonceStart: nonceStart.toString(),
        scanned: totalScanned.toString(),
      });
      continue;
    }

    const candidate = {
      nonce: hit.nonce,
      hash: hit.hash,
      epoch: hit.epoch,
      worker: hit.worker,
    } satisfies PendingCandidate;

    logger.info("Candidate nonce found", {
      worker: hit.worker,
      nonce: hit.nonce.toString(),
      hash: hit.hash,
      epoch: hit.epoch.toString(),
      hashrateHps: Math.round(hit.hashrate),
    });
    totalScanned += hit.scanned;
    dashboard.update({
      lastCandidate: `${hit.nonce.toString()} ${compactHash(hit.hash)}`,
      worker: hit.worker,
      hashrateHps: Math.round(hit.hashrate),
      status: "找到候选，正在复核",
    });
    dashboard.event(`找到候选 nonce：${hit.nonce.toString()}`);

    const handled = await handleCandidate(
      candidate,
      undefined,
      activeContractClient,
      submitter,
      wallet.address,
      dashboard,
      stateStore,
      totalScanned,
    );
    if (handled.action === "retry-later") {
      pendingCandidate = candidate;
      dashboard.update({ status: "本区块满额，等待下一区块重试" });
      dashboard.event("本区块 mint 名额已满，下一区块自动重试");
      await sleepUntilNextBlock(readPool, snapshot.blockNumber, config.epochRefreshMs);
      continue;
    }
    if (handled.action === "epoch-changed") {
      currentEpoch = handled.currentEpoch;
      nonceStart = initialNonceStart();
      continue;
    }
    nonceStart = hit.nonce + 1n;
    stateStore.saveLastState({
      epoch: hit.epoch.toString(),
      nextNonceStart: nonceStart.toString(),
      scanned: totalScanned.toString(),
    });
  }

  stateStore.saveLastState({
    epoch: currentEpoch?.toString() ?? "0",
    nextNonceStart: nonceStart.toString(),
    scanned: totalScanned.toString(),
  });
  dashboard.stop();
}

function sleep(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

main().catch((error) => {
  logger.error("Fatal error", {
    error: error instanceof Error ? error.message : String(error),
    stack: error instanceof Error ? error.stack : undefined,
  });
  process.exitCode = 1;
});

type CandidateAction =
  | { action: "sent-or-final"; nonce: bigint; epoch: bigint }
  | { action: "retry-later"; nonce: bigint; epoch: bigint }
  | { action: "epoch-changed"; currentEpoch: bigint };

async function handleCandidate(
  candidate: PendingCandidate,
  suppliedSnapshot: Awaited<ReturnType<HashContractClient["readSnapshot"]>> | undefined,
  contractClient: HashContractClient,
  submitter: TransactionSubmitter,
  walletAddress: string,
  dashboard: TerminalDashboard,
  stateStore: MinerStateStore,
  totalScanned: bigint,
): Promise<CandidateAction> {
  const freshSnapshot = suppliedSnapshot ?? (await contractClient.readSnapshot(walletAddress));
  if (freshSnapshot.mining.epoch !== candidate.epoch) {
    logger.warn("Candidate nonce expired due to epoch change; discarding", {
      hitEpoch: candidate.epoch.toString(),
      currentEpoch: freshSnapshot.mining.epoch.toString(),
    });
    dashboard.update({ status: "候选已过期" });
    dashboard.event("候选已丢弃：epoch 已变化");
    return { action: "epoch-changed", currentEpoch: freshSnapshot.mining.epoch };
  }

  const result = contractClient.computeResult(freshSnapshot.challenge, candidate.nonce);
  if (!contractClient.isHashValid(result, freshSnapshot.mining.difficulty)) {
    logger.warn("Candidate failed local recheck against fresh difficulty; discarding", {
      result,
      difficulty: freshSnapshot.mining.difficulty.toString(),
    });
    dashboard.update({ status: "候选本地复核失败" });
    dashboard.event("候选已丢弃：本地复核失败");
    return { action: "sent-or-final", nonce: candidate.nonce, epoch: candidate.epoch };
  }

  const used = await contractClient.isProofUsed(walletAddress, candidate.nonce, candidate.epoch);
  if (used) {
    logger.warn("Candidate proof already used on-chain; discarding", { nonce: candidate.nonce.toString() });
    dashboard.update({ status: "候选已被使用" });
    dashboard.event("候选已丢弃：链上已使用");
    return { action: "sent-or-final", nonce: candidate.nonce, epoch: candidate.epoch };
  }

  const submission = await submitter.simulateAndMaybeSubmit(candidate.nonce, candidate.epoch);
  logger.info("Submission result", submission);
  dashboard.update({
    lastSubmission: submission.sent ? `sent ${submission.txHash}` : submission.reason,
    lastTxHash: submission.sent ? submission.txHash : undefined,
    status: submission.sent ? `交易已发送 status=${submission.receiptStatus ?? "pending"}` : "挖矿中",
  });
  dashboard.event(submission.sent ? `交易已发送：${submission.txHash}` : `未发送：${submission.reason}`);

  if (!submission.sent && submission.retryable && submission.contractError === "BlockCapReached") {
    return { action: "retry-later", nonce: candidate.nonce, epoch: candidate.epoch };
  }

  stateStore.saveLastState({
    epoch: candidate.epoch.toString(),
    nextNonceStart: (candidate.nonce + 1n).toString(),
    scanned: totalScanned.toString(),
  });
  return { action: "sent-or-final", nonce: candidate.nonce, epoch: candidate.epoch };
}

async function sleepUntilNextBlock(
  readPool: RotatingProvider,
  previousBlockNumber: number,
  fallbackMs: number,
): Promise<void> {
  const deadline = Date.now() + 18_000;
  while (Date.now() < deadline) {
    try {
      const blockNumber = await readPool.withProvider((provider) => provider.getBlockNumber());
      if (blockNumber > previousBlockNumber) {
        return;
      }
    } catch (error) {
      logger.warn("Block wait RPC check failed", { error: error instanceof Error ? error.message : String(error) });
    }
    await sleep(750);
  }
  await sleep(fallbackMs);
}

function pendingCandidateNextNonce(nonce: bigint): bigint {
  return nonce + 1n;
}
