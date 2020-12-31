/*
 * Copyright (c) 2020-2021, yzrh <yzrh@noema.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include "cnki.h"

int
cnki_outline_tree(object_outline_tree_t **outline_tree,
	object_outline_t **outline, int *ids)
{
	if (*outline_tree != NULL || *outline == NULL)
		return 1;

	int pos = 0;

	*outline_tree = malloc(sizeof(object_outline_tree_t));

	if (*outline_tree == NULL)
		return 1;

	object_outline_tree_t *tree = *outline_tree;

	tree->id = ids[pos++];
	tree->item = NULL;
	tree->up = NULL;
	tree->left = NULL;
	tree->right = NULL;

	object_outline_t *ptr = *outline;
	while (ptr != NULL) {
		if (tree->item == NULL ||
			ptr->depth == tree->item->depth) {
			while (tree->left != NULL)
				tree = tree->left;

			tree->left = malloc(sizeof(object_outline_tree_t));

			if (tree->left == NULL)
				return 1;

			tree->left->id = ids[pos++];
			tree->left->item = ptr;
			tree->left->up = tree;
			tree->left->left = NULL;
			tree->left->right = NULL;

			tree = tree->left;
		} else if (ptr->depth == tree->item->depth + 1) {
			tree->right = malloc(sizeof(object_outline_tree_t));

			if (tree->right == NULL)
				return 1;

			tree->right->id = ids[pos++];
			tree->right->item = ptr;
			tree->right->up = tree;
			tree->right->left = NULL;
			tree->right->right = NULL;

			tree = tree->right;
		} else {
			tree = tree->up;
			continue;
		}
		ptr = ptr->next;
	}

	return 0;
}
