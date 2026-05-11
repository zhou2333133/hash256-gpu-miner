const AVERAGE_BLOCK_SECONDS = 12;
const TWO_POW_256 = 2 ** 256;

export function estimateEpochHitProbability(
  hashrateHps: number | undefined,
  difficulty: bigint,
  epochBlocksLeft: bigint,
  blockSeconds = AVERAGE_BLOCK_SECONDS,
): number {
  if (hashrateHps === undefined || !Number.isFinite(hashrateHps) || hashrateHps <= 0) {
    return 0;
  }
  if (difficulty <= 0n || epochBlocksLeft <= 0n) {
    return 0;
  }

  const perHashProbability = Number(difficulty) / TWO_POW_256;
  if (!Number.isFinite(perHashProbability) || perHashProbability <= 0) {
    return 0;
  }

  const remainingSeconds = Number(epochBlocksLeft) * blockSeconds;
  const expectedHits = hashrateHps * remainingSeconds * perHashProbability;
  const probability = 1 - Math.exp(-expectedHits);
  return clamp(probability, 0, 1);
}

export function formatEpochHitProbability(probability: number): string {
  if (!Number.isFinite(probability) || probability <= 0) {
    return "0.00%";
  }
  const percent = probability * 100;
  return `${percent.toFixed(percent < 0.1 ? 4 : 2)}%`;
}

export function currentEpochHitProbabilityLabel(
  hashrateHps: number | undefined,
  difficulty: bigint,
  epochBlocksLeft: bigint,
): string {
  if (hashrateHps === undefined || hashrateHps <= 0) {
    return "-";
  }
  return formatEpochHitProbability(estimateEpochHitProbability(hashrateHps, difficulty, epochBlocksLeft));
}

function clamp(value: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, value));
}
