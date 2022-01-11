ENTRY( main )

MEMORY
{
  rom : ORIGIN = 0x000000, LENGTH = 0x280000
  ram : ORIGIN = 0x280000, LENGTH = 0x020000
}

/* 描述输出文件的内存布局。 */
SECTIONS 
{
  /* `.`: location counter */
  . = 0x0000;
  .text : { *(.text) } > rom AT > ram
  .rodata : { *(.rodata) }> ram
  .data : { *(.data) }> ram
  .bss : { *(.bss) }> ram
}