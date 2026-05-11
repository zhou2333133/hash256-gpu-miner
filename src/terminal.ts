import { formatUnits } from "ethers";
import { type AppConfig } from "./config.js";
import { MINE_SELECTOR, TARGET_CONTRACT } from "./safety.js";

type DashboardState = {
  wallet?: string;
  ethBalance?: string;
  rpc?: string;
  blockNumber?: number;
  genesisComplete?: boolean;
  epoch?: string;
  epochBlocksLeft?: string;
  epochProbability?: string;
  difficulty?: string;
  challenge?: string;
  rewardHash?: string;
  remainingHash?: string;
  nonceStart?: string;
  nonceEnd?: string;
  worker?: string;
  hashrateHps?: number;
  gpuUtilization?: number | string;
  scannedBatch?: string;
  scannedTotal?: string;
  lastCandidate?: string;
  lastSubmission?: string;
  lastTxHash?: string;
  status?: string;
};

export class TerminalDashboard {
  private state: DashboardState = {};
  private events: string[] = [];
  private timer?: NodeJS.Timeout;
  private startedAt = Date.now();
  private lastLineCount = 0;

  readonly enabled: boolean;

  constructor(private readonly config: AppConfig) {
    this.enabled = config.terminalDashboard && process.stdout.isTTY;
  }

  start(): void {
    if (!this.enabled || this.timer) {
      return;
    }
    process.stdout.write("\x1b[?25l");
    this.lastLineCount = 0;
    this.timer = setInterval(() => this.render(), 500);
    this.render();
  }

  stop(): void {
    if (!this.enabled) {
      return;
    }
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = undefined;
    }
    this.render();
    process.stdout.write("\x1b[?25h\n");
  }

  update(partial: DashboardState): void {
    this.state = { ...this.state, ...partial };
    // render() is called by 500ms interval; no need to call here
  }

  event(message: string): void {
    const time = new Date().toLocaleTimeString("en-US", { hour12: false });
    this.events.unshift(`${time} ${message}`);
    this.events = this.events.slice(0, 8);
    // render() is called by 500ms interval; no need to call here
  }

  private render(): void {
    if (!this.enabled) {
      return;
    }

    const mode = this.config.autoSend && !this.config.dryRun ? "自动发送已开启" : "只模拟，不发送";
    const modeColor = this.config.autoSend && !this.config.dryRun ? "\x1b[31m" : "\x1b[33m";
    const hashRate = this.state.hashrateHps ?? 0;
    const mh = hashRate / 1_000_000;
    const uptime = Math.floor((Date.now() - this.startedAt) / 1000);
    const lines = [
      `${green("HASH256 GPU 挖矿器")} ${dim(new Date().toISOString())}`,
      `${modeColor}${mode}\x1b[0m  ${dim("函数选择器")} ${MINE_SELECTOR}  ${dim("转账金额")} 0  ${dim("目标合约")} ${short(TARGET_CONTRACT)}`,
      "",
      `${label("钱包")} ${this.state.wallet ?? "-"}  ${label("ETH 余额")} ${this.state.ethBalance ?? "-"}`,
      `${label("RPC")} ${this.state.rpc ?? "-"}  ${label("运行时长")} ${formatDuration(uptime)}`,
      "",
      `${label("区块")} ${this.state.blockNumber ?? "-"}  ${label("Epoch")} ${this.state.epoch ?? "-"}  ${label("本轮剩余区块")} ${this.state.epochBlocksLeft ?? "-"}`,
      `${label("创世完成")} ${this.state.genesisComplete === undefined ? "-" : yesNo(this.state.genesisComplete)}  ${label("单次奖励")} ${this.state.rewardHash ?? "-"} HASH  ${label("剩余可挖")} ${this.state.remainingHash ?? "-"} HASH`,
      `${label("当前 epoch 预计命中概率")} ${this.state.epochProbability ?? "-"}`,
      `${label("难度/目标")} ${this.state.difficulty ?? "-"}`,
      `${label("挑战值")} ${this.state.challenge ?? "-"}`,
      "",
      `${label("计算后端")} ${this.state.worker ?? "-"}  ${label("算力")} ${hashRate.toLocaleString()} H/s (${mh.toFixed(3)} MH/s)  ${label("GPU 利用率")} ${formatGpu(this.state.gpuUtilization)}`,
      `${label("累计扫描")} ${this.state.scannedTotal ?? "0"}  ${label("当前批次")} ${this.state.scannedBatch ?? "0"}`,
      `${label("Nonce 范围")} ${this.state.nonceStart ?? "-"} -> ${this.state.nonceEnd ?? "-"}`,
      "",
      `${label("候选 Nonce")} ${this.state.lastCandidate ?? "-"}`,
      `${label("提交结果")} ${translateStatus(this.state.lastSubmission)}`,
      `${label("交易哈希")} ${this.state.lastTxHash ?? "-"}`,
      `${label("当前状态")} ${translateStatus(this.state.status ?? "running")}`,
      "",
      green("最近事件"),
      ...(this.events.length ? this.events.map((event) => `  ${event}`) : ["  -"]),
      "",
      dim("按 Ctrl+C 会保存状态并退出。完整日志：logs/miner.log"),
    ];

    const output = lines.join("\n") + "\n";
    if (this.lastLineCount > 0) {
      process.stdout.write(`\x1b[${this.lastLineCount}A\x1b[J`);
    }
    process.stdout.write(output);
    this.lastLineCount = lines.length;
  }
}

export function formatHashAmount(raw: bigint): string {
  return formatUnits(raw, 18);
}

function label(value: string): string {
  return `\x1b[36m${value}:\x1b[0m`;
}

function green(value: string): string {
  return `\x1b[32m${value}\x1b[0m`;
}

function dim(value: string): string {
  return `\x1b[2m${value}\x1b[0m`;
}

function short(value: string): string {
  return `${value.slice(0, 8)}...${value.slice(-6)}`;
}

function yesNo(value: boolean): string {
  return value ? "是" : "否";
}

function formatGpu(value: number | string | undefined): string {
  if (value === -1 || value === "-1") {
    return "未读取";
  }
  if (value === undefined) {
    return "-";
  }
  return String(value);
}

function translateStatus(value: string | undefined): string {
  if (!value) {
    return "-";
  }
  return value
    .replace("running", "运行中")
    .replace("mining", "挖矿中")
    .replace("dry run", "只模拟")
    .replace("auto-send armed", "自动发送已就绪")
    .replace("candidate found; rechecking", "找到候选，正在复核")
    .replace("candidate expired", "候选已过期")
    .replace("candidate failed local recheck", "候选本地复核失败")
    .replace("candidate already used", "候选已被使用")
    .replace("mining exhausted", "挖矿供应已耗尽")
    .replace("DRY_RUN=true or AUTO_SEND=false", "只模拟或自动发送未开启")
    .replace("nonce already recorded as submitted", "该 nonce 已记录为提交过");
}

function formatDuration(seconds: number): string {
  const h = Math.floor(seconds / 3600);
  const m = Math.floor((seconds % 3600) / 60);
  const s = seconds % 60;
  return `${String(h).padStart(2, "0")}:${String(m).padStart(2, "0")}:${String(s).padStart(2, "0")}`;
}
