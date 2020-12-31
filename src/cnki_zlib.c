/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>

#include "zlib.h"

int
cnki_zlib(char **dst, int *dst_size,
	const char * restrict src, int src_size)
{
	int32_t size;
	memcpy(&size, src + 20, 4);

	*dst_size = size;

	if (strinflate(dst, size, src + 24, size - 24) != 0)
		return 1;

	return 0;
}
