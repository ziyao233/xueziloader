// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/platform-ls2k0300-early.S
 *	Copyright 2026 (C) Yao Zi <me@ziyao.cc>.
 */

#include <addrmap.h>

	.text
	.global		platform_early_init

platform_early_init:
	/*
	 * Enable ddr4_regs_default, somehow this affects instruction fetching
	 * behavior after jumping out of SPI0 BOOT. Maybe it's caused by
	 * speculative execution.
	 */
	li.d		$t0,		0x16000100
	ld.w		$t1,		$t0,		0
	ori		$t1,		$t1,		1 << 5
	st.w		$t1,		$t0,		0

	/*
	 * Enable burst and fast-read mode for SPI flash, and double the
	 * frequency to speed up instruction fetch.
	 */
	li.d		$t0,		ADDR_SPI0CFG
	li.w		$t1,		(0x4 << 4) | \
					(1 << 2) | (1 << 1) | (1 << 0)
	st.b		$t1,		$t0,		4

	ret
