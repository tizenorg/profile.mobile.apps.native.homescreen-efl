/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _TREE_H_
#define _TREE_H_

#include <stdbool.h>
#include "app_item.h"

#define TREE_NODE_FOREACH(parent, node) \
for (node = parent->first; node != NULL; node = node->next)

/*
 * data		- data bind to node
 * parent	- parent node
 * next		- next sibling
 * prev		- previous sibling
 * first	- first child
 * last		- last child
 */
typedef struct _tree_node {
	app_item_t *data;
	struct _tree_node *parent;

	unsigned int count;
	struct _tree_node *next;
	struct _tree_node *prev;

	struct _tree_node *first;
	struct _tree_node *last;
} Tree_node_t;


/*
 * @brief signature of callback function for tree_in_depth_browse() function
 */
typedef bool(*tree_browse_cb_t)(Tree_node_t*, Tree_node_t*, void*);


/*
 * @brief Creates new tree node
 * @details All node fields are initialized with \0
 */
bool tree_node_new(Tree_node_t **node);

/*
 * @brief Detatch node
 */
bool tree_node_detatch(Tree_node_t *node);

/*
 * @brief Append child node to parent node
 */
bool tree_node_append(Tree_node_t *parent, Tree_node_t *node);

/*
 * @brief Append node to tree after relative node
 */
bool tree_node_append_relative(Tree_node_t *node, Tree_node_t *relative);

/*
 * @brief Prepend child node to parent node
 */
bool tree_node_prepend(Tree_node_t *parent, Tree_node_t *node);

/*
 * @brief Prepend node to tree before relative node
 */
bool tree_node_prepend_relative(Tree_node_t *node, Tree_node_t *relative);

/*
 * @brief Browse the tree in-depth
 */
bool tree_in_depth_browse(Tree_node_t *node, tree_browse_cb_t func_cb,
	void *data);

/*
 * Updates a node with new values
 */
void tree_node_update(Tree_node_t *node);

/*
 * @brief frees memory of a single node in a safe manner. Returns count and
 * pointers to children of the node.
 * Updates nodes parent and siblings. Nodes children become
 * detached from the tree.
 * @param node[IN] - node to free
 * @param first_child - nodes first child
 * @param last_child - nodes last child
 * @param child_count - nodes children count
 */
void tree_node_free(Tree_node_t *node, Tree_node_t **first_child,
	Tree_node_t **last_child, int *child_count);

#endif
