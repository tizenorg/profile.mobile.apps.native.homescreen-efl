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

#ifndef __FOLDER_PANEL_H__
#define __FOLDER_PANEL_H__

#include <stdbool.h>
#include "homescreen-efl.h"
#include "app_item.h"
#include "dynamic_index.h"
#include "tree.h"

typedef struct {
	Eina_List *miniatures;
	Evas_Object *folder_scroller;
} folder_data_t;

typedef enum {
	FOLDER_PANEL_CHECKED_NONE = 0,
	FOLDER_PANEL_CHECKED_ANY,
	FOLDER_PANEL_CHECKED_ALL,
} folder_checked_t;

/**
 * @brief Initializes folder module
 */
void folder_panel_init_module(void);

/**
 * @brief Frees all data initialized before.
 */
void folder_panel_deinit_module(void);

/**
 * @brief Creates folder panel.
 * @param data folder parent in data model
 * @return app_item_t pointer
 */
extern app_item_t *folder_panel_create(Tree_node_t *data);

/**
 * @brief Ignores folder click action if ignore is true.
*/
extern void folder_panel_set_click_ignore_state(bool ignore);

/**
 * @brief Enables or disables folder entry editing.
 * @param state the state to put folder entry in
 */
extern void folder_panel_enable_entry(bool state);

/**
 * @brief Attaches icon object to grid of currently opened folder.
 * @param icon - app_item_t->layout
 */
extern Elm_Object_Item *folder_panel_attach_to_grid(Evas_Object *icon,
	folder_data_t *folder_data);

/**
 * @brief Shows folder panel layout on screen
 * @param folder pointer to app_item_t object
 * @param keypad_show imf keyboard visibility flatg
 *
 * @return true if no errors.
 */
extern bool folder_panel_open_folder(app_item_t *folder, bool keypad_show);

/**
 * @brief Closes folder panel view.
 */
extern void folder_panel_close_folder(void);

/**
 * @brief Cancels appending new items into the folder by add icon press.
 */
extern void folder_panel_cancel_add_icon(void);

/**
 * @brief Returns folder visibility state
 * @return true if folder is visible otherwise returns false
 */
extern bool folder_panel_is_folder_visible(void);

/**
 * @brief Shows checkboxes on the folder icon.
 */
extern folder_checked_t folder_panel_set_content_checked(Tree_node_t *node);

/**
 * @brief Function returns pointer to data node of the opened folder
 * @return Tree_Node_t pointer.
 */
extern Tree_node_t *folder_panel_get_opened_folder_node(void);

/**
 * @brief Function returns pointer to app_item of opened folder
 */
extern const app_item_t *folder_panel_get_opened_folder_item(void);

/**
 * @brief Return number of items inserted into the folder.
 */
extern int folder_panel_get_items_count(Tree_node_t *folder);

/**
 * @brief Removes empty folder after reposition.
 */
extern void folder_panel_remove_empty_folder(Tree_node_t *folder);

/**
 * @brief Return folder popup geometry
 *
 * @param x position
 * @param y position
 * @param w width
 * @param h height
 */
extern void folder_panel_get_folder_geometry(int *x, int *y, int *w, int *h);

/**
 * @brief Function close entry input panel connected with folder name entry
 * widget
 */
extern void folder_panel_hide_input_panel(void);

#endif /* FOLDER_PANEL_H_ */
