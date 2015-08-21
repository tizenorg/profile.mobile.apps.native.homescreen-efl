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

#ifndef LIVEBOX_ALL_PAGES_H_
#define LIVEBOX_ALL_PAGES_H_

#include <stdbool.h>
#include <Elementary.h>

/**
 * @brief Shows all pages panel
 * @return Pointer to all_pages layout
 */
extern Evas_Object *livebox_all_pages_show(void);

/**
 * @brief Hides all pages layout
 * @param page_index if this value is bigger than 0 it closes the all pages view and sets page index as current home page
 */
extern void livebox_all_pages_hide(void);

/**
 * @brief Check if page at index is an add_page
 * @param index page index
 * @return true if given index indicates add page
 */
extern bool livebox_all_pages_is_add_icon_page(int index);

/**
 * @brief Function used for moves page thumb during reposition.
 * @param x int value of x mouse position
 * @param y int value of y mouse position
 */
extern void livebox_all_pages_move_item(int x, int y);

/**
 * @brief Set item to proper position
 *
 */
extern void livebox_all_pages_up_item(void);



#endif /* LIVEBOX_ALL_PAGES_H_ */
