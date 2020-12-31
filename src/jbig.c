/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h> /* FIXME: test */
#include <stdlib.h>
#include <string.h>

#include <jbig.h>

int
strdec_jbig(char **bitmap, int *bitmap_size,
	const char * restrict data, int data_size)
{
	struct jbg_dec_state sd;

	jbg_dec_init(&sd);

	unsigned char *data_ptr[1] = {(unsigned char *) data};

	/* FIXME: test */
	int ret;
	if ((ret = jbg_dec_in(&sd, (unsigned char *) data_ptr,
		data_size, NULL)) != JBG_EOK) {
		printf("%s", jbg_strerror(ret));
		jbg_dec_free(&sd);
		return 1;
	}

	*bitmap_size = jbg_dec_getsize(&sd);
	*bitmap = malloc(*bitmap_size);

	if (*bitmap != NULL)
		memcpy(*bitmap, jbg_dec_getimage(&sd, 0), *bitmap_size);

	jbg_dec_free(&sd);

	return 0;
}
