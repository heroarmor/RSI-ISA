# Lane C: Rocket or Large BOOM via Chipyard 1.13.0

| Item | Pin |
|---|---|
| Repository | https://github.com/ucb-bar/chipyard |
| Tag | `1.13.0` |
| BOOM submodule | `d2a64f7ca9fd914d9c686cb23edcd32d3465a02e` |
| Rocket config | `FastRTLSimRocketConfig` (upstream, `generators/chipyard/src/main/scala/config/RocketConfigs.scala`) |
| Large BOOM config | `FastRTLSimLargeBoomV4Config`, add the snippet in `chipyard/FastRTLSimLargeBoomV4Config.scala` to `BoomConfigs.scala` |
| Simulator build | `cd sims/verilator && make CONFIG=<config>` |
| Simulator binary | `sims/verilator/simulator-chipyard.harness-<config>` |

## Cores

- **Rocket** (`WithNHugeCores(1)`): in-order 5-stage, two integer
  register-file read ports. Three-source instructions require adding a read
  port, which is a hardware change the candidate must make and pay for.
- **Large BOOM v4** (`WithNLargeBooms(1)`, system bus width 128):
  3-wide out-of-order, six integer read ports natively.

The candidate may modify the Chisel generators (decode tables, execution
units, register-file ports, rename) and the compiler, then regenerate RTL
and rebuild the simulator. The baseline is the unmodified generator at the
pinned tag running the `rv64g_zbb` reference build.

## Build notes recorded by the author

- Use JDK 17 (`JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64`); JDK 21
  crashed sbt in the author's environment.
- Use firtool 1.62.x; a newer system firtool rejected Chipyard's
  printf-verification ops.
- RTL generation plus Verilator compilation of Large BOOM takes hours on a
  many-core host; Rocket is faster. Budget accordingly.

## Running

```
simulator-chipyard.harness-<config> +permissive +verbose +permissive-off <elf>
```

`scripts/run_chipyard.sh` wraps this and applies the completion gate
(`*** PASSED ***` and `Completed after N cycles` on stderr). Put the
directory containing `libriscv.so` on `LD_LIBRARY_PATH`.
