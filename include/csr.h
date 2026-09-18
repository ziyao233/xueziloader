// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	include/csr.h
 *	Copyright (C) 2026 Yao Zi.
 */

#ifndef _CSR_H_
#define _CSR_H_

#define LOONGARCH_CSR_CRMD		0x0
#define  CSR_CRMD_DA			(1 << 3)
#define  CSR_CRMD_PG			(1 << 4)
#define LOONGARCH_CSR_IMPCTL1		0x80
#define  CSR_AUTO_FLUSHSFB		(1 << 9)
#define  CSR_FASTLDQ			(1 << 12)
#define LOONGARCH_CSR_MCSR2		0xc2
#define LOONGARCH_CSR_MCSR9		0xc9
#define LOONGARCH_CSR_MCSR24		0xf0
#define  MCSR24_MCSRLOCK		(1 << 0)
#define  MCSR24_NAPEN			(1 << 1)
#define  MCSR24_VPUCG			(1 << 2)
#define  MCSR24_RAMCG			(1 << 3)
#define LOONGARCH_CSR_DMWIN0		0x180
#define LOONGARCH_CSR_DMWIN1		0x181
#define LOONGARCH_CSR_DMWIN2		0x182
#define LOONGARCH_CSR_DMWIN3		0x183
#define LOONGARCH_CSR_PERFCTRL0		0x200
#define  CSR_PERFCTRL_PLV0		(1 << 16)

#endif // _CSR_H_
