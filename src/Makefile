#
# Copyright (c) 2020, yzrh <yzrh@noema.org>
#
# SPDX-License-Identifier: Apache-2.0
#

src =	melon.c iconv.c zlib.c \
	cnki_caj.c cnki_hn.c cnki_kdh.c cnki_outline_tree.c \
	cnki_pdf.c cnki_xml.c cnki_zlib.c cnki.c \
	pdf_cnki.c pdf_get.c pdf_parser.c pdf_writer.c pdf.c
inc =	extern.h version.h iconv.h zlib.h \
	cnki.h pdf_cnki.h pdf.h

obj = ${src:.c=.o}

PREFIX = /usr/local

CFLAGS = -O3 -march=native -pipe -flto=thin -Wall
LDFLAGS = -Wl,-O3 -lcrypto -liconv -lz -Wl,--as-needed

CFLAGS += -I/usr/local/include
LDFLAGS += -L/usr/local/lib

all: ${obj} ${inc}
	${CC} ${LDFLAGS} -o melon ${obj}

clean:
	rm -f melon ${obj}

install:
	install -d ${PREFIX}/bin
	install melon ${PREFIX}/bin/

deinstall:
	rm -f ${PREFIX}/bin/melon

.PHONY: all clean install deinstall
