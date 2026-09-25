// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	include/bitops.h
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#ifndef _BITOPS_H_
#define _BITOPS_H_

#define BIT(n)		(1ULL << (n))
#define GENMASK(h, l)	(((1ULL << ((h) - (l) + 1)) - 1) << (l))

#endif // _BITOPS_H_
