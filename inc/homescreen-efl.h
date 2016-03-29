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

#ifndef __HOMESCREEN_EFL_H__
#define __HOMESCREEN_EFL_H__

#include <app.h>
#include <Elementary.h>

#include <system_settings.h>
#include <dlog.h>

#include "conf.h"
#include "app_item.h"
#include "tree.h"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.homescreen-efl"
#endif


typedef enum {
	HOMESCREEN_VIEW_HOME = 0,
	HOMESCREEN_VIEW_HOME_EDIT,
	HOMESCREEN_VIEW_HOME_ALL_PAGES,
	HOMESCREEN_VIEW_ALL_APPS,
	HOMESCREEN_VIEW_ALL_APPS_EDIT,
	HOMESCREEN_VIEW_ALL_APPS_CHOOSE,
	HOMESCREEN_VIEW_UNKNOWN,
} homescreen_view_t;

typedef enum {
	INDICE_NEXT_PAGE_ON = 0,
	INDICE_PREV_PAGE_ON,
	INDICE_OFF,
} indice_type_t;

/**
 * @return returns screen width
 */
extern int home_screen_get_root_width(void);

/**
 * @return returns screen height
 */
extern int home_screen_get_root_height(void);

/**
 * @brief Returns homescreen window pointer.
 * @return Evas_Object pointer to elm_win widget or NULL if falsse
 */
extern Evas_Object *home_screen_get_win(void);

/**
 * @brief Returns elm_win main layout.
 * @details Function returns pointer to elm_layout widget with is a parent
 * for other views used in home
 * @return Evas_Object pointer to elm_layout widget
 */
extern Evas_Object *home_screen_get_layout(void);

/**
 * @brief Returns homescreen view type
 * @details Possible types :
 *
 *HOMESCREEN_VIEW_HOME,
 *HOMESCREEN_VIEW_HOME_EDIT,
 *HOMESCREEN_VIEW_HOME_ALL_PAGES,
 *HOMESCREEN_VIEW_ALL_APPS,
 *HOMESCREEN_VIEW_ALL_APPS_EDIT,
 *HOMESCREEN_VIEW_ALL_APPS_CREATE_FOLDER,
 *HOMESCREEN_VIEW_UNKNOWN,
 *
 * @return homescreen_view_t value
 */
extern homescreen_view_t home_screen_get_view_type(void);

/**
 * @brief set homescreen view type
 * @param view homescreen_view_t type
 */
extern void home_screen_set_view_type(homescreen_view_t view);

/**
 * @brief Function sets indice state
 * @details During page switching left or right side of the screen should be
 * highlighted. This function is repsponsible for this logic.
 *
 * @param indice type of performed action
 */
extern void home_screen_set_indice_state(indice_type_t indice);

/**
 * @brief Update the view after application uninstall
 */
extern void home_screen_mvc_item_uninstall_cb(Tree_node_t *item);

/**
 * @brief Update view after folder name change
 */
extern void home_screen_mvc_update_view(void);

/**
 * @brief Update view after move an item into the folder
 */
extern void home_screen_mvc_drag_item_to_folder_cb(Tree_node_t *dragged_item,
	Tree_node_t *folder);

/**
 * @brief home branch test function
 * @details Function for debug purpose only. Displays
 * the data stored in home branch of the data model tree
 */
extern void home_screen_print_tree(void);

/**
 * @brief Function updates the view after drag an item from the folder.
 */
extern void home_screen_mvc_drag_item_from_folder_cb(void);

/**
 * @brief Multi resolution support function for texts in edje.
 *
 * @param layout Elm_Layout Pointer
 */
extern void home_screen_mvc_set_ly_scale(Evas_Object *layout);

/**
 * @brief Close all apps choose view.
 *
 */
extern void home_screen_close_all_apps_choose_view(void);

#endif /* __homescreen-efl_H__ */
