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

#ifndef DYNAMIC_INDEX_H_
#define DYNAMIC_INDEX_H_

#include <Evas.h>
#include <Eina.h>

#include "page_scroller.h"
#include "conf.h"

typedef struct {
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *indices[INDEX_MAX_PAGE_COUNT];
	int page_count;
	int page_width;
	int page_current;
	int x_current;
} dynamic_index_t;

/**
 * @brief Creates new dynamic index object
 * @param page_scroller Evas_Object pointer to elm_scroller widget
 */
dynamic_index_t *dynamic_index_new(Evas_Object *page_scroller);

/**
 * @brief Removes dynamic inxed object
 * @param dynamic_index Pointer to dynamic_index_t type.
 */
void dynamic_index_del(dynamic_index_t *dynamic_index);

/**
 * @brief Clears content and set current index.
 * @details [long description]
 *
 * @param dynamic_index [description]
 */
void dynamic_index_reload(dynamic_index_t *dynamic_index, int idx, int page_count);

#endif /* DYNAMIC_INDEX_H_ */
