# HASH256 NVIDIA GPU Miner

这是一个安全优先的 HASH256 挖矿程序骨架：Node.js / TypeScript 负责链上读取、状态调度、签名、Gas 风控和硬编码安全闸门；CUDA worker 只负责搜索 nonce，不接触私钥。

目标合约：`0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc`  
网络：Ethereum Mainnet，chainId `1`  
唯一发送函数：`mine(uint256)`，selector `0x4d474898`

## 安装依赖

```bash
cd hash256-gpu-miner
npm install
npm run check
```

## Windows + WSL + NVIDIA CUDA

推荐在 WSL2 Ubuntu 中运行 CUDA worker：

1. 安装最新版 NVIDIA Windows 驱动，确认支持 WSL CUDA。
2. 在 WSL 中安装 CUDA Toolkit，并确认：

```bash
nvidia-smi
nvcc --version
```

3. 编译 CUDA worker：

```bash
npm run build:cuda
```

Windows 原生 PowerShell 也可以尝试：

```powershell
npm run build:cuda:win
```

如果没有 CUDA 或 worker 未编译，程序会使用 CPU 回退 worker。CPU 回退用于验证流程，不适合高性能挖矿。

## 配置 .env

```bash
cp .env.example .env
```

最简只需要填一个变量：

```env
MINER_PRIVATE_KEY=0x...
```

`.env` 已加入 `.gitignore`。不要使用主钱包，不要填写助记词，只使用新钱包私钥，并且只放少量 ETH 作为 gas。

默认内置多个公共主网 RPC，并会在读取、模拟、估 Gas、广播交易时自动轮换。当前内置：

```text
https://ethereum.publicnode.com
https://ethereum-rpc.publicnode.com
https://eth-mainnet.public.blastapi.io
```

如果你想加自己的 RPC，再额外加；你的 RPC 会排在默认公共 RPC 前面：

```env
RPC_URL_PRIMARY=https://your-mainnet-rpc
RPC_URL_BACKUP_1=
RPC_URL_BACKUP_2=
```

其他安全参数都有内置默认值，需要改 Gas 上限或开启自动发送时再加。

## 先 dry run

默认配置：

```env
DRY_RUN=true
AUTO_SEND=false
```

运行：

```bash
npm run dev
```

dry run 会：

- 打印钱包地址、ETH 余额、目标合约地址、chainId
- 在 WSL/终端里显示实时刷新状态面板
- 读取 `genesisState()` 和 `miningState()`
- 搜索 nonce
- 找到候选后执行 `eth_call` 模拟
- 不发送真实交易

## 开启 AUTO_SEND=true

必须同时设置：

```env
DRY_RUN=false
AUTO_SEND=true
```

程序仍会在每次发送前检查：

- `to` 是目标合约
- `data` selector 是 `mine(uint256)` 的 `0x4d474898`
- `value` 是 `0`
- `chainId` 是 `1`
- EIP-1559 type 2
- gasLimit、maxFeePerGas、单笔 ETH 成本、每小时 ETH 成本都在上限内
- `eth_call` 模拟成功

交易发送时会先在本地用 `MINER_PRIVATE_KEY` 签好原始交易，再把同一笔 signed transaction 轮流广播到多个 RPC，避免单个 RPC 抽风导致“挖到了但发不出去”。

开启后运行：

```bash
npm run dev
```

## WSL 终端实时面板

默认 `TERMINAL_DASHBOARD=true`，在 WSL 里运行 `npm run dev` 会显示实时刷新状态页，包括：

- 当前钱包、ETH 余额、RPC
- block、epoch、剩余 epoch blocks
- difficulty、challenge、reward、remaining
- CUDA/CPU worker、H/s、MH/s、GPU utilization
- 已扫描 nonce、当前 nonce range
- 最近候选 nonce、提交结果、tx hash

关闭面板、恢复普通滚动日志：

```env
TERMINAL_DASHBOARD=false
```

完整日志仍然写入：

```bash
tail -f logs/miner.log
```

## Gas 上限

内置默认：

```env
MAX_FEE_GWEI_CAP=3
MAX_PRIORITY_FEE_GWEI_CAP=0.5
GAS_MULTIPLIER=1.15
MAX_GAS_LIMIT=150000
MAX_ETH_PER_TX=0.0002
MAX_ETH_PER_HOUR=0.002
```

策略：

- `maxPriorityFeePerGas = 当前建议值 * GAS_MULTIPLIER`，不超过 `MAX_PRIORITY_FEE_GWEI_CAP`
- `maxFeePerGas = baseFee * 2 + priorityFee`，不超过 `MAX_FEE_GWEI_CAP`
- `estimatedCost = gasLimit * maxFeePerGas`
- 超过单笔或每小时上限时继续挖矿，但不自动发送

如果当前主网 base fee 高于默认 `MAX_FEE_GWEI_CAP=3`，程序会继续挖矿但不发交易。想提高广播成功率，可在 `.env` 中提高上限，例如：

```env
MAX_FEE_GWEI_CAP=50
MAX_PRIORITY_FEE_GWEI_CAP=3
GAS_MULTIPLIER=1.30
MAX_ETH_PER_TX=0.006
MAX_ETH_PER_HOUR=0.012
NONCE_BATCH_SIZE=1000000000
```

批次建议：`1000000000` 在 `130 MH/s` 左右约 7.5 秒一批，通常比 1000 万更合适；不建议盲目调到 100 亿以上，epoch 变化时会浪费较多时间。

注意：`MAX_FEE_GWEI_CAP=50` 不是固定用 50 gwei 发，而是允许程序跟随链上 `baseFee * 2 + priorityFee`，最高不超过 50 gwei。真正会不会发送，还会受 `MAX_ETH_PER_TX` 和 `MAX_ETH_PER_HOUR` 限制。

## BlockCapReached 补救

合约错误 `0x4992976a` 是 `BlockCapReached()`，意思是当前区块的 mint 名额已满。程序会先尝试 `pending` 模拟；如果 pending 仍不可行，会保留该候选 nonce，在同一 epoch 的下一个区块继续重试。若 epoch 已经切换，旧 nonce 的 challenge 会改变，无法再补救。

## 如何确认只调用 mine(uint256)

本项目在 `src/safety.ts` 中硬编码：

- `TARGET_CONTRACT = 0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc`
- `MINE_SELECTOR = 0x4d474898`
- 禁止 selector 列表包含 `approve`、`permit`、`transfer`、`transferFrom`、`mintGenesis` 等

发送前 `assertSafeTransactionRequest()` 会检查 `to`、`data`、`value`、`chainId`、`type`、gas 上限。不满足就 throw error 并退出发送路径。

## 日志与断点续跑

- 提交记录：`data/submitted_nonces.jsonl`
- 运行状态：`data/last_state.json`
- 程序日志：`logs/miner.log`

Ctrl+C 会保存状态。启动时会读取已提交 nonce，避免重复提交同一个 epoch/nonce。

## 自检

验证 TypeScript / CUDA worker 的哈希格式是否与合约一致：

```bash
npm run selftest
```

通过时会显示：

```text
SELFTEST OK: CUDA hash matches ethers abi.encode(bytes32,uint256) + keccak256.
```

## 手动提交旧 Nonce

如果网页钱包挖到 nonce，但因为 MetaMask/OKX/RPC 没连上导致没有成功签名或广播，可以在当前 epoch 未变化时尝试：

```bash
npm run submit-nonce -- 你的十进制nonce
```

真实发送仍要求 `.env` 中同时设置：

```env
DRY_RUN=false
AUTO_SEND=true
```

## 风险说明

- 失败交易也会扣 Gas。
- epoch 每 100 个区块变化，epoch 变化会导致旧 nonce 失效。
- Gas 过高可能导致挖矿亏损。
- 只能用新钱包，不要放主力资产。
- 不要把主钱包私钥、助记词、浏览器钱包或 MetaMask 用在这个项目里。

## 合约分析

详见 `CONTRACT_ANALYSIS.md`。核心哈希规则：

```text
challenge = keccak256(abi.encode(chainId, contract, miner, epoch))
result = keccak256(abi.encode(challenge, nonce))
success iff uint256(result) < currentDifficulty
```
