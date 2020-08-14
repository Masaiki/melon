/*
 * Copyright (c) 2020, yzrh <yzrh@tuta.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include "cnki.h"

int
cnki_caj(cnki_t **param)
{
	if (*param == NULL)
		return 1;

	if ((*param)->stat > 0)
		printf("Begin 'CAJ' conversion\n");

	if ((*param)->stat > 1)
		printf("Reading document body address at %x\n", ADDRESS_CAJ_BODY);

	int addr;

	fseek((*param)->fp_i, ADDRESS_CAJ_BODY, SEEK_SET);
	fread(&addr, 4, 1, (*param)->fp_i);
	fseek((*param)->fp_i, addr, SEEK_SET);
	fread(&addr, 4, 1, (*param)->fp_i);
	fseek((*param)->fp_i, addr, SEEK_SET);

	if ((*param)->stat > 0)
		printf("Advised document body address is %x\n", addr);

	cnki_pdf(param);

	if ((*param)->stat > 0)
		printf("Conversion ended\n");

	return 0;
}
