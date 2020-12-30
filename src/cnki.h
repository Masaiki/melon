/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdio.h>

#define ADDRESS_HEAD		0x0000

#define ADDRESS_CAJ_PAGE	0x0010
#define ADDRESS_CAJ_OUTLINE	0x0110
#define ADDRESS_CAJ_BODY	0x0014

#define ADDRESS_HN_PAGE		0x0090
#define ADDRESS_HN_OUTLINE	0x0158

#define ADDRESS_KDH_BODY	0x00fe

#define KEY_KDH			"FZHMEI"
#define KEY_KDH_LENGTH		6

typedef struct _file_stat_t {
	char type[4];
	int32_t page;
	int32_t outline;
} file_stat_t;

typedef struct _object_outline_t {
	char title[256]; /* Starting at file_stat_t->outline + 4 */
	char hierarchy[24];
	char page[12];
	char text[12];
	int32_t depth;
	struct _object_outline_t *next;
} object_outline_t;

typedef struct _object_outline_tree_t {
	int id;
	struct _object_outline_t *item;
	struct _object_outline_tree_t *up;
	struct _object_outline_tree_t *left;
	struct _object_outline_tree_t *right;
} object_outline_tree_t;

typedef enum _hn_code {
	CCITTFAX,
	DCT_0,
	DCT_1, /* Inverted */
	JBIG2,
	JPX
} hn_code;

typedef struct _hn_image_t {
	int32_t format; /* hn_code */
	int32_t address;
	int32_t size;
	char *image;
} hn_image_t;

typedef struct _object_hn_t {
	int32_t address; /* Starting at end of object_outline_t */
	int32_t text_size;
	int16_t image_length;
	int16_t page;
	int32_t unknown[2]; /* TODO: what is it? */
	char *text;
	struct _hn_image_t *image_data;
	struct _object_hn_t *next;
} object_hn_t;

typedef struct _cnki_t {
	int stat;
	int size_buf;
	FILE *fp_i;
	FILE *fp_o;
	file_stat_t *file_stat;
	object_outline_t *object_outline;
	object_hn_t *object_hn;
} cnki_t;

/* cnki_pdf.c */
int cnki_pdf(cnki_t **param);

/* cnki_outline_tree.c */
int cnki_outline_tree(object_outline_tree_t **outline_tree,
	object_outline_t **outline, int *ids);

/* cnki_zlib.c */
int cnki_zlib(char **dst, int *dst_size,
	const char * restrict src, int src_size);

/* cnki_xml.c */
int cnki_xml(char **xml, FILE **fp);
