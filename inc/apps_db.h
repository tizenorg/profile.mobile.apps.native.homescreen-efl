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

#ifndef __APPS_DB_H__
#define __APPS_DB_H__

#include <Elementary.h>
#include "apps_data.h"

bool apps_db_create(void);
bool apps_db_close();
bool apps_db_update(app_data_t *item);
bool apps_db_insert(app_data_t *item);
bool apps_db_delete(app_data_t *item);
bool apps_db_delete_by_pkg_str(const char* pkg);

bool apps_db_get_list(Eina_List **apps);
bool apps_db_get_app_list(Eina_List **apps);


#endif /* __APPS_DB_H__ */
