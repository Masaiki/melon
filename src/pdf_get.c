/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "pdf.h"

static int
_id_in(int id, int *ids)
{
	for (int i = 1; i <= ids[0]; i++)
		if (ids[i] == id)
			return 1;

	return 0;
}

int
pdf_get_obj(pdf_object_t **pdf, int id, pdf_object_t **obj)
{
	if (*pdf == NULL || id <= 0)
		return 1;

	pdf_object_t *ptr = *pdf;
	while (ptr->next != NULL) {
		if (ptr->next->id == id) {
			*obj = ptr->next;
			return 0;
		}
		ptr = ptr->next;
	}

	return 1;
}

int
pdf_get_count(pdf_object_t **pdf)
{
	if (*pdf == NULL)
		return 1;

	int count = 0;

	pdf_object_t *ptr = (*pdf)->next;
	while (ptr != NULL) {
		count++;
		ptr = ptr->next;
	}

	return count;
}

int
pdf_get_size(pdf_object_t **pdf)
{
	if (*pdf == NULL)
		return 1;

	int size = 0;

	pdf_object_t *ptr = (*pdf)->next;
	while (ptr != NULL) {
		size += ptr->size;
		ptr = ptr->next;
	}

	return size;
}

int
pdf_get_free_id(pdf_object_t **pdf)
{
	if (*pdf == NULL)
		return 1;

	int free_id = 0;

	pdf_object_t *ptr;

	int id = 0;

	for (int i = 1; i < 99999999; i++) {
		ptr = (*pdf)->next;
		while (ptr != NULL) {
			if (ptr->id == i) {
				id = i;
				break;
			}
			ptr = ptr->next;
		}

		if (i != id) {
			free_id = i;
			break;
		}
	}

	return free_id;
}

int
pdf_get_free_ids(pdf_object_t **pdf, int **ids, int count)
{
	if (*pdf == NULL || *ids != NULL || count <= 0)
		return 1;

	*ids = malloc(count * sizeof(int));

	if (*ids == NULL)
		return 1;

	int pos = 0;
	int id = 0;

	pdf_object_t *ptr;
	for (int i = 1; i < 99999999; i++) {
		ptr = (*pdf)->next;
		while (ptr != NULL) {
			if (ptr->id == i) {
				id = i;
				break;
			}
			ptr = ptr->next;
		}

		if (i != id) {
			(*ids)[pos] = i;

			if (pos == count)
				return 0;

			pos++;
		}
	}

	return 1;
}

int
pdf_get_catalog_id(pdf_object_t **pdf)
{
	if (*pdf == NULL)
		return 1;

	int catalog_id = 0;

	pdf_object_t *ptr = (*pdf)->next;

	while (ptr != NULL) {
		if (ptr->dictionary != NULL &&
			strstr(ptr->dictionary, "/Catalog") != NULL)
			catalog_id = ptr->id;

		ptr = ptr->next;
	}

	return catalog_id;
}

int
pdf_get_parent_id(pdf_object_t **pdf, int **id)
{
	if (*pdf == NULL || *id != NULL)
		return 1;

	int id_size = 1;
	*id = malloc(sizeof(int));

	if (*id == NULL)
		return 1;

	(*id)[0] = 0;

	pdf_object_t *ptr = (*pdf)->next;

	char *head;
	char *tail;

	char str[8];
	int str_val;

	int *ret;

	while (ptr != NULL) {
		if (ptr->dictionary != NULL &&
			(head = strstr(ptr->dictionary, "/Parent ")) != NULL &&
			(tail = strchr(head + 8, ' ')) != NULL) {
			memset(str, 0, 8);
			strncpy(str, head + 8, (tail - head) - 8);
			str_val = atoi(str);

			if (!_id_in(str_val, *id)) {
				ret = realloc(*id, ++id_size * sizeof(int));

				if (ret == NULL)
					return 1;
				else
					*id = ret;

				(*id)[0]++;
				(*id)[id_size - 1] = str_val;
			}
		}
		ptr = ptr->next;
	}

	return 0;
}

int
pdf_get_kid_id(pdf_object_t **pdf, int id, int **kid)
{
	if (*pdf == NULL || *kid != NULL)
		return 1;

	int kid_size = 1;
	*kid = malloc(sizeof(int));

	if (*kid == NULL)
		return 1;

	pdf_object_t *ptr = (*pdf)->next;

	char str[32];
	int *ret;

	snprintf(str, 32, "/Parent %d 0 R", id);

	while (ptr != NULL) {
		if (ptr->id == id) {
			(*kid)[0] = 0;
			return 1;
		}

		if (ptr->dictionary != NULL &&
			strstr(ptr->dictionary, str) != NULL) {
			ret = realloc(*kid, ++kid_size * sizeof(int));

			if (ret == NULL)
				return 1;
			else
				*kid = ret;

			(*kid)[kid_size - 1] = ptr->id;
		}

		ptr = ptr->next;
	}

	(*kid)[0] = kid_size - 1;

	return 0;
}

int
pdf_get_kid_count(pdf_object_t **pdf, int id)
{
	if (*pdf == NULL || id <= 0)
		return 1;

	int count = 0;

	pdf_object_t *ptr = (*pdf)->next;

	char id_str[32];
	char *pos;

	char str[8];
	int str_val;

	snprintf(id_str, 32, "/Parent %d 0 R", id);

	while (ptr != NULL) {
		if (ptr->dictionary != NULL &&
			strstr(ptr->dictionary, id_str) != NULL &&
			(pos = strstr(ptr->dictionary, "/Count ")) != NULL) {
			for (int i = 8; i >= 0; i--) {
				if (i + 7 <= ptr->dictionary_size - (pos - ptr->dictionary) &&
					pos[i + 7] >= '0' && pos[i + 7] <= '9') {
					memset(str, 0, 8);
					strncpy(str, pos + 7, i + 1);
					str_val = atoi(str);
					count += str_val;
					break;
				}
			}
		}
		ptr = ptr->next;
	}

	return count;
}
