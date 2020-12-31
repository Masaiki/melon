/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

/*
 * order (MSB first):
 * 0
 * 0
 * 0
 * 0
 * HITOLO
 * SEQ
 * ILEAVE (default)
 * SMID (default)
 *
 * options (MSB first):
 * 0
 * LRLTWO
 * VLENGTH
 * TPDON (default)
 * TPBON (default)
 * DPON (default)
 * DPPRIV
 * DPLAST
 */
typedef struct _bih_t {
	char d_l; /* Initial resolution layer */
	char d; /* Final resolution layer */
	char p; /* Number of bit-planes, for bi-level image, always 1 */
	char fill; /* Always 0 */
	/* MSB first */
	int32_t x_d; /* Horizontal dimension at highestresolution */
	int32_t y_d; /* Vertical dimension at highest resolution */
	int32_t l_0; /* Number of lines per stripe at lowest resolution */
	char m_x; /* Maximum horizontal offsets (default: 8) */
	char m_y; /* Maximum vertical offsets (default: 0) */
	char order;
	char options;
	char *dptable; /* 0 or 1728 */
} bih_t;

typedef enum _dib_compression_code {
	BI_RGB,
	BI_RLE8,
	BI_RLE4,
	BI_BITFIELDS,
	BI_JPEG,
	BI_PNG,
	BI_ALPHABITFIELDS,
	BI_CMYK = 11,
	BI_CMYKRLE8 = 12,
	BI_CMYKRLE4 = 13
} dib_compression_code;

typedef struct _dib_t {
	uint32_t dib_size; /* Always 40 */
	int32_t width;
	int32_t height;
	uint16_t plane; /* Always 1 */
	uint16_t depth;
	uint32_t compression; /* dib_compression_code */
	uint32_t size;
	uint32_t resolution_h;
	uint32_t resolution_v;
	uint32_t colour;
	uint32_t colour_used;
} dib_t;

typedef struct _colour_table {
	uint16_t blue;
	uint16_t green;
	uint16_t red;
	uint16_t fill; /* Always 0 */
} colour_table;
