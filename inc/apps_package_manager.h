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

#ifndef __APPS_PACKAGE_MANAGER_H__
#define __APPS_PACKAGE_MANAGER_H__


#include <package_manager.h>
#include <app_manager.h>
#include <package_info.h>
#include <app_info.h>
#include <Elementary.h>

void apps_package_manager_init();
Eina_Bool apps_package_manger_get_list(Eina_List **list);

#endif /* __APPS_PACKAGE_MANAGER_H__ */
