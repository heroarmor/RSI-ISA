#!/usr/bin/env bash
# Build the 13 kernels into bare-metal ELFs for one -march configuration.
#
# Usage:
#   MARCH=rv64g_zbb CONFIG=zbb scripts/build_kernels.sh            # all kernels
#   MARCH=rv64g     CONFIG=base scripts/build_kernels.sh sha aes   # a subset
#
# Environment:
#   MARCH   required, e.g. rv64g, rv64g_zbb, or rv64g_zbb_x<vendor-ext>
#   CONFIG  label used in output file names (default: $MARCH)
#   CLANG   clang driver (default: clang-18). A stock LLVM 18 build is the
#           reference toolchain for the base and zbb configurations.
#   GCC     riscv64-unknown-elf-gcc used only for linking (default in PATH)
#   OUT     output directory (default: build/$CONFIG)
#
# Output: $OUT/elf/${CONFIG}_<kernel>.riscv plus .s and .o for inspection.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MARCH="${MARCH:?set MARCH, e.g. rv64g or rv64g_zbb}"
CONFIG="${CONFIG:-$MARCH}"
CLANG="${CLANG:-clang-18}"
GCC="${GCC:-riscv64-unknown-elf-gcc}"
OUT="${OUT:-$ROOT/build/$CONFIG}"
RT="$ROOT/runtime"
ALL=(adpcm aes basicmath bitcount crc32 dijkstra fft fir_filter matmul qsort sha sha256_full stringsearch)
KERNELS=("$@"); [ ${#KERNELS[@]} -eq 0 ] && KERNELS=("${ALL[@]}")
mkdir -p "$OUT/elf" "$OUT/asm" "$OUT/obj"

CF=(--target=riscv64-unknown-elf -march="$MARCH" -mabi=lp64d -mcmodel=medany -fno-builtin -nostdlib -I"$RT")
LF=(-march=rv64imafdc -mabi=lp64d -mcmodel=medany -static -nostartfiles -nostdlib -T "$RT/link.ld")

"$CLANG" "${CF[@]}" -O2 -c "$RT/crt0.S"     -o "$OUT/obj/crt0.o"
"$CLANG" "${CF[@]}" -O2 -c "$RT/syscalls.c" -o "$OUT/obj/syscalls.o"
for k in "${KERNELS[@]}"; do
  src="$ROOT/kernels/$k.c"
  [ -f "$src" ] || { echo "no such kernel: $k" >&2; exit 2; }
  "$CLANG" "${CF[@]}" -O3 -S "$src" -o "$OUT/asm/$k.s"
  "$CLANG" "${CF[@]}" -O3 -c "$src" -o "$OUT/obj/$k.o"
  "$GCC" "${LF[@]}" "$OUT/obj/crt0.o" "$OUT/obj/syscalls.o" "$OUT/obj/$k.o" -lgcc \
       -o "$OUT/elf/${CONFIG}_$k.riscv"
  echo "built $OUT/elf/${CONFIG}_$k.riscv"
done
