/*
 * Copyright (c) 2020, yzrh <yzrh@tuta.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "cnki.h"
#include "iconv.h"
#include "pdf.h"

/*
 * It will write first, list, and count to *stat
 * so that when called recursively, it knows
 * what to do
 */
static int
_outline(pdf_object_t **pdf, object_outline_tree_t **outline_tree, int id, int **stat)
{
	*stat = malloc(3 * sizeof(int));

	if (*stat == NULL)
		return 1;

	int size;
	char *str;

	int *ret;

	char buf[64];
	char dictionary[1024];

	object_outline_tree_t *ptr = *outline_tree;

	(*stat)[0] = ptr->id;
	(*stat)[2] = 0;

	while (ptr != NULL) {
		memset(dictionary, 0, 1024);

		strcat(dictionary, "<<\n");

		size = 512;
		str = NULL;

		if (strconv(&str, "UTF-16BE",
			ptr->item->title, "GB18030",
			&size) == 0) {
			strcat(dictionary, "/Title <feff");

			for (int i = 0; i < size; i++) {
				snprintf(buf, 64, "%02x", (unsigned char) str[i]);
				strcat(dictionary, buf);
			}

			strcat(dictionary, ">\n");
		}

		free(str);

		snprintf(buf, 64, "/Parent %d 0 R\n", id);
		strcat(dictionary, buf);

		if (ptr->up != NULL && ptr->up->id != id) {
			snprintf(buf, 64, "/Prev %d 0 R\n", ptr->up->id);
			strcat(dictionary, buf);
		}

		if (ptr->left != NULL) {
			snprintf(buf, 64, "/Next %d 0 R\n", ptr->left->id);
			strcat(dictionary, buf);
		}

		if (ptr->right != NULL) {
			_outline(pdf, &ptr->right, ptr->id, &ret);

			snprintf(buf, 64, "/First %d 0 R\n", ret[0]);
			strcat(dictionary, buf);

			snprintf(buf, 64, "/Last %d 0 R\n", ret[1]);
			strcat(dictionary, buf);

			snprintf(buf, 64, "/Count -%d\n", ret[2]);
			strcat(dictionary, buf);

			free(ret);
		}

		/* Page starts from 0 */
		snprintf(buf, 64, "/Dest [%d /XYZ null null null]\n>>\n",
			atoi(ptr->item->page) - 1);
		strcat(dictionary, buf);

		pdf_obj_append(pdf, ptr->id, NULL, dictionary, NULL);

		if (ptr->left == NULL)
			(*stat)[1] = ptr->id;

		(*stat)[2]++;

		ptr = ptr->left;
	}

	return 0;
}

int
pdf_cnki_outline(pdf_object_t **pdf, object_outline_t **outline, int **ids)
{
	if (*pdf == NULL || *outline == NULL || *ids == NULL)
		return 1;

	object_outline_tree_t *outline_tree = NULL;
	cnki_outline_tree(&outline_tree, outline, *ids);

	char buf[128];
	int *ret;

	_outline(pdf, &outline_tree->left, outline_tree->id, &ret);

	free(outline_tree);

	snprintf(buf, 128,
		"<<\n/Type Outlines\n/First %d 0 R\n/Last %d 0 R\n/Count %d\n>>\n",
		ret[0], ret[1], ret[2]);

	free(ret);

	pdf_obj_append(pdf, (*ids)[0], NULL, buf, NULL);

	return 0;
}
