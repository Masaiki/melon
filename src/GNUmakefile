#
# Copyright (c) 2020, yzrh <yzrh@noema.org>
#
# SPDX-License-Identifier: Apache-2.0
#

src != ls *.c
obj = ${src:.c=.o}

PREFIX = /usr/local

CFLAGS = -O3 -march=native -pipe -Wall
LDFLAGS = -Wl,-O3 -lcrypto -Wl,--as-needed

all: ${obj}
	${CC} ${LDFLAGS} -o melon $^

clean:
	rm -f melon ${obj}

install:
	install -d ${PREFIX}/bin
	install melon ${PREFIX}/bin/

deinstall:
	rm -f ${PREFIX}/bin/melon

.PHONY: all clean install deinstall
