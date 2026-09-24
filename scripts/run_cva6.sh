#!/usr/bin/env bash
# Run a directory of ELFs on the CVA6 Verilator model (lanes A and B) and
# record cycle counts. Completion is gated on the rvfi_tracer line
# "terminated after N cycles"; a bare "*** SUCCESS ***" without that line
# means the simulation was killed by the wall-clock timeout and is NOT a result.
#
# Usage:
#   SIM=/path/to/Variane_testharness ELFD=build/zbb/elf OUT=results/zbb scripts/run_cva6.sh
#
# Environment:
#   SIM      required, the fixed CVA6 Verilator binary
#   ELFD     required, directory of <config>_<kernel>.riscv files
#   OUT      output directory; writes $OUT/cycles.csv and $OUT/logs/
#   TIMEOUT  wall-clock seconds per run (default 5400)
#   SIMTO    simulated-cycle cap passed as +time_out (default 400000000)
#   JOBS     parallel runs (default: number of CPUs)
#   LD_LIBRARY_PATH must contain the directory holding libriscv.so.
set -uo pipefail
SIM="${SIM:?set SIM}"; ELFD="${ELFD:?set ELFD}"; OUT="${OUT:?set OUT}"
TIMEOUT="${TIMEOUT:-5400}"; SIMTO="${SIMTO:-400000000}"; JOBS="${JOBS:-$(nproc)}"
LOGD="$OUT/logs"; CSV="$OUT/cycles.csv"
mkdir -p "$LOGD"
echo "config,kernel,cycles,rvfi_term,status,tohost" > "$CSV"

run_one() {
  local elf="$1" base cfg kernel jobdir log
  base="$(basename "$elf" .riscv)"; cfg="${base%%_*}"; kernel="${base#*_}"
  log="$LOGD/run_${cfg}_${kernel}.log"
  jobdir="$(mktemp -d)"   # the model writes multi-GB trace files into its cwd
  ( cd "$jobdir" && timeout "$TIMEOUT" "$SIM" +time_out="$SIMTO" +tohost_addr=80001000 "$elf" >"$log" 2>&1 )
  rm -rf "$jobdir"
  local rv cyc st th
  rv=$(grep -oP 'terminated after\s+\K[0-9]+' "$log" | tail -1)
  cyc=$(grep -oP 'after \K[0-9]+(?= cycles)' "$log" | tail -1)
  st=$(grep -oP '\*\*\* \K(SUCCESS|FAILED)' "$log" | head -1)
  th=$(grep -oP 'tohost = \K[0-9]+' "$log" | head -1)
  echo "$cfg,$kernel,${cyc:-NA},${rv:-NONE},${st:-TIMEOUT},${th:-NA}"
}
export -f run_one; export SIM LOGD TIMEOUT SIMTO
ls "$ELFD"/*.riscv | xargs -P "$JOBS" -I{} bash -c 'run_one "$1"' _ {} >> "$CSV"
echo "wrote $CSV ($(($(wc -l < "$CSV")-1)) runs)"
