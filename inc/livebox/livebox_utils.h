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

#ifndef UTILS_H_
#define UTILS_H_

#include <Elementary.h>

#include "../util.h"
#include "../conf.h"

/**
 * @brief Check interection between @p rect and list items bounding
 * Eina_Rectangles
 * @details Check interection between @p rect and list items bounding
 * Eina_Rectangle.
 * If current list item == @p obj the intersection check won't be performed.
 * By standard the @p rect is the @p obj bounding rectangle.
 * So this helps to avoid interesction check with obj itself
 * @param rect rectangle
 * @param obj filter object. Can be NULL
 * @param list of objects
 * @return TRUE if intersection was found ELSE otherwise
 */
extern bool livebox_utils_check_rect_list_grid_interesction(
	Eina_Rectangle *rect, Evas_Object *obj, Eina_List *list);

/**
 * @brief Returns the list of liveboxes on grid
 * @details Function returns liveboxes put on @p grid but but without the
 * @a shadow object
 * @param grid
 * @return list of liveboxes. Has to be freed
 */
extern Eina_List *livebox_utils_get_liveboxes_on_gird(Evas_Object *grid);

/**
 * @brief Returns the list of liveboxes on page
 * @details Page is a layout described by "livebox_container.edc" Which
 * contains a elm_grid object. All objects in the grid are returned.
 * @param obj livebox page
 * @return list of liveboxes. Has to be freed
 */
extern Eina_List *livebox_utils_get_liveboxes_on_page_list(Evas_Object *obj);


/**
 * @brief Returns the livebox container that contains the selected livebox
 * @return Livebox container
 */
extern Evas_Object *livebox_utils_get_selected_livebox_layout(void);

/**
 * @brief Returns the that previously contained the selected livebox.
 * Used when livebox in repositioned to different container
 * @return Livebox container
 */
extern Evas_Object *livebox_utils_get_prev_livebox_layout(void);

/**
 * @brief Returns the selected livebox
 * @return selected livebox
 */
extern Evas_Object *livebox_utils_selected_livebox_get(void);

/**
 * @brief Sets the selected livebox
 * @note Use this when livebox is selected
 * @param livebox Selected livebox
 * @param layout livbox container holding the selected livebox
 */
extern void livebox_utils_set_selected_livebox(Evas_Object *livebox,
	Evas_Object *layout);

/**
 * @brief Crate the shadow visible when a object is repositioned
 * @param parent Shadow parent
 */
extern void livebox_utils_create_shadow(Evas_Object *parent);

/**
 * @brief Pack the shadow object to given container based on livebox position
 * @note Used in reposition
 * @param livebox item repositioned using drag&drop
 * @param page container where the shadow will be packed
 */
extern void livebox_utils_set_shadow_pack(Evas_Object *livebox,
	Evas_Object *page);

/**
 * @brief Shadow is repacked from one livebox_container to different one
 * @details Shadow object is unpacked from its current grid and packed to grid
 * in page_to livebox_container
 * @param livebox livebox item repositioned using drag&drop
 * @param page_to livebo_container where the shadow will be packed
 */
extern void livebox_utils_repack_shadow(Evas_Object *livebox,
	Evas_Object *page_to);

/**
 * @brief Sets the visiblity of the shadow object
 * @param visible TRUE shadow will be visible
 */
extern void livebox_utils_set_shadow_visibility(bool visible);

/**
 * @brief Creates the clipper for the shadow object
 * @details The clipper is added to the @p livebox_container and is clipped
 * by the shadow object.
 * This way only the part of the clipper will be visible where the repositioned
 * livebox can be dropped
 * @param livebox_container Layout containing the clipper
 */
extern void livebox_utils_create_grid_shadow_clipper(Evas_Object*
	livebox_container);

/**
 * @brief Returns position fitting given grid
 * @details The input parameteters should be given in virtual coordinates of
 * a grid. If the item coordinates don't fit the grid they will be modified
 * properly.
 *	@code
 *		if(obj_x < 0)
 *		{
 *			*out_x = 0;
 *		}
 *
 *		if obj_x + obj_w > grid_width
 *		{
 *			*out_x = grid_width - obj_w
 *		}
 *	@endcode
 *
 * @param[in] obj_x
 * @param[in] obj_y
 * @param[in] obj_w
 * @param[in] obj_h
 * @param[out] out_x
 * @param[out] out_y
 */
extern void livebox_utils_normalize_grid_pos(int obj_x, int obj_y, int obj_w,
	int obj_h, int *out_x, int *out_y);

/**
 * @brief Returns the grid put into the @p container widget
 * @param container The container wodth grid
 * @return Grid object or NULL on error
 */
extern Evas_Object *livebox_utils_get_livebox_container_grid(Evas_Object *container);

/**
 * @brief Returns the shadow object
 * @return shadow object
 */
extern Evas_Object *livebox_utils_get_shadow(void);

/**
 * @brief unpacks the grid object from the grid its currently packed to
 */
extern void livebox_utils_shadow_unpack(void);

/**
 * @brief Transforms the @p livebox geometry to @p grid virtual coordinates
 * @param[in] livebox object to read the geometry from
 * @param[in] grid elm_grid which virtual coordinates will be used
 * @param[out] out_x @p livebox transformed horizontal position
 * @param[out] out_y @p livebox transformed vertical position
 * @param[out] out_w @p livebox transformed width
 * @param[out] out_h @p livebox transformed height
 */
extern void livebox_utils_convert_size_to_grid_coord(Evas_Object *livebox,
	Evas_Object *grid, int *out_x, int *out_y, int *out_w, int *out_h);

/**
 * @brief Sets the packing of @p item_to_pack to the @p grid
 * @details The @p livebox object is used to determine the position of @p
 * item_to_pack in the @p grid.
 * Note that the @p item_to_pack has to be packet in the gird allready
 * @param livebox Position of this object will be used to determine the
 * position of @p item_to_pack in the @p grid
 * @param item_to_pack Object to be packed to the @p grid
 * @param grid The grid object. Must be elm_grid
 */
extern void livebox_utils_set_grid_object_pack(Evas_Object *livebox,
	Evas_Object *item_to_pack, Evas_Object *grid);

/**
 * @brief Repacks @p item_to_pack from @p grid_from to @p grid_to
 * @details The @p livebox object is used to determine the position of @p
 * item_to_pack in the @p grid_to.
 * The item_to_pack will be unpacked from @p grid_from and @p packed to @p
 * grid_to
 *
 * @param livebox ivebox Position of this object will be used to determine
 * the position of @p item_to_pack in the @p grid_to
 * @param item_to_pack Repacked item
 * @param grid_from Source grid
 * @param grid_to D Destination grid
 */
extern void livebox_utils_repack_grid_object(Evas_Object *livebox,
	Evas_Object *item_to_pack, Evas_Object *grid_from,
	Evas_Object *grid_to);

/**
 * @brief Transforms virtual @ grid virtual coordinates to screen coordinates
 * @param grid elm_grid
 * @param x Horizontal position in @p grid
 * @param y Vertical position in @p grid
 * @param w Width in @p grid
 * @param h Height in @p grid
 * @return Eina_Rectangle containing the postion transformed
 * to screen coordinates
 */
extern Eina_Rectangle *livebox_utils_convert_virtual_grid_geo_to_screen(
	Evas_Object *grid, int x, int y, int w, int h);

/**
 * @brief Returns the bounding rectangle of the @p obj object
 * @details Bounding rectangle contains (x position, y position, width, height)
 * of the @p obj
 * @param obj to get the bounding rectangle from
 * @return Bounding rectangle
 */
extern Eina_Rectangle *livebox_utils_get_widget_rectangle(Evas_Object *obj);

/**
 * @brief Returns the bounding rectangle of the @p obj object in grid virtual
 * coordinates
 * @details Bounding rectangle contains (x position, y position, width, height)
 * of the @p obj.
 * @p Obj has to be packed into a grid.
 * @param obj to get the bounding rectangle from
 * @return Bounding rectangle
 */
extern Eina_Rectangle *livebox_utils_get_grid_widget_rectangle(
	Evas_Object *obj);

/**
 * @brief get the current cursor position
 * @param mx position x
 * @param my position y
 */
extern void livebox_utils_get_cursor_pos(int *mx, int *my);


#endif /* UTILS_H_ */
