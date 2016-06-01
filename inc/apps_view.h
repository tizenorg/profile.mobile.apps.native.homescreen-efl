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

#ifndef __APPS_VIEW_H__
#define __APPS_VIEW_H__

#include <Elementary.h>

#include "view.h"
#include "apps_data.h"

Evas_Object *apps_view_create(Evas_Object *parent);
void apps_view_init(void);
void apps_view_app_terminate(void);
void apps_view_show(void);
void apps_view_hide(void);
void apps_view_show_anim(double pos);
void apps_view_hide_anim(double pos);
void apps_view_reroder(void);
void apps_view_folder_reroder(void);
void apps_view_update_folder_icon(app_data_t *item);

Evas_Object* apps_view_create_icon(app_data_t *item);
void apps_view_delete_icon(app_data_t *item);
void apps_view_unset_all(void);
bool apps_view_icon_set(app_data_t *item);
void apps_view_icon_unset(app_data_t *item);
void apps_view_icon_add(app_data_t *item);

void apps_view_hw_menu_key(void);
bool apps_view_hw_home_key(void);
bool apps_view_hw_back_key(void);
view_state_t apps_view_get_state(void);
void apps_view_set_state(view_state_t state);

#endif /* __APPS_VIEW_H__ */
