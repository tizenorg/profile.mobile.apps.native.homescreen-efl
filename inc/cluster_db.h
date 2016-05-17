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

#ifndef __CLUSTER_DB_H__
#define __CLUSTER_DB_H__

#include <Elementary.h>
#include "cluster_data.h"

bool cluster_db_create(void);
bool cluster_db_close();
bool cluster_db_update(widget_data_t *item);
bool cluster_db_insert(widget_data_t *item);
bool cluster_db_delete(widget_data_t *item);

bool cluster_db_get_list(Eina_List **cluster_list);

#endif /* __CLUSTER_DB_H__ */
