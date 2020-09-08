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

	for (;;) {
		fread(buf, (*param)->size_buf, 1, (*param)->fp_i);

		for (int i = 0; i < (*param)->size_buf; i++) {
			buf[i] ^= key[key_cur % key_len];
			key_cur++;
		}

		fwrite(buf, (*param)->size_buf, 1, (*param)->fp_o);

		if (ftell((*param)->fp_i) == size)
			break;
	}

	if ((*param)->stat > 0)
		printf("Decryption ended total %ld byte(s) written\n",
			ftell((*param)->fp_o));

	return 0;
}
