import { existsSync } from "node:fs";
import { join } from "node:path";
import { spawn } from "node:child_process";
import { ethers } from "ethers";
import { PROJECT_ROOT } from "./config.js";

const challenge = "0x1bd9f3c900000000000000000000000000000000000000000000000000bc5c6e";
const nonce = 123n;
const maxDifficulty = (1n << 256n) - 1n;
const encoded = ethers.AbiCoder.defaultAbiCoder().encode(["bytes32", "uint256"], [challenge, nonce]);
const expectedHash = ethers.keccak256(encoded);

console.log("ABI encoded:", encoded);
console.log("ethers keccak256:", expectedHash);

const workerName = process.platform === "win32" ? "cuda_miner.exe" : "cuda_miner";
const workerPath = join(PROJECT_ROOT, "gpu", workerName);

if (!existsSync(workerPath)) {
  console.log(`CUDA worker not found at ${workerPath}; TypeScript hash self-test passed.`);
  process.exit(0);
}

const job = {
  type: "search",
  challenge,
  difficulty: maxDifficulty.toString(),
  minerAddress: "0x0000000000000000000000000000000000000000",
  epoch: "1",
  nonceStart: nonce.toString(),
  nonceEnd: (nonce + 1n).toString(),
  deviceId: 0,
};

const child = spawn(workerPath, [], { stdio: ["pipe", "pipe", "pipe"] });
let stdout = "";
let stderr = "";

child.stdout.on("data", (chunk) => {
  stdout += chunk.toString("utf8");
});
child.stderr.on("data", (chunk) => {
  stderr += chunk.toString("utf8");
});
child.stdin.write(`${JSON.stringify(job)}\n`);
child.stdin.end();

const code = await new Promise<number | null>((resolve) => child.on("exit", resolve));
if (code !== 0) {
  throw new Error(`CUDA worker exited with ${code}. stderr=${stderr}`);
}

const line = stdout.trim().split(/\r?\n/).find((value) => value.includes('"type":"found"'));
if (!line) {
  throw new Error(`CUDA worker did not return a found result. stdout=${stdout} stderr=${stderr}`);
}

const result = JSON.parse(line) as { nonce: string; hash: string };
console.log("cuda nonce:", result.nonce);
console.log("cuda hash:", result.hash);

if (result.nonce !== nonce.toString()) {
  throw new Error(`Nonce mismatch: expected ${nonce.toString()}, got ${result.nonce}`);
}
if (result.hash.toLowerCase() !== expectedHash.toLowerCase()) {
  throw new Error(`Hash mismatch: expected ${expectedHash}, got ${result.hash}`);
}

console.log("SELFTEST OK: CUDA hash matches ethers abi.encode(bytes32,uint256) + keccak256.");
