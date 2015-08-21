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

#ifndef __DB_H__
#define __DB_H__

#include "app_item.h"
#include "util.h"

typedef struct
{
	int id;
	app_item_type_t type;
	char *appid;

	int first_id;
	int next_id;

	int x;
	int y;
	int w;
	int h;

	char *content_info;
} db_item_t;

/**
 * @brief Create database table
 */
extern bool db_create_tables(void);

/**
 * @brief Get the applications list from the database
 * @param apps Eina_List pointer
 */
extern bool db_get_apps(Eina_List **apps);

/**
 * @brief Frees the application list
 * @param apps Pointer to Eina_List.
 */
extern void db_free_apps(Eina_List *apps);

/**
 * @brief Updates the database
 * @param updated item
 * @param insert flag.
 */
extern bool db_update_apps(db_item_t item, bool insert);

#endif /* __DB_H__ */
