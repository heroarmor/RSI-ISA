# Lane A: CVA6, fixed prebuilt simulator

The fixed target is a Verilator 5.020 model of CVA6 (`cv64a6_imafdc_sv39`
family) with the CV-X-IF coprocessor interface enabled and three integer
register-file read ports. It was built from upstream
`openhwgroup/cva6` commit `02e11e3b9d9e6a087dc4811b933bf0cc80156e1a`
**plus modifications by the author to the decoder, ALU, operand-read
stage, configuration packages, and the CV-X-IF example coprocessor**.
Those modifications are intentionally undisclosed: the binary is an
opaque, immutable piece of hardware. Candidates in this lane change only
what the compiler emits.

## Assets

Distributed as GitHub release assets of this repository (`v0.1-lane-a`):

| File | SHA-256 |
|---|---|
| `Variane_testharness` | `ee04124464e15640078a15d847e2c884868f807fe283f5eb728b96d64d51dadf` |
| `libriscv.so` (fesvr/HTIF, from riscv-isa-sim `0ad4592`) | `a005288e267921a9b7d1d6ef5fec6cae315ac56975480aec13dba3009fe94d6e` |

The binary is dynamically linked against `libriscv.so`, `libstdc++`,
`libm`, `libgcc_s`, and glibc (x86-64 Linux). Put the directory containing
`libriscv.so` on `LD_LIBRARY_PATH`.

## Running

```
Variane_testharness +time_out=<max simulated cycles> +tohost_addr=80001000 <elf>
```

`scripts/run_cva6.sh` wraps this, runs each ELF in a private temporary
directory (the model writes multi-gigabyte `trace_*.dasm` files into its
working directory), and applies the completion gate.

## Behaviour to know

- Throughput is roughly 18 to 28 thousand simulated cycles per second.
- With CV-X-IF enabled, an instruction the decoder does not recognise is
  offloaded to the coprocessor. If the coprocessor does not accept it the
  core never retires it and the simulation runs until `+time_out` or the
  wall-clock timeout. Always run with both limits.
- `*** SUCCESS ***` is printed by the HTIF front end when `tohost` is
  written; the `rvfi_tracer ... terminated after N cycles` line is the only
  reliable sign that the program itself finished. The runner records both.

## Reproducibility

This lane is **not** reproducible from public source: the modified RTL is
withheld, and a clean `make verilate` of the upstream commit hits a
Verilator 5.020 internal error in the author's environment (see lane B).
Reviewers can re-run the binary, not rebuild it.
