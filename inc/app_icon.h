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

#ifndef APP_ICON_H_
#define APP_ICON_H_

#include <Elementary.h>
#include <stdbool.h>

#include "homescreen-efl.h"
#include "app_item.h"
#include "tree.h"

typedef struct {
	char *app_name;
	char *app_appid;
} app_icon_popup_info_t;

typedef enum {
	APP_ICON_TYPE_APP = 0,
	APP_ICON_TYPE_FOLDER,
	APP_ICON_TYPE_MINIATURE,
} app_icon_type_t;

typedef enum {
	FOLDER_ICON_FRAME_POSSIBLE = 0,
	FOLDER_ICON_FRAME_IMPOSSIBLE,
	FOLDER_ICON_FRAME_NONE,
} folder_icon_frame_type_t;

/**
 * @brief Sets ignore click flag.
 *
 * @param ignore if it is true click event on icon will not be processed.
 */
extern void app_icon_set_click_ignore(bool ignore);

/**
 * @brief Remove icon object
 * @details Function removes elm_layout widget responsible for application
 * shortcut view. It deletes also elm_image widget (application shortuc image)
 * with is a child of icon_layout.
 *
 * @param icon_layout Evas_Object pointer to item witch will be deleted.
 */
extern void app_icon_destroy(Evas_Object *icon_layout);

/**
 * @brief Sets badge number and shows it on application icon.
 *
 * @param app_icon_layout application icon layout.
 * @param int number with will be displayed in badge layout.
 */
extern void app_icon_show_badge(Evas_Object *app_icon_layout,
	const unsigned int number);

/**
 * @brief Hides badge icon on application.
 * @details [long description]
 * @param application icon layout pointer.
 */
extern void app_icon_hide_badge(Evas_Object *app_icon_layout);

/**
 * @brief Sets icon label visible
 * @param app_icon_layout pointer to icon layout.
 */
extern void app_icon_show_label(Evas_Object *app_icon_layout);

/**
 * @brief Sets icon label invisible.
 * @param app_icon_layout pointer to icon layout.
 */
extern void app_icon_hide_label(Evas_Object *app_icon_layout);

/**
 * @brief Function chage state of icon regarind to passed view type
 *
 * @param icon Pointer to icon object which should be changed.
 * @param view view type
 * @param in_folder value indicating whether given item is in folder
 *
 * @return true if success
 */
bool app_icon_set_view_mode(Evas_Object *icon, homescreen_view_t view,
	bool in_folder);

/**
 * @brief Creates icon using app_item_t structure
 *
 * @param parent Parent widget pointer
 * @param item pointer to app_item_t structure
 *
 * @return Pointer to created icon or NULL.
 */
Evas_Object *app_icon_create(Tree_node_t *tree_data, app_icon_type_t type);

/**
 * @brief Updates miniatures in folder icon
 *
 * @param folder_icon_layout folder icon layout to update
 * @param folder_miniatures list of miniatures shown in folder icon
 * @param folder_app_tree applications on the first page of folder
 */
void app_icon_update_folder_icon(Tree_node_t *folder_node);

/**
 * @brief Highlights folder icon with specified frame type
 *
 * @param folder_icon_layout folder icon layout to highlight
 * @param frame_type type of frame to use on folder icon
 */
void app_icon_highlight_folder_icon(Evas_Object *folder_icon_layout,
	folder_icon_frame_type_t frame_type);

/**
 * @brief Sets icon's label
 *
 * @param icon_layout icon layout in which label is set
 * @param label label to set
 */
void app_icon_set_label(Evas_Object *icon_layout, const char* label);

/**
 * @brief Sets icon layout's color
 *
 * @param icon_layout icon layout to set color
 * @param r red component of given color
 * @param g green component of given color
 * @param b blue component of given color
 * @param a alpha component of given color
 */
void app_icon_set_color(Evas_Object *icon_layout, int r, int g, int b, int a);

/**
 * @brief Sets icon's label color to black
 *
 * @param icon_layout icon layout in which label's color is changed
 */
void app_icon_set_label_black(Evas_Object *icon_layout);

/**
 * @brief Removes all data connected with icon_layout
 * @param icon_layout Pointer to app_icon layout.
 */
void app_icon_remove(Evas_Object *icon_layout);

#endif /* APP_ICON_H_ */
