/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

typedef struct _pdf_object_t {
	int address;
	int size;
	int id;
	int object_size;
	char *object;
	int dictionary_size;
	char *dictionary;
	int stream_size;
	char *stream;
	struct _pdf_object_t *next;
} pdf_object_t;

/* pdf.c */
/* TODO: Rewrite object dictionary */
/* TODO: Compact object id */
/* TODO: `mutool clean -gggsz' */
int pdf_obj_create(pdf_object_t **pdf);
void pdf_obj_destroy(pdf_object_t **pdf);
int pdf_obj_add(pdf_object_t **pdf, int id,
	const char * restrict object,
	const char * restrict dictionary,
	const char * restrict stream);
int pdf_obj_del(pdf_object_t **pdf, int id);
int pdf_obj_prepend(pdf_object_t **pdf, int id,
	const char * restrict object,
	const char * restrict dictionary,
	const char * restrict stream);
int pdf_obj_append(pdf_object_t **pdf, int id,
	const char * restrict object,
	const char * restrict dictionary,
	const char * restrict stream);
int pdf_obj_sort(pdf_object_t **pdf);

/* pdf_parser.c */
int pdf_load(pdf_object_t **pdf, FILE **fp, int size_buf);

/* pdf_writer.c */
int pdf_dump_obj(pdf_object_t **pdf, FILE **fp);
int pdf_dump_header(pdf_object_t **pdf, FILE **fp);
int pdf_dump_xref(pdf_object_t **pdf, FILE **fp);
int pdf_dump_trailer(pdf_object_t **pdf, FILE **fp, int xref);

/* pdf_get.c */
int pdf_get_obj(pdf_object_t **pdf, int id, pdf_object_t **obj);
int pdf_get_count(pdf_object_t **pdf);
int pdf_get_size(pdf_object_t **pdf);
int pdf_get_free_id(pdf_object_t **pdf);
int pdf_get_free_ids(pdf_object_t **pdf, int **ids, int count);
int pdf_get_catalog_id(pdf_object_t **pdf);
int pdf_get_parent_id(pdf_object_t **pdf, int **id);
int pdf_get_kid_id(pdf_object_t **pdf, int id, int **kid);
int pdf_get_kid_count(pdf_object_t **pdf, int id);
