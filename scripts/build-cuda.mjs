#!/usr/bin/env node
import { execFileSync } from "node:child_process";
import { existsSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const projectRoot = dirname(scriptDir);
const source = join(projectRoot, "gpu", "cuda_miner.cu");
const output = process.platform === "win32" ? join(projectRoot, "gpu", "cuda_miner.exe") : join(projectRoot, "gpu", "cuda_miner");

if (!existsSync(source)) {
  throw new Error(`CUDA source not found: ${source}`);
}

const archs = resolveArchitectures();
const nvccVersion = detectNvccVersion();
validateToolkitForArchitectures(archs, nvccVersion);

const args = ["-O3", "-std=c++17"];
for (const arch of archs) {
  args.push(`-gencode=arch=compute_${arch},code=sm_${arch}`);
  args.push(`-gencode=arch=compute_${arch},code=compute_${arch}`);
}
args.push("-o", output, source);

console.log(`Building CUDA worker for ${archs.map((arch) => `sm_${arch}`).join(", ")}`);
execFileSync("nvcc", args, { stdio: "inherit", cwd: projectRoot });
console.log(`Built ${output}`);

function resolveArchitectures() {
  const override = process.env.CUDA_ARCH?.trim();
  if (override) {
    const parsed = parseArchitectureList(override);
    if (parsed.length === 0) {
      throw new Error(`CUDA_ARCH is set but no valid architectures were found: ${override}`);
    }
    return dedupe(parsed);
  }

  try {
    const raw = execFileSync(
      "nvidia-smi",
      ["--query-gpu=compute_cap", "--format=csv,noheader,nounits"],
      { encoding: "utf8", cwd: projectRoot },
    );
    const detected = raw
      .split(/\r?\n/)
      .map((line) => line.trim())
      .filter(Boolean)
      .map(normalizeComputeCapability)
      .filter((arch) => Boolean(arch));
    if (detected.length > 0) {
      return dedupe(detected);
    }
  } catch {
    // Fall back below.
  }

  return ["89"];
}

function parseArchitectureList(raw) {
  return raw
    .split(/[\s,]+/)
    .map((value) => normalizeArchitectureToken(value))
    .filter((value) => Boolean(value));
}

function normalizeArchitectureToken(token) {
  const value = token.trim().toLowerCase();
  if (!value) {
    return "";
  }
  if (value.startsWith("sm_")) {
    return value.slice(3);
  }
  if (value.startsWith("compute_")) {
    return value.slice(8);
  }
  if (/^\d+(\.\d+)?$/.test(value)) {
    return normalizeComputeCapability(value);
  }
  if (/^\d+$/.test(value)) {
    return value;
  }
  return "";
}

function normalizeComputeCapability(value) {
  const match = value.match(/^(\d+)(?:\.(\d+))?$/);
  if (!match) {
    return "";
  }
  const major = Number(match[1]);
  const minor = Number(match[2] ?? "0");
  if (!Number.isInteger(major) || !Number.isInteger(minor)) {
    return "";
  }
  return `${major}${minor}`;
}

function detectNvccVersion() {
  try {
    const output = execFileSync("nvcc", ["--version"], { encoding: "utf8", cwd: projectRoot });
    const match = output.match(/release\s+(\d+)\.(\d+)/);
    if (!match) {
      return undefined;
    }
    return {
      major: Number(match[1]),
      minor: Number(match[2]),
    };
  } catch {
    return undefined;
  }
}

function validateToolkitForArchitectures(architectures, version) {
  const requiresBlackwellSupport = architectures.some((arch) => Number(arch) >= 100);
  if (!requiresBlackwellSupport) {
    return;
  }
  if (!version || version.major < 12 || (version.major === 12 && version.minor < 8)) {
    const label = version ? `${version.major}.${version.minor}` : "unknown";
    throw new Error(
      `Selected CUDA architectures ${architectures.map((arch) => `sm_${arch}`).join(", ")} require CUDA Toolkit 12.8+; current nvcc is ${label}.`,
    );
  }
}

function dedupe(values) {
  return [...new Set(values)];
}
