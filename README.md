# RSI-ISA

**Fixed microarchitecture, open instruction set.** RSI-ISA is a research
environment for the question: *given a fixed RISC-V processor implementation,
what instruction-set extension makes a fixed workload run in the fewest
cycles?* Everything that defines the experiment is here: the workload, the
bare-metal runtime, the simulators or their exact build recipes, the
completion gate, the baseline, and the scoring rule. The method for
answering the question is deliberately **not** here.

This repository is the public problem statement behind an
[OpenRSI Index](https://index.openrsi.foundation/) task proposal. It is a
subset snapshot of the author's `Best_ISA` project (source commit
`9746957645289f64587ff54f50556a2b946385a9`). The project's ISA-synthesis
pipeline, its discovered instruction sets, and all measured results are
withheld on purpose so that they cannot leak into an agent's workspace.

## The problem

A RISC-V core executes a fixed set of 13 self-contained kernels
(`kernels/`). The reference build compiles them with an unmodified LLVM 18
for `rv64g_zbb`: the ratified bit-manipulation extension Zbb is the human
baseline. A candidate replaces the instruction set the compiler is allowed
to use (and, in lanes that permit it, the hardware that implements it),
runs the same kernel sources on the same simulator, and is scored by the
geometric mean of its cycle ratio against the baseline.

The environment fixes:

- the kernel sources and the runtime (`runtime/crt0.S`, `syscalls.c`, `link.ld`);
- the compiler flags of the reference build (`scripts/build_kernels.sh`);
- the simulator for each lane, or its pinned upstream commit and build recipe (`lanes/`);
- the completion gate and cycle-count extraction (`scripts/run_cva6.sh`, `scripts/run_chipyard.sh`);
- the score (`scripts/geomean.py`).

What a candidate may change is defined per lane below and pinned precisely
by the OpenRSI task built from this environment, not by this README.

## Lanes

| Lane | Core | What is fixed | What the candidate may touch | Reproducible from source |
|---|---|---|---|---|
| **A** `lanes/A-cva6-fixed.md` | CVA6 (in-order, 6-stage, CV-X-IF on, 3 integer read ports) | A prebuilt Verilator binary distributed as a release asset | Compiler and code generation only; the hardware is opaque and immutable | No. The binary embeds undisclosed RTL modifications; it is the fixed target |
| **B** `lanes/B-cva6-upstream.md` | CVA6 upstream `02e11e3` | The upstream RTL at the pinned commit and the build recipe | RTL (decoder, ALU, operand read, CV-X-IF coprocessor) and compiler | In principle yes; a known Verilator 5.020 blocker is documented |
| **C** `lanes/C-chipyard.md` | Rocket (2 read ports) or Large BOOM (3-wide OoO, 6 read ports), Chipyard 1.13.0 | Upstream generators at the pinned tag and the two configs | Chisel (decode, execution units, register-file ports) and compiler | Yes |

All lanes are CPU-only. Cycle-accurate Verilator simulation of these cores
runs at roughly 20 to 30 thousand cycles per second, so the largest kernel
(`matmul`, about 19 million cycles on CVA6) takes 10 to 20 minutes and the
13 kernels finish in well under an hour when run in parallel.

## Workload

Thirteen self-contained kernels, written as simplified re-implementations
of MiBench programs so that they run bare-metal with no libc and terminate
through HTIF `tohost`:

| Kernel | Origin | Kernel | Origin |
|---|---|---|---|
| `adpcm` | telecomm/adpcm | `fir_filter` | DSP FIR |
| `aes` | security/aes (table-based) | `matmul` | dense integer matrix multiply |
| `basicmath` | automotive/basicmath | `qsort` | automotive/qsort |
| `bitcount` | automotive/bitcount | `sha` | security/sha (SHA-1) |
| `crc32` | telecomm/CRC32 | `sha256_full` | SHA-256 |
| `dijkstra` | network/dijkstra | `stringsearch` | office/stringsearch |
| `fft` | telecomm/FFT (fixed point) | | |

Each kernel is compiled with `-O3 -fno-builtin -nostdlib -mcmodel=medany
-mabi=lp64d`, linked with `runtime/link.ld` at `0x80000000`, and writes
`tohost = 1` on completion. Correctness is checked by the simulator's
`tohost` exit status; a kernel that traps, hangs or exits non-zero is not
scored.

## Quick start (baseline on lane A)

```bash
# 1. toolchain: stock clang 18 and a riscv64-unknown-elf GCC for linking
export CLANG=clang-18 GCC=/path/to/riscv64-unknown-elf-gcc

# 2. build the two reference configurations
MARCH=rv64g     CONFIG=base scripts/build_kernels.sh
MARCH=rv64g_zbb CONFIG=zbb  scripts/build_kernels.sh

# 3. fetch the lane-A simulator (see lanes/A-cva6-fixed.md for checksums)
export LD_LIBRARY_PATH=/path/to/dir/with/libriscv.so
SIM=/path/to/Variane_testharness ELFD=build/zbb/elf OUT=results/zbb scripts/run_cva6.sh

# 4. score a candidate against the baseline
scripts/geomean.py --baseline results/zbb/cycles.csv --candidate results/cand/cycles.csv
```

## Scoring

`scripts/geomean.py` computes, over the 13 kernels,
`exp(mean(log(cycles_candidate / cycles_baseline)))`. Lower is better. A
candidate with any kernel that did not complete under the gate is
**unscored**, not penalised: the run is reported as invalid.

Completion gates:

- **CVA6 (lanes A, B):** the `rvfi_tracer` line `terminated after N cycles`
  must be present and the harness must print `*** SUCCESS ***`. A `SUCCESS`
  line without the `rvfi_tracer` line means the wall-clock timeout killed
  the run and the printed cycle count is meaningless.
- **Chipyard (lane C):** the harness must print `*** PASSED ***` and
  `Completed after N cycles`.

## Toolchain pins

| Component | Pin |
|---|---|
| Compiler for the reference build | LLVM/clang 18 (tested with Ubuntu clang 18.1.3) |
| Linker | `riscv64-unknown-elf-gcc` from any riscv-gnu-toolchain build with `rv64imafdc` multilib |
| HTIF/fesvr library (`libriscv.so`) | riscv-isa-sim (Spike) commit `0ad45926ac6f42d0d39e936abf4ab1cb9bdc5086` |
| Verilator | 5.020 (the version the lane-A binary was built with) |
| CVA6 | `openhwgroup/cva6` commit `02e11e3b9d9e6a087dc4811b933bf0cc80156e1a` |
| Chipyard | tag `1.13.0`; BOOM submodule `d2a64f7ca9fd914d9c686cb23edcd32d3465a02e` |

## License

AGPL-3.0-only. The kernels are the author's simplified re-implementations
of the corresponding MiBench programs and are licensed under the same terms.
