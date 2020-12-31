/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cnki.h"

/* cnki.c */
int cnki_create(cnki_t **param);
void cnki_destroy(cnki_t **param);
int cnki_info(cnki_t **param);

/* cnki_caj.c */
int cnki_caj(cnki_t **param);

/* cnki_hn.c */
int cnki_hn(cnki_t **param);

/* cnki_kdh.c */
int cnki_kdh(cnki_t **param);
