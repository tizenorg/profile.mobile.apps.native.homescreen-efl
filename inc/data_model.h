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

#ifndef DATA_MODEL_H_
#define DATA_MODEL_H_

#include <stdbool.h>
#include "app_mgr.h"
#include "tree.h"

typedef enum {
	REPOSITION_SIDE_TO_LEFT,
	REPOSITION_SIDE_TO_RIGHT
} reposition_side_t;

/**
 * @brief gets data model of the application
 */
extern Tree_node_t *data_model_get_data(void);

/**
 * @brief gets data model for the all apps view.
 */
extern Tree_node_t *data_model_get_all_apps(void);

/**
 * @brief gets data model for the home view.
 */
extern Tree_node_t *data_model_get_home(void);

/*
 * @brief Get applications from app manager
 */
extern void data_model_load_app_mgr(void);

/*
 * @brief Get applications from database
 */
extern Eina_Bool data_model_load_db(void);

/*
 * @brief free entire memory allocated for model structure
 * and nodes data
 */
extern void data_model_free(void);

/**
 * @brief Creates folder in the data model/
 */
extern Tree_node_t *data_model_create_folder(app_item_t *node);

/**
 * @brief Appends an item into the folder.
 */
extern void data_model_append_node_to_folder(Tree_node_t *folder,
	Tree_node_t *node);

/**
 * @brief Removes empty pages from the all apps view in the data model.
 */
extern void data_model_free_empty_pages(Tree_node_t *folder_or_some_other_root);

/*
 * @brief Delete folder and move apps to all apps
 */
extern void data_model_delete_folder(Tree_node_t *node);

/**
 * @brief Append an newly installed application to the data model.
 */
extern Tree_node_t *data_model_install_application(
	app_mgr_item_t *app_mgr_item);

/**
 * @brief Uninstall an application
 */
extern void data_model_uninstall_application(Tree_node_t *node);

/*
 * @brief set checked state to all icons
 */
extern void data_model_check_all_apps(Tree_node_t *node, bool checked);

/**
 * @brief returns count of checked applications in the all apps edit mode view.
 */
extern int data_model_get_app_check_state(void);

/**
 * @brief change position of an application
 */
extern void data_model_reposition_item(Tree_node_t *node,
	Tree_node_t *destinations_parent, Tree_node_t *destination,
	reposition_side_t side, int page_limit);

/**
 * @brief set the view type of the data models items
 */
extern void data_model_set_view_mode(homescreen_view_t view);

/**
 * @brief iterates over the whole tree and applies func_cb over each node
 * @param node - tree node, pass NULL if You want
 * to start from the root of the model tree
 * @param func_cb - function to call on each node
 * @param data - data - to be passed to func_cb
 * @details [long description]
 */
extern void data_model_iterate(Tree_node_t *node, tree_browse_cb_t func_cb,
	void *data);

/**
 * @brief This one takes pages abstraction into account.
 * Iterates through pages, and it's items.
 * @param node - tree node, pass NULL if You want
 * to start from the root of the model tree
 * @param item_func - function to call on each item
 * @param page_func - function to call on each page
 * @param data - data - to be passed to func_cb
 */
extern void data_model_iterate_pages(Tree_node_t *node,
	tree_browse_cb_t page_func, tree_browse_cb_t item_func, void *data);

/**
 * @brief sorts in place using sort_compare_function
 */
extern void data_model_sort(Eina_Compare_Cb sort_compare_function);

/**
 * @brief Creates new page
 * @details Creates new page node, adds it to the tree structure and
 * returns the created page
 * @return new page node
 */
extern Tree_node_t *data_model_add_widget_page(void);

/**
 * @brief Creates new widget and adds it to @p page
 * @param page Page the new widget will be added to
 * @param widget_id Created widget
 * @param col column to put widget in
 * @param row row to put widget in
 * @param col_span column span of widget
 * @param row_span row span of widget
 * @return New widget node
 */
extern Tree_node_t *data_model_add_widget(Tree_node_t *page,
	const char *widget_id, int col, int row, int col_span, int row_span,
	const char *content_info);

/**
 * @brief [brief description]
 * @details [long description]
 */
extern void data_model_reposition_widget(Tree_node_t *page_node,
	Tree_node_t *obj_node);

/**
 * @brief [brief description]
 * @details [long description]
 */
extern void data_model_resize_widget(Tree_node_t *item_node, int col, int row,
	int col_span, int row_span);

extern void data_model_update_content_info(Tree_node_t *item_node, const char *content_info);

/**
 * @brief [brief description]
 * @details [long description]
 */
extern void data_model_insert_after(Tree_node_t *prev_node,
	Tree_node_t *item_node);

/**
 * @brief [brief description]
 * @details [long description]
 */
extern void data_model_del_item(Tree_node_t *node);

/**
 * @brief [brief description]
 * @details [long description]
 */
extern void data_model_update_item(Tree_node_t *node);

/**
 * @brief Unpin a repositioned node from the folder.
 *
 * @param folder_node Folder node pointer.
 * @param node repositioned node pointer.
 */
extern void data_model_detach_from_folder(Tree_node_t *folder_node,
	Tree_node_t *node);

/**
 * @brief Search node with package name.
 *
 * @param package name
 * @return node or NULL
 */
Tree_node_t *data_model_search_package(const char* pkg_str);


#endif /* DATA_MODEL_H_ */
