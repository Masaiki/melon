/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <openssl/md5.h>

#include "pdf.h"

int
pdf_dump_obj(pdf_object_t **pdf, FILE **fp)
{
	if (*pdf == NULL || *fp == NULL)
		return 1;

	long cur;

	pdf_object_t *ptr = (*pdf)->next;
	while (ptr != NULL) {
		ptr->address = cur = ftell(*fp);

		fprintf(*fp, "%d 0 obj\n", ptr->id);

		if (ptr->dictionary != NULL) {
			fwrite(ptr->dictionary, ptr->dictionary_size, 1, *fp);
			fputs("\n", *fp);
		} else if (ptr->object != NULL) {
			fwrite(ptr->object, ptr->object_size, 1, *fp);
			fputs("\n", *fp);
		} else if (ptr->stream == NULL) {
			fputs("null\n", *fp);
		}

		if (ptr->stream != NULL) {
			fputs("stream\r\n", *fp);
			fwrite(ptr->stream, ptr->stream_size, 1, *fp);
			fputs("endstream\n", *fp);
		}

		fputs("endobj\n", *fp);

		ptr->size = ftell(*fp) - cur;

		ptr = ptr->next;
	}

	return 0;
}

int
pdf_dump_header(pdf_object_t **pdf, FILE **fp)
{
	if (*pdf == NULL || *fp == NULL)
		return 1;

	fputs("%PDF-1.7\n", *fp);

	const unsigned char bin[4] = {
		0xf6,
		0xe4,
		0xfc,
		0xdf,
	};

	fputs("%", *fp);
	fwrite(bin, 4, 1, *fp);
	fputs("\n", *fp);

	return 0;
}

int
pdf_dump_xref(pdf_object_t **pdf, FILE **fp)
{
	if (*pdf == NULL || *fp == NULL)
		return 1;

	fputs("xref\n", *fp);

	pdf_object_t *ptr = *pdf;

	pdf_object_t *start = ptr;
	int count = 1;

	while (ptr != NULL) {
		if (ptr->next == NULL ||
			(ptr->next != NULL && ptr->next->id != ptr->id + 1)) {
			fprintf(*fp, "%d %d\n", start->id, count);

			for (; count > 0; count--) {
				fprintf(*fp, "%010d %05d %s\r\n",
					start->address,
					start->address > 0 ? 0 : 65535,
					start->size > 0 ? "n" : "f");
				start = start->next;
			}

			if (ptr->next != NULL)
				start = ptr->next;
		}

		ptr = ptr->next;
		count++;
	}

	return 0;
}

int
pdf_dump_trailer(pdf_object_t **pdf, FILE **fp, int xref)
{
	if (*pdf == NULL || *fp == NULL)
		return 1;

	fputs("trailer\n", *fp);

	fputs("<<\n", *fp);

	/*
	 * File identifiers should be generated using
	 * (a) Current time
	 * (b) File path
	 * (c) Size of file
	 * (d) Values of all entries in the
	 * file's document information dictionary
	 *
	 * It is recommended to be computed according to RFC 1321
	 */

	time_t timestamp = time(NULL);
	int size = pdf_get_size(pdf);

	int buf_size;
	char buf[64];

	buf_size = snprintf(buf, 64, "%lx%x", timestamp, size);

	unsigned char str[64];
	memcpy(str, buf, 64);

	unsigned char fid[MD5_DIGEST_LENGTH];
	MD5(str, buf_size, fid);

	pdf_object_t *ptr = *pdf;
	while (ptr->next != NULL)
		ptr = ptr->next;

	/*
	 * TODO: Document information dictionary
	 * `"/Producer (Melon)"'
	 * `"/CreationDate (D:YYYYMMDDHHmmSS+00'00')"'
	 *
	 * Trailer dictionary
	 * `"/Info %d 0 R"'
	 */
	fprintf(*fp,
		"/Size %d\n/Root %d 0 R\n",
		ptr->id + 1,
		pdf_get_catalog_id(pdf));

	fputs("/ID [", *fp);

	for (int i = 0; i < 2; i++) {
		fputs("<", *fp);

		for (int j = 0; j < MD5_DIGEST_LENGTH; j++)
			fprintf(*fp, "%02x", fid[j]);

		fputs(">", *fp);

		if (i < 1)
			fputs(" ", *fp);
	}

	fputs("]\n", *fp);

	fputs(">>\n", *fp);

	fputs("startxref\n", *fp);

	fprintf(*fp, "%d\n", xref);

	fputs("%%EOF\n", *fp);

	return 0;
}
