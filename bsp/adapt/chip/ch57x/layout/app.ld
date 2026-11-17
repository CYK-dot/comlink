/* app.ld - app 镜像布局：0x4000 起，220K（boot 占 0x0000..0x3FFF）。
 *
 * 由 BSP 的 ld/CH572.ld 复制而来，改了 MEMORY 的分区名与 ORIGIN/LENGTH；
 * flash 分区名 APPFLASH，boot 的 bootflash，RAM 统一叫 SRAM；另加 .metainfo 段。
 */

ENTRY( _start )

MEMORY
{
	APPFLASH (rx) : ORIGIN = 0x00004000, LENGTH = 220K
	SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 12K
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
	} >APPFLASH AT>APPFLASH

	/* 镜像信息：紧跟 4 字节的 .init（基址 + 4），由签名脚本回填 */
	.imginfo.magic :
	{
		PROVIDE(_imginfo_magic = .);
		KEEP(*(.imginfo.magic))
		. = ALIGN(4);
	} >APPFLASH AT>APPFLASH

	.imginfo.length :
	{
		PROVIDE(_imginfo_length = .);
		KEEP(*(.imginfo.length))
		. = ALIGN(4);
	} >APPFLASH AT>APPFLASH

	.imginfo.crc32 :
	{
		PROVIDE(_imginfo_crc32 = .);
		KEEP(*(.imginfo.crc32))
		. = ALIGN(4);
	} >APPFLASH AT>APPFLASH

	/* 版本号：签名脚本不写，编译期写死（基址 + 16） */
	.metainfo :
	{
		PROVIDE(_app_metainfo_start = .);
		KEEP(*(.metainfo))
		. = ALIGN(4);
	} >APPFLASH AT>APPFLASH

	.highcode_initlalign :
	{
		. = ALIGN(4);
		PROVIDE(_highcode_init_lma = .);
	} >APPFLASH AT>APPFLASH

	.highcodelalign :
	{
		. = ALIGN(4);
		PROVIDE(_highcode_lma = .);
	} >APPFLASH AT>APPFLASH

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
	} >SRAM AT>APPFLASH

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
	} >APPFLASH AT>APPFLASH

	.fini :
	{
		KEEP(*(SORT_NONE(.fini)))
		. = ALIGN(4);
	} >APPFLASH AT>APPFLASH

	PROVIDE( _etext = . );
	PROVIDE( _eitcm = . );

	.preinit_array :
	{
	  PROVIDE_HIDDEN (__preinit_array_start = .);
	  KEEP (*(.preinit_array))
	  PROVIDE_HIDDEN (__preinit_array_end = .);
	} >APPFLASH AT>APPFLASH

	.init_array :
	{
	  PROVIDE_HIDDEN (__init_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
	  KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
	  PROVIDE_HIDDEN (__init_array_end = .);
	} >APPFLASH AT>APPFLASH

	.fini_array :
	{
	  PROVIDE_HIDDEN (__fini_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
	  KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
	  PROVIDE_HIDDEN (__fini_array_end = .);
	} >APPFLASH AT>APPFLASH

	.ctors :
	{
	  KEEP (*crtbegin.o(.ctors))
	  KEEP (*crtbegin?.o(.ctors))
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
	  KEEP (*(SORT(.ctors.*)))
	  KEEP (*(.ctors))
	} >APPFLASH AT>APPFLASH

	.dtors :
	{
	  KEEP (*crtbegin.o(.dtors))
	  KEEP (*crtbegin?.o(.dtors))
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
	  KEEP (*(SORT(.dtors.*)))
	  KEEP (*(.dtors))
	} >APPFLASH AT>APPFLASH

	.dlalign :
	{
		. = ALIGN(4);
		PROVIDE(_data_lma = .);
	} >APPFLASH AT>APPFLASH

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
	} >SRAM AT>APPFLASH

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
	} >SRAM AT>APPFLASH

	PROVIDE( _end = _ebss);
	PROVIDE( end = . );

	.stack ORIGIN(SRAM)+LENGTH(SRAM) :
	{
		. = ALIGN(4);
		PROVIDE(_eusrstack = . );
	} >SRAM

	/* 头的地址是 boot 与 app 之间的约定（见 app_meta.h），这里守住它 */
	ASSERT(_imginfo_magic == ORIGIN(APPFLASH) + 4,
	       "app: .imginfo.magic must start at image base + 4")
	ASSERT(_imginfo_length == ORIGIN(APPFLASH) + 8,
	       "app: .imginfo.length must start at image base + 8")
	ASSERT(_imginfo_crc32 == ORIGIN(APPFLASH) + 12,
	       "app: .imginfo.crc32 must start at image base + 12")
	ASSERT(_app_metainfo_start == ORIGIN(APPFLASH) + 16,
	       "app: .metainfo must start at image base + 16")
}
