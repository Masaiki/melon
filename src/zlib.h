/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

int strinflate(char **dst, int dst_size,
	const char * restrict src, int src_size);

int strdeflate(char **dst, int *dst_size,
	const char * restrict src, int src_size);
