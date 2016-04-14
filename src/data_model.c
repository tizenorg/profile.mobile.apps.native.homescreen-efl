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

#include <alloca.h>
#include "homescreen-efl.h"
#include "data_model.h"
#include "db/db.h"
#include "app_icon.h"
#include "app_item.h"
#include "app_grid.h"
#include "folder_panel.h"
#include "all_apps.h"
#include "livebox/livebox_widget.h"
#include "popup.h"

static struct {
	Tree_node_t *data;
	Tree_node_t *all_apps;
	Tree_node_t *home;
} s_info = {
	.data = NULL,
	.all_apps = NULL,
	.home = NULL,
};

typedef struct {
	Tree_node_t *data;
	char *search;
} search_package_t;

static bool __data_model_get_empty_page(Tree_node_t *nothing, Tree_node_t *page, void *data);
static void __data_model_load_item(Tree_node_t *parent, Tree_node_t **item, db_item_t* db_item, Eina_List* apps_db);
static void __data_model_free_subtree(Tree_node_t *root_node);
static bool __data_model_append_item(Tree_node_t *parent, Tree_node_t *node, int container_limit);
static void __data_model_convert_db_list_to_tree(Tree_node_t *parent, int id, Eina_List *list);
static bool __data_model_update_tree_cb(Tree_node_t *parent, Tree_node_t *node, void *data);
static bool __data_model_check_cb(Tree_node_t *parent, Tree_node_t *node, void *data);
static bool __data_model_check_count_cb(Tree_node_t *parent, Tree_node_t *node, void *data);
static bool __data_model_set_set_cb(Tree_node_t *parent, Tree_node_t *node, void *data);
static void __data_model_remove_empty(Tree_node_t *node);
static int __data_model_set_lexical_compare_function(const void *data1, const void *data2);
static Eina_Inarray *__data_model_sort_children(const Tree_node_t *const root,
						unsigned int size_hint,
						Eina_Compare_Cb sort_compare_function);
static void __add_default_widget(widget_info_t *widget);

static Eina_Bool __data_model_reattach_children(const void *container, void *node, void *fdata);
static bool __data_model_search_package_cb(Tree_node_t *parent, Tree_node_t *node, void *data);


HAPI Tree_node_t *data_model_get_data(void)
{
	return s_info.data;
}

HAPI Tree_node_t *data_model_get_all_apps(void)
{
	return s_info.all_apps;
}

HAPI Tree_node_t *data_model_get_home(void)
{
	return s_info.home;
}


HAPI void data_model_load_app_mgr(void)
{
	Eina_List *livebox_list = NULL;
	Eina_List *apps = NULL;
	Eina_List *it = NULL;
	app_mgr_item_t *app_mgr_item = NULL;
	Tree_node_t *page = NULL;
	Tree_node_t *item = NULL;
	widget_info_t *widget = NULL;

	LOGD("LOADING DATA MODEL FROM APP MGR");
	db_create_tables();

	if (!tree_node_new(&s_info.data)) {
		LOGE("app_mgr_load -> tree_node_new failure1");
		return;
	}

	s_info.data->data = app_item_create(APP_ITEM_ROOT, -1, "ROOT", NULL, NULL, "ROOT", NULL, false, 0, 0, 0, 0, NULL);

	if (!tree_node_new(&s_info.all_apps)) {
		LOGE("app_mgr_load -> tree_node_new failure2");
		return;
	}
	s_info.all_apps->data = app_item_create(APP_ITEM_ALL_APPS_ROOT, -1, "ALL APPS", NULL, NULL, "ALL APPS", NULL, false, 0, 0, 0, 0, NULL);
	tree_node_append(s_info.data, s_info.all_apps);

	if (!tree_node_new(&s_info.home)) {
		LOGE("app_mgr_load -> tree_node_new failure2");
		return;
	}
	s_info.home->data = app_item_create(APP_ITEM_HOME, -1, "HOME", NULL, NULL, "HOME", NULL, false, 0, 0, 0, 0, NULL);
	tree_node_append(s_info.data, s_info.home);


	apps = app_mgr_get_apps();

	EINA_LIST_FOREACH(apps, it, app_mgr_item) {
		if (!tree_node_new(&item)) {
			LOGE("app_mgr_load -> tree_node_new failure2");
			continue;
		}
		item->data = app_item_create(
				APP_ITEM_ICON,
				-1,
				app_mgr_item->label,
				app_mgr_item->icon,
				app_mgr_item->exec,
				app_mgr_item->appid,
				NULL,
				app_mgr_item->removable,
				0,
				0,
				0,
				0,
				NULL);

		if (!item->data) {
			LOGE("app_mgr_load -> app_item-create failure");
			tree_node_free(item, NULL, NULL, NULL);
			continue;
		}

		if (!__data_model_append_item(s_info.all_apps, item, APPS_PAGE))
			tree_node_free(item, NULL, NULL, NULL);
	}

	livebox_list = livebox_widget_get_list();
	if (!livebox_list) {
		LOGW(" livebox_list == NULL");

		page = data_model_add_widget_page();
		if (!page) {
			LOGE("page == NULL");
			return;
		}

		return;
	}

	widget = eina_list_nth(livebox_list, 0);
	if (!widget) {
		LOGE("widget == NULL");
		return;
	}

	__add_default_widget(widget);


	widget = eina_list_nth(livebox_list, 1);
	if (!widget) {
		LOGE("widget == NULL");
		return;
	}

	__add_default_widget(widget);

	app_mgr_free_apps(apps);

	/*by default model is being sorted:*/
	data_model_sort(__data_model_set_lexical_compare_function);
}

HAPI Eina_Bool data_model_load_db(void)
{
	Eina_List *apps_db = NULL;
	Eina_List *apps_mgr = NULL;
	Eina_List *it = NULL;
	db_item_t *db_item = NULL;

	LOGD("LOADING DATA MODEL FROM DB");

	db_get_apps(&apps_db);

	if (apps_db == NULL || eina_list_count(apps_db) == 0)
		return EINA_FALSE;

	EINA_LIST_FOREACH(apps_db, it, db_item) {
		switch (db_item->type) {
		case APP_ITEM_ROOT:
			if (!s_info.data) {
				__data_model_load_item(NULL, &s_info.data, db_item, apps_db);
			} else {
				LOGE("DB corrupted s_info.data != NULL");
			}
		break;
		case APP_ITEM_ALL_APPS_ROOT:
			if (!s_info.all_apps) {
				__data_model_load_item(s_info.data, &s_info.all_apps, db_item, apps_db);
			} else {
				LOGE("DB corrupted s_info.all_apps != NULL");
			}
		break;
		case APP_ITEM_HOME:
			if (!s_info.home) {
				__data_model_load_item(s_info.data, &s_info.home, db_item, apps_db);
			} else {
				LOGE("DB corrupted s_info.home != NULL");
			}
		break;
		default:
			LOGW("Unknown type");
		}
	}
	db_free_apps(apps_db);

	apps_mgr = app_mgr_get_apps();
	tree_in_depth_browse(s_info.all_apps, __data_model_update_tree_cb, apps_mgr);
	app_mgr_free_apps(apps_mgr);

	if (!s_info.home) {
		LOGE("s_info.home == NULL");

		if (!tree_node_new(&s_info.home)) {
			LOGE("app_mgr_load -> tree_node_new failure2");
			return EINA_FALSE;
		}
		s_info.home->data = app_item_create(APP_ITEM_HOME, -1, "HOME", NULL, NULL, "HOME", NULL, false, 0, 0, 0, 0, NULL);
		tree_node_append(s_info.data, s_info.home);
	}

	if (s_info.home->count == 0) {
		LOGE("s_info.home == s_info.home->count");
		data_model_add_widget_page();
	}

	home_screen_print_tree();

	/*by default model is being sorted:*/
	data_model_sort(__data_model_set_lexical_compare_function);
	return EINA_TRUE;
}

HAPI void data_model_free(void)
{
	__data_model_free_subtree(s_info.data);
}

HAPI Tree_node_t *data_model_create_folder(app_item_t *new_folder_data)
{
	Tree_node_t *folder = NULL, *new_page = NULL;

	if (new_folder_data) {
		tree_node_new(&folder);
		if (folder) {
			folder->data = new_folder_data;
			__data_model_append_item(s_info.all_apps, folder, APPS_PAGE);
			/*also give it one initial page:*/
			tree_node_new(&new_page);
			if (new_page) {
				new_page->data = app_item_create(APP_ITEM_PAGE, -1, NULL, NULL, NULL, "PAGE", NULL, false, 0, 0, 0, 0, NULL);
				if (new_page->data) {
					tree_node_append(folder, new_page);
				} else {
					tree_node_free(new_page, NULL, NULL, NULL);
				}
			}
		}
	}

	return folder;
}

HAPI void data_model_append_node_to_folder(Tree_node_t *folder, Tree_node_t *node)
{
	if (!node || !folder)
		return;
	tree_node_detatch(node);
	__data_model_append_item(folder, node, FOLDER_APPS_PAGE);
}

HAPI void data_model_free_empty_pages(Tree_node_t *folder_or_some_other_root)
{
	Tree_node_t *empty_page = NULL;

	if (!folder_or_some_other_root)
		folder_or_some_other_root = data_model_get_all_apps();

	do {
		empty_page = NULL;
		data_model_iterate_pages(folder_or_some_other_root, __data_model_get_empty_page, NULL, &empty_page);
		__data_model_free_subtree(empty_page);
	} while (empty_page);
}

HAPI void data_model_delete_folder(Tree_node_t *node)
{
	Tree_node_t *page = NULL, *it = NULL, *it_tmp_next = NULL;

	/*attach child elements to the last page:*/
	TREE_NODE_FOREACH(node, page)
	for (it = page->first; it; it = it_tmp_next) {
		it_tmp_next = it->next;
		tree_node_detatch(it);
		tree_node_append(s_info.all_apps->last, it);
	}

	/*out from the tree:*/
	tree_node_detatch(node);
	/*folder with empty pages subtree can be deleted:*/
	__data_model_free_subtree(node);
}

HAPI Tree_node_t *data_model_install_application(app_mgr_item_t *app_mgr_item)
{
	Tree_node_t *item = NULL;

	tree_node_new(&item);
	item->data = app_item_create(
			APP_ITEM_ICON,
			-1,
			app_mgr_item->label,
			app_mgr_item->icon,
			app_mgr_item->exec,
			app_mgr_item->appid,
			NULL,
			app_mgr_item->removable,
			0,
			0,
			0,
			0,
			NULL);

	if (!item->data) {
		tree_node_free(item, NULL, NULL, NULL);
		return NULL;
	}

	__data_model_append_item(s_info.all_apps, item, APPS_PAGE);

	return item;
}

HAPI void data_model_uninstall_application(Tree_node_t *node)
{
	tree_node_detatch(node);

	/*this will free memory since application node is always empty:*/
	__data_model_remove_empty(node);
}

HAPI void data_model_check_all_apps(Tree_node_t *node, bool checked)
{
	LOGI("");
	tree_in_depth_browse(node, __data_model_check_cb, &checked);
}

HAPI int data_model_get_app_check_state(void)
{
	LOGI("");
	int count = 0;
	tree_in_depth_browse(s_info.all_apps, __data_model_check_count_cb, &count);
	return count;
}

HAPI void data_model_reposition_item(Tree_node_t *node, Tree_node_t *destinations_parent, Tree_node_t *destination,
										reposition_side_t side, int page_limit)
{
	if (!node || !destinations_parent) {
		LOGE("item_reposition INVALID usage: %d %d %d", node, destinations_parent, destination);
		return;
	}

	if (node == destination)
		return;

	tree_node_detatch(node);

	if (destination)
		/*append relatively to sibling:*/
		side == REPOSITION_SIDE_TO_RIGHT ? tree_node_append_relative(node, destination) : tree_node_prepend_relative(node, destination);
	else
		/*append to page:*/
		side == REPOSITION_SIDE_TO_RIGHT ? tree_node_append(destinations_parent, node) : tree_node_prepend(destinations_parent, node);

	/*now, it may be that we have more items than we can on this page...*/
	while (destinations_parent->count > page_limit) {
		tree_node_detatch(node = destinations_parent->last);

		/*next page (create, or prepend):*/
		if (!destinations_parent->next) { /*there seems to be no pages left, create one:*/
			__data_model_append_item(destinations_parent->parent, node, page_limit);
			break;
		} else {/*there is a next page, so lets prepend*/
			destinations_parent = destinations_parent->next;
			tree_node_prepend(destinations_parent, node);
		}
	}
}

HAPI void data_model_iterate(Tree_node_t *node, tree_browse_cb_t func_cb, void *data)
{
	if (!node)
		tree_in_depth_browse(s_info.all_apps, func_cb, data);
	else
		tree_in_depth_browse(node, func_cb, data);
}

HAPI void data_model_iterate_pages(Tree_node_t *node, tree_browse_cb_t page_func, tree_browse_cb_t item_func, void *data)
{
	Tree_node_t *page = NULL, *item = NULL;

	if (!node)
		node = s_info.all_apps;

	/*for each page:*/
	for (page = node->first; page; page = page->next) {
		if (!page->data || page->data->type != APP_ITEM_PAGE) {
			LOGE("Model unstable [or node != root], skipping: %d %d", page->data, page->data ? page->data->type : -1);
			continue;
		}
		if (page_func)
			page_func(NULL, page, data);
		/*for each item in page:*/
		if (item_func) {
			for (item = page->first; item; item = item->next) {
				item_func(page, item, data);
				LOGI("[%s]", item->data->label);
			}
		}
	}
}

HAPI void data_model_update_item(Tree_node_t *node)
{
	if (!node) {
		LOGE("node == NULL");
		return;
	}

	tree_node_update(node);
}

HAPI void data_model_set_view_mode(homescreen_view_t view)
{
	tree_in_depth_browse(s_info.all_apps, __data_model_set_set_cb, &view);
}

HAPI void data_model_detach_from_folder(Tree_node_t *folder_node, Tree_node_t *node)
{
	if (!folder_node || !node) {
		LOGE("[INVALID_PARAMS]");
		return;
	}

	data_model_reposition_item(node, s_info.all_apps->last, NULL,
				REPOSITION_SIDE_TO_RIGHT, APPS_PAGE);
	data_model_sort(__data_model_set_lexical_compare_function);
}


/* -----=========================== widget ===================================*/

HAPI Tree_node_t *data_model_add_widget_page()
{
	Tree_node_t *item = NULL;

	if (!tree_node_new(&item)) {
		LOGE(" app_mgr_load -> tree_node_new failure2");
		return NULL;
	}

	item->data = app_item_create(APP_ITEM_PAGE, -1, "WIDGET_PAGE", NULL, NULL, "WIDGET_PAGE", NULL, false, 0, 0, 0, 0, NULL);
	if (!item->data) {
		tree_node_free(item, NULL, NULL, NULL);
		return NULL;
	}

	tree_node_append(s_info.home, item);

	return item;
}

HAPI Tree_node_t *data_model_add_widget(Tree_node_t *page, const char *widget_id, int col, int row, int col_span, int row_span, const char *content_info)
{
	Tree_node_t *item = NULL;

	if (!tree_node_new(&item)) {
		LOGE(" app_mgr_load -> tree_node_new failure2");
		return NULL;
	}

	item->data = app_item_create(APP_ITEM_WIDGET, -1,
			NULL,
			NULL,
			NULL,
			widget_id,
			NULL,
			false,
			col,
			row,
			col_span,
			row_span,
			content_info);

	if (!item->data) {
		tree_node_free(item, NULL, NULL, NULL);
		return NULL;
	}

	tree_node_append(page, item);

	return item;
}

HAPI void data_model_reposition_widget(Tree_node_t *page_node, Tree_node_t *obj_node)
{
	tree_node_detatch(obj_node);
	tree_node_append(page_node, obj_node);
}

HAPI void data_model_insert_after(Tree_node_t *prev_node, Tree_node_t *item_node)
{
	tree_node_detatch(item_node);

	if (prev_node) {
		tree_node_append_relative(item_node, prev_node);
	} else {
		tree_node_prepend(s_info.home, item_node);
	}
}

HAPI void data_model_del_item(Tree_node_t *node)
{
	tree_node_detatch(node);
	__data_model_free_subtree(node);
}

HAPI void data_model_resize_widget(Tree_node_t *item_node, int col, int row, int col_span, int row_span)
{
	if (!item_node) {
		LOGE("item_node == NULL");
		return;
	}

	if (!item_node->data) {
		LOGE("item_node->data == NULL");
		return;
	}

	app_item_geometry_update(item_node->data, col, row, col_span, row_span);
	tree_node_update(item_node);
}

HAPI void data_model_update_content_info(Tree_node_t *item_node, const char *content_info)
{
	if (!item_node) {
		LOGE("item_node == NULL");
		return;
	}

	if (!item_node->data) {
		LOGE("item_node->data == NULL");
		return;
	}

	app_item_update_content_info(item_node->data, content_info);
	tree_node_update(item_node);
}

/*===================================== widget ===============================*/

HAPI void data_model_sort(Eina_Compare_Cb sort_compare_function)
{
	Eina_Inarray *sorted_children = NULL;
	Tree_node_t *page = NULL, *it = NULL;

	if (!sort_compare_function)
		sort_compare_function = __data_model_set_lexical_compare_function;

	if (!s_info.all_apps)
		return;

	/*First we have to sort top level applications and folders:*/
	sorted_children = __data_model_sort_children(s_info.all_apps, s_info.all_apps->count*APPS_PAGE, sort_compare_function);
	eina_inarray_foreach(sorted_children, __data_model_reattach_children, s_info.all_apps);
	eina_inarray_free(sorted_children);

	/*Second we have to sort all items in all folders:
	find folders:*/
	for (page = s_info.all_apps->first; page; page = page->next) {
		for (it = page->first; it; it = it->next) {
			if (it->data && it->data->type == APP_ITEM_FOLDER) {
				sorted_children = __data_model_sort_children(it, it->count*FOLDER_APPS_PAGE, sort_compare_function);
				eina_inarray_foreach(sorted_children, __data_model_reattach_children, it);
				eina_inarray_free(sorted_children);
			}
		}
	}
}

Tree_node_t *data_model_search_package(const char* pkg_str)
{
	search_package_t search;
	search.data = NULL;
	search.search = (char *)pkg_str;

	tree_in_depth_browse(s_info.all_apps, __data_model_search_package_cb, &search);

	return search.data;
}

static bool __data_model_search_package_cb(Tree_node_t *parent, Tree_node_t *node, void *data)
{
	search_package_t *search_data = (search_package_t *)data;

	if (node->data->appid && strcmp(search_data->search, node->data->appid) == 0)
		search_data->data = node;

	return true;
}

static bool __data_model_get_empty_page(Tree_node_t *nothing, Tree_node_t *page, void *data)
{
	Tree_node_t **empty_page = (Tree_node_t **)data;

	if (page->count == 0) {
		*empty_page = page;
		return false;
	}

	return true;
}

static void __data_model_load_item(Tree_node_t *parent, Tree_node_t **item, db_item_t* db_item, Eina_List* apps_db)
{
	if (!tree_node_new(item)) {
		LOGE("*item == NULL");
		return;
	}

	(*item)->data = app_item_create(db_item->type,
			db_item->id,
			NULL,
			NULL, NULL,
			db_item->appid,
			NULL,
			false,
			db_item->x,
			db_item->y,
			db_item->w,
			db_item->h,
			db_item->content_info);

	if (parent) {
		if (!tree_node_append(parent, *item)) {
			LOGE("failed to append home to data");
			__data_model_free_subtree(*item);
			return;
		}

		__data_model_convert_db_list_to_tree(*item, db_item->first_id, apps_db);
	}
}


static bool __data_model_append_item(Tree_node_t *parent, Tree_node_t *node, int container_limit)
{
	Tree_node_t *page = NULL;

	if (!parent || !node)
		return false;

	page = parent->last;
	if (!page || page->count >= container_limit) {
		if (!tree_node_new(&page))
			return false;
		page->data = app_item_create(APP_ITEM_PAGE, -1, "PAGE", NULL, NULL, "PAGE", NULL, false, 0, 0, 0, 0, NULL);
		if (!page->data) {
			tree_node_free(page, NULL, NULL, NULL);
			return NULL;
		}

		if (!tree_node_append(parent, page)) {
			data_model_del_item(page);
			return false;
		}
	}

	return tree_node_append(page, node);
}

static void __data_model_convert_db_list_to_tree(Tree_node_t *parent, int id, Eina_List *list)
{
	Eina_List *it = NULL;
	db_item_t *db_item = NULL;
	Tree_node_t *node = NULL;

	if (id == -1)
		return;

	EINA_LIST_FOREACH(list, it, db_item) {
		if (db_item->id == id)
			break;
	}

	if (!db_item)
		return;

	tree_node_new(&node);
	node->data = app_item_create(db_item->type,
			db_item->id,
			"",
			"",
			"",
			db_item->appid,
			NULL,
			db_item->type == APP_ITEM_FOLDER,
			db_item->x,
			db_item->y,
			db_item->w,
			db_item->h,
			db_item->content_info);

	if (!node->data) {
		tree_node_free(node, NULL, NULL, NULL);
		return;
	}

	tree_node_append(parent, node);

	__data_model_convert_db_list_to_tree(parent, db_item->next_id, list);
	__data_model_convert_db_list_to_tree(node, db_item->first_id, list);
}

static bool __data_model_update_tree_cb(Tree_node_t *parent, Tree_node_t *node, void *data)
{
	if (node->data->type == APP_ITEM_ICON) {
		Eina_List *it;
		app_mgr_item_t *app_mgr;
		EINA_LIST_FOREACH((Eina_List *)data, it, app_mgr) {
			if (!strncmp(app_mgr->appid, node->data->appid, strlen(node->data->appid)))
				break;
		}

		if (app_mgr) {
			node->data->label = strdup(app_mgr->label);
			node->data->icon = strdup(app_mgr->icon);
			node->data->exec = strdup(app_mgr->exec);

			node->data->removable = app_mgr->removable;
		}
	} else if (node->data->type == APP_ITEM_FOLDER) {
		node->data->label = strdup(node->data->appid);
	}
	return true;
}

static bool __data_model_check_cb(Tree_node_t *parent, Tree_node_t *node, void *data)
{
	if (node->data->type == APP_ITEM_ICON)
		node->data->is_checked = *(bool *)data;

	return true;
}

static bool __data_model_check_count_cb(Tree_node_t *parent, Tree_node_t *node, void *data)
{
	if (node && node->data && node->data->is_checked) {
		LOGI("%s", node->data->label);
		++*(int *)data;
	}
	return true;
}

static bool __data_model_set_set_cb(Tree_node_t *parent, Tree_node_t *node, void *data)
{
	if (node->data->type == APP_ITEM_ICON || node->data->type == APP_ITEM_FOLDER) {
		if (node->parent->parent && node->parent->parent->data->type == APP_ITEM_FOLDER) {
			app_icon_set_view_mode(node->data->layout, *(homescreen_view_t *)data, true);
		} else {
			app_icon_set_view_mode(node->data->layout, *(homescreen_view_t *)data, false);
		}
	} else if (node->data->type == APP_ITEM_PAGE && node->parent->data->type == APP_ITEM_ALL_APPS_ROOT) {
		app_grid_set_view_mode(node->data->layout, *(homescreen_view_t *)data);
	}
	return true;
}

/*
static void _data_model_full_page_reorder(Tree_node_t *node)
{
	if (node->parent->count > APPS_PAGE)
	{
		Tree_node_t *parent = node->parent;
		Tree_node_t *last = parent->last;
		tree_node_detatch(last);

		last->data->grid_item = app_grid_insert_item_relative(parent->next->data->layout, last->data->layout, parent->next->first->data->grid_item);

		//TODO check if next exist
		tree_node_prepend(parent->next, last);
		_data_model_full_page_reorder(last);
	}
}
*/

static void __data_model_remove_empty(Tree_node_t *node)
{
	if (!node || !node->data)
		return;

	if ((node->data->type == APP_ITEM_PAGE || node->data->type == APP_ITEM_FOLDER) && node->count == 0) {
		app_item_free(node->data);
		/*since it's empty, we pass NULLs:*/
		tree_node_free(node, NULL, NULL, NULL);
	}
}

static Eina_Bool __data_model_reattach_children(const void *container, void *node, void *fdata)
{
	unsigned int page_size_limit = FOLDER_APPS_PAGE;
	bool place_found = false;
	Tree_node_t *parent = (Tree_node_t *)fdata, *child = *((Tree_node_t **)node),  *page = NULL;

	if (parent->data->type == APP_ITEM_ALL_APPS_ROOT)
		page_size_limit = APPS_PAGE;

	/*now find first free page:*/
	for (page = parent->first; page; page = page->next) {
		if (page->count < page_size_limit) {
			tree_node_append(page, child);
			place_found = true;
			LOGI("reattach_children %s found place in %s", child->data->label, parent->data->label);
			break;
		}
	}

	/*sanity check:*/
	if (!place_found) {
		LOGE("reattach_children %s could not find place", child->data->label);
		__data_model_append_item(parent, child, page_size_limit);
	}
	return EINA_TRUE;
}

static Eina_Inarray *__data_model_sort_children(const Tree_node_t *const root, unsigned int size_hint, Eina_Compare_Cb sort_compare_function)
{
	Eina_Inarray *array = NULL;
	Tree_node_t *page = NULL, *it = NULL, *it_tmp = NULL;

	if (!root || !root->data ||
		!(root->data->type == APP_ITEM_FOLDER ||
			root->data->type == APP_ITEM_ALL_APPS_ROOT)
	)
		return NULL;
	/*now we know this is root or folder for sure, so it contains pages:*/

	array = eina_inarray_new(sizeof(Tree_node_t *), size_hint);

	if (!array) {
		LOGE("sort_children: allocation error");
		return NULL;
	}

	/*for each "page":*/
	for (page = root->first; page; page = page->next) {
		/*sanity check:*/
		if (!page->data || page->data->type != APP_ITEM_PAGE) {
			LOGE("sort_children problem: data: %d type: %d", page->data, page->data ? page->data->type : -1);
			return array;
		}

		/*for each node in "page":*/
		for (it = page->first; it; it = it_tmp) {
			it_tmp = it->next;
			tree_node_detatch(it);

			if (eina_inarray_insert_sorted(array, &it, sort_compare_function) < 0) {
				LOGE("sort_children: item insertion error");
				return array;
			}
		}
	}

	return array;
}

static void __data_model_free_subtree(Tree_node_t *root_node)
{
	Tree_node_t *first_child = NULL, *last_child = NULL, *child_tmp = NULL;
	int children_count = 0;

	if (!root_node) {
		LOGE("root_node == NULL");
		return;
	}

	tree_node_free(root_node, &first_child, &last_child, &children_count);

	if ((!first_child || !last_child) && children_count > 0) {
		LOGE("Node has %d children but pointers are first: %d, last: %d", children_count, first_child, last_child);
		return;
	}

	while (children_count-- > 0) {
		child_tmp = first_child->next;
		__data_model_free_subtree(first_child);
		first_child = child_tmp;
	}
}

static int __data_model_set_lexical_compare_function(const void *data1, const void *data2)
{
	char *l_upper = NULL, *r_upper = NULL;
	Tree_node_t *l = *((Tree_node_t **)data1), *r = *((Tree_node_t **)data2);
	int res = 0;

	if (!l || !l->data || !l->data->label)
		return 1;

	if (!r || !r->data || !r->data->label)
		return -1;

	l_upper = (char *)alloca(NAME_MAX*sizeof(char));
	r_upper = (char *)alloca(NAME_MAX*sizeof(char));

	strncpy(l_upper, l->data->label, NAME_MAX - sizeof('\0'));
	strncpy(r_upper, r->data->label, NAME_MAX - sizeof('\0'));

	eina_str_toupper(&l_upper);
	eina_str_toupper(&r_upper);

	return !(res = strcoll(l_upper, r_upper)) ? 1 : res;
}

static void __add_default_widget(widget_info_t *widget)
{
	Tree_node_t *item = NULL;

	Tree_node_t *page = data_model_add_widget_page();
	if (!page) {
		LOGE("item == NULL");
		return;
	}

	item = data_model_add_widget(page, widget->widget_id, 0, 0, widget->width,
			widget->height, NULL);
	if (!item) {
		LOGE("item == NULL");
		return;
	}

	LOGD("Widget: %s with size (%d, %d) added", widget->widget_id,
			widget->width, widget->height);
}
