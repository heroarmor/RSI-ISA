#!/usr/bin/env bash
# Run a directory of kernel ELFs on a Chipyard Verilator simulator (lane C).
# For each <config>_<kernel>.riscv the timing ELF gives the cycle count
# ("*** PASSED *** Completed after N simulation cycles" on stderr) and the
# matching <config>_<kernel>.chk.riscv gives the checksum, which the harness
# prints as "*** FAILED *** (tohost = N)" before it stops.
#
# Usage:
#   SIM=chipyard/sims/verilator/simulator-chipyard.harness-FastRTLSimLargeBoomV4Config \
#   ELFD=build/zbb/elf OUT=results/boom_zbb scripts/run_chipyard.sh
# Environment: SIM, ELFD, OUT required; TIMEOUT seconds (default 7200);
#   JOBS parallel kernels (default: number of CPUs);
#   LD_LIBRARY_PATH must contain the directory holding libriscv.so (fesvr).
set -uo pipefail
SIM="${SIM:?set SIM}"; ELFD="${ELFD:?set ELFD}"; OUT="${OUT:?set OUT}"
TIMEOUT="${TIMEOUT:-7200}"; JOBS="${JOBS:-$(nproc)}"
LOGD="$OUT/logs"; CSV="$OUT/cycles.csv"
mkdir -p "$LOGD"

sim_one() {  # $1 = elf, $2 = log
  timeout "$TIMEOUT" "$SIM" +permissive +verbose +permissive-off "$1" 2>&1 >/dev/null \
    | grep -E "PASSED|FAILED|Completed|tohost" > "$2"
}
run_kernel() {
  local elf="$1" base cfg kernel tlog clog cyc ck st
  base="$(basename "$elf" .riscv)"; cfg="${base%%_*}"; kernel="${base#*_}"
  tlog="$LOGD/time_${cfg}_${kernel}.log"; clog="$LOGD/chk_${cfg}_${kernel}.log"
  sim_one "$elf" "$tlog" & sim_one "${elf%.riscv}.chk.riscv" "$clog" & wait
  cyc=$(grep -oP 'PASSED \*\*\* Completed after\s+\K[0-9]+' "$tlog" | head -1)
  ck=$(grep -oP 'tohost = \K[0-9]+' "$clog" | head -1)
  grep -q 'PASSED' "$clog" && ck=0                  # checksum 0 exits 0 and passes
  st=TIMEOUT; [ -n "$cyc" ] && st=PASS
  grep -q 'FAILED' "$tlog" && st=FAIL
  echo "$cfg,$kernel,${cyc:-NA},$st,${ck:-NA}"
}
export -f sim_one run_kernel; export SIM LOGD TIMEOUT
echo "config,kernel,cycles,status,checksum" > "$CSV"
ls "$ELFD"/*.riscv | grep -v '\.chk\.riscv$' | xargs -P "$JOBS" -I{} bash -c 'run_kernel "$1"' _ {} >> "$CSV"
echo "wrote $CSV ($(($(wc -l < "$CSV")-1)) kernels)"
