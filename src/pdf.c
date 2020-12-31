/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "pdf.h"

static int
_min_id(pdf_object_t **pdf)
{
	int min = 0;

	pdf_object_t *ptr = (*pdf)->next;
	while (ptr != NULL) {
		if (min == 0 || ptr->id < min)
			min = ptr->id;

		ptr = ptr->next;
	}

	return min;
}

int
pdf_obj_create(pdf_object_t **pdf)
{
	if (*pdf != NULL)
		return 1;

	*pdf = malloc(sizeof(pdf_object_t));

	if (*pdf == NULL)
		return 1;

	(*pdf)->address = 0;
	(*pdf)->size = 0;
	(*pdf)->id = 0;
	(*pdf)->object_size = 0;
	(*pdf)->object = NULL;
	(*pdf)->dictionary_size = 0;
	(*pdf)->dictionary = NULL;
	(*pdf)->stream_size = 0;
	(*pdf)->stream= NULL;
	(*pdf)->next = NULL;

	return 0;
}

void
pdf_obj_destroy(pdf_object_t **pdf)
{
	pdf_object_t *ptr;
	while ((ptr = *pdf) != NULL) {
		*pdf = (*pdf)->next;
		free(ptr->object);
		free(ptr->dictionary);
		free(ptr->stream);
		free(ptr);
	}
}

int
pdf_obj_add(pdf_object_t **pdf, int id,
	const char * restrict object,
	const char * restrict dictionary,
	const char * restrict stream,
	int stream_size)
{
	if (*pdf != NULL || id <= 0 ||
		(object != NULL && dictionary != NULL))
		return 1;

	*pdf = malloc(sizeof(pdf_object_t));

	if (*pdf == NULL)
		return 1;

	(*pdf)->address = 0;
	(*pdf)->size = 0;

	(*pdf)->id = id;

	if (dictionary != NULL) {
		(*pdf)->dictionary_size = strlen(dictionary);
		(*pdf)->dictionary = malloc((*pdf)->dictionary_size);

		if ((*pdf)->dictionary == NULL)
			return 1;

		memcpy((*pdf)->dictionary, dictionary, (*pdf)->dictionary_size);

		(*pdf)->object_size = 0;
		(*pdf)->object = NULL;
	} else if (object != NULL) {
		(*pdf)->object_size = strlen(object);
		(*pdf)->object = malloc((*pdf)->object_size);

		if ((*pdf)->object == NULL)
			return 1;

		memcpy((*pdf)->object, object, (*pdf)->object_size);

		(*pdf)->dictionary_size = 0;
		(*pdf)->dictionary = NULL;
	} else {
		(*pdf)->object_size = 0;
		(*pdf)->object = NULL;
		(*pdf)->dictionary_size = 0;
		(*pdf)->dictionary = NULL;
	}

	if (stream != NULL && stream_size > 0) {
		(*pdf)->stream_size = stream_size + 1;
		(*pdf)->stream = malloc((*pdf)->stream_size);

		if ((*pdf)->stream == NULL)
			return 1;

		memcpy((*pdf)->stream, stream, (*pdf)->stream_size);
		(*pdf)->stream[(*pdf)->stream_size - 1] = '\n';
	} else {
		(*pdf)->stream_size = 0;
		(*pdf)->stream = NULL;
	}

	(*pdf)->next = NULL;

	return 0;
}

int
pdf_obj_del(pdf_object_t **pdf, int id)
{
	if (*pdf == NULL || id <= 0)
		return 1;

	pdf_object_t *ptr = *pdf;
	while (ptr->next != NULL) {
		if (ptr->next->id == id) {
			ptr->next = ptr->next->next;
			break;
		}

		ptr = ptr->next;
	}

	return 0;
}

int
pdf_obj_prepend(pdf_object_t **pdf, int id,
	const char * restrict object,
	const char * restrict dictionary,
	const char * restrict stream,
	int stream_size)
{
	if (*pdf == NULL)
		return 1;

	if (id <= 0)
		id = pdf_get_free_id(pdf);

	pdf_object_t *ptr = NULL;

	if (pdf_obj_add(&ptr, id, object, dictionary,
		stream, stream_size) != 0) {
		free(ptr);
		return 1;
	}

	ptr->next = (*pdf)->next;
	(*pdf)->next = ptr;

	return 0;
}

int
pdf_obj_append(pdf_object_t **pdf, int id,
	const char * restrict object,
	const char * restrict dictionary,
	const char * restrict stream,
	int stream_size)
{
	if (*pdf == NULL)
		return 1;

	if (id <= 0)
		id = pdf_get_free_id(pdf);

	pdf_object_t *ptr = *pdf;
	while (ptr->next != NULL)
		ptr = ptr->next;

	if (pdf_obj_add(&ptr->next, id, object, dictionary,
		stream, stream_size) != 0)
		return 1;

	return 0;
}

int
pdf_obj_replace(pdf_object_t **pdf, int id,
	const char * restrict object,
	const char * restrict dictionary,
	const char * restrict stream,
	int stream_size)
{
	pdf_object_t *ptr;
	char *ret;

	if (pdf_get_obj(pdf, id, &ptr) != 0)
		return 1;

	if (object != NULL && dictionary != NULL)
		return 1;

	if (dictionary != NULL) {
		ret = realloc(ptr->dictionary, strlen(dictionary));

		if (ret == NULL)
			return 1;

		ptr->dictionary_size = strlen(dictionary);
		ptr->dictionary = ret;

		memcpy(ptr->dictionary, dictionary, ptr->dictionary_size);
	} else if (object != NULL) {
		ret = realloc(ptr->object, strlen(object));

		if (ret == NULL)
			return 1;

		ptr->object_size = strlen(object);
		ptr->object = ret;

		memcpy(ptr->object, object, ptr->object_size);
	}

	if (stream != NULL && stream_size > 0) {
		ret = realloc(ptr->stream, stream_size + 1);

		if (ret == NULL)
			return 1;

		ptr->stream_size = stream_size + 1;
		ptr->stream = ret;

		memcpy(ptr->stream, stream, ptr->stream_size);
		ptr->stream[ptr->stream_size - 1] = '\n';
	}

	return 0;
}

int
pdf_obj_sort(pdf_object_t **pdf)
{
	if (*pdf == NULL)
		return 1;

	int id;
	pdf_object_t *tmp;
	pdf_object_t *ptr;

	ptr = *pdf;
	while (ptr->next != NULL) {
		id = _min_id(&ptr->next);

		if (id == 0)
			return 1;

		if (id < ptr->next->id) {
			pdf_get_obj(&ptr->next, id, &tmp);
			pdf_obj_del(&ptr->next, id);

			tmp->next = ptr->next;
			ptr->next = tmp;
		}

		ptr = ptr->next;
	}

	return 0;
}
