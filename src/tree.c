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

#include <stdlib.h>
#include <string.h>
#include <dlog.h>

#include "tree.h"
#include "db/db.h"

const db_item_t __tree_node_convert(Tree_node_t *node);

HAPI bool tree_node_new(Tree_node_t **node)
{
	*node = (Tree_node_t *)calloc(1, sizeof(Tree_node_t));

	return (*node) != NULL;
}

/*
 * detatch node
 */
HAPI bool tree_node_detatch(Tree_node_t *node)
{
	if (!node)
		return false;

	if (node->prev) {
		node->prev->next = node->next;
		db_update_apps(__tree_node_convert(node->prev), true);
	} else {
		node->parent->first = node->next;
		db_update_apps(__tree_node_convert(node->parent), true);
	}

	if (node->next) {
		node->next->prev = node->prev;
	} else {
		node->parent->last = node->prev;
	}

	if (node->parent)
		node->parent->count--;

	node->next = NULL;
	node->prev = NULL;
	node->parent = NULL;

	db_update_apps(__tree_node_convert(node), true);

	return true;
}

HAPI bool tree_node_append(Tree_node_t *parent, Tree_node_t *node)
{
	if (!node || !parent)
		return false;

	if (!parent->last) {
		parent->first = node;
		node->prev = NULL;
		db_update_apps(__tree_node_convert(parent), true);
	} else {
		parent->last->next = node;
		node->prev = parent->last;
		db_update_apps(__tree_node_convert(parent->last), true);
	}

	parent->last = node;
	node->parent = parent;
	node->parent->count++;
	node->next = NULL;

	db_update_apps(__tree_node_convert(node), true);

	return true;
}

HAPI bool tree_node_append_relative(Tree_node_t *node, Tree_node_t *relative)
{
	if (!relative->next)
		return tree_node_append(relative->parent, node);

	node->parent = relative->parent;
	node->parent->count++;
	relative->next->prev = node;
	node->next = relative->next;
	relative->next = node;
	node->prev = relative;

	db_update_apps(__tree_node_convert(node), true);
	db_update_apps(__tree_node_convert(relative), true);

	return true;
}

HAPI bool tree_node_prepend(Tree_node_t *parent, Tree_node_t *node)
{
	if (!node || !parent)
		return false;

	if (!parent->first) {
		/*No elements in parent*/
		parent->last = node;
		node->next = NULL;
	} else {
		parent->first->prev = node;
		node->next = parent->first;
		db_update_apps(__tree_node_convert(parent), true);
	}

	parent->first = node;
	node->parent = parent;
	node->parent->count++;
	node->prev = NULL;

	db_update_apps(__tree_node_convert(node), true);

	return true;
}

HAPI bool tree_node_prepend_relative(Tree_node_t *node, Tree_node_t *relative)
{
	if (!relative->prev)
		return tree_node_prepend(relative->parent, node);

	node->parent = relative->parent;
	node->parent->count++;
	relative->prev->next = node;
	node->prev = relative->prev;
	relative->prev = node;
	node->next = relative;

	db_update_apps(__tree_node_convert(node), true);
	db_update_apps(__tree_node_convert(node->prev), true);

	return true;
}

HAPI bool tree_in_depth_browse(Tree_node_t *node, tree_browse_cb_t func_cb, void *data)
{
	if (!node)
		return false;

	if (func_cb) {
		if (!func_cb(NULL, node, data))
			return false;
	}

	Tree_node_t *it;
	TREE_NODE_FOREACH(node, it) {
		if (!tree_in_depth_browse(it, func_cb, data))
			return false;
	}

	return true;
}

void tree_node_update(Tree_node_t *node)
{
	if (!node) {
		LOGE("node == NULL");
		return;
	}

	db_update_apps(__tree_node_convert(node), true);
}

HAPI void tree_node_free(Tree_node_t *node, Tree_node_t **first_child, Tree_node_t **last_child, int *child_count)
{
	Tree_node_t *it = NULL;

	if (!node)
		return;

	db_update_apps(__tree_node_convert(node), false);

	if (first_child)
		*first_child = node->first;

	if (last_child)
		*last_child = node->last;

	if (child_count)
		*child_count = node->count;

	if (node->parent) {
		node->parent->count--;

		if (node->parent->first == node)
			node->parent->first = node->next;

		if (node->parent->last == node)
			node->parent->last = node->prev;
	}

	if (node->prev)
		node->prev->next = node->next;

	if (node->next)
		node->next->prev = node->prev;

	for (it = node->first; it; it = it->next)
		it->parent = NULL;

	app_item_free(node->data);
	free(node);
}

HAPI const db_item_t __tree_node_convert(Tree_node_t *node)
{
	db_item_t item = {
		id : node->data->unique_id,
		type : node->data->type,
		appid : node->data->appid ? (char *)node->data->appid : (char *)"",
		first_id : node->first ? node->first->data->unique_id : -1,
		next_id : node->next ? node->next->data->unique_id : -1,
		x : node->data->col,
		y : node->data->row,
		w : node->data->col_span,
		h : node->data->row_span,
		content_info : node->data->content_info,
	};
	return item;
}
