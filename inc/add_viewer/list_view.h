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

#ifndef LIST_VIEW_H_
#define LIST_VIEW_H_

#include "homescreen-efl.h"

/**
 * @brief Create content of the list
 *
 * @details Function creates content of the genlist with found widgets
 *
 * @param naviframe Evas_Object naviframe where content should be created
 * @param genlist Evas_Object genlist to be created
 *
 * @return layout Evas_Object created layout to be displayed
 */
Evas_Object *list_view_create_content(Evas_Object *naviframe, Evas_Object *genlist);

#endif
