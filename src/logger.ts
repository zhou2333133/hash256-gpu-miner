import { existsSync, mkdirSync, appendFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const moduleRoot = dirname(dirname(fileURLToPath(import.meta.url)));
const rootDir = existsSync(join(process.cwd(), "package.json")) ? process.cwd() : moduleRoot;
const logPath = join(rootDir, "logs", "miner.log");
let consoleLogging = true;

mkdirSync(dirname(logPath), { recursive: true });

type LogLevel = "info" | "warn" | "error" | "debug";

function write(level: LogLevel, message: string, meta?: unknown): void {
  const timestamp = new Date().toISOString();
  const suffix = meta === undefined ? "" : ` ${JSON.stringify(meta, jsonBigIntReplacer)}`;
  const line = `[${timestamp}] ${level.toUpperCase()} ${message}${suffix}`;

  if (consoleLogging) {
    if (level === "error") {
      console.error(line);
    } else if (level === "warn") {
      console.warn(line);
    } else {
      console.log(line);
    }
  }

  appendFileSync(logPath, `${line}\n`, { encoding: "utf8" });
}

function jsonBigIntReplacer(_key: string, value: unknown): unknown {
  return typeof value === "bigint" ? value.toString() : value;
}

export const logger = {
  info: (message: string, meta?: unknown) => write("info", message, meta),
  warn: (message: string, meta?: unknown) => write("warn", message, meta),
  error: (message: string, meta?: unknown) => write("error", message, meta),
  debug: (message: string, meta?: unknown) => {
    if (process.env.DEBUG === "true") {
      write("debug", message, meta);
    }
  },
};

export function setConsoleLogging(enabled: boolean): void {
  consoleLogging = enabled;
}

export function stringifyForLog(value: unknown): string {
  return JSON.stringify(value, jsonBigIntReplacer);
}
