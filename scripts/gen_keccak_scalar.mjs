// Generate scalar register-based keccakf256 C++ code.
// Each of the 25 state lanes is an individual uint64_t variable (sXY).
// All 24 rounds fully unrolled with θ→ρ→π→χ→ι steps.

// Correct Keccak ρ offsets: ρ[x][y]
const RHO = [
  [0, 36,  3, 41, 18],
  [1, 44, 10, 45,  2],
  [62, 6, 43, 15, 61],
  [28, 55, 25, 21, 56],
  [27, 20, 39,  8, 14],
];

const RNDC = [
  "0x0000000000000001ULL","0x0000000000008082ULL",
  "0x800000000000808aULL","0x8000000080008000ULL",
  "0x000000000000808bULL","0x0000000080000001ULL",
  "0x8000000080008081ULL","0x8000000000008009ULL",
  "0x000000000000008aULL","0x0000000000000088ULL",
  "0x0000000080008009ULL","0x000000008000000aULL",
  "0x000000008000808bULL","0x800000000000008bULL",
  "0x8000000000008089ULL","0x8000000000008003ULL",
  "0x8000000000008002ULL","0x8000000000000080ULL",
  "0x000000000000800aULL","0x800000008000000aULL",
  "0x8000000080008081ULL","0x8000000000008080ULL",
  "0x0000000080000001ULL","0x8000000080008008ULL"
];

// π mapping: src_idx → dst_idx
// π(x,y) = (y, (2x+3y)%5)
const PI_FWD = (() => {
  const f = [];
  for (let i = 0; i < 25; i++) {
    const x = i % 5, y = Math.floor(i / 5);
    const dst_x = y;
    const dst_y = (2 * x + 3 * y) % 5;
    f.push(dst_x + 5 * dst_y);
  }
  return f;
})();

console.log("// π forward mapping:", PI_FWD.join(","));

function s(x, y) { return `s${x}${y}`; }
function rotl(a, b) { return `rotl64(${a}, ${b})`; }

function generateScalarKeccakf() {
  let lines = [];

  lines.push(`// Fully scalar register-based keccakf — 25 individual uint64_t lanes,`);
  lines.push(`// all 24 rounds fully unrolled. Produced by gen_keccak_scalar.mjs`);
  lines.push(`__device__ __forceinline__ void keccakf_scalar(`);
  const params = [];
  for (let y = 0; y < 5; y++)
    for (let x = 0; x < 5; x++)
      params.push(`    uint64_t &${s(x,y)}`);
  lines.push(params.join(",\n"));
  lines.push(`) {`);
  lines.push(``);

  for (let round = 0; round < 24; round++) {
    lines.push(`    { // ─── Round ${round} ───`);
    for (let x = 0; x < 5; x++) {
      const vars = [];
      for (let y = 0; y < 5; y++) vars.push(s(x,y));
      lines.push(`    uint64_t c${x} = ${vars.join(" ^ ")};`);
    }
    for (let x = 0; x < 5; x++) {
      const prev = (x + 4) % 5, next = (x + 1) % 5;
      lines.push(`    uint64_t d${x} = c${prev} ^ ${rotl(`c${next}`, "1")};`);
    }
    for (let y = 0; y < 5; y++)
      for (let x = 0; x < 5; x++)
        lines.push(`    ${s(x,y)} ^= d${x};`);
    lines.push(``);

    // ── ρ (rotate each lane by its ρ offset) ──
    lines.push(`    // ─── Round ${round} (ρ) ───`);
    for (let y = 0; y < 5; y++)
      for (let x = 0; x < 5; x++)
        if (!(x === 0 && y === 0))
          lines.push(`    ${s(x,y)} = ${rotl(s(x,y), RHO[x][y])};`);
    lines.push(``);

    // ── π (permute via temps) ──
    lines.push(`    // ─── Round ${round} (π) ───`);
    for (let i = 0; i < 25; i++) {
      const x = i % 5, y = Math.floor(i / 5);
      lines.push(`    uint64_t p${i} = ${s(x,y)};`);
    }
    lines.push(``);
    for (let src = 0; src < 25; src++) {
      const dst = PI_FWD[src];
      if (src === dst) continue; // (0,0) stays
      const dx = dst % 5, dy = Math.floor(dst / 5);
      lines.push(`    ${s(dx, dy)} = p${src};`);
    }
    lines.push(``);

    // ── χ ──
    lines.push(`    // ─── Round ${round} (χ) ───`);
    for (let y = 0; y < 5; y++) {
      const v = [s(0,y), s(1,y), s(2,y), s(3,y), s(4,y)];
      lines.push(`    {`);
      lines.push(`    uint64_t bc0 = ${v[0]} ^ ((~${v[1]}) & ${v[2]});`);
      lines.push(`    uint64_t bc1 = ${v[1]} ^ ((~${v[2]}) & ${v[3]});`);
      lines.push(`    uint64_t bc2 = ${v[2]} ^ ((~${v[3]}) & ${v[4]});`);
      lines.push(`    uint64_t bc3 = ${v[3]} ^ ((~${v[4]}) & ${v[0]});`);
      lines.push(`    uint64_t bc4 = ${v[4]} ^ ((~${v[0]}) & ${v[1]});`);
      lines.push(`    ${v[0]} = bc0; ${v[1]} = bc1; ${v[2]} = bc2; ${v[3]} = bc3; ${v[4]} = bc4;`);
      lines.push(`    }`);
    }
    lines.push(``);

    lines.push(`    s00 ^= ${RNDC[round]};`);
    lines.push(`    }`);
    lines.push(``);
  }

  lines.push(`}`);
  return lines.join("\n");
}

console.log(generateScalarKeccakf());
