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

#ifndef ALL_APPS_H_
#define ALL_APPS_H_

#include <Elementary.h>
#include "app_grid.h"
#include "data_model.h"

/**
 * @brief Creates all apps panel
 * @param parent elm_layout widget with proper swallow part
 * @return pointer to Evas_Object
 */
Evas_Object *all_apps_create(Evas_Object *parent, void *data);

/**
 * @brief Shows all_apps panel
 */
void all_apps_show(void);

/**
 * @brief [Hides all apps panel
 */
void all_apps_hide(void);

/**
 * @brief [brief description]
 * @details [long description]
 */
void all_apps_del(void);

/**
 * @brief Sets view mode of all apps view
 */
void all_apps_set_view_mode(homescreen_view_t view);

/**
 * @brief Function blocks srolling of the all_apps panel.
 * @details It is used during items reposition.
 *
 * @param is_blocked bool value, true if should be blocked
 */
void all_apps_set_scrolling_blocked_state(bool is_blocked);

/**
 * @brief detaches app icon from grid:
 */
void all_apps_detach_from_grid(Elm_Object_Item *grid_item);

/**
 * @brief attaches app icon to grid number grid_num:
 */
Elm_Object_Item *all_apps_attach_to_grid(Evas_Object *icon, int grid_num);

/**
 * @brief appends new page to the page scroller
 */
bool all_apps_append_page(Tree_node_t *page_item);

/**
 * @brief appends new page to the page scroller
 */
void all_apps_remove_empty_pages(void);

/**
 * @brief it simply updates all pages dynamic index display
 */
void all_apps_update_dynamic_index_count(void);

#endif /* ALL_APPS_H_ */
