// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/start.S
 *	Copyright (C) 2026 Yao Zi.
 */

#include <addrmap.h>

	.section .text.init
	.global		_start

_start:
	bl		platform_early_init

uart_init:
	/*
	 * Select pinmux for UART, configuring GPIO32-47 pinmux cfg register
	 * (GPIO pinmux configuration register 2)
	 */
	li.d		$t0,		0x16000498

	/*
	 * For GPIO40, GPIO41 (17:16 and 19:18), selecting the main function
	 * (index 0x3), i.e., UART0. GPIO32-47 is cleared on boot, so
	 * overwriting is okay.
	 */
	li.w		$t1,		(0x3 << 18) | (0x3 << 16)
	st.w		$t1,		$t0,		0

	li.d		$t0,		ADDR_BOOTCONSOLE

	/* Set LCR[7] so we could configure divisors */
	li.w		$t1,		0x83
	st.b		$t1,		$t0,		 0x3

	/*
	 * Configuring UART clock, by default PLLs are bypassed, so clk_apb
	 * gets feed by 120MHz refclk directly, divides it by 1
	 * (the default apb_freqscale), and feeds it into UART.
	 *
	 * 120MHz / 16 / 115200 ~= 65.10, i.e.,
	 * DL_H = 0, DL_L = 65, DL_D = 10
	 */
	st.b		$t1,		$zero,		0x1

	li.w		$t1,		65
	st.b		$t1,		$t0,		0x0

	li.w		$t1,		10
	st.b		$t1,		$t0,		0x2

	/* Closing divisor registers */
	li.w		$t1,		0x3
	st.b		$t1,		$t0,		 0x3

	li.w		$a0,		's'
	bl		putc

.hlt:
	nop
	b		.hlt

putc:
	li.w		$t0,		ADDR_BOOTCONSOLE

.poll:
	ld.b		$t1,		$t0,		0x5
	andi		$t1,		$t1,		(1 << 5)
	beqz		$t1,		.poll

	st.b		$a0,		$t0,		0x0

	ret

	.data

hello:	.asciz		"hello world\r\n"
