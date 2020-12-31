/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "cnki_jbig.h"
#include "jbig.h"

int
cnki_jbig(char **bitmap, int *bitmap_size,
	int *bitmap_width, int *bitmap_height,
	const char * restrict jbig, int jbig_size)
{
	dib_t *dib = malloc(sizeof(dib_t));

	if (dib == NULL)
		return 1;

	memcpy(dib, jbig, 40);

	bih_t *bih = malloc(sizeof(bih_t));

	if (bih == NULL) {
		free(dib);
		return 1;
	}

	memset(bih, 0, sizeof(bih_t));

	bih->d_l = 0;
	bih->d = 0;

	bih->p = 1;

	bih->fill = 0;

	bih->x_d = dib->width;
	bih->y_d = dib->height;
	bih->l_0 = bih->y_d / 35;

	while (bih->l_0 > 128)
		bih->l_0--;
	if (bih->l_0 < 2)
		bih->l_0 = 2;

	bih->m_x = 8;
	bih->m_y = 0;

	bih->order |= 1 << 1;
	bih->order |= 1 << 0;

	bih->options |= 1 << 4;
	bih->options |= 1 << 3;
	bih->options |= 1 << 2;

	bih->dptable = NULL;

	int bie_size = jbig_size - 28; /* - 40 - 8 + 20 */
	char *bie = malloc(bie_size);

	if (bie == NULL) {
		free(dib);
		free(bih);
		return 1;
	}

	memcpy(bie, bih, 20);
	memcpy(bie + 20, jbig + 48, jbig_size - 48);

	int ret = strdec_jbig(bitmap, bitmap_size, bie, bie_size);

	if (ret == 0) {
		*bitmap_width = bih->x_d;
		*bitmap_height = bih->y_d;
	}

	free(dib);
	free(bih);
	free(bie);

	if (ret != 0)
		return 1;

	return 0;
}
