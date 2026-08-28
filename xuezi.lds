OUTPUT_ARCH("loongarch")

ENTRY(_start)

MEMORY {
	xipflash (rx) : ORIGIN = 0x1c000000, LENGTH = 1M
	ram (rwx) : ORIGIN = 0x0, LENGTH = 64M
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
		PROVIDE(__text_end = .);
	} > xipflash : text

	.data : {
		PROVIDE(__data_start = .);
		*(*.sdata .sdata*) *(.data .data.*)
		PROVIDE(__data_end = .);
	} > ram AT > xipflash : data

	.bss : {
		PROVIDE(__bss_start = .);
		*(.sbss .sbss.*) *(.bss .bss.*)
		PROVIDE(__bss_end = .);
	} > ram : bss

	PROVIDE(__executable_end = .);
}
