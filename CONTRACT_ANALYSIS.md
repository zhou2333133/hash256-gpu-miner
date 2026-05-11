# HASH256 合约分析

分析日期：2026-05-11  
网络：Ethereum Mainnet，chainId = 1  
目标合约：`0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc`

## 来源与匹配性

- Sourcify full match：`repo.sourcify.dev/contracts/full_match/1/0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc`
- 编译器：Solidity `0.8.26+commit.8a97fa7a`
- 编译目标：`src/Hash.sol:Hash`
- 合约是 ERC20 token、Uniswap V4 hook、PoW miner 的同一份 bytecode。

Sourcify full match 表示源码、metadata 与主网已部署字节码匹配。本项目只根据该源码和链上只读调用实现 `mine(uint256)`，不按网页文案猜测哈希格式。

## mine(uint256) 参数

```solidity
function mine(uint256 nonce) external nonReentrant
```

唯一参数是 `uint256 nonce`。调用不需要 ETH value，函数 stateMutability 为 `nonpayable`。

method selector：

```text
mine(uint256) = 0x4d474898
```

## challenge 如何生成

源码：

```solidity
function _challenge(address miner) internal view returns (bytes32) {
    return keccak256(abi.encode(
        block.chainid,
        address(this),
        miner,
        _epoch()
    ));
}
```

外部只读函数：

```solidity
function getChallenge(address miner) external view returns (bytes32)
```

严格复现方式：

```text
challenge = keccak256(abi.encode(uint256 chainId, address contract, address miner, uint256 epoch))
```

在主网本合约上即：

```text
challenge = keccak256(abi.encode(1, 0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc, miner, epoch))
```

## nonce 验证拼接方式

源码：

```solidity
bytes32 result = keccak256(abi.encode(_challenge(msg.sender), nonce));
if (uint256(result) >= currentDifficulty) revert InsufficientWork();
```

严格复现方式：

```text
result = keccak256(abi.encode(bytes32 challenge, uint256 nonce))
success iff uint256(result) < currentDifficulty
```

这里使用 Solidity ABI 编码 `abi.encode`，不是 `abi.encodePacked`，也不是字符串拼接。

## epoch 如何轮换

源码：

```solidity
uint256 public constant EPOCH_BLOCKS = 100;

function _epoch() internal view returns (uint256) {
    return block.number / EPOCH_BLOCKS;
}
```

epoch 每 100 个以太坊区块轮换一次。epoch 改变后，`_challenge(msg.sender)` 改变，旧 nonce 搜索结果会失效。

外部只读函数：

```solidity
function epochBlocksLeft() external view returns (uint256)
function miningState() external view returns (
    uint256 era,
    uint256 reward,
    uint256 difficulty,
    uint256 minted,
    uint256 remaining,
    uint256 epoch,
    uint256 epochBlocksLeft_
)
```

## difficulty / target 如何读取

公开变量：

```solidity
uint256 public currentDifficulty;
```

`miningState()` 返回第 3 个字段 `difficulty`。合约把它当作 target 使用：hash 数值必须小于 `currentDifficulty`，数值越小越难。

初始设置发生在 `_seedBody()`：

```solidity
currentDifficulty = type(uint256).max >> 32;
```

每 `ADJUSTMENT_INTERVAL = 2016` 次 mint 调整一次：

```solidity
target = ADJUSTMENT_INTERVAL * TARGET_BLOCKS_PER_MINT;
next = FullMath.mulDiv(old, blocksTaken, target);
next 被限制在 old/4 到 old*4 之间，且最小为 1。
```

## nonce 是否绑定钱包地址

绑定。`challenge` 包含 `miner`，而 `mine()` 内部传入的是 `msg.sender`：

```solidity
keccak256(abi.encode(block.chainid, address(this), msg.sender, _epoch()))
```

因此 GPU 搜索必须使用实际发交易的钱包地址。换钱包后 nonce 不能复用。

重复证明 key 也绑定地址和 epoch：

```solidity
bytes32 key = keccak256(abi.encode(msg.sender, nonce, _epoch()));
if (usedProofs[key]) revert ProofAlreadyUsed();
usedProofs[key] = true;
```

## 成功 mint 条件

`mine(uint256)` 成功需要同时满足：

- `genesisComplete == true`
- `totalMiningMinted < MINING_SUPPLY`
- 当前区块 `mintsInBlock[block.number] < MAX_MINTS_PER_BLOCK`
- `uint256(keccak256(abi.encode(_challenge(msg.sender), nonce))) < currentDifficulty`
- `usedProofs[keccak256(abi.encode(msg.sender, nonce, _epoch()))] == false`
- 当前 reward 不为 0 且 mining supply 未耗尽

成功后：

- 标记 `usedProofs[key] = true`
- `mintsInBlock[block.number]++`
- `totalMints++`
- 必要时调整 difficulty
- 计算 era 和 reward：`era = totalMints / ERA_MINTS`，`reward = BASE_REWARD >> era`
- 从合约地址向 `msg.sender` 转 HASH token
- 触发 `Mined(miner, nonce, reward, era)`

## 失败 revert 条件

`mine(uint256)` 可能 revert：

- `GenesisNotComplete()`：创世 mint / pool seed 尚未完成
- `SupplyExhausted()`：挖矿供应耗尽，或 reward 为 0
- `BlockCapReached()`：当前区块已达到 `MAX_MINTS_PER_BLOCK = 10`
- `InsufficientWork()`：hash 不低于 currentDifficulty
- `ProofAlreadyUsed()`：同地址、同 nonce、同 epoch 已使用
- `ReentrancyGuardReentrantCall()`：重入保护触发

此外 ERC20 `_transfer(address(this), msg.sender, reward)` 理论上可能因合约余额不足等 ERC20 条件失败，但正常状态下 mining supply 在 seed 时已 mint 到合约地址。

## 是否需要 value

不需要。`mine(uint256)` 是 `nonpayable`，交易 `value` 必须为 `0`。本项目硬编码强制 `value = 0`。

## 是否涉及 approve 或 permit

`mine(uint256)` 不涉及用户侧 `approve`、`permit`、`transfer`、`transferFrom`。合约在 `_seedBody()` 内部曾对自身 token 做 Permit2/PositionManager 授权以初始化流动性，但这与矿工调用 `mine(uint256)` 无关。

本项目禁止实现或调用：

- `approve`
- `permit`
- `transfer`
- `transferFrom`
- `setApprovalForAll`
- `mintGenesis`
- `swap`
- `addLiquidity`
- `multicall`

## 额外安全观察

- 目标网页当前显示 mining 尚未开放，原因是 `genesisComplete` 可能仍为 false。程序启动会读取 `genesisState()`；未开放时只会等待/刷新，不会发送交易。
- 因 epoch 每 100 区块轮换，临近 epoch 结束时找到的 nonce 可能在交易上链前失效。
- 失败交易也会消耗 gas；因此真实发送前必须 `eth_call` 模拟，并再次检查 epoch、selector、to、value、chainId 与 gas 上限。
