// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/start.S
 *	Copyright (C) 2026 Yao Zi.
 */

#include <addrmap.h>
#include <addrspace.h>
#include <csr.h>

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

	li.w		$a0,		'u'
	bl		putc

lock_scache:
	/*
	 * "Locking" a scache prevents cache lines hitting the configured
	 * window being swapped out, making use of this to create a scratch
	 * SRAM to work on.
	 */
	li.d		$t0,		ADDR_SCACHE

	li.d		$t1,		~(SIZE_LOCKED_SCACHE - 1)
	st.d		$t1,		$t0,		0x40

	li.d		$t1,		ADDR_LOCKED_SCACHE | (1 << 63)
	st.d		$t1,		$t0,		0x0

	li.w		$a0,		's'
	bl		putc

enable_mmu:
	/*
	 * Trampoline to enable MMU, setup temporary DMW setttings,
	 *	0x0000 0000 0000 0000 -> strong ordered, uncached, where we're
	 *				 now
	 * 	0x9000 0000 0000 0000 -> coherent, cached
	 */
	li.d		$t0,		(0x0 << 60) | (0x0 << 4) | (0x1 << 0)
	csrwr		$t0,		LOONGARCH_CSR_DMWIN0
	li.d		$t0,		(0x9 << 60) | (0x1 << 4) | (0x1 << 0)
	csrwr		$t0,		LOONGARCH_CSR_DMWIN1


	li.d		$t0,		CSR_CRMD_PG
	csrwr		$t0,		LOONGARCH_CSR_CRMD

	la.abs		$ra,		.mmu_enabled
	jr		$ra

.mmu_enabled:
	/*
	 * Clean up the temporary DWM0 and setup it as
	 *	0x8000 0000 0000 0000 -> strong ordered, uncached
	 *
	 * Now the address layout looks similar to the one used by Linux kernel.
	 */
	li.d		$t0,		(0x8 << 60) | (0x0 << 4) | (0x1 << 0)
	csrwr		$t0,		LOONGARCH_CSR_DMWIN0


	li.w		$a0,		'm'
	bl		putc

	/* We could finally prepare for C execution */
	la.abs		$t0,		__text_end
	la.abs		$t1,		__data_start
	la.abs		$t2,		__data_end

	// $t2 = (__data_end - __data_start + 7) >> 3
	sub.d		$t2,		$t2,		$t1
	addi.d		$t2,		$t2,		7
	srli.d		$t2,		$t2,		3

.copy_data:
	ld.d		$t3,		$t0,		0
	st.d		$t3,		$t1,		0
	addi.d		$t0,		$t0,		8
	addi.d		$t1,		$t1,		8
	addi.d		$t2,		$t2,		-1
	bnez		$t2,		.copy_data

	li.w		$a0,		'd'
	bl		putc


	la.abs		$t0,		__bss_start
	la.abs		$t1,		__bss_end
.clear_bss:
	st.d		$zero,		$t0,		0
	addi.d		$t0,		$t0,		8
	blt		$t0,		$t1,		.clear_bss

	li.w		$a0,		'b'
	bl		putc

.setup_stack:
	li.d		$sp,		TO_CACHED(ADDR_LOCKED_SCACHE) + \
					SIZE_LOCKED_SCACHE

	b		main

	.global		rawputc
rawputc:
putc:
	// Fortunately, in direct address translation mode, only [PALEN-1:0] of
	// virtual addresses are considered when accessing memory. So this
	// TO_UNCACHED() conversion is also valid in DA mode.
	li.d		$t0,		TO_UNCACHED(ADDR_BOOTCONSOLE)

.poll:
	ld.b		$t1,		$t0,		0x5
	andi		$t1,		$t1,		(1 << 5)
	beqz		$t1,		.poll

	st.b		$a0,		$t0,		0x0

	ret

	.data

hello:	.asciz		"hello world\r\n"
