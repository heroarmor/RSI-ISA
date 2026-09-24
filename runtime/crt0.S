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

    # Always exit with success (code 0 -> tohost = 1)
    li t0, 1          # (0 << 1) | 1 = 1 = success
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
