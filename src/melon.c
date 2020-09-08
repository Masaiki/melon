/*
 * Copyright (c) 2020, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "extern.h"
#include "version.h"

int
main(int argc, char **argv, char **envp)
{
	printf("Melon " VERSION "." RELEASE "." PATCH EXTRA "\n");
	printf("Copyright (c) 2020, yzrh <yzrh@noema.org>\n\n");

	cnki_t *param = NULL;

	if (cnki_create(&param) != 0) {
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}

	int c;

	for (;;) {
		static struct option long_options[] = {
			{"output", required_argument, 0, 'o'},
			{"buffer", required_argument, 0, 'b'},
			{"verbose", no_argument, 0, 'v'},
			{0, 0, 0, 0}
		};

		int option_index = 0;

		c = getopt_long(argc, argv, "o:b:v",
			long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 'o':
				if ((param->fp_o = fopen(optarg, "w")) == NULL) {
					fprintf(stderr, "%s: %s\n", argv[0],
						strerror(errno));
					return EXIT_FAILURE;
				}
				break;
			case 'b':
				param->size_buf = atoi(optarg);
				break;
			case 'v':
				param->stat += 1;
				break;
			case '?':
				break;
			default:
				abort();
		}
	}

	if (argc - optind == 1) {
		if (param->fp_o == NULL) {
			if (param->stat == 0) {
				param->fp_o = stdout;
			} else {
				fprintf(stderr, "%s: --verbose ", argv[0]);
				fprintf(stderr, "must not be set ");
				fprintf(stderr, "when using stdout\n");
				return EXIT_FAILURE;
			}
		}

		if ((param->fp_i = fopen(argv[optind], "r")) == NULL) {
			fprintf(stderr, "%s: %s\n", argv[0],
				strerror(errno));
			return EXIT_FAILURE;
		}

		cnki_info(&param);

		if (strcmp(param->file_stat->type, "%PDF") == 0) {
			if (cnki_pdf(&param) != 0) {
				fprintf(stderr, "%s: %s\n", argv[0],
					strerror(errno));
				return EXIT_FAILURE;
			}
		} else if (strcmp(param->file_stat->type, "CAJ") == 0) {
			if (cnki_caj(&param) != 0) {
				fprintf(stderr, "%s: %s\n", argv[0],
					strerror(errno));
				return EXIT_FAILURE;
			}
		} else if (strcmp(param->file_stat->type, "HN") == 0) {
			if (cnki_nh(&param) != 0) {
				fprintf(stderr, "%s: %s\n", argv[0],
					strerror(errno));
				return EXIT_FAILURE;
			}
		} else if (strcmp(param->file_stat->type, "KDH ") == 0) {
			if (cnki_kdh(&param) != 0) {
				fprintf(stderr, "%s: %s\n", argv[0],
					strerror(errno));
				return EXIT_FAILURE;
			}
		} else {
			fprintf(stderr, "%s: %s\n", argv[0],
				"Invalid file");
			return EXIT_FAILURE;
		}

		fclose(param->fp_i);
		fclose(param->fp_o);
	} else {
		fprintf(stderr, "Usage: %s ", argv[0]);
		fprintf(stderr, "[--output --buffer --verbose] file\n");
		return EXIT_FAILURE;
	}

	cnki_destroy(&param);
}
