OUTPUT_ARCH( "riscv" )
ENTRY(_start)

SECTIONS
{
  . = 0x80000000;
  .text : { 
    *(.text.init) 
    *(.text) 
    *(.text.*) 
  }
  . = ALIGN(0x1000);
  .tohost : { *(.tohost) }
  . = ALIGN(0x40);
  .rodata : { *(.rodata) *(.rodata.*) }
  .data : { *(.data) *(.data.*) }
  . = ALIGN(8);
  __bss_start = .;
  .bss : { *(.bss) *(.bss.*) *(.sbss) *(.sbss.*) *(COMMON) }
  _end = .;
}
