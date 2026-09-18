// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/platform-ls2k0300-early.S
 *	Copyright 2026 (C) Yao Zi <me@ziyao.cc>.
 */

#include <addrmap.h>
#include <csr.h>

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

	/*
	 * Override CPUCFG5, on LS2K0300 it carries wrong frequency (100MHz v.s.
	 * 120MHz).
	 */
	// Reference clock 120MHz, which is multipled by one, then divided by
	// one, then fed into the constant frequency timer.
	li.d		$t0,		(1 << 48) | (1 << 32) | 120000000
	csrwr		$t0,		LOONGARCH_CSR_MCSR2

	/*
	 * Override cache index size
	 */
	li.d		$t0,		(10 << (16 + 32))
	li.d		$t1,		(0xff << (16 + 32))
	csrxchg		$t0,		$t1,		LOONGARCH_CSR_MCSR9

	/*
	 * Enable auto flush store forwarding buffer, disable fast ld.q which
	 * isn't supported by 2K0300.
	 */
	li.w		$t0,		CSR_AUTO_FLUSHSFB
	csrxchg		$t0,		$t0,		LOONGARCH_CSR_IMPCTL1

	li.w		$t0,		CSR_FASTLDQ
	csrxchg		$zero,		$t0,		LOONGARCH_CSR_IMPCTL1

	/*
	 * Allow perf counter to be read from PLV0
	 */
	li.w		$t0,		CSR_PERFCTRL_PLV0
	csrxchg		$t0,		$t0,		LOONGARCH_CSR_PERFCTRL0

	/*
	 * Enable power-saving mechanisms, we don't know what they really do.
	 * The *CG bits might refer to clock gating.
	 */
	li.w		$t0,		MCSR24_NAPEN | \
					MCSR24_VPUCG | \
					MCSR24_RAMCG
	csrxchg		$t0,		$t0,		LOONGARCH_CSR_MCSR24

	/*
	 * TODO:
	 * - Vendor U-Boot overrides BIT(24) of PRID, the purpose is unknown
	 */

	/* Make USB/DC/GMAC/APB devices coherent and cachable */
	li.d		$t0,		0x1600010c
	ld.w		$t1,		$t0,		0
	li.d		$t2,		(0x1f << 9) | (0x1f << 1)
	or		$t1,		$t1,		$t2
	st.w		$t1,		$t0,		0

	ret
