# Project Agent Notes

This project is a security-first Ethereum mainnet miner for HASH256.

## Non-Negotiable Safety Boundaries

- Only target contract: `0xAC7b5d06fa1e77D08aea40d46cB7C5923A87A0cc`.
- Only live transaction selector: `mine(uint256) = 0x4d474898`.
- Chain must be Ethereum mainnet, `chainId = 1`.
- Transaction `value` must always be `0`.
- Default live mode is disabled: `DRY_RUN=true`, `AUTO_SEND=false`.
- Private key source is only `.env` variable `MINER_PRIVATE_KEY`; never request or store seed phrases.
- Do not read browser wallets, MetaMask, local wallet files, cookies, or unrelated secret stores.
- GPU worker must remain keyless and only receive public mining parameters.
- Terminal dashboard is local CLI output only; do not add a web dashboard unless the user explicitly asks again.

## Forbidden Features

Do not add or call:

- `approve`
- `permit`
- `transfer`
- `transferFrom`
- `setApprovalForAll`
- `mintGenesis`
- `swap`
- `addLiquidity`
- `multicall`
- DEX, Router, or Aggregator integrations

## Verification Before Shipping Changes

Run:

```bash
npm run check
npm run build
```

For behavior checks, use dry run only unless the user explicitly confirms live sending:

```bash
DRY_RUN=true AUTO_SEND=false npm run dev
```

Do not run live transactions during routine validation.

## Key Files

- `CONTRACT_ANALYSIS.md` documents the verified `mine(uint256)` logic.
- `SECURITY.md` documents hard safety boundaries.
- `src/safety.ts` is the central transaction gate.
- `src/tx.ts` is the only module that may send a transaction.
- `gpu/cuda_miner.cu` must stay isolated from private keys and RPC logic.
