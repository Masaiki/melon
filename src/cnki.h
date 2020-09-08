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

typedef enum _nh_code {
	CCITTFAX,
	DCT_0,
	DCT_1,
	JBIG2,
	JPX
} nh_code;

typedef struct _object_nh_t {
	int32_t address; /* Starting at end of object_outline_t */
	int32_t size;
	int16_t page[2];
	int32_t zero[2];
	char *text;
	int32_t image_format; /* nh_code */
	int32_t image_address;
	int32_t image_size;
	char *image;
	struct _object_nh_t *next;
} object_nh_t;

typedef struct _cnki_t {
	int stat;
	int size_buf;
	FILE *fp_i;
	FILE *fp_o;
	file_stat_t *file_stat;
	object_outline_t *object_outline;
	object_nh_t *object_nh;
} cnki_t;

/* cnki_pdf.c */
int cnki_pdf(cnki_t **param);

/* cnki_outline_tree.c */
int cnki_outline_tree(object_outline_tree_t **outline_tree,
	object_outline_t **outline, int *ids);

/* cnki_xml.c */
int cnki_xml(char **xml, FILE **fp);
