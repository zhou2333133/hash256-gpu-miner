# 安全边界

这个项目只做一件事：为 Ethereum mainnet 上的 HASH 合约搜索 `mine(uint256)` nonce，并在显式开启后提交 `mine(uint256)` 交易。

## 硬编码限制

- 唯一允许的目标合约：`0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc`
- 唯一允许的 method selector：`mine(uint256) = 0x4d474898`
- 唯一允许的 chainId：`1`
- 所有交易 `value` 强制为 `0`
- 默认 `DRY_RUN=true`，默认 `AUTO_SEND=false`
- 默认 `TERMINAL_DASHBOARD=true`，仅在本地终端显示状态，不启动网页服务
- 私钥只从 `.env` 的 `MINER_PRIVATE_KEY` 读取
- 签名在本地完成；广播可跨多个 RPC 自动重试
- GPU worker 只接收公开挖矿参数，不接触私钥、RPC、交易签名或钱包文件

## 禁止功能

项目不实现、不调用：

- `approve`
- `permit`
- `transfer`
- `transferFrom`
- `setApprovalForAll`
- `mintGenesis`
- `swap`
- `addLiquidity`
- `multicall`
- DEX、Router、Aggregator
- 浏览器钱包、MetaMask、本机钱包文件读取

## 真实发送前的检查

每笔真实交易发送前都必须通过：

- `eth_call` / `staticCall` 模拟成功
- `to` 等于目标合约
- `data` 前 4 bytes 等于 `0x4d474898`
- `value == 0`
- `chainId == 1`
- `type == 2`
- `gasLimit <= MAX_GAS_LIMIT`
- `maxFeePerGas <= MAX_FEE_GWEI_CAP`
- `maxPriorityFeePerGas <= MAX_PRIORITY_FEE_GWEI_CAP`
- `gasLimit * maxFeePerGas <= MAX_ETH_PER_TX`
- 最近 1 小时累计 gas 成本不超过 `MAX_ETH_PER_HOUR`

任一检查失败，程序直接抛错或跳过发送。

## 钱包建议

只使用新钱包，只放少量 ETH 作为 gas。不要导入主钱包私钥、助记词或含有主力资产的钱包。
