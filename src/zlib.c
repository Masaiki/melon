/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include <zlib.h>

int
strinflate(char **dst, int dst_size,
	const char * restrict src, int src_size)
{
	*dst = malloc(dst_size);

	if (*dst == NULL)
		return 1;

	unsigned long size = dst_size;

	uncompress((Bytef *) *dst, &size, (const Bytef *) src, src_size);

	if (size != dst_size) {
		free(*dst);
		return 1;
	}

	return 0;
}
