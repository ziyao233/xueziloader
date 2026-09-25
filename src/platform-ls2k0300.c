// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/platform-ls2k0300.c
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#include <addrspace.h>
#include <addrmap.h>
#include <barrier.h>
#include <bitops.h>
#include <io.h>
#include <stdio.h>

#define NODE_PLL_CONF0		0x0
#define  PLL_ODIV_MASK		GENMASK(30, 24)
#define  PLL_ODIV_SHIFT		24
#define  PLL_DIV_LOOPC_MASK	GENMASK(23, 15)
#define  PLL_DIV_LOOPC_SHIFT	15
#define  PLL_DIV_REFC_MASK	GENMASK(14, 8)
#define  PLL_DIV_REFC_SHIFT	8
#define  PLL_LOCKED		BIT(7)
#define  PLL_POWERDOWN		BIT(5)
#define  PLL_BYPASS		BIT(4)
#define  PLL_SOFT_SET		BIT(3)
#define NODE_PLL_CONF1		0x4
#define  PLL_ODIV_I2S_MASK	GENMASK(14, 8)
#define  PLL_ODIV_I2S_SHIFT	8
#define  PLL_ODIV_GMAC_MASK	GENMASK(6, 0)
#define  PLL_ODIV_GMAC_SHIFT	0
#define DDR_PLL_CONF0		0x8
#define DDR_PLL_CONF1		0xc
#define  PLL_MEMDIV_MODE_MASK	GENMASK(19, 18)
#define  PLL_MEMDIV_MODE_SHIFT	18
#define  PLL_SOFTMC_RSTN	BIT(17)
#define  PLL_MEMDIV_RSTN	BIT(16)
#define  PLL_ODIV_DEV_MASK	GENMASK(14, 8)
#define  PLL_ODIV_DEV_SHIFT	8
#define  PLL_ODIV_NETWORK_MASK	GENMASK(6, 0)
#define  PLL_ODIV_NETWORK_SHIFT	0
#define PIX_PLL_CONF0		0x10
#define PIX_PLL_CONF1		0x14
#define  PLL_ODIV_GMACBP_MASK	GENMASK(6, 0)
#define  PLL_ODIV_GMACBP_SHIFT	0

static void
wait_pll_locked(volatile void *conf0)
{
	while (!(readl(conf0) & PLL_LOCKED))
		nop();
}

static void
pll_init(volatile void *conf0,
	 unsigned int div, unsigned int mult, unsigned int outDiv,
	 uint32_t sel)
{
	writel(PLL_POWERDOWN, conf0);

	uint32_t reg;
	reg = (div << PLL_DIV_REFC_SHIFT)	|
	      (mult << PLL_DIV_LOOPC_SHIFT)	|
	      (outDiv << PLL_ODIV_SHIFT);
	writel(reg, conf0);

	reg |= PLL_SOFT_SET;
	writel(reg, conf0);

	wait_pll_locked(conf0);

	reg |= sel;
	writel(reg, conf0);
}

void
platform_late_init(void)
{
	volatile uint8_t *clkc = (volatile uint8_t *)TO_UNCACHED(ADDR_CLKC);
	uint32_t reg;

	/*
	 * Initialize PLLs,
	 * - refclk / div must be greater than
	 * - refclk / div * mult must be between 1000MHz - 3200MHz
	 */
	/*
	 * 120MHz / 6 * 100  = 2000MHz
	 * CPU = 2000MHz / 2 = 1000MHz, for CPU node
	 * I2S_MCLK = 2000MHz / 10 = 200MHz
	 * GMAC = 2000MHz / 16 = 125MHz
	 */
	reg = readl(clkc + NODE_PLL_CONF1);
	reg &= ~(PLL_ODIV_I2S_MASK | PLL_ODIV_GMAC_MASK);
	reg |= (10 << PLL_ODIV_I2S_SHIFT)	|
	       (16 << PLL_ODIV_GMAC_SHIFT);
	writel(reg, clkc + NODE_PLL_CONF1);
	pll_init(clkc + NODE_PLL_CONF0, 6, 100, 2, GENMASK(2, 0));

	printf("Node PLL configuration done, 0x%lx 0x%lx\n",
	       readl(clkc + NODE_PLL_CONF0), readl(clkc + NODE_PLL_CONF1));

	/* Ensure UART gets empty before re-clocking its supplier's parent */
	volatile uint8_t *uart = (volatile void *)TO_UNCACHED(ADDR_BOOTCONSOLE);
	while (!(readb(uart + 0x5) & BIT(6)))
		nop();

	/*
	 * 120MHz / 3 * 40 = 1600MHz
	 * DDR4 = 1600MHz / 2 = 800MHz, for DDR4
	 * DEV = 1600MHz / 8 = 200MHz
	 * NETWORK_CLOCK = 1600MHz / 8 = 200MHz
	 */
	reg = readl(clkc + DDR_PLL_CONF1);
	reg &= ~(PLL_ODIV_DEV_MASK | PLL_ODIV_NETWORK_MASK | PLL_MEMDIV_MODE_MASK);
	reg |= (8 << PLL_ODIV_DEV_SHIFT)		|
	       (8 << PLL_ODIV_NETWORK_SHIFT)		|
	       (1 << PLL_MEMDIV_MODE_SHIFT);
	writel(reg, clkc + DDR_PLL_CONF1);

	/* Reset memdiv (what's this?) */
	reg &= ~PLL_MEMDIV_RSTN;
	writel(reg, clkc + DDR_PLL_CONF1);
	reg |= PLL_MEMDIV_RSTN;
	writel(reg, clkc + DDR_PLL_CONF1);

	pll_init(clkc + DDR_PLL_CONF0, 3, 40, 2, GENMASK(2, 0));

	/*
	 * Re-clock UART immediately, 200MHz / 16 / 115200 ~= 108.51, i.e.
	 * DL_H = 0, DL_L = 108, DL_D = 51
	 */
	// Open divisor settings
	writeb(0x83, uart + 0x3);

	writeb(108,	uart + 0x0);
	writeb(0,	uart + 0x1);
	writeb(51,	uart + 0x2);

	// Close divisor settings
	writeb(0x3, uart + 0x3);

	printf("DDR PLL configuration done, 0x%lx 0x%lx\n",
	       readl(clkc + DDR_PLL_CONF0), readl(clkc + DDR_PLL_CONF1));

	/*
	 * 120MHz / 3 * 50 = 2000MHz
	 * PIX = 2000MHz / 20 = 100MHz
	 * GMAC_BP = 2000MHz / 16 = 125MHz
	 */
	reg = readl(clkc + PIX_PLL_CONF1);
	reg &= ~PLL_ODIV_GMACBP_MASK;
	reg |= 16 << PLL_ODIV_GMACBP_SHIFT;
	writel(reg, clkc + PIX_PLL_CONF1);

	pll_init(clkc + PIX_PLL_CONF0, 3, 50, 20, GENMASK(1, 0));

	printf("PIX PLL configuration done, 0x%lx 0x%lx\n",
	       readl(clkc + PIX_PLL_CONF0), readl(clkc + PIX_PLL_CONF1));
}
