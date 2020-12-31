/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "cnki.h"

int
cnki_create(cnki_t **param)
{
	if (*param != NULL)
		return 1;

	*param = malloc(sizeof(cnki_t));

	if (*param == NULL)
		return 1;

	(*param)->stat = 0;
	(*param)->size_buf = 524288;
	(*param)->fp_i = NULL;
	(*param)->fp_o = NULL;

	(*param)->file_stat = malloc(sizeof(file_stat_t));

	if ((*param)->file_stat== NULL)
		return 1;

	memset((*param)->file_stat, 0, sizeof(file_stat_t));

	(*param)->object_outline = NULL;
	(*param)->object_hn = NULL;

	return 0;
}

void
cnki_destroy(cnki_t **param)
{
	if (*param != NULL) {
		if ((*param)->file_stat != NULL)
			free((*param)->file_stat);
		if ((*param)->object_outline != NULL)
			free((*param)->object_outline);
		if ((*param)->object_hn != NULL)
			free((*param)->object_hn);
		free(*param);
	}
}

int
cnki_info(cnki_t **param)
{
	if (*param == NULL)
		return 1;

	if ((*param)->stat > 1)
		printf("Reading file header at 0x%x\n", ADDRESS_HEAD);

	int addr[2];

	fseek((*param)->fp_i, ADDRESS_HEAD, SEEK_SET);
	fread((*param)->file_stat->type, 4, 1, (*param)->fp_i);

	if ((*param)->stat > 0)
		printf("File type is '%s'\n", (*param)->file_stat->type);

	if (strncmp((*param)->file_stat->type, "%PDF", 4) == 0) {
		return 0;
	} else if (strncmp((*param)->file_stat->type, "CAJ", 3) == 0) {
		addr[0] = ADDRESS_CAJ_PAGE;
		addr[1] = ADDRESS_CAJ_OUTLINE;
	} else if (strncmp((*param)->file_stat->type, "HN", 2) == 0) {
		addr[0] = ADDRESS_HN_PAGE;
		addr[1] = ADDRESS_HN_OUTLINE;
	} else if (strncmp((*param)->file_stat->type, "KDH ", 4) == 0) {
		return 0;
	} else {
		return 1;
	}

	if ((*param)->stat > 1)
		printf("Reading page count at 0x%x\n", addr[0]);

	fseek((*param)->fp_i, addr[0], SEEK_SET);
	fread(&(*param)->file_stat->page, 4, 1, (*param)->fp_i);

	if ((*param)->stat > 0)
		printf("Advised %d page(s)\n",
			(*param)->file_stat->page);

	if ((*param)->stat > 1)
		printf("Reading outline count at 0x%x\n", addr[1]);

	fseek((*param)->fp_i, addr[1], SEEK_SET);
	fread(&(*param)->file_stat->outline, 4, 1, (*param)->fp_i);

	if ((*param)->stat > 0)
		printf("Advised %d outline(s)\n",
			(*param)->file_stat->outline);

	if ((*param)->file_stat->outline > 0) {
		if ((*param)->stat > 1) {
			printf("Loading outline(s)\n");
			printf("\t%16s\t%-24s\t%12s\t%12s\t%5s\n",
				"title",
				"hierarchy",
				"page",
				"text",
				"depth");
		}

		(*param)->object_outline = malloc(sizeof(object_outline_t));

		if ((*param)->object_outline == NULL)
			return 1;

		object_outline_t *ptr = (*param)->object_outline;
		for (int i = 0; i < (*param)->file_stat->outline; i++) {
			fread(ptr->title, 256, 1, (*param)->fp_i);
			fread(ptr->hierarchy, 24, 1, (*param)->fp_i);
			fread(ptr->page, 12, 1, (*param)->fp_i);
			fread(ptr->text, 12, 1, (*param)->fp_i);
			fread(&ptr->depth, 4, 1, (*param)->fp_i);

			ptr->next = NULL;

			if ((*param)->stat > 1) {
				printf("\t");
				for (int j = 1; j <= 256; j++) {
					printf("%02x", (unsigned char) ptr->title[j - 1]);

					if (j % 8 == 0 && ptr->title[j] == '\0')
						break;

					if (j % 8 == 0)
						printf("\n\t");
					else if (j % 2 == 0)
						printf(" ");
				}
				printf("\t%-24s\t%12s\t%12s\t%5d\n",
					ptr->hierarchy,
					ptr->page,
					ptr->text,
					ptr->depth);
			}

			if (i < (*param)->file_stat->outline - 1) {
				ptr->next = malloc(sizeof(object_outline_t));

				if (ptr->next == NULL)
					return 1;
			}

			ptr = ptr->next;
		}

		if ((*param)->stat > 0)
			printf("Loaded %d outline(s)\n",
				(*param)->file_stat->outline);
	}

	return 0;
}
