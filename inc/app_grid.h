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

#ifndef APP_GRID_H_
#define APP_GRID_H_

#include <Elementary.h>
#include "homescreen-efl.h"
#include "tree.h"

typedef enum {
	APP_GRID_TYPE_ALL_APPS = 0,
	APP_GRID_TYPE_FOLDER
} app_grid_type_t;
/**
 * @brief Function creates app_grid wgt
 * @details This is a kind of wrapper for a gengrid widget but handle drag and
 * drop events between different gengrids created by this function. Reposition
 * mechanism found active gengrid behind mouse pointer. Reposition is gengeric.
 * If you want to modify it you should set some callbacks in function
 * app_grid_custom_dnd_handlers_set.
 *
 * @param parent Pointer to eveas_object. Smart parent widget for app grid.
 * Basically you can add it into win.
 * @param data Pointer to Eina_List object with content will be inserted in
 * applicaiton list.
 * @param item_width int describes width of an item in grid.
 * @param item_height ind describes height of an item in grid
 * @return pointer to created app_grid
 */
extern Evas_Object *app_grid_create(Evas_Object *parent, Tree_node_t *data,
	Evas_Coord gengrid_width, Evas_Coord gengrid_height,
	app_grid_type_t type);

/**
 * @brief Append an imte inot app_grid
 * @param app_grid Pointer to elm_gengrid widget
 * @param icon_to_append pointer to appended item
 *
 * @return Elm_Object_Item if no errors, otherwise NULL
 */
extern Elm_Object_Item *app_grid_append_item(Evas_Object *app_grid,
	Evas_Object *icon_to_append);

/**
 * @brief Function inserts an item into specific position. If idx is inproper
 * function tries to insert item at the end of the grid.
 *
 * @param app_grid Pointer to app_grid
 * @param item Pointer to item evas_object
 * @param idx index where item should be inserted
 *
 * @return Elm_Object_Item if no errors, otherwise NULL
 */
extern Elm_Object_Item *app_grid_insert_item_relative(Evas_Object *app_grid,
	Evas_Object *item, Elm_Object_Item *rel_it);

/**
 * @brief Function change state of app grids in homescreen
 *
 * @param app_grid Pointer to elm_layout widget
 * @param view type of view mode
 */
extern void app_grid_set_view_mode(Evas_Object *app_grid,
	homescreen_view_t view);

/**
 * @brief Function unpack item from the app_grid
 *
 * @param item pointer to stored data model
 */
extern void app_grid_unpack_item(Elm_Object_Item *item);

/**
 * @brief Function pack object to box
 */
extern Evas_Object *app_grid_item_pack(Evas_Object *p, Evas_Object *c);

/**
 * @brief Function get icon and data from app_grid item
 */
extern Eina_Bool app_grid_get_item_content(Elm_Object_Item *item,
	Evas_Object **icon, Tree_node_t **data);

#endif /* APP_GRID_H_ */
