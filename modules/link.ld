ENTRY(start)
SECTIONS {
	.module_info 0x0 :
	{
		*(.module_info)
		. = ALIGN(4096);
	}

	.text 0x01000000 :
	{
		code = .; _code = .; __code = .;
		*(.text)
 		. = ALIGN(4096);
	}

	.data :
	{
		data = .; _data = .; __data = .;
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}

	.bss :
	{
		bss = .; _bss = .; __bss = .;
		*(.bss)
		. = ALIGN(4096);
	}

	end = .; _end = .; __end = .;
}