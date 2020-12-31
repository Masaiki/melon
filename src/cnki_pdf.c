/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "cnki.h"
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

	if ((*param)->stat > 1)
		printf("Searching for parent object(s)\n");

	int *parent = NULL;
	pdf_get_parent_id(&pdf, &parent);

	if (parent[0] == 0)
		return 1;

	if ((*param)->stat > 0)
		printf("Discovered %d parent object(s)\n", parent[0]);

	char buf[64];

	int parent_missing[parent[0]];
	int *kid;
	int dictionary_size;
	char *dictionary;

	for (int i = 1; i <= parent[0]; i++) {
		if ((*param)->stat > 1)
			printf("Searching for object %d\n", parent[i]);

		kid = NULL;
		pdf_get_kid_id(&pdf, parent[i], &kid);

		if (kid[0] != 0) {
			if ((*param)->stat > 0)
				printf("Object is missing\n");

			if ((*param)->stat > 1)
				printf("Generating object\n");

			dictionary_size = 64 + 12 * kid[0];
			dictionary = malloc(dictionary_size);

			if (dictionary == NULL)
				return 1;

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

			pdf_obj_prepend(&pdf, parent[i], NULL, dictionary, NULL);

			parent_missing[i - 1] = 1;

			if ((*param)->stat > 0)
				printf("Generated object for %d child(ren)\n",
					kid[0]);

			free(dictionary);
		} else {
			parent_missing[i - 1] = 0;

			if ((*param)->stat > 0)
				printf("Object exists\n");
		}

		free(kid);
	}

	if ((*param)->stat > 1)
		printf("Searching for root object\n");

	dictionary_size = 128;
	dictionary = malloc(dictionary_size);

	if (dictionary == NULL)
		return 1;

	memset(dictionary, 0, dictionary_size);

	int root = 0;

	int root_kid = 0;
	for (int i = 0; i < parent[0]; i++)
		if (parent_missing[i])
			root_kid++;

	if (root_kid <= 1) {
		if (root_kid == 0) {
			for (int i = 1; i <= parent[0]; i++)
				if (root == 0 || root < parent[i])
					root = parent[i];
		} else {
			for (int i = 0; i < parent[0]; i++)
				if (parent_missing[i])
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

		for (int i = 0; i < parent[0]; i++) {
			if (parent_missing[i]) {
				snprintf(buf, 64, "%d 0 R", parent[i + 1]);
				strcat(dictionary, buf);
				if (i < root_kid)
					strcat(dictionary, " ");
			}
		}

		if (parent[0] > 1)
			strcat(dictionary, "]");

		strcat(dictionary, "\n");

		snprintf(buf, 64, "/Count %d\n", (*param)->file_stat->page);
		strcat(dictionary, buf);

		strcat(dictionary, ">>");

		pdf_obj_prepend(&pdf, root, NULL, dictionary, NULL);

		memset(dictionary, 0, dictionary_size);

		if ((*param)->stat > 0)
			printf("Generated root object %d.\n",
				root);
	}

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

		pdf_obj_append(&pdf, 0, NULL, dictionary, NULL);

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
