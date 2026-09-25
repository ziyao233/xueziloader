#	SPDX-License-Identifier: MPL-2.0
#	xueziloader
#	platform/ls2k0300/Makefile
#	Copyright (C) 2026 Yao Zi <me@ziyao.cc>

ifeq ($(LS2K0300_DDR_LIB),)
$(error "Please set LS2K0300_DDR_LIB to the DDR initialization library")
endif

ifeq ($(BOARD_NAME),)
$(error "ls2k0300 is a SoC platform, and shouldn't be targetted")
endif

SOC	= ls2k0300
OBJS	+= platform/ls2k0300/early.o
EXTOBJS += $(LS2K0300_DDR_LIB)
