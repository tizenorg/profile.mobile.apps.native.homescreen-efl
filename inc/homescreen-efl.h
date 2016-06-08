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

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.homescreen-efl"
#endif

typedef enum {
	HOMESCREEN_VIEW_HOME = 0,
	//HOMESCREEN_VIEW_HOME_EDIT,
	//HOMESCREEN_VIEW_HOME_ALL_PAGES,
	//HOMESCREEN_VIEW_HOME_ADD_VIEWER,
	HOMESCREEN_VIEW_APPS,
	//HOMESCREEN_VIEW_APPS_EDIT,
	//HOMESCREEN_VIEW_APPS_CHOOSE,
	HOMESCREEN_VIEW_UNKNOWN,
} homescreen_view_t;
Evas_Object *homescreen_efl_get_win(void);

void homescreen_efl_hw_menu_key_release(void);
void homescreen_efl_hw_home_key_release(void);
void homescreen_efl_hw_back_key_release(void);

void homescreen_efl_btn_show(homescreen_view_t view_t);
void homescreen_efl_btn_hide(homescreen_view_t view_t);

#endif /* __HOMESCREEN_EFL_H__ */
