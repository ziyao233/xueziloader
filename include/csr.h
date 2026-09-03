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
#define LOONGARCH_CSR_DMWIN0		0x180
#define LOONGARCH_CSR_DMWIN1		0x181
#define LOONGARCH_CSR_DMWIN2		0x182
#define LOONGARCH_CSR_DMWIN3		0x183

#endif // _CSR_H_
