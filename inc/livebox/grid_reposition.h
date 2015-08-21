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

#ifndef GRID_REPOSITION_H_
#define GRID_REPOSITION_H_

#include <Elementary.h>

/**
 * @brief Returns the grid that the repositioned object will be packed to
 * @return origination elm_grid or NULL on error
 */
extern Evas_Object *grid_reposition_get_current_grid(void);

/**
 * @brief Returns the currently repositioned object
 * @return origination elm_grid or NULL on error or if there is no
 * repositioned object
*/
extern Evas_Object *grid_reposition_get_repositioned_item(void);

/**
 * @brief Initialize the reposition process
 * @details
 * Function call order:
 * @code
 * grid_reposition_init()
 * -> grid_reposition_start() -> grid_reposition_move() -> grid_reposition_end()
 * @endcode
 *
 * grid_reposition_move() should be called on mouse move
 *
 * @param grid
 * @param repositioned_item
 */
extern void grid_reposition_init(Evas_Object *grid, Evas_Object
	*repositioned_item);

/**
 * @brief Start the reposition process
 * @details Check the grid_reposition_init() function
 *
 */
extern void grid_reposition_start(void);

/**
 * @brief Move the repositioned object
 * @details Check the grid_reposition_init() function
 * @param mouse_x
 * @param mouse_y
 */
extern void grid_reposition_move(int mouse_x, int mouse_y);

/**
 * @brief End the reposition object
 * @details Check the grid_reposition_init() function
 */
extern void grid_reposition_end(void);

/**
 * @brief Sets the grid that the repostioned object will be dropped to
 * @param grid must be elm_grid
 */
extern void grid_reposition_set_current_grid(Evas_Object *grid);

#endif /* GRID_REPOSITION_H_ */
