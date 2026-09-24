# Lane B: CVA6, unmodified upstream

| Item | Pin |
|---|---|
| Repository | https://github.com/openhwgroup/cva6 |
| Commit | `02e11e3b9d9e6a087dc4811b933bf0cc80156e1a` (2026-03-20) |
| Target | `cv64a6_imafdc_sv39` |
| Configuration for this lane | `CvxifEn = 1`, `NrRgprPorts = 3` in the target's config package |
| Simulator build | `make verilate target=cv64a6_imafdc_sv39` producing `work-ver/Variane_testharness` |
| fesvr | riscv-isa-sim commit `0ad45926ac6f42d0d39e936abf4ab1cb9bdc5086` |

In this lane the candidate may modify the RTL (decoder, ALU, operand-read
stage, CV-X-IF coprocessor) as well as the compiler, and must rebuild the
Verilator model after each hardware change. The baseline is the unmodified
upstream RTL at the pinned commit with the configuration above, running
the `rv64g_zbb` reference build.

## Known blocker

In the author's environment a clean `make verilate` of this commit fails
with a Verilator **5.020** internal error. Whether another Verilator
release builds it has not been verified. This lane is admissible only once
a from-source build has been demonstrated; until then use lane A or C.
