/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "cnki.h"
#include "iconv.h"
#include "zlib.h"
#include "jpeg.h"
#include "pdf.h"
#include "pdf_cnki.h"

int
cnki_pdf(cnki_t **param)
{
	if (*param == NULL)
		return 1;

	pdf_object_t *pdf = NULL;

	if (pdf_obj_create(&pdf) != 0)
		return 1;

	if ((*param)->stat > 0)
		printf("Begin processing PDF\n");

	if ((*param)->stat > 1)
		printf("Loading object(s)\n");

	if (pdf_load(&pdf, &(*param)->fp_i, (*param)->size_buf) != 0)
		return 1;

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
		printf("Loaded %d object(s)\n",
			pdf_get_count(&pdf));

	int dictionary_size;
	char *dictionary;

	char buf[64];

	if ((*param)->stat > 1)
		printf("Searching for parent object(s)\n");

	int *parent = NULL;
	pdf_get_parent_id(&pdf, &parent);

	if (parent[0] == 0)
		return 1;

	if ((*param)->stat > 0)
		printf("Discovered %d parent object(s)\n", parent[0]);

	int *parent_missing = malloc(parent[0] * sizeof(int));

	if (parent_missing == NULL)
		return 1;

	int *kid;

	for (int i = 1; i <= parent[0]; i++) {
		if ((*param)->stat > 1)
			printf("Searching for object %d\n", parent[i]);

		kid = NULL;
		pdf_get_kid_id(&pdf, parent[i], &kid);

		if (kid[0] != 0) {
			if ((*param)->stat > 0)
				printf("Object %d is missing\n", parent[i]);

			if ((*param)->stat > 1)
				printf("Generating object\n");

			dictionary_size = 64 + 12 * kid[0];
			dictionary = malloc(dictionary_size);

			if (dictionary == NULL) {
				free(parent);
				free(parent_missing);
				return 1;
			}

			memset(dictionary, 0, dictionary_size);

			snprintf(buf, 64,
				"<<\n/Type /Pages\n/Kids [");
			strcat(dictionary, buf);

			for (int j = 1; j <= kid[0]; j++) {
				snprintf(buf, 64,
					"%d 0 R",
					kid[j]);
				strcat(dictionary, buf);

				if (j < kid[0])
					strcat(dictionary, " ");
			}

			snprintf(buf, 64,
				"]\n/Count %d\n>>",
				pdf_get_kid_count(&pdf, parent[i]));
			strcat(dictionary, buf);

			pdf_obj_prepend(&pdf, parent[i], NULL, dictionary, NULL, 0);

			parent_missing[i - 1] = 1;

			if ((*param)->stat > 0)
				printf("Generated object for %d child(ren)\n",
					kid[0]);

			free(dictionary);
		} else {
			parent_missing[i - 1] = 0;

			if ((*param)->stat > 0)
				printf("Object %d exists\n", parent[i]);
		}

		free(kid);
	}

	if ((*param)->stat > 1)
		printf("Searching for root object\n");

	dictionary_size = 128;
	dictionary = malloc(dictionary_size);

	if (dictionary == NULL) {
		free(parent);
		free(parent_missing);
		return 1;
	}

	memset(dictionary, 0, dictionary_size);

	int root = 0;

	int root_kid = 0;
	for (int i = 0; i < parent[0]; i++)
		if (parent_missing[i] == 1)
			root_kid++;

	if (root_kid <= 1) {
		if (root_kid == 0) {
			for (int i = 1; i <= parent[0]; i++)
				if (root == 0 || root < parent[i])
					root = parent[i];
		} else {
			for (int i = 0; i < parent[0]; i++)
				if (parent_missing[i] == 1)
					root = i;
		}

		if ((*param)->stat > 0)
			printf("Root object is %d.\n",
				root);
	} else {
		if ((*param)->stat > 0)
			printf("Root object is missing\n");

		if ((*param)->stat > 1)
			printf("Generating root object\n");

		root = pdf_get_free_id(&pdf);

		snprintf(buf, 64,
			"<<\n/Type /Pages\n/Kids ");
		strcat(dictionary, buf);

		if (parent[0] > 1)
			strcat(dictionary, "[");

		for (int i = 0, j = 0; i < parent[0]; i++) {
			if (parent_missing[i]) {
				snprintf(buf, 64, "%d 0 R", parent[i + 1]);
				strcat(dictionary, buf);

				if (++j < root_kid)
					strcat(dictionary, " ");
			}
		}

		if (parent[0] > 1)
			strcat(dictionary, "]");

		strcat(dictionary, "\n");

		snprintf(buf, 64, "/Count %d\n", (*param)->file_stat->page);
		strcat(dictionary, buf);

		strcat(dictionary, ">>");

		pdf_obj_prepend(&pdf, root, NULL, dictionary, NULL, 0);

		memset(dictionary, 0, dictionary_size);

		if ((*param)->stat > 0)
			printf("Generated root object %d.\n",
				root);
	}

	free(parent);
	free(parent_missing);

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
		printf("Searching for catalog object\n");

	int catalog = pdf_get_catalog_id(&pdf);

	if (catalog != 0) {
		if ((*param)->stat > 0)
			printf("Catalog object is %d.\n", catalog);
	} else {
		if ((*param)->stat > 0)
			printf("Catalog object is missing\n");

		if ((*param)->stat > 1)
			printf("Generating catalog object\n");

		snprintf(buf, 64,
			"<<\n/Type /Catalog\n/Pages %d 0 R\n",
			root);
		strcat(dictionary, buf);

		if (ids != NULL) {
			snprintf(buf, 64,
				"/Outlines %d 0 R\n/PageMode /UseOutlines\n",
				ids[0]);
			strcat(dictionary, buf);
		}

		strcat(dictionary, ">>");

		pdf_obj_append(&pdf, 0, NULL, dictionary, NULL, 0);

		if ((*param)->stat > 0)
			printf("Generated catalog object\n");
	}

	if ((*param)->stat > 1)
		printf("Searching for xref object\n");

	int xref = pdf_get_xref_id(&pdf);

	if (xref != 0) {
		if ((*param)->stat > 0)
			printf("Xref object is %d.\n", xref);

		if ((*param)->stat > 1)
			printf("Deleting xref object\n");

		pdf_obj_del(&pdf, xref);

		if ((*param)->stat > 0)
			printf("Deleted xref object\n");
	} else {
		if ((*param)->stat > 0)
			printf("Xref object is missing\n");
	}

	free(dictionary);

	if ((*param)->stat > 1)
		printf("Sorting object(s)\n");

	pdf_obj_sort(&pdf);

	if ((*param)->stat > 0)
		printf("Sorted object(s)\n");

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

	long cur_xref = ftell((*param)->fp_o);

	if ((*param)->stat > 1)
		printf("Writing cross-reference table\n");

	if (pdf_dump_xref(&pdf, &(*param)->fp_o) != 0) {
		if ((*param)->stat > 0)
			printf("Cross-reference table not written\n");
	} else {
		if ((*param)->stat > 0)
			printf("Cross-reference table %ld byte(s) written\n",
				ftell((*param)->fp_o) - cur_xref);
	}

	if ((*param)->stat > 1)
		printf("Writing trailer\n");

	if ((*param)->stat > 0)
		cur = ftell((*param)->fp_o);

	if (pdf_dump_trailer(&pdf, &(*param)->fp_o, cur_xref) != 0) {
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

	return 0;
}

int
cnki_pdf_hn(cnki_t **param)
{
	if (*param == NULL)
		return 1;

	pdf_object_t *pdf = NULL;

	if (pdf_obj_create(&pdf) != 0)
		return 1;

	if ((*param)->stat > 1)
		printf("Generating PDF object(s)\n");

	int dictionary_size;
	char *dictionary;

	char buf[64];

	int *ids = NULL;

	int cnt = 0;
	int *root_kid = malloc((*param)->file_stat->page * sizeof(int));

	if (root_kid == NULL)
		return 1;

	memset(root_kid, 0, (*param)->file_stat->page);

	object_hn_t *ptr = (*param)->object_hn;
	while (ptr != NULL) {
		/*
		 * External object (ptr->image_length) +
		 * content object +
		 * resource object +
		 * page object
		 */
		pdf_get_free_ids(&pdf, &ids, ptr->image_length + 3);

		int stream_size;
		char *stream;

		int *dim = malloc(2 * ptr->image_length * sizeof(int));

		int ret;
		int wh[2];

		if (dim == NULL) {
			free(root_kid);
			return 1;
		}

		for (int i = 0; i < ptr->image_length; i++) {
			dictionary_size = 128;
			dictionary = malloc(dictionary_size);

			if (dictionary == NULL) {
				free(root_kid);
				free(dim);
				return 1;
			}

			memset(dictionary, 0, dictionary_size);

			strcat(dictionary, "<<\n/Type /XObject\n"
				"/Subtype /Image\n");

			if ((*param)->stat > 2)
				printf("\tDecoding data, page %04d item %02d... ",
					ptr->page, i);

			switch (ptr->image_data[i].format) {
				case JBIG:
					ret = cnki_jbig(&stream,
						&stream_size,
						&wh[0],
						&wh[1],
						ptr->image_data[i].image,
						ptr->image_data[i].size);

					if (ret != 0) {
						dim[i * 2] = 0;
						dim[i * 2 + 1] = 0;
						break;
					}

					snprintf(buf, 64, "/Width %d\n/Height %d\n",
						wh[0], wh[1]);
					strcat(dictionary, buf);

					strcat(dictionary, "/ColorSpace /DeviceGray\n"
						"/BitsPerComponent 1\n");

					snprintf(buf, 64, "/Length %d\n",
						stream_size);
					strcat(dictionary, buf);

					strcat(dictionary, "/Filter /CCITTFaxDecode\n");

					dim[i * 2] = wh[0];
					dim[i * 2 + 1] = wh[1];
					break;
				case DCT_0:
				case DCT_1:
					ret = strinfo_jpeg_dim(&wh[0],
						&wh[1],
						ptr->image_data[i].image,
						ptr->image_data[i].size);

					if (ret != 0) {
						dim[i * 2] = 0;
						dim[i * 2 + 1] = 0;
						break;
					}

					stream_size = ptr->image_data[i].size;
					stream = malloc(stream_size);
					if (stream == NULL) {
						free(dictionary);
						free(root_kid);
						free(dim);
						return 1;
					}
					memcpy(stream, ptr->image_data[i].image, stream_size);

					snprintf(buf, 64, "/Width %d\n/Height %d\n",
						wh[0], wh[1]);
					strcat(dictionary, buf);

					strcat(dictionary, "/ColorSpace /DeviceRGB\n"
						"/BitsPerComponent 8\n");

					snprintf(buf, 64, "/Length %d\n",
						stream_size);
					strcat(dictionary, buf);

					strcat(dictionary, "/Filter /DCTDecode\n");

					dim[i * 2] = wh[0];
					dim[i * 2 + 1] = wh[1];
					break;
				case JBIG2:
				case JPX:
				default:
					ret = -1;
					dim[i * 2] = -1;
					dim[i * 2 + 1] = -1;
					break;
			}

			strcat(dictionary, ">>");

			if (ret == 0) {
				if ((*param)->stat > 2)
					printf("Done\n");

				pdf_obj_append(&pdf, ids[i],
					NULL, dictionary, stream, stream_size);

				free(dictionary);
				free(stream);
			} else if (ret == 1) {
				free(dictionary);

				pdf_obj_append(&pdf, ids[i], NULL, NULL, NULL, 0);
			} else {
				free(dictionary);
			}
		}

		dictionary_size = 128;
		dictionary = malloc(dictionary_size);

		if (dictionary == NULL) {
			free(root_kid);
			free(dim);
			return 1;
		}

		memset(dictionary, 0, dictionary_size);

		strcat(dictionary, "<<\n/XObject <<");

		for (int i = 0; i < ptr->image_length; i++) {
			snprintf(buf, 64, "/Im%d %d 0 R", i, ids[i]);
			strcat(dictionary, buf);

			if (i + 1 < ptr->image_length)
				strcat(dictionary, " ");
		}

		strcat(dictionary, ">>\n>>");

		pdf_obj_append(&pdf, ids[ptr->image_length], NULL, dictionary, NULL, 0);

		free(dictionary);

		int conv_size;
		char *conv_dst;
		char conv_src[2];
		char conv_hex[3];

		if (strncmp(ptr->text + 8, "COMPRESSTEXT", 12) == 0) {
			cnki_zlib(&stream, &stream_size, ptr->text, ptr->text_size);

			dictionary_size = stream_size / 8 + 7;
			dictionary = malloc(dictionary_size);

			if (dictionary == NULL) {
				free(root_kid);
				free(dim);
				return 1;
			}

			memset(dictionary, 0, dictionary_size);

			strcat(dictionary, "<feff");

			for (int i = 0; i < stream_size; i += 16) {
				conv_src[0] = stream[i + 7];
				conv_src[1] = stream[i + 6];

				conv_size = 6;

				if (strconv(&conv_dst, "UTF-16BE",
					conv_src, "GB18030", &conv_size) == 0) {
					for (int j = 0; j < conv_size - 2; j++) {
						snprintf(conv_hex, 3,
							"%02x", (unsigned char) conv_dst[j]);
						strcat(dictionary, conv_hex);
					}
					free(conv_dst);
				}
			}
			free(stream);

			strcat(dictionary, ">");
		} else {
			dictionary_size = ptr->text_size;
			dictionary = malloc(dictionary_size);

			if (dictionary == NULL) {
				free(root_kid);
				free(dim);
				return 1;
			}

			memset(dictionary, 0, dictionary_size);

			strcat(dictionary, "<feff");

			for (int i = 0; i < ptr->text_size; i += 4) {
				conv_src[0] = ptr->text[i + 3];
				conv_src[1] = ptr->text[i + 2];

				conv_size = 6;

				if (strconv(&conv_dst, "UTF-16BE",
					conv_src, "GB18030", &conv_size) == 0) {
					for (int j = 0; j < conv_size - 2; j++) {
						snprintf(conv_hex, 3,
							"%02x", (unsigned char) conv_dst[j]);
						strcat(dictionary, conv_hex);
					}
					free(conv_dst);
				}
			}

			strcat(dictionary, ">");
		}

		/* FIXME: Use the text somehow? */
		free(dictionary);

		dictionary_size = 64 + 12 * ptr->image_length;
		dictionary = malloc(dictionary_size);

		if (dictionary == NULL) {
			free(root_kid);
			free(dim);
			return 1;
		}

		memset(dictionary, 0, dictionary_size);

		strcat(dictionary, "q\n");

		strcat(dictionary, "0.120000 0 0 0.120000 0 0 cm\n");

		for (int i = 0; i < ptr->image_length; i++) {
			if (dim[i * 2] <= 0 || dim[i * 2 + 1] <= 0)
				continue;

			/* Apply transformation matrix */
			if (ptr->image_data[i].format == DCT_1)
				strcat(dictionary, "-1 0 0 -1 0 0 cm\n");

			snprintf(buf, 64, "%d 0 0 %d 0 0 cm\n",
				dim[i * 2], dim[i * 2 + 1]);
			strcat(dictionary, buf);

			snprintf(buf, 64, "/Im%d Do\n", i);
			strcat(dictionary, buf);
		}

		strcat(dictionary, "Q");

		if (strdeflate(&stream, &stream_size, dictionary, strlen(dictionary)) != 0) {
			free(dictionary);
			free(root_kid);
			free(dim);
			return 1;
		}

		memset(dictionary, 0, dictionary_size);

		strcat(dictionary, "<<\n");

		snprintf(buf, 64, "/Length %d\n", stream_size);
		strcat(dictionary, buf);

		strcat(dictionary, "/Filter /FlateDecode\n");

		strcat(dictionary, ">>");

		pdf_obj_append(&pdf, ids[ptr->image_length + 1],
			NULL, dictionary, stream, stream_size);

		free(stream);

		memset(dictionary, 0, dictionary_size);

		strcat(dictionary, "<<\n/Type /Page\n");

		snprintf(buf, 64, "/Resources %d 0 R\n", ids[ptr->image_length]);
		strcat(dictionary, buf);

		snprintf(buf, 64, "/Contents %d 0 R\n", ids[ptr->image_length + 1]);
		strcat(dictionary, buf);

		/* A4 paper */
		strcat(dictionary, "/MediaBox [ 0 0 595.276 841.89 ]\n");

		/* Add /Parent when we know root */
		pdf_obj_append(&pdf, ids[ptr->image_length + 2], NULL, dictionary, NULL, 0);

		free(dictionary);

		root_kid[cnt++] = ids[ptr->image_length + 2];

		free(ids);
		ids = NULL;

		free(dim);

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

	ids = NULL;

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
		printf("Generating root object\n");

	dictionary_size = 64 + 12 * (*param)->file_stat->page;
	dictionary = malloc(dictionary_size);

	if (dictionary == NULL) {
		free(root_kid);
		return 1;
	}

	memset(dictionary, 0, dictionary_size);

	int root = pdf_get_free_id(&pdf);

	snprintf(buf, 64, "<<\n/Type /Pages\n/Kids ");
	strcat(dictionary, buf);

	if ((*param)->file_stat->page > 1)
		strcat(dictionary, "[");

	for (int i = 0; i < (*param)->file_stat->page; i++) {
		snprintf(buf, 64, "%d 0 R", root_kid[i]);
		strcat(dictionary, buf);
		if (i + 1 < (*param)->file_stat->page)
			strcat(dictionary, " ");
	}

	if ((*param)->file_stat->page > 1)
		strcat(dictionary, "]");

	strcat(dictionary, "\n");

	snprintf(buf, 64, "/Count %d\n", (*param)->file_stat->page);
	strcat(dictionary, buf);

	strcat(dictionary, ">>");

	pdf_obj_prepend(&pdf, root, NULL, dictionary, NULL, 0);

	free(dictionary);

	dictionary_size = 128;
	dictionary = malloc(dictionary_size);

	if (dictionary == NULL) {
		free(root_kid);
		return 1;
	}

	pdf_object_t *tmp = NULL;

	/* Add /Parent to page object */
	for (int i = 0; i < (*param)->file_stat->page; i++) {
		if (pdf_get_obj(&pdf, root_kid[i], &tmp) != 0) {
			free(dictionary);
			free(root_kid);
			return 1;
		}

		memset(dictionary, 0, dictionary_size);

		memcpy(dictionary, tmp->dictionary, tmp->dictionary_size);

		snprintf(buf, 64, "/Parent %d 0 R\n>>", root);
		strcat(dictionary, buf);

		if (pdf_obj_replace(&pdf, root_kid[i], NULL, dictionary, NULL, 0) != 0) {
			free(dictionary);
			free(root_kid);
			return 1;
		}
	}

	free(root_kid);

	memset(dictionary, 0, dictionary_size);

	if ((*param)->stat > 0)
		printf("Generated root object %d.\n",
			root);

	if ((*param)->stat > 1)
		printf("Generating catalog object\n");

	snprintf(buf, 64,
		"<<\n/Type /Catalog\n/Pages %d 0 R\n",
		root);
	strcat(dictionary, buf);

	if (ids != NULL) {
		snprintf(buf, 64,
			"/Outlines %d 0 R\n/PageMode /UseOutlines\n",
			ids[0]);
		strcat(dictionary, buf);
	}

	strcat(dictionary, ">>");

	pdf_obj_append(&pdf, 0, NULL, dictionary, NULL, 0);

	free(dictionary);

	if ((*param)->stat > 0)
		printf("Generated catalog object\n");

	if ((*param)->stat > 1)
		printf("Sorting object(s)\n");

	pdf_obj_sort(&pdf);

	if ((*param)->stat > 0)
		printf("Sorted object(s)\n");

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

	return 0;
}
