// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/platform-ls2k0300-early.S
 *	Copyright 2026 (C) Yao Zi <me@ziyao.cc>.
 */
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

	ret
