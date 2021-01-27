#
# Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
#
# SPDX-License-Identifier: Apache-2.0
#

src =	melon.c iconv.c zlib.c jbig.c jpeg.c \
	cnki_caj.c cnki_hn.c cnki_kdh.c cnki_outline_tree.c \
	cnki_pdf.c cnki_xml.c cnki_zlib.c cnki_jbig.c cnki.c \
	pdf_cnki.c pdf_get.c pdf_parser.c pdf_writer.c pdf.c
inc =	extern.h version.h iconv.h zlib.h jbig.h jpeg.h \
	cnki.h pdf_cnki.h cnki_jbig.h pdf.h

obj = ${src:.c=.o}

PREFIX = /usr/local

CFLAGS = -O3 -march=native -pipe -flto -Wall -Wextra -Wno-unused-parameter
LDFLAGS = -Wl,-O3 -lcrypto -liconv -lz -ljbig -ljpeg -Wl,--as-needed

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
