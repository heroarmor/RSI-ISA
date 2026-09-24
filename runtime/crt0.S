# Minimal bare-metal CRT for Chipyard cycle-accurate benchmarking
# Provides: _start, tohost/fromhost, putchar via HTIF

.section .text.init
.globl _start
_start:
    # Set up stack pointer (4KB stack at end of 256MB memory)
    li sp, 0x90000000

    # Clear BSS
    la t0, __bss_start
    la t1, _end
1:  bge t0, t1, 2f
    sd zero, 0(t0)
    addi t0, t0, 8
    j 1b
2:
    # Read cycle counter before main
    rdcycle s0

    # Call main
    jal ra, main

    # Read cycle counter after main
    rdcycle s1
    sub s2, s1, s0    # s2 = cycles elapsed

    # Store cycle count at a known address for extraction
    la t0, _cycle_count
    sd s2, 0(t0)

#ifdef CHECKSUM_EXITCODE
    # Checksum ELF (<config>_<kernel>.chk.riscv): carry the low 31 bits of main's
    # return value in the HTIF exit code, tohost = (checksum31 << 1) | 1. Both the
    # CVA6 harness ("(tohost = N)") and the Chipyard harness ("*** FAILED ***
    # (tohost = N)") print it. This ELF is run only to read the checksum.
    slli t0, a0, 33
    srli t0, t0, 33   # t0 = a0 & 0x7fffffff
    slli t0, t0, 1
    ori  t0, t0, 1
#else
    # Timing ELF (<config>_<kernel>.riscv): always exit 0 so both harnesses print
    # their cycle count (Chipyard prints it only on a successful exit). The kernel
    # object code is identical to the checksum ELF; only this exit stub differs.
    li t0, 1          # (0 << 1) | 1 = exit code 0
#endif
    la t1, tohost
    sd t0, 0(t1)
3:  j 3b

.section .data
.globl _cycle_count
.align 3
_cycle_count: .dword 0

# HTIF tohost/fromhost
.section ".tohost","aw",@progbits
.align 6
.globl tohost
tohost: .dword 0
.align 6
.globl fromhost
fromhost: .dword 0

# Provide weak BSS markers
.section .bss
.globl __bss_start
__bss_start:
