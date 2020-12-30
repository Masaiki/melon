/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "cnki.h"
#include "iconv.h"
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
		printf("\t%8s\t%8s\t%6s\t%4s\t%16s\t%4s\t%8s\t%8s\n",
			"address",
			"text",
			"length",
			"page",
			"unknown",
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
		fread(&ptr->unknown, 8, 1, (*param)->fp_i);

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
			printf("\t%08x\t%8d\t%6d\t%4d\t{%4d, %8d}",
				ptr->address,
				ptr->text_size,
				ptr->image_length,
				ptr->page,
				ptr->unknown[0],
				ptr->unknown[1]);

		ptr->image_data = malloc(ptr->image_length * sizeof(hn_image_t));

		if (ptr->image_data == NULL)
			return 1;

		for (int i = 0; i < ptr->image_length; i++) {
			fread(&ptr->image_data[i].format, 4, 1, (*param)->fp_i);
			fread(&ptr->image_data[i].address, 4, 1, (*param)->fp_i);
			fread(&ptr->image_data[i].size, 4, 1, (*param)->fp_i);
			fseek((*param)->fp_i,
				ptr->image_data[i].address + ptr->image_data[i].size,
				SEEK_SET);
		}

		for (int i = 0; i < ptr->image_length; i++) {
			ptr->image_data[i].image = malloc(ptr->image_data[i].size);

			if (ptr->image_data[i].image == NULL)
				return 1;

			fseek((*param)->fp_i, ptr->image_data[i].address, SEEK_SET);
			fread(ptr->image_data[i].image,
				ptr->image_data[i].size, 1,
				(*param)->fp_i);

			if ((*param)->stat > 1) {
				if (i == 0) {
					printf("\t%4d\t%08x\t%8d\n",
						ptr->image_data[i].format,
						ptr->image_data[i].address,
						ptr->image_data[i].size);
				} else {
					printf("\t%8s\t%8s\t%6s\t%4s\t%16s\t%4d\t%08x\t%8d\n",
						"",
						"",
						"",
						"",
						"",
						ptr->image_data[i].format,
						ptr->image_data[i].address,
						ptr->image_data[i].size);
				}
			}
		}

		ptr = ptr->next;
	}

	if ((*param)->stat > 1)
		printf("Loaded %d page(s)\n", (*param)->file_stat->page);

	if ((*param)->stat > 1)
		printf("Generating PDF object(s)\n");

	pdf_object_t *pdf = NULL;

	if (pdf_obj_create(&pdf) != 0)
		return 1;

	int buf_size;
	char *buf;

	int str_size;
	char *str;

	int conv_size;
	char *conv_dst;
	char conv_src[2];
	char conv_hex[3];

	ptr = (*param)->object_hn;
	while (ptr != NULL) {
		if (strncmp(ptr->text + 8, "COMPRESSTEXT", 12) == 0) {
			cnki_zlib(&buf, &buf_size, ptr->text, ptr->text_size);

			str_size = buf_size / 8 + 7;
			str = malloc(str_size);

			if (str == NULL)
				return 1;

			memset(str, 0, str_size);

			strcat(str, "<feff");

			for (int i = 0; i < buf_size; i += 16) {
				conv_src[0] = buf[i + 7];
				conv_src[1] = buf[i + 6];

				conv_size = 512;

				if (strconv(&conv_dst, "UTF-16BE",
					conv_src, "GB18030", &conv_size) == 0) {
					for (int j = 0; j < conv_size - 2; j++) {
						snprintf(conv_hex, 3,
							"%02x", (unsigned char) conv_dst[j]);
						strcat(str, conv_hex);
					}
					free(conv_dst);
				}
			}
			free(buf);

			strcat(str, ">");
		} else {
			str_size = ptr->text_size;
			str = malloc(str_size);

			if (str == NULL)
				return 1;

			memset(str, 0, str_size);

			strcat(str, "<feff");

			for (int i = 0; i < ptr->text_size; i += 4) {
				conv_src[0] = ptr->text[i + 3];
				conv_src[1] = ptr->text[i + 2];

				conv_size = 512;

				if (strconv(&conv_dst, "UTF-16BE",
					conv_src, "GB18030", &conv_size) == 0) {
					for (int j = 0; j < conv_size - 2; j++) {
						snprintf(conv_hex, 3,
							"%02x", (unsigned char) conv_dst[j]);
						strcat(str, conv_hex);
					}
					free(conv_dst);
				}
			}

			strcat(str, ">");
		}

		pdf_obj_append(&pdf, 0, str, NULL, NULL);

		free(str);

		ptr = ptr->next;
	}

	if ((*param)->stat > 1) {
		printf("\t%8s\t%12s\t%12s\t%12s\n",
			"id",
			"object",
			"dictionary",
			"stream");

		pdf_object_t *ptr = pdf->next;
		while (ptr != NULL) {
			printf("\t%8d\t%12d\t%12d\t%12d\n",
				ptr->id,
				ptr->object_size,
				ptr->dictionary_size,
				ptr->stream_size);
			ptr = ptr->next;
		}
	}

	if ((*param)->stat > 0)
		printf("Generated %d object(s)\n",
			pdf_get_count(&pdf));

	int *ids = NULL;

	if ((*param)->file_stat->outline > 0) {
		if ((*param)->stat > 1)
			printf("Generating outline object(s)\n\t%8s\n", "id");

		pdf_get_free_ids(&pdf, &ids, (*param)->file_stat->outline + 1);
		int outline = pdf_cnki_outline(&pdf, &(*param)->object_outline, &ids);

		if ((*param)->stat > 1)
			for (int i = 0; i < (*param)->file_stat->outline + 1; i++)
				printf("\t%8d\n", ids[i]);

		if ((*param)->stat > 0) {
			if (outline != 0)
				printf("No outline information\n");
			else
				printf("Generated %d outline object(s)\n",
					(*param)->file_stat->outline + 1);
		}
	}

	if ((*param)->stat > 1)
		printf("Writing header\n");

	long cur = 0;

	if ((*param)->stat > 0)
		cur = ftell((*param)->fp_o);

	if (pdf_dump_header(&pdf, &(*param)->fp_o) != 0) {
		fprintf(stderr, "Header not written\n");
		return 1;
	} else {
		if ((*param)->stat > 0)
			printf("Header %ld byte(s) written\n",
				ftell((*param)->fp_o) - cur);
	}

	if ((*param)->stat > 1)
		printf("Writing object(s)\n");

	pdf_dump_obj(&pdf, &(*param)->fp_o);

	if ((*param)->stat > 1) {
		printf("\t%8s\t%8s\t%8s\t%12s\t%12s\t%12s\n",
			"address",
			"size",
			"id",
			"object",
			"dictionary",
			"stream");

		pdf_object_t *ptr = pdf->next;
		while (ptr != NULL) {
			printf("\t%08x\t%8d\t%8d\t%12d\t%12d\t%12d\n",
				ptr->address,
				ptr->size,
				ptr->id,
				ptr->object_size,
				ptr->dictionary_size,
				ptr->stream_size);
			ptr = ptr->next;
		}
	}

	if ((*param)->stat > 0)
		printf("%d object(s) %ld byte(s) written\n",
			pdf_get_count(&pdf),
			ftell((*param)->fp_o));

	long xref = ftell((*param)->fp_o);

	if ((*param)->stat > 1)
		printf("Writing cross-reference table\n");

	if (pdf_dump_xref(&pdf, &(*param)->fp_o) != 0) {
		if ((*param)->stat > 0)
			printf("Cross-reference table not written\n");
	} else {
		if ((*param)->stat > 0)
			printf("Cross-reference table %ld byte(s) written\n",
				ftell((*param)->fp_o) - xref);
	}

	if ((*param)->stat > 1)
		printf("Writing trailer\n");

	if ((*param)->stat > 0)
		cur = ftell((*param)->fp_o);

	if (pdf_dump_trailer(&pdf, &(*param)->fp_o, xref) != 0) {
		if ((*param)->stat > 0)
			printf("Trailer not written\n");
	} else {
		if ((*param)->stat > 0)
			printf("Trailer %ld byte(s) written\n",
				ftell((*param)->fp_o) - cur);
	}

	if ((*param)->stat > 0)
		printf("Total %ld byte(s) written\n",
			ftell((*param)->fp_o));

	pdf_obj_destroy(&pdf);

	if ((*param)->stat > 0)
		printf("Conversion ended (partial)\n");

	/* TODO: Finish me please :) */
	return 0;
}
