// SPDX-License-Identifer: MPL-2.0
/*
 *	xueziloader
 *	platform/ls2k0300-foreverpi/late.c
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#include "../ls2k0300/late.c"

static void
ddr4_platform_init_param(ddr_ctrl *ctrl)
{
	// For some reason, on LS2K0300, mc0_memsize is set to the real memory
	// size multipled by 4.
	ctrl->table.two_t_mode_enable	= 1;
	ctrl->odt.mc_dqs_odt_1cs	= 0x8;
	ctrl->paster.mc0_memsize	= 512 * 4;
	ctrl->paster.mc0_dram_type	= 0xc;
	ctrl->paster.mc0_dimm_type	= 2;
	ctrl->paster.mc0_ba_num		= 0;
	ctrl->paster.mc0_bg_num		= 1;
	ctrl->paster.mc0_csmap		= 1;
	ctrl->paster.mc0_dram_width	= 2;
	ctrl->paster.mc0_col_num	= 12 - 10;
	ctrl->paster.mc0_row_num	= 18 - 15;
}
