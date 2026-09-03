OUTPUT_ARCH("loongarch")

ENTRY(_start)

MEMORY {
	xipflash (rx) : ORIGIN = 0x900000001c000000, LENGTH = 1M
	scache (rwx) : ORIGIN = 0x9000000200000000, LENGTH = 256K
}

PHDRS {
	text	PT_LOAD;
	data	PT_LOAD;
	bss	PT_LOAD;
}

SECTIONS {
	.text : {
		PROVIDE(__text_start = .);
		*(.text.init) *(.text .text.*)
		. = ALIGN(8);
		PROVIDE(__text_end = .);
	} > xipflash : text

	.data : {
		PROVIDE(__data_start = .);
		*(*.sdata .sdata*) *(.data .data.*)
		PROVIDE(__data_end = .);
	} > scache AT > xipflash : data

	.bss : {
		. = ALIGN(8);
		PROVIDE(__bss_start = .);
		*(.sbss .sbss.*) *(.bss .bss.*)
		PROVIDE(__bss_end = .);
	} > scache : bss

	PROVIDE(__executable_end = .);
}
