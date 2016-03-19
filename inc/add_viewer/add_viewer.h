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

#ifndef ADD_VIEWER_H_
#define ADD_VIEWER_H_

#include "homescreen-efl.h"

/**
 * @brief Create "add viewer" window
 *
 * @details Creates basic window that shows genlist with
 * all found widgets on device.
 *
 * @param w Evas_Coord width of the window
 * @param h Evas_Coord height of the window
 */
void add_viewer_window_create(Evas_Coord w, Evas_Coord h);

/**
 * @brief Delete "add viewer" window
 *
 * @details Deletes "add viewer" window and goes back to
 * homescreen home view
 */
void add_viewer_window_delete(void);

/**
 * @brief Get "add viewer" layout
 *
 * @return layout Evas_Object
 */
Evas_Object *add_viewer_get_layout(void);

/**
 * @brief Get "add viewer" main theme
 *
 * @return theme Elm_Theme
 */
Elm_Theme *add_viewer_get_main_theme(void);

#endif
