/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include "cnki.h"

int
cnki_nh(cnki_t **param)
{
	if (*param == NULL)
		return 1;

	if ((*param)->stat > 0)
		printf("Begin 'HN' conversion\n");

	if ((*param)->file_stat->page > 0)
		(*param)->object_nh = malloc(sizeof(object_nh_t));
	else
		return 1;

	if ((*param)->object_nh == NULL)
		return 1;

	if ((*param)->stat > 1) {
		printf("Loading page(s)\n");
		printf("\t%8s\t%8s\t%13s\t%6s\t%4s\t%8s\t%8s\n",
			"address",
			"text",
			"page",
			"zero",
			"code",
			"address",
			"image");
	}

	object_nh_t *ptr = (*param)->object_nh;
	for (int i = 0; i < (*param)->file_stat->page; i++) {
		fread(&ptr->address, 4, 1, (*param)->fp_i);
		fread(&ptr->size, 4, 1, (*param)->fp_i);
		fread(&ptr->page, 4, 1, (*param)->fp_i);
		fread(&ptr->zero, 8, 1, (*param)->fp_i);

		ptr->text = NULL;
		ptr->image_format = -1;
		ptr->image_address = 0;
		ptr->image_size = 0;
		ptr->image = NULL;
		ptr->next = NULL;

		if (i < (*param)->file_stat->page - 1) {
			ptr->next = malloc(sizeof(object_nh_t));

			if (ptr->next == NULL)
				return 1;
		}

		ptr = ptr->next;
	}

	ptr = (*param)->object_nh;
	while (ptr != NULL) {
		ptr->text = malloc(ptr->size);

		if (ptr->text == NULL)
			return 1;

		fseek((*param)->fp_i, ptr->address, SEEK_SET);
		fread(ptr->text, ptr->size, 1, (*param)->fp_i);
		fread(&ptr->image_format, 4, 1, (*param)->fp_i);
		fread(&ptr->image_address, 4, 1, (*param)->fp_i);
		fread(&ptr->image_size, 4, 1, (*param)->fp_i);

		ptr->image = malloc(ptr->image_size);

		if (ptr->image == NULL)
			return 1;

		fseek((*param)->fp_i, ptr->image_address, SEEK_SET);
		fread(ptr->image, ptr->image_size, 1, (*param)->fp_i);

		if ((*param)->stat > 1)
			printf("\t%08x\t%8d\t{%d, %8d}\t{%d, %d}\t%4d\t%08x\t%8d\n",
				ptr->address,
				ptr->size,
				ptr->page[0],
				ptr->page[1],
				ptr->zero[0],
				ptr->zero[1],
				ptr->image_format,
				ptr->image_address,
				ptr->image_size);

		ptr = ptr->next;
	}

	if ((*param)->stat > 1)
		printf("Loaded %d page(s)\n", (*param)->file_stat->page);

	/* TODO: Study signed int __fastcall CAJDoc::OpenNHCAJFile(int a1, int a2) */

	if ((*param)->stat > 0)
		printf("Conversion ended\n");

	/* TODO: Finish me please :) */
	return 1;
}
