/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __linux__

#define _GNU_SOURCE

#endif /* __linux__ */

#include <stdlib.h>
#include <string.h>

#include "pdf.h"

static void *
_memmem_whitespace(const void *p0, size_t s0, const void *p1, size_t s1)
{
	const char whitespace[6] = {
		'\r',
		'\n',
		'\f',
		'\t',
		'\0',
		' '
	};

	char tmp[s1 + 1];
	memcpy(tmp, p1, s1);

	char *ret;

	for (int i = 0; i < 6; i++) {
		tmp[s1] = whitespace[i];
		if((ret = memmem(p0, s0, tmp, s1 + 1)) != NULL)
			return ret;
	}

	return NULL;
}

static int
_locate(pdf_object_t **pdf, FILE **fp, int size_buf)
{
	pdf_object_t *ptr = *pdf;
	while (ptr->next != NULL)
		ptr = ptr->next;

	char buf[size_buf];

	long cur = ftell(*fp);
	long end;

	fseek(*fp, 0, SEEK_END);
	end = ftell(*fp);
	fseek(*fp, cur, SEEK_SET);

	int head = 0;
	int tail = 0;
	char *pos;
	char *tmp;

	for (;;) {
		fread(buf, size_buf, 1, *fp);

		if (head == 0 && (pos = _memmem_whitespace(buf, size_buf, " 0 obj", 6)) != NULL)
			head = cur + (pos - buf) + 7;

		if (tail == 0 && (pos = _memmem_whitespace(buf, size_buf, "endobj", 6)) != NULL) {
			/* We need to check if it is the object stored in stream */
			while (memcmp(pos + 7,
				"\r\nendstream", 11) == 0 &&
				(tmp = _memmem_whitespace(pos + 6,
				size_buf - (pos - buf) - 6,
				"endobj", 6)) != NULL)
					pos = tmp;

			if (pos - buf < size_buf - 7)
				tail = cur + (pos - buf);
		}

		if (tail > head) {
			if (ptr->next == NULL) {
				ptr->next = malloc(sizeof(pdf_object_t));

				if (ptr->next == NULL)
					return 1;

				ptr->next->id = 0;
				ptr->next->object_size = 0;
				ptr->next->object = NULL;
				ptr->next->dictionary_size = 0;
				ptr->next->dictionary = NULL;
				ptr->next->stream_size = 0;
				ptr->next->stream = NULL;
				ptr->next->next = NULL;
				ptr = ptr->next;
			}

			ptr->address = head;
			ptr->size = tail - head;

			fseek(*fp, tail + 6, SEEK_SET);
			head = tail = 0;
		} else {
			fseek(*fp, -6, SEEK_CUR);
		}

		if ((cur = ftell(*fp)) + 6 >= end)
			break;
	}

	return 0;
}

int
pdf_load(pdf_object_t **pdf, FILE **fp, int size_buf)
{
	if (*pdf == NULL || *fp == NULL || size_buf < 7)
		return 1;

	if (_locate(pdf, fp, size_buf) != 0)
		return 1;

	pdf_object_t *ptr = (*pdf)->next;

	char *buf;
	char *head;
	char *tail;
	char *tmp;

	while (ptr != NULL) {
		buf = malloc(ptr->size);

		if (buf == NULL)
			return 1;

		memset(buf, 0, ptr->size);

		fseek(*fp, ptr->address - 12, SEEK_SET);
		fread(buf, 8, 1, *fp);

		for (int i = 0; i < 8; i++) {
			if (buf[i] >= '0' && buf[i] <= '9') {
				ptr->id = atoi(buf + i);
				break;
			}
		}

		fseek(*fp, ptr->address, SEEK_SET);
		fread(buf, ptr->size, 1, *fp);

		if ((head = memmem(buf, ptr->size, "<<", 2)) != NULL &&
			(tail = _memmem_whitespace(buf, ptr->size, ">>", 2)) != NULL) {
			/*
			 * A dictionary object may have nested dictionary,
			 * but it should not be in a stream
			 */
			while ((tmp = _memmem_whitespace(tail + 2,
				ptr->size - (tail - buf) - 2,
				">>", 2)) != NULL &&
				memmem(tail + 2,
				ptr->size - (tail - buf) - 2,
				"stream\r\n", 8) == NULL)
				tail = tmp;

			ptr->dictionary_size = tail - head + 2;
			ptr->dictionary = malloc(ptr->dictionary_size + 1);

			if (ptr->dictionary == NULL)
				return 1;

			memset(ptr->dictionary, 0, ptr->dictionary_size + 1);
			memcpy(ptr->dictionary, head, ptr->dictionary_size);

			if ((head = memmem(tail,
				ptr->size - (tail - buf),
				"stream\r\n", 8)) != NULL &&
				(tail = _memmem_whitespace(head,
				ptr->size - (head - buf),
				"endstream", 9)) != NULL) {
				/*
				 * An object may contain a stream that
				 * contains another object that
				 * contains another stream
				 */
				while (_memmem_whitespace(tail,
					ptr->size - (tail - buf),
					"endobj", 6) != NULL &&
					(tmp = _memmem_whitespace(tail + 9,
					ptr->size - (tail - buf) - 9,
					"endstream", 9)) != NULL)
						tail = tmp;

				ptr->stream_size = (tail - head) - 8;
				ptr->stream = malloc(ptr->stream_size);

				if (ptr->stream == NULL)
					return 1;

				memcpy(ptr->stream, head + 8, ptr->stream_size);
			}
		} else {
			ptr->object_size = ptr->size;
			ptr->object = malloc(ptr->object_size + 1);

			if (ptr->object == NULL)
				return 1;

			memset(ptr->object, 0, ptr->object_size + 1);
			memcpy(ptr->object, buf, ptr->object_size);
		}

		free(buf);

		ptr = ptr->next;
	}

	return 0;
}
