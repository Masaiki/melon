/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include "cnki.h"
#include "pdf.h"
#include "pdf_cnki.h"

int
cnki_hn(cnki_t **param)
{
	if (*param == NULL)
		return 1;

	if ((*param)->stat > 0)
		printf("Begin 'HN' conversion\n");

	if ((*param)->file_stat->page > 0)
		(*param)->object_hn = malloc(sizeof(object_hn_t));
	else
		return 1;

	if ((*param)->object_hn == NULL)
		return 1;

	if ((*param)->stat > 1) {
		printf("Loading page(s)\n");
		printf("\t%8s\t%8s\t%6s\t%4s\t%6s\t%6s\t%4s\t%8s\t%8s\n",
			"address",
			"text",
			"length",
			"page",
			"zero",
			"#",
			"code",
			"address",
			"image");
	}

	object_hn_t *ptr = (*param)->object_hn;
	for (int i = 0; i < (*param)->file_stat->page; i++) {
		fread(&ptr->address, 4, 1, (*param)->fp_i);
		fread(&ptr->text_size, 4, 1, (*param)->fp_i);
		fread(&ptr->image_length, 2, 1, (*param)->fp_i);
		fread(&ptr->page, 2, 1, (*param)->fp_i);
		fread(&ptr->zero, 8, 1, (*param)->fp_i);

		ptr->text = NULL;
		ptr->image_data = NULL;
		ptr->next = NULL;

		if (i < (*param)->file_stat->page - 1) {
			ptr->next = malloc(sizeof(object_hn_t));

			if (ptr->next == NULL)
				return 1;
		}

		ptr = ptr->next;
	}

	ptr = (*param)->object_hn;
	while (ptr != NULL) {
		ptr->text = malloc(ptr->text_size);

		if (ptr->text == NULL)
			return 1;

		fseek((*param)->fp_i, ptr->address, SEEK_SET);
		fread(ptr->text, ptr->text_size, 1, (*param)->fp_i);

		if ((*param)->stat > 1)
			printf("\t%08x\t%8d\t%6d\t%4d\t{%d, %d}",
				ptr->address,
				ptr->text_size,
				ptr->image_length,
				ptr->page,
				ptr->zero[0],
				ptr->zero[1]);

		ptr->image_data = malloc(ptr->image_length * sizeof(hn_image_t));

		if (ptr->image_data == NULL)
			return 1;

		for (int i = 0; i < ptr->image_length; i++) {
			fread(&ptr->image_data[i].format, 4, 1, (*param)->fp_i);
			fread(&ptr->image_data[i].address, 4, 1, (*param)->fp_i);
			fread(&ptr->image_data[i].size, 4, 1, (*param)->fp_i);
		}

		for (int i = 0; i < ptr->image_length; i++) {
			ptr->image_data[i].image = malloc(ptr->image_data[i].size);

			if (ptr->image_data[i].image == NULL)
				return 1;

			fseek((*param)->fp_i, ptr->image_data[i].address, SEEK_SET);
			fread(ptr->image_data[i].image,
				ptr->image_data[i].size, 1,
				(*param)->fp_i);

			if ((*param)->stat > 1)
				printf("\t%6d\t%4d\t%08x\t%8d\n",
					i,
					ptr->image_data[i].format,
					ptr->image_data[i].address,
					ptr->image_data[i].size);
		}

		ptr = ptr->next;
	}

	if ((*param)->stat > 1)
		printf("Loaded %d page(s)\n", (*param)->file_stat->page);

	if ((*param)->stat > 1)
		printf("Creating PDF object(s)\n");

	pdf_object_t *pdf = NULL;

	if (pdf_obj_create(&pdf) != 0)
		return 1;

	if ((*param)->stat > 0)
		printf("Conversion ended\n");

	/* TODO: Finish me please :) */
	return 1;
}
