/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <jpeglib.h>

int
strinfo_jpeg_dim(int *jpeg_width, int *jpeg_height,
	const char * restrict data, int data_size)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, (unsigned char *) data, data_size);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_calc_output_dimensions(&cinfo);

	*jpeg_width = cinfo.output_width;
	*jpeg_height = cinfo.output_height;

	jpeg_destroy((struct jpeg_common_struct *) &cinfo);

	jpeg_destroy_decompress(&cinfo);

	return 0;
}
