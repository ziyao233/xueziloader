// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	platform/ls2k0300/late.c
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#include <addrspace.h>
#include <addrmap.h>
#include <barrier.h>
#include <bitops.h>
#include <io.h>
#include <stdio.h>
#include <stddef.h>

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

static void
clock_init(void)
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

typedef struct {
	uint64_t enable_early_printf		: 1;
	uint64_t ddr_param_store		: 1;
	uint64_t ddr3_dimm			: 1;
	uint64_t low_speed			: 1;
	uint64_t auto_ddr_config		: 1;
	uint64_t enable_ddr_leveling		: 1;
	uint64_t print_ddr_leveling		: 1;
	uint64_t enable_mc_vref_training	: 1;
	uint64_t vref_training_debug		: 1;
	uint64_t enable_ddr_vref_training	: 1;
	uint64_t enable_bit_training		: 1;
	uint64_t bit_training_debug		: 1;
	uint64_t enable_write_training		: 1;
	uint64_t debug_write_training		: 1;
	uint64_t print_dll_sample		: 1;
	uint64_t disable_dq_odt_training	: 1;
	uint64_t lvl_debug			: 1;
	uint64_t disable_dram_crc		: 1;
	uint64_t two_t_mode_enable		: 1;
	uint64_t disable_dimm_ecc		: 1;
	uint64_t disable_read_dbi		: 1;
	uint64_t disable_write_dbi		: 1;
	uint64_t disable_dm			: 1;
	uint64_t preamble2			: 1;
	uint64_t set_by_protocol		: 1;
	uint64_t param_set_from_spd_debug	: 1;
	uint64_t refresh_1x			: 1;
	uint64_t spd_only			: 1;
	uint64_t ddr_debug_param		: 1;
	uint64_t ddr_soft_clksel		: 1;
	uint64_t str				: 1;
	uint64_t pda_mode			: 1;
	uint64_t signal_test			: 1;
} ddr_feature;

typedef struct {
	uint64_t rl_manualy	   : 5;
	uint64_t bit_width	   : 7;
	uint64_t nc16_map	   : 3;
	uint64_t mc_vref_adjust    : 5;
	uint64_t gate_mode	   : 2;
	uint64_t pad_reset_po      : 2;
	uint64_t wrlevel_count_low : 8;
	uint64_t ref_manualy       : 32;
} ddr_param;

typedef struct {
	uint8_t		RCD;
	uint8_t 	RP;
	uint8_t		RAS;
	uint16_t	REF;
	uint16_t	RFC;
	uint8_t		dll_ck_mc0;
	uint8_t		dll_ck_mc1;
	uint8_t		dll_ck_mc2;
	uint8_t		dll_ck_mc3;
} parameter;

typedef struct {
	char     mc_vref_adjust;
	char     ddr_vref_adjust;
	uint8_t  vref_range;
	uint8_t  vref_value;
	uint8_t  vref_init;
	uint8_t  vref_bits_per;
	uint8_t  vref_bit;
} vref_param;

typedef struct {
	uint64_t rtt_nom_1r_1slot	: 4;
	uint64_t rtt_nom_2r_1slot	: 4;
	uint64_t rtt_nom_1r_2slot_cs0	: 4;
	uint64_t rtt_nom_1r_2slot_cs1	: 4;
	uint64_t rtt_nom_2r_2slot_cs0	: 4;
	uint64_t rtt_nom_2r_2slot_cs2	: 4;
	uint64_t rtt_park_1r_1slot	: 4;
	uint64_t rtt_park_2r_1slot	: 4;
	uint64_t rtt_park_1r_2slot_cs0	: 4;
	uint64_t rtt_park_1r_2slot_cs1	: 4;
	uint64_t rtt_park_2r_2slot_cs0	: 4;
	uint64_t rtt_park_2r_2slot_cs2	: 4;
	uint64_t mc_dqs_odt_1cs		: 4;
	uint64_t mc_dq_odt_1cs		: 4;
	uint64_t mc_dqs_odt_2cs		: 4;
	uint64_t mc_dq_odt_2cs		: 4;
} ddr_odt;

typedef struct {
	uint64_t pad_ds_split;
	uint8_t pad_clk_ocd;
	uint8_t pad_ctrl_ocd;
} pad_ocd;

typedef struct {
	uint8_t mc0_enable;
	uint8_t mc1_enable;

	uint32_t mc0_memsize;
	uint8_t mc0_dram_type;
	uint8_t mc0_dimm_type;
	uint8_t mc0_module_type;
	uint8_t mc0_cid_num;
	uint8_t mc0_ba_num;
	uint8_t mc0_bg_num;
	uint8_t mc0_csmap;
	uint8_t mc0_dram_width;
	uint8_t mc0_module_width;
	uint8_t mc0_ecc;
	uint8_t mc0_sdram_capacity;
	uint8_t mc0_col_num;
	uint8_t mc0_row_num;
	uint8_t mc0_addr_mirror;
	uint8_t mc0_bg_mirror;

	uint32_t mc1_memsize;
	uint8_t mc1_dram_type;
	uint8_t mc1_dimm_type;
	uint8_t mc1_module_type;
	uint8_t mc1_cid_num;
	uint8_t mc1_ba_num;
	uint8_t mc1_bg_num;
	uint8_t mc1_csmap;
	uint8_t mc1_dram_width;
	uint8_t mc1_module_width;
	uint8_t mc1_ecc;
	uint8_t mc1_sdram_capacity;
	uint8_t mc1_col_num;
	uint8_t mc1_row_num;
	uint8_t mc1_addr_mirror;
	uint8_t mc1_bg_mirror;
} paster_t;

typedef struct {
	uint16_t	param_offset;
	uint64_t	param_change;
} param_array;

typedef struct {
	uint8_t		node_id;
	uint8_t		mc_id;
	param_array	*param;
} param_debug;

typedef struct {
	uint32_t mc_type;
	uint32_t spi_base;
	uint64_t uart_base;
	uint64_t l2xbar_conf_addr;
	uint64_t mc_regs_base;
	uint64_t cache_mem_base;
	uint64_t mem_base;
	uint64_t ddr_freq;
	uint64_t ddr_freq_2slot;
	uint64_t dimm_info_in_flash_offset;
	uint64_t sameba_adj;
	uint64_t samebg_adj;
	uint64_t samec_adj;
	uint64_t samecs_adj;
	uint64_t diffcs_adj;
	uint64_t mc_interleave_offset;
	uint16_t ref_clk;
	uint8_t node_offset;
	uint8_t tot_node_num;
	uint8_t node_mc_num;
	uint8_t	channel_width;
	uint8_t	dll_bypass;
	ddr_feature table;
	ddr_param data;
	parameter param;
	vref_param vref;
	ddr_odt odt;
	pad_ocd ocd;
	paster_t paster;
	struct i2c_param *i2c_node;
	param_debug *param_reg_array;
} ddr_ctrl;

param_debug paramInfo[] = {
	{
		.node_id	= 0xff,
		.mc_id		= 0xf,
		.param		= NULL,
	},
};

ddr_ctrl ddrCtrl = {
	.mc_type		= 10,
	.mc_interleave_offset	= 8,
	.mc_regs_base		= TO_UNCACHED(0xff00000),
	.cache_mem_base		= TO_CACHED(0),
	.ddr_freq		= 800,
	.ddr_freq_2slot		= 800,
	.node_offset		= 44,
	.tot_node_num		= 1,
	.node_mc_num		= 1,
	.ref_clk		= 120,
	.uart_base		= TO_UNCACHED(ADDR_BOOTCONSOLE),
	.l2xbar_conf_addr	= TO_UNCACHED(0x16000100),
	.channel_width		= 64,
	.dll_bypass		= 0,
	.mem_base		= 0x80000000,
	.table			= {
		.enable_early_printf		= 0,
		.enable_mc_vref_training	= 1,
		.enable_ddr_vref_training	= 1,
		.enable_bit_training		= 0,
		.disable_dimm_ecc		= 1,
		.low_speed			= 0,
		.auto_ddr_config		= 0,
		.enable_ddr_leveling		= 1,
		.print_ddr_leveling		= 0,
		.vref_training_debug		= 0,
		.bit_training_debug		= 1,
		.enable_write_training		= 1,
		.debug_write_training		= 0,
		.print_dll_sample		= 0,
		.disable_dq_odt_training	= 1,
		.lvl_debug			= 0,
		.disable_dram_crc		= 1,
		.disable_read_dbi		= 1,
		.disable_write_dbi		= 1,
		.disable_dm			= 0,
		.preamble2			= 0,
		.set_by_protocol		= 1,
		.param_set_from_spd_debug	= 0,
		.refresh_1x			= 1,
		.spd_only			= 0,
		.ddr_debug_param		= 0,
		.ddr_soft_clksel		= 1,
		.pda_mode			= 1,
		.signal_test			= 0,
	},
	.vref			= {
		.vref_range		= 0x00,
		.vref_value		= 0x50,
		.mc_vref_adjust		= 0x0,
		.ddr_vref_adjust	= 0x0,
		.vref_init		= 0x20,
		.vref_bits_per		= 0x0,
		.vref_bit		= 0x0,
	},
	.data			= {
		.rl_manualy		= 0,
		.bit_width		= 16,
		.nc16_map		= 0,
		.gate_mode		= 0,
		.pad_reset_po		= 0x0,
		.wrlevel_count_low	= 0x0,
		.ref_manualy		= 0x0,
	},
	.param			= {
		.dll_ck_mc0		= 0x44,
		.dll_ck_mc1		= 0x44,
	},
	.ocd			= {
		.pad_clk_ocd	= 0x5,
		.pad_ctrl_ocd	= 0xe,
#define PAD_DS_SPLIT ((8ULL << 12) | (7ULL << 3) | (7ULL << 0))
		.pad_ds_split	= (PAD_DS_SPLIT << 48)	|
				  (PAD_DS_SPLIT << 32)	|
				  (PAD_DS_SPLIT << 16)	|
				  (PAD_DS_SPLIT << 0),
	},
	.odt			= {
		.rtt_nom_1r_1slot		= 3, // RTT_40
		.rtt_park_1r_1slot		= 2, // RTT_120
		.mc_dq_odt_1cs			= 4,
		.rtt_nom_2r_1slot		= 2, // RTT_120
		.rtt_park_2r_1slot		= 5, // RTT_48
		.rtt_nom_1r_2slot_cs0		= 6, // RTT_80
		.rtt_park_1r_2slot_cs0		= 3, // RTT_40
		.rtt_nom_1r_2slot_cs1		= 2, // RTT_120
		.rtt_park_1r_2slot_cs1		= 1, // RTT_60
		.rtt_nom_2r_2slot_cs0		= 2, // RTT_120,
		.rtt_park_2r_2slot_cs0		= 5, // RTT_48
		.rtt_nom_2r_2slot_cs2		= 2, // RTT_120
		.rtt_park_2r_2slot_cs2		= 1, // RTT_60
		.mc_dqs_odt_2cs			= 5,
		.mc_dq_odt_2cs			= 4,
	},
#define tR2R_sameba_adj	(0x2ULL & 0x3f)
#define tR2W_sameba_adj	(0x2ULL & 0x3f)
#define tR2P_sameba_adj	(0x0ULL & 0x3f)
#define tW2R_sameba_adj	(0x2ULL & 0x3f)
#define tW2W_sameba_adj	(0x2ULL & 0x3f)
#define tW2P_sameba_adj	(0x0ULL & 0x3f)
#define tR2R_samebg_adj	(0x2ULL & 0x3f)
#define tR2W_samebg_adj	(0x4ULL & 0x3f)
#define tR2P_samebg_adj	(0x0ULL & 0x3f)
#define tW2R_samebg_adj	(0x2ULL & 0x3f)
#define tW2W_samebg_adj	(0x2ULL & 0x3f)
#define tW2P_samebg_adj	(0x0ULL & 0x3f)
#define tR2R_samec_adj	(0x2ULL & 0x3f)
#define tR2W_samec_adj	(0x4ULL & 0x3f)
#define tR2P_samec_adj	(0x0ULL & 0x3f)
#define tW2R_samec_adj	(0x2ULL & 0x3f)
#define tW2W_samec_adj	(0x2ULL & 0x3f)
#define tW2P_samec_adj	(0x0ULL & 0x3f)
#define tR2R_samecs_adj	(0x2ULL & 0x3f)
#define tR2W_samecs_adj	(0x0ULL & 0x3f)
#define tR2P_samecs_adj	(0x0ULL & 0x3f)
#define tW2R_samecs_adj	(0x2ULL & 0x3f)
#define tW2W_samecs_adj	(0x2ULL & 0x3f)
#define tW2P_samecs_adj	(0x0ULL & 0x3f)
#define tR2R_diffcs_adj	(0x4ULL & 0x3f)
#define tR2W_diffcs_adj	(0x4ULL & 0x3f)
#define tR2P_diffcs_adj	(0x0ULL & 0x3f)
#define tW2R_diffcs_adj	(0x4ULL & 0x3f)
#define tW2W_diffcs_adj	(0x4ULL & 0x3f)
#define tW2P_diffcs_adj	(0x0ULL & 0x3f)
	.sameba_adj			= (tW2P_sameba_adj << 40)	|
					  (tW2W_sameba_adj << 32)	|
					  (tW2R_sameba_adj << 24)	|
					  (tR2P_sameba_adj << 16)	|
					  (tR2W_sameba_adj << 8)	|
					  (tR2R_sameba_adj << 0),
	.samebg_adj			= (tW2P_samebg_adj << 40)	|
					  (tW2W_samebg_adj << 32)	|
					  (tW2R_samebg_adj << 24)	|
					  (tR2P_samebg_adj << 16)	|
					  (tR2W_samebg_adj << 8)	|
					  (tR2R_samebg_adj << 0),
	.samec_adj			= (tW2P_samec_adj << 40)	|
					  (tW2W_samec_adj << 32)	|
					  (tW2R_samec_adj << 24)	|
					  (tR2P_samec_adj << 16)	|
					  (tR2W_samec_adj << 8)		|
					  (tR2R_samec_adj << 0),
	.samecs_adj			= (tW2P_samecs_adj << 40)	|
					  (tW2W_samecs_adj << 32)	|
					  (tW2R_samecs_adj << 24)	|
					  (tR2P_samecs_adj << 16)	|
					  (tR2W_samecs_adj << 8)	|
					  (tR2R_samecs_adj << 0),
	.diffcs_adj			= (tW2P_diffcs_adj << 40)	|
					  (tW2W_diffcs_adj << 32)	|
					  (tW2R_diffcs_adj << 24)	|
					  (tR2P_diffcs_adj << 16)	|
					  (tR2W_diffcs_adj << 8)	|
					  (tR2R_diffcs_adj << 0),
	.paster				= {
		.mc0_enable		= 1,
		.mc1_enable		= 0,
		.mc0_module_type	= 2,
		.mc0_cid_num		= 0,
		.mc0_sdram_capacity	= 0,
		.mc0_bg_mirror		= 0,
	},
	.param_reg_array		= paramInfo,
};

static void ddr4_platform_init_param(ddr_ctrl *ctrl);
extern int ddr4_init(uint64_t nodeNum, ddr_ctrl *ctrl);
extern ddr_ctrl mc_ctrl;

/*
 * Symbols required by the external memory initialization routine
 */
void
loop_delay(unsigned long long count)
{
	while (count--)
		nop();
}

uint64_t
get_hex(void)
{
	printf("%s gets called!\n", __func__);

	return 0;
}

static void
ddrc_init(void)
{
	ddr4_platform_init_param(&ddrCtrl);

	puts("Initializing DDR4 controller");

	/* Toggle DDRC reset */
	volatile uint32_t *iocsr = (volatile void *)TO_UNCACHED(0x16000000);
	uint32_t reg = readl(iocsr + 0x11c);

	reg &= BIT(0);
	writel(reg, iocsr + 0x11c);

	reg |= BIT(0);
	writel(reg, iocsr + 0x11c);

	int ret = ddr4_init(1, &ddrCtrl);
	if (ret)
		puts("DDR4 initialization failed!");

	volatile uint32_t *mysteriousDDRReg = (volatile void *)TO_UNCACHED(0x16002108);
	reg = readl(mysteriousDDRReg);
	reg |= BIT(31);
	writel(reg, mysteriousDDRReg);

	puts("DDR4 controller initialized");
}

void
platform_late_init(void)
{
	clock_init();
	ddrc_init();
}
