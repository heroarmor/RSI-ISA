#!/usr/bin/env bash
# Run a directory of kernel ELFs on the CVA6 Verilator model (lanes A and B).
# For each <config>_<kernel>.riscv the timing ELF gives the cycle count and the
# matching <config>_<kernel>.chk.riscv gives the checksum via the exit code.
#
# Completion gate: the rvfi_tracer line "terminated after N cycles" must be
# present in the timing run; a harness "*** SUCCESS/FAILED ***" line without it
# means the wall-clock timeout killed the run and the cycle count is meaningless.
#
# Usage:
#   SIM=/path/to/Variane_testharness ELFD=build/zbb/elf OUT=results/zbb scripts/run_cva6.sh
# Environment:
#   SIM, ELFD, OUT   required
#   TIMEOUT          wall-clock seconds per run (default 5400)
#   SIMTO            simulated-cycle cap passed as +time_out (default 400000000)
#   JOBS             parallel runs (default: number of CPUs)
#   LD_LIBRARY_PATH  must contain the directory holding libriscv.so
set -uo pipefail
SIM="${SIM:?set SIM}"; ELFD="${ELFD:?set ELFD}"; OUT="${OUT:?set OUT}"
TIMEOUT="${TIMEOUT:-5400}"; SIMTO="${SIMTO:-400000000}"; JOBS="${JOBS:-$(nproc)}"
LOGD="$OUT/logs"; CSV="$OUT/cycles.csv"
mkdir -p "$LOGD"

sim_one() {  # $1 = elf, $2 = log ; runs in a private tmp dir (the model dumps trace files in cwd)
  local jobdir; jobdir="$(mktemp -d)"
  ( cd "$jobdir" && timeout "$TIMEOUT" "$SIM" +time_out="$SIMTO" +tohost_addr=80001000 "$1" >"$2" 2>&1 )
  rm -rf "$jobdir"
}
run_kernel() {
  local elf="$1" base cfg kernel tlog clog rv cyc ck
  base="$(basename "$elf" .riscv)"; cfg="${base%%_*}"; kernel="${base#*_}"
  tlog="$LOGD/time_${cfg}_${kernel}.log"; clog="$LOGD/chk_${cfg}_${kernel}.log"
  sim_one "$elf" "$tlog" & sim_one "${elf%.riscv}.chk.riscv" "$clog" & wait
  rv=$(grep -oP 'terminated after\s+\K[0-9]+' "$tlog" | tail -1)
  cyc=$(grep -oP '\*\*\* SUCCESS \*\*\*.*after \K[0-9]+(?= cycles)' "$tlog" | tail -1)
  ck=$(grep -oP 'tohost = \K[0-9]+' "$clog" | head -1)
  grep -q 'terminated after' "$clog" || ck=""      # checksum run must itself have finished
  local st=TIMEOUT; [ -n "$rv" ] && [ -n "$cyc" ] && st=DONE
  echo "$cfg,$kernel,${cyc:-NA},${rv:-NONE},$st,${ck:-NA}"
}
export -f sim_one run_kernel; export SIM LOGD TIMEOUT SIMTO
echo "config,kernel,cycles,rvfi_term,status,checksum" > "$CSV"
ls "$ELFD"/*.riscv | grep -v '\.chk\.riscv$' | xargs -P "$JOBS" -I{} bash -c 'run_kernel "$1"' _ {} >> "$CSV"
echo "wrote $CSV ($(($(wc -l < "$CSV")-1)) kernels)"
