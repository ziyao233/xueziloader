// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	include/addrspace.h
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#ifndef _ADDRMAP_H_
#define _ADDRMAP_H_

#define TO_PHYS(x)		((x) & 0x0fffffffffffffff)
#define TO_UNCACHED(x)		((x) | (0x8ULL << 60))
#define TO_CACHED(x)		((x) | (0x9ULL << 60))

#endif // _ADDRMAP_H_
