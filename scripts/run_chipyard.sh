#!/usr/bin/env bash
# Run a directory of ELFs on a Chipyard Verilator simulator (lane C) and
# record cycle counts. A run counts only if the harness prints
# "*** PASSED ***" together with "Completed after N cycles".
#
# Usage:
#   SIM=chipyard/sims/verilator/simulator-chipyard.harness-FastRTLSimLargeBoomV4Config \
#   ELFD=build/zbb/elf OUT=results/boom_zbb scripts/run_chipyard.sh
#
# Environment: SIM, ELFD, OUT required; TIMEOUT seconds (default 3600);
#   JOBS parallel runs (default: number of CPUs);
#   LD_LIBRARY_PATH must contain the directory holding libriscv.so (fesvr).
set -uo pipefail
SIM="${SIM:?set SIM}"; ELFD="${ELFD:?set ELFD}"; OUT="${OUT:?set OUT}"
TIMEOUT="${TIMEOUT:-3600}"; JOBS="${JOBS:-$(nproc)}"
LOGD="$OUT/logs"; CSV="$OUT/cycles.csv"
mkdir -p "$LOGD"
echo "config,kernel,cycles,status" > "$CSV"

run_one() {
  local elf="$1" base cfg kernel log
  base="$(basename "$elf" .riscv)"; cfg="${base%%_*}"; kernel="${base#*_}"
  log="$LOGD/run_${cfg}_${kernel}.log"
  timeout "$TIMEOUT" "$SIM" +permissive +verbose +permissive-off "$elf" 2> >(grep -E "PASSED|FAILED|Completed" > "$log") >/dev/null
  local cyc st
  cyc=$(grep -oP 'Completed after\s+\K[0-9]+' "$log" | head -1)
  st=TIMEOUT
  grep -q PASSED "$log" && st=PASS
  grep -q FAILED "$log" && st=FAIL
  echo "$cfg,$kernel,${cyc:-NA},$st"
}
export -f run_one; export SIM LOGD TIMEOUT
ls "$ELFD"/*.riscv | xargs -P "$JOBS" -I{} bash -c 'run_one "$1"' _ {} >> "$CSV"
echo "wrote $CSV ($(($(wc -l < "$CSV")-1)) runs)"
