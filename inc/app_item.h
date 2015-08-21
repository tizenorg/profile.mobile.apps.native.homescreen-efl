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

#ifndef APP_ITEM_H_
#define APP_ITEM_H_

#include <stdbool.h>
#include <Elementary.h>

typedef enum {
	APP_ITEM_ROOT = 0,
	APP_ITEM_HOME,
	APP_ITEM_ALL_APPS_ROOT,
	APP_ITEM_PAGE,
	APP_ITEM_ICON,
	APP_ITEM_FOLDER,
	APP_ITEM_WIDGET
} app_item_type_t;

typedef struct {
	app_item_type_t type;
	int unique_id; /*from db table; used for composing tree structure when
	reading flattened db representation*/
	char *label; /*this is the name of the application
	viewed in homescreen layout*/
	const char *icon;
	const char *exec; /*app exec path - used for notifications*/
	const char *appid;
	unsigned int badge_count; /*notification count viewed in icon badge*/
	void *data; /*pointer is used by folder to store the folder layout.*/
	bool removable;
	bool is_checked;
	Evas_Object *layout;
	Elm_Object_Item *grid_item;
	int col;
	int row;
	int col_span;
	int row_span;

	char *content_info;
} app_item_t;

/**
 * @brief creates new app_item_t object
 * @param type object type
 * @param unique_id object id
 * @param label label text
 * @param icon path to icon
 * @param exec app to exec on click
 * @param appid application id
 * @param data extra data
 * @param is_removeable na the app be removed
 * @param col object column (used only for widgets)
 * @param row object row (used only for widgets)
 * @param col_span object column span (used only for widgets)
 * @param row_span object row span (used only for widgets)
 * @return
 */
app_item_t *app_item_create(const app_item_type_t type, const int unique_id,
	const char *label, const char *icon, const char *exec,
	const char *appid, void *data, bool is_removeable,
	int col, int row, int col_span, int row_span,
	const char *content_info);

/**
 * @brief Function frees all data connected with the item
 * @param item Pointer to application item.
 */
void app_item_free(app_item_t *item);

/**
 * @brief updates app_item_geometry
 * @param app_item app_item to be updated
 * @param x new pos
 * @param y new pos
 * @param w new width
 * @param h new height
 */
void app_item_geometry_update(app_item_t *app_item, int x, int y, int w, int h);

void app_item_update_content_info(app_item_t *app_item, const char *content_info);

#endif /* APP_ITEM_H_ */
