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

#ifndef GRID_ITEM_RESIZE_H_
#define GRID_ITEM_RESIZE_H_

#include <Elementary.h>
#include <Evas.h>
#include <stdbool.h>

#include "../util.h"
#include "livebox_utils.h"
#include "../conf.h"

/**
 * @brief Adds callbacks to livebox_container
 * @param livebox_scroller scroller with livebox_container
 * @param livebox_container Layout created with file: livebox_container.edj,
 * with grid as its main part
 * @return true on success
 */
extern bool grid_item_init_resize_callbacks(Evas_Object *livebox_scroller,
	Evas_Object *livebox_container);

/**
 * @brief Set the resize sliders on @p page based on @p livebox geometry
 */
extern void grid_item_set_resize_livebox_sliders(Evas_Object *livebox,
	Evas_Object *page);

/**
 * @brief Set the visibility of resize frame
 * @param page Page containing the frame
 * @param visible Visibility of the frame
 */
extern void grid_item_set_resize_sliders_visibility(Evas_Object *page,
	bool visible);

#endif /* GRID_ITEM_RESIZE_H_ */
