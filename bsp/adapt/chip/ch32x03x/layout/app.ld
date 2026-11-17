/* app.ld - app 镜像布局：0x2000 起，54K（0x2000..0xF7FF）；flash 分区名 APPFLASH。
 * SRAM 分成两段：RV32SRAM（16K，本镜像可用）与 PIOCSRAM（4K，PIOC 代码运行区）。
 * PIOCSRAM 在这里声明但不放段，app 不得占用它。
 */

ENTRY( _start )

__stack_size = 2048;

PROVIDE( _stack_size = __stack_size );


MEMORY
{  
	APPFLASH (rx) : ORIGIN = 0x00002000, LENGTH = 54K
	RV32SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 16K
	PIOCSRAM (xrw) : ORIGIN = 0x20004000, LENGTH = 4K
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

	/* 镜像头：紧跟 .init（基址 + 4），由签名脚本回填 */
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

	/* 版本号：编译期写入（基址 + 16） */
	.metainfo :
	{
		PROVIDE(_app_metainfo_start = .);
		KEEP(*(.metainfo))
		. = ALIGN(4);
	} >APPFLASH AT>APPFLASH
  	.vector :
  	{
      *(.vector);
	  . = ALIGN(64);
  	} >APPFLASH AT>APPFLASH

	.text :
	{
		. = ALIGN(4);
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata*)
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

	.preinit_array  :
	{
	  PROVIDE_HIDDEN (__preinit_array_start = .);
	  KEEP (*(.preinit_array))
	  PROVIDE_HIDDEN (__preinit_array_end = .);
	} >APPFLASH AT>APPFLASH 
	
	.init_array     :
	{
	  PROVIDE_HIDDEN (__init_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
	  KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
	  PROVIDE_HIDDEN (__init_array_end = .);
	} >APPFLASH AT>APPFLASH 
	
	.fini_array     :
	{
	  PROVIDE_HIDDEN (__fini_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
	  KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
	  PROVIDE_HIDDEN (__fini_array_end = .);
	} >APPFLASH AT>APPFLASH 
	
	.ctors          :
	{
	  /* gcc uses crtbegin.o to find the start of
	     the constructors, so we make sure it is
	     first.  Because this is a wildcard, it
	     doesn't matter if the user does not
	     actually link against crtbegin.o; the
	     linker won't look for a file to match a
	     wildcard.  The wildcard also means that it
	     doesn't matter which directory crtbegin.o
	     is in.  */
	  KEEP (*crtbegin.o(.ctors))
	  KEEP (*crtbegin?.o(.ctors))
	  /* We don't want to include the .ctor section from
	     the crtend.o file until after the sorted ctors.
	     The .ctor section from the crtend file contains the
	     end of ctors marker and it must be last */
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
	  KEEP (*(SORT(.ctors.*)))
	  KEEP (*(.ctors))
	} >APPFLASH AT>APPFLASH 
	
	.dtors          :
	{
	  KEEP (*crtbegin.o(.dtors))
	  KEEP (*crtbegin?.o(.dtors))
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
	  KEEP (*(SORT(.dtors.*)))
	  KEEP (*(.dtors))
	} >APPFLASH AT>APPFLASH 

	.dalign :
	{
		. = ALIGN(4);
		PROVIDE(_data_vma = .);
	} >RV32SRAM AT>APPFLASH	

	.dlalign :
	{
		. = ALIGN(4); 
		PROVIDE(_data_lma = .);
	} >APPFLASH AT>APPFLASH

	.data :
	{
    	*(.gnu.linkonce.r.*)
    	*(.data .data.*)
    	*(.gnu.linkonce.d.*)
		. = ALIGN(8);
    	PROVIDE( __global_pointer$ = . + 0x800 );
    	*(.sdata .sdata.*)
		*(.sdata2.*)
    	*(.gnu.linkonce.s.*)
    	. = ALIGN(8);
    	*(.srodata.cst16)
    	*(.srodata.cst8)
    	*(.srodata.cst4)
    	*(.srodata.cst2)
    	*(.srodata .srodata.*)
    	. = ALIGN(4);
		PROVIDE( _edata = .);
	} >RV32SRAM AT>APPFLASH

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
	} >RV32SRAM AT>APPFLASH

	PROVIDE( _end = _ebss);
	PROVIDE( end = . );

    .stack ORIGIN(RV32SRAM) + LENGTH(RV32SRAM) - __stack_size :
    {
        PROVIDE( _heap_end = . );   
        . = ALIGN(4);
        PROVIDE(_susrstack = . );
        . = . + __stack_size;
        PROVIDE( _eusrstack = .);
    } >RV32SRAM 

	/* 头的地址是 boot 与 app 之间的约定（见 bsp/inc/app_meta.h） */
	ASSERT(_imginfo_magic == ORIGIN(APPFLASH) + 4, "app: .imginfo.magic must be at image base + 4")
	ASSERT(_imginfo_length == ORIGIN(APPFLASH) + 8, "app: .imginfo.length must be at image base + 8")
	ASSERT(_imginfo_crc32 == ORIGIN(APPFLASH) + 12, "app: .imginfo.crc32 must be at image base + 12")
	ASSERT(_app_metainfo_start == ORIGIN(APPFLASH) + 16, "app: .metainfo must be at image base + 16")

	/* PIOC 运行区必须紧接在 RV32 可用区之上，且不得与它重叠 */
	ASSERT(ORIGIN(PIOCSRAM) >= ORIGIN(RV32SRAM) + LENGTH(RV32SRAM),
	       "app: PIOCSRAM must not overlap RV32SRAM")
}
