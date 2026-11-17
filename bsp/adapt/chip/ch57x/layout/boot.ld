/* boot.ld - bootloader 镜像布局：0x0000 起，16K（0x0000..0x3FFF）。
 *
 * 由 BSP 的 ld/CH572.ld 复制而来，改了 MEMORY 的分区名与 boot 槽长度：
 * 本镜像只能占 boot 槽，app 从 0x4000 起，两区不得重叠——见文件末尾的 ASSERT。
 * 段序与符号必须保持不变，启动汇编依赖它们。
 */

ENTRY( _start )

MEMORY
{
	BOOTFLASH (rx) : ORIGIN = 0x00000000, LENGTH = 16K
	SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 12K
	APPFLASH (rx) : ORIGIN = 0x00004000, LENGTH = 220K
}

SECTIONS
{
	.init :
	{
		_sinit = .;
		. = ALIGN(4);
		KEEP(*(SORT_NONE(.init)))
		. = ALIGN(4);
		_einit = .;
	} >BOOTFLASH AT>BOOTFLASH

	.highcode_initlalign :
	{
		. = ALIGN(4);
		PROVIDE(_highcode_init_lma = .);
	} >BOOTFLASH AT>BOOTFLASH

	.highcodelalign :
	{
		. = ALIGN(4);
		PROVIDE(_highcode_lma = .);
	} >BOOTFLASH AT>BOOTFLASH

	.highcode :
	{
		. = ALIGN(4);
		PROVIDE(_highcode_vma_start = .);
		. = ALIGN(1024);
		*(.vector);
		*(.highcode);
		*(.highcode.*);
		. = ALIGN(4);
		PROVIDE(_highcode_vma_end = .);
	} >SRAM AT>BOOTFLASH

	.text :
	{
		. = ALIGN(4);
		KEEP(*(SORT_NONE(.handle_reset)))
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata*)
		*(.sdata2.*)
		*(.glue_7)
		*(.glue_7t)
		*(.gnu.linkonce.t.*)
		. = ALIGN(4);
	} >BOOTFLASH AT>BOOTFLASH

	.fini :
	{
		KEEP(*(SORT_NONE(.fini)))
		. = ALIGN(4);
	} >BOOTFLASH AT>BOOTFLASH

	PROVIDE( _etext = . );
	PROVIDE( _eitcm = . );

	.preinit_array :
	{
	  PROVIDE_HIDDEN (__preinit_array_start = .);
	  KEEP (*(.preinit_array))
	  PROVIDE_HIDDEN (__preinit_array_end = .);
	} >BOOTFLASH AT>BOOTFLASH

	.init_array :
	{
	  PROVIDE_HIDDEN (__init_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
	  KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
	  PROVIDE_HIDDEN (__init_array_end = .);
	} >BOOTFLASH AT>BOOTFLASH

	.fini_array :
	{
	  PROVIDE_HIDDEN (__fini_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
	  KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
	  PROVIDE_HIDDEN (__fini_array_end = .);
	} >BOOTFLASH AT>BOOTFLASH

	.ctors :
	{
	  KEEP (*crtbegin.o(.ctors))
	  KEEP (*crtbegin?.o(.ctors))
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
	  KEEP (*(SORT(.ctors.*)))
	  KEEP (*(.ctors))
	} >BOOTFLASH AT>BOOTFLASH

	.dtors :
	{
	  KEEP (*crtbegin.o(.dtors))
	  KEEP (*crtbegin?.o(.dtors))
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
	  KEEP (*(SORT(.dtors.*)))
	  KEEP (*(.dtors))
	} >BOOTFLASH AT>BOOTFLASH

	.dlalign :
	{
		. = ALIGN(4);
		PROVIDE(_data_lma = .);
	} >BOOTFLASH AT>BOOTFLASH

	.data :
	{
		. = ALIGN(4);
		PROVIDE(_data_vma = .);
		*(.gnu.linkonce.r.*)
		*(.data .data.*)
		*(.gnu.linkonce.d.*)
		. = ALIGN(8);
		PROVIDE( __global_pointer$ = . + 0x800 );
		*(.sdata .sdata.*)
		*(.gnu.linkonce.s.*)
		. = ALIGN(8);
		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)
		. = ALIGN(4);
		PROVIDE( _edata = .);
	} >SRAM AT>BOOTFLASH

	.bss :
	{
		. = ALIGN(4);
		PROVIDE( _sbss = .);
		*(.sbss*)
		*(.gnu.linkonce.sb.*)
		*(.bss*)
		*(.gnu.linkonce.b.*)
		*(COMMON*)
		. = ALIGN(4);
		PROVIDE( _ebss = .);
	} >SRAM AT>BOOTFLASH

	PROVIDE( _end = _ebss);
	PROVIDE( end = . );

	/* The app slot: declared only so this image can read its bounds as symbols
	 * (bspboot.c); no section of the bootloader is placed in it. */
	PROVIDE(_app_image_base  = ORIGIN(APPFLASH));
	PROVIDE(_app_image_limit = ORIGIN(APPFLASH) + LENGTH(APPFLASH));

	.stack ORIGIN(SRAM)+LENGTH(SRAM) :
	{
		. = ALIGN(4);
		PROVIDE(_eusrstack = . );
	} >SRAM

	/* 分区守卫：boot 槽与 app 槽不得重叠 */
	ASSERT(ORIGIN(APPFLASH) >= ORIGIN(BOOTFLASH) + LENGTH(BOOTFLASH),
	       "boot: the application region overlaps the boot region")
}
