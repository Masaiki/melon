/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cnki.h"

int
cnki_kdh(cnki_t **param)
{
	if (*param == NULL)
		return 1;

	if ((*param)->stat > 0)
		printf("Begin 'KDH' decryption\n");

	fseek((*param)->fp_i, 0, SEEK_END);

	long size = ftell((*param)->fp_i);

	fseek((*param)->fp_i, ADDRESS_KDH_BODY, SEEK_SET);

	const char key[] = KEY_KDH;
	const int key_len = KEY_KDH_LENGTH;
	long key_cur = 0;

	char buf[(*param)->size_buf];

	FILE *tmp = tmpfile();

	if (tmp == NULL)
		return 1;

	for (;;) {
		fread(buf, (*param)->size_buf, 1, (*param)->fp_i);

		for (int i = 0; i < (*param)->size_buf; i++) {
			buf[i] ^= key[key_cur % key_len];
			key_cur++;
		}

		fwrite(buf, (*param)->size_buf, 1, tmp);

		if (ftell((*param)->fp_i) == size)
			break;
	}

	if ((*param)->stat > 0)
		printf("Decrypted %ld byte(s)\n", ftell(tmp));

	fseek(tmp, 0, SEEK_SET);

	FILE *orig = (*param)->fp_i;
	(*param)->fp_i = tmp;

	cnki_pdf(param);

	(*param)->fp_i = orig;
	fclose(tmp);

	if ((*param)->stat > 0)
		printf("Conversion ended\n");

	return 0;
}
