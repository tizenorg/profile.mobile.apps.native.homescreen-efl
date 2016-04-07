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

#ifndef WIDGET_H_
#define WIDGET_H_

#include "homescreen-efl.h"

typedef struct {
	char *widget_id;
	char *app_id;

	Eina_List *preview_list;
	Elm_Object_Item *genlist_item;

	int size_types_count;
	int size_types;
} widget_t;

typedef struct {
	int type;
	char *path;
} preview_t;

/**
 * @brief Initialize widget list
 *
 * @details Function creates list with all target widgets, previews
 * and its sizes.
 *
 * @return 0 on success, 1 on failure
 */
int widget_init(void);

/**
 * @brief Remove widget list
 *
 * @details This function removes widget list and frees all its allocated
 * memory
 */
void widget_fini(void);

/**
 * @brief Get widget list
 *
 * @details Function returns list with all found widgets and their data.
 *
 * @return widget_list Eina_List
 */
Eina_List* widget_get_widget_list(void);

#endif
