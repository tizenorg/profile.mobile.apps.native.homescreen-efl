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

#ifndef __CLUSTER_VIEW_H__
#define __CLUSTER_VIEW_H__

#include <Elementary.h>

#include "view.h"

Evas_Object *cluster_view_create(Evas_Object *parent);
void cluster_view_app_terminate(void);

void cluster_view_show(void);
void cluster_view_hide(void);
void cluster_view_show_anim(double pos);
void cluster_view_hide_anim(double pos);

void cluster_view_hw_menu_key(void);
bool cluster_view_hw_home_key(void);
bool cluster_view_hw_back_key(void);
view_state_t cluster_view_get_state(void);
void cluster_view_set_state(view_state_t state);

#endif /* __CLUSTER_VIEW_H__ */
