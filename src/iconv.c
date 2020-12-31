/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include <iconv.h>

int
strconv(char **dst,
	const char * restrict dst_code,
	const char * restrict src,
	const char * restrict src_code,
	int *size)
{
	size_t dst_size = *size;
	char *dst_conv = malloc(dst_size);

	if (dst_conv == NULL)
		return 1;

	size_t src_size = strlen(src) + 1;
	char *src_conv = malloc(src_size);

	if (src_conv == NULL) {
		free(dst_conv);
		return 1;
	}

	strncpy(src_conv, src, src_size);

	char *dst_start = dst_conv;
	char *src_start = src_conv;

	iconv_t conv_src_dst = iconv_open(dst_code, src_code);

	if (conv_src_dst == (iconv_t) - 1) {
		free(dst_conv);
		free(src_conv);
		return 1;
	}

	if (iconv(conv_src_dst,
		&src_conv, &src_size,
		&dst_conv, &dst_size) == (size_t) - 1) {
		free(dst_start);
		free(src_start);
		return 1;
	} else {
		*size -= dst_size;

		*dst = malloc(*size);

		if (*dst != NULL)
			memcpy(*dst, dst_start, *size);

		free(dst_start);
		free(src_start);
	}

	if (iconv_close(conv_src_dst) != 0 || *dst == NULL)
		return 1;

	return 0;
}
