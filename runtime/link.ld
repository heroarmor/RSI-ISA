OUTPUT_ARCH( "riscv" )
ENTRY(_start)

/* One RWX program header. The CVA6 test harness preloads only the ELF
   segment whose address is exactly 0x80000000, so .tohost, .rodata and .data
   must live in the same PT_LOAD as .text or they would be left zeroed. */
PHDRS
{
  all PT_LOAD FLAGS(7);
}

SECTIONS
{
  . = 0x80000000;
  .text : {
    *(.text.init)
    *(.text)
    *(.text.*)
  } :all
  . = ALIGN(0x1000);
  .tohost : { *(.tohost) } :all
  . = ALIGN(0x40);
  .rodata : { *(.rodata) *(.rodata.*) } :all
  .data : { *(.data) *(.data.*) } :all
  . = ALIGN(8);
  __bss_start = .;
  .bss : { *(.bss) *(.bss.*) *(.sbss) *(.sbss.*) *(COMMON) } :all
  _end = .;
}
