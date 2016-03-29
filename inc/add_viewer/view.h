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

#ifndef VIEW_H_
#define VIEW_H_

#include "homescreen-efl.h"

typedef int(* indexing_cb)(Evas_Object *layout, const char *letter, void *data);

/**
 * @brief Create content of the "add_viewer" layout
 *
 * @details Function creates additional object of the layout, index etc.
 *
 * @param naviframe Evas_Object naviframe where content should be created
 * @param genlist Evas_Object genlist to be created
 *
 * @return layout Evas_Object created layout to be displayed
 */
Evas_Object *view_content_create(Evas_Object *frame, void *data);

/**
 * @brief Show content
 *
 * @details Function sends signal to layout to show its content
 *
 * @param layout Evas_Object object to be shown
 */
void view_content_show(Evas_Object *layout);

/**
 * @brief Show index
 *
 * @details Function sends signal to layout to show index
 *
 * @param layout Evas_Object layout that index belongs to
 */
void view_content_show_index(Evas_Object *layout);

/**
 * @brief Select index item
 *
 * @details Function selects choosen item of the index
 *
 * @return 0 on success, 1 on failure
 */
int view_index_set_index(const char *idx);

/**
 * @brief Get index object
 *
 * @details Function returns index object
 *
 * @return index Evas_Object
 */
Evas_Object *view_get_index(void);

#endif
