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

#ifndef __MENU_H__
#define __MENU_H__

#include <app.h>
#include <Elementary.h>

typedef enum {
    MENU_DEFAULT = -1,
    MENU_CLUSTER_EDIT = 0,
    MENU_CLUSTER_ADD_WIDGET,
    MENU_CLUSTER_CHANGE_WALLPAPER,
    MENU_CLUSTER_ALL_PAGES,
    MENU_APPS_EDIT,
    MENU_APPS_CREATE_FOLDER,
    MENU_MAX
} menu_item_t;

void menu_show(Eina_Hash* hash_table);
void menu_hide(void);
bool menu_is_show(void);
void menu_change_state_on_hw_menu_key(Eina_Hash* hash_table);

#endif /* __MENU_H__ */
