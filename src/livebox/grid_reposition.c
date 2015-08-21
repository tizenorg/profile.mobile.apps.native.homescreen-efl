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

#include <math.h>

#include "livebox/grid_reposition.h"

#include "homescreen-efl.h"
#include "livebox/livebox_utils.h"
#include "livebox/livebox_animator.h"
#include "livebox/livebox_panel.h"
#include "util.h"
#include "conf.h"
#include "data_model.h"

typedef struct {
	Evas_Object *obj;
	Eina_Rectangle *pos_in_grid;
} Repositioned_Object_Data_t;

static struct {
	Evas_Object *repositioned_object;
	Evas_Object *current_grid;
	Evas_Object *origination_grid;
	Repositioned_Object_Data_t repositioned_object_data;
	Eina_List *repositioned_objects_in_grid;
	Anim_Data_t *ad;
	int offset_x;
	int offset_y;
	bool intersection_found;
} s_info = {
	.repositioned_object = NULL,
	.current_grid = NULL,
	.origination_grid = NULL,
	.repositioned_object_data = {
		.obj = NULL,
		.pos_in_grid = NULL
	},
	.repositioned_objects_in_grid = NULL,
	.ad = NULL,
	.offset_x = 0,
	.offset_y = 0,
	.intersection_found = true
};

static void __grid_reposition_drop_anim_cb(Anim_Data_t **ad);
static bool __grid_reposition_check_grid_intersection(Evas_Object *moved_item,
	Evas_Object *grid);
static bool __grid_reposition_check_item_intersection(Evas_Object *moved_item,
	Evas_Object *grid_item);
static void __grid_reposition_save_item_start_data(Evas_Object *object);
static void __grid_reposition_make_space(void);
static void __grid_reposition_update_item_position(Eina_Rectangle *new_geometry);

Evas_Object *grid_reposition_get_current_glrid(void)
{
	return s_info.current_grid;
}

Evas_Object *grid_reposition_get_repositioned_item(void)
{
	return s_info.repositioned_object;
}

void grid_reposition_init(Evas_Object *grid, Evas_Object *repositioned_item)
{
	int mx = -1;
	int my = -1;
	int ox = -1;
	int oy = -1;
	Evas *e = NULL;

	if (!grid) {
		LOGE("grid == NULL");
		return;
	}

	if (!repositioned_item) {
		LOGE("repositioned_item == NULL");
		return;
	}

	e = evas_object_evas_get(repositioned_item);
	if (!e) {
		LOGE("e == NULL");
		return;
	}

	evas_pointer_canvas_xy_get(e, &mx, &my);
	evas_object_geometry_get(repositioned_item, &ox, &oy, NULL, NULL);

	s_info.offset_x = mx - ox;
	s_info.offset_y = my - oy;

	LOGD("MOUSE: (%d, %d); OFFSET: (%d, %d)", mx, my, s_info.offset_x,
		s_info.offset_y);

	s_info.current_grid = s_info.origination_grid = grid;
	s_info.repositioned_object = repositioned_item;

	__grid_reposition_save_item_start_data(repositioned_item);
}

void grid_reposition_start(void)
{
	int mx = -1;
	int my = -1;
	Evas_Object *page = NULL;
	Evas_Object *grid = NULL;

	int x = -1, y = -1, w = -1, h = -1;

	page = livebox_utils_get_selected_livebox_layout();
	if (!page) {
		LOGE("page == NULL");
		return;
	}

	grid = livebox_utils_get_livebox_container_grid(page);
	if (!grid) {
		LOGE("grid == NULL");
		return;
	}


	elm_grid_unpack(s_info.current_grid, s_info.repositioned_object);
	evas_object_geometry_get(s_info.repositioned_object, &x, &y, &w, &h);
	LOGD("Packing. Reposition start: %d; %d; %d; %d", x, y, w, h);

	livebox_utils_get_cursor_pos(&mx, &my);
	grid_reposition_move(mx, my);

	livebox_utils_repack_grid_object(s_info.repositioned_object,
		livebox_utils_get_shadow(), NULL, grid);
}

void grid_reposition_move(int mouse_x, int mouse_y)
{
	if (!s_info.repositioned_object) {
		LOGE("s_info.repositioned_object == NULL");
		return;
	}

	if (!s_info.current_grid) {
		LOGE("s_info.current_grid == NULL");
		return;
	}

	evas_object_move(s_info.repositioned_object,
		mouse_x - s_info.offset_x, mouse_y - s_info.offset_y);
	s_info.intersection_found =
	__grid_reposition_check_grid_intersection(s_info.repositioned_object,
		s_info.current_grid);


	if (livebox_panel_is_add_page_selected())  {
		/*no intersection found but we can't drop on 'add page' page */
		s_info.intersection_found = true;
		livebox_utils_set_shadow_visibility(false);
	} else if (!s_info.intersection_found) {
		/*no intersection found so we can show the shadow */
		livebox_utils_set_grid_object_pack(s_info.repositioned_object,
			livebox_utils_get_shadow(), s_info.current_grid);
		livebox_utils_set_shadow_visibility(true);
	} else if (s_info.current_grid != s_info.origination_grid) {
	/*intersection found and livebox was moved from diferent grid.
	We can hide the shadow*/
		__grid_reposition_make_space();
		livebox_utils_set_shadow_visibility(false);
	} else {
	/* intersection found and livebox was moved from current grid.
	We should display the shadow on the origination position */

		__grid_reposition_make_space();
		elm_grid_pack_set(livebox_utils_get_shadow(),
				s_info.repositioned_object_data.pos_in_grid->x,
				s_info.repositioned_object_data.pos_in_grid->y,
				s_info.repositioned_object_data.pos_in_grid->w,
				s_info.repositioned_object_data.pos_in_grid->h);

		livebox_utils_set_shadow_visibility(true);
	}
}

void grid_reposition_end(void)
{
	Eina_Rectangle *start = NULL;
	Eina_Rectangle *end   = NULL;
	int x = -1;
	int y = -1;
	int w = -1;
	int h = -1;
	Evas_Object *destination_grid = NULL;

	if (!s_info.repositioned_object) {
		LOGE("s_info.repositioned_object == NULL");
		return;
	}

	if (!s_info.current_grid) {
		LOGE("s_info.current_grid == NULL");
		return;
	}

	if (!s_info.origination_grid) {
		LOGE("s_info.origination_grid == NULL");
		return;
	}

	s_info.intersection_found = __grid_reposition_check_grid_intersection(
		s_info.repositioned_object, s_info.current_grid);

	if (!s_info.intersection_found) {
		livebox_utils_convert_size_to_grid_coord(s_info.repositioned_object,
			s_info.current_grid, &x, &y, &w, &h);
		livebox_utils_normalize_grid_pos(x, y, w, h, &x, &y);
		destination_grid = s_info.current_grid;
	} else {
		x = s_info.repositioned_object_data.pos_in_grid->x;
		y = s_info.repositioned_object_data.pos_in_grid->y;
		w = s_info.repositioned_object_data.pos_in_grid->w;
		h = s_info.repositioned_object_data.pos_in_grid->h;
		destination_grid = s_info.origination_grid;
	}

	start = livebox_utils_get_widget_rectangle(s_info.repositioned_object);
	end = livebox_utils_convert_virtual_grid_geo_to_screen(destination_grid,
			x, y, w, h);

	livebox_animator_play_geometry_set(s_info.repositioned_object,
			LIVEBOX_REPOSITION_ANIM_TIME,
			ECORE_POS_MAP_DECELERATE_FACTOR, 2, 0,
			start, end,
			__grid_reposition_drop_anim_cb, &s_info.ad);
}

void grid_reposition_set_current_grid(Evas_Object *grid)
{
	if (grid == NULL) {
		LOGE("grid == NULL");
		s_info.current_grid = s_info.origination_grid;
		return;
	}

	s_info.current_grid = grid;
}

/* ================================== intersection check =====================*/

static Eina_Rectangle *_reposition_grid_item_pos_new_get(
	Eina_Rectangle *grid_item, Eina_Rectangle *moved_item)
{
	int new_x = 0;
	int new_y = 0;

	if (!grid_item) {
		LOGE("grid_item == NULL");
		return NULL;
	}

	if (!moved_item) {
		LOGE("moved_item == NULL");
		return NULL;
	}

	if (moved_item->y - grid_item->h >= 0) {
		new_y = moved_item->y - grid_item->h;
	} else {
		new_y = moved_item->y + grid_item->h;
	}

	if (new_y < 0 || new_y > LIVEBOX_TOTAL_ROWS)
		return NULL;

	livebox_utils_normalize_grid_pos(grid_item->x, new_y, grid_item->w,
		grid_item->h, &new_x, &new_y);

	return eina_rectangle_new(new_x, new_y, grid_item->w, grid_item->h);
}

static bool _rectangle_containment_check(Eina_Rectangle *outer,
	Eina_Rectangle *inner)
{
	if (!outer) {
		LOGE("outer == NULL");
		return false;
	}

	if (!inner) {
		LOGE("inner == NULL");
		return false;
	}

	if (inner->x >= outer->x && inner->y >= outer->y &&
		inner->x + inner->w <= outer->x + outer->w &&
		inner->y + inner->h <= outer->y + outer->h) {
		LOGD("RECTANGLE: [%d, %d - %d, %d] is IN [%d, %d - %d, %d]",
				inner->x, inner->y, inner->w, inner->h,
				outer->x, outer->y, outer->w, outer->h);

		return true;
	} else {
		LOGD("RECTANGLE: [%d, %d - %d, %d] is NOT [%d, %d - %d, %d]",
				inner->x, inner->y, inner->w, inner->h,
				outer->x, outer->y, outer->w, outer->h);

		return false;
	}
}

static void __grid_reposition_make_space(void)
{
	Eina_Rectangle *repo_obj_rec = NULL;
	Eina_Rectangle *grid_obj_rec = NULL;
	Eina_Rectangle *grid_obj_new_pos = NULL;
	Eina_List *items_in_grid = NULL;
	Eina_List *l = NULL;
	Evas_Object *obj = NULL;
	int rx = -1, ry = -1, rw = -1, rh = -1;
	bool contaiment = false;
	bool intersection_in_grid = false;
	bool can_space_make = false;

	if (!s_info.current_grid) {
		LOGE("s_info_current_grid == NULL");
		return;
	}

	if (!s_info.origination_grid) {
		LOGE("s_info.origination_grid == NULL");
		return;
	}

	items_in_grid = livebox_utils_get_liveboxes_on_gird(
		s_info.current_grid);
	if (!items_in_grid) {
		LOGE("items_in_grid == NULL");
		return;
	}

	livebox_utils_convert_size_to_grid_coord(s_info.repositioned_object,
		s_info.current_grid, &rx, &ry, &rw, &rh);
	livebox_utils_normalize_grid_pos(rx, ry, rw, rh, &rx, &ry);

	repo_obj_rec = eina_rectangle_new(rx, ry, rw, rh);
	if (!repo_obj_rec) {
		LOGE("repo_obj_rec == NULL");
		return;
	}

	EINA_LIST_FOREACH(items_in_grid, l, obj) {
		if (!obj) {
			LOGE("obj == NULL");
			continue;
		}

		grid_obj_rec = livebox_utils_get_grid_widget_rectangle(obj);
		if (!grid_obj_rec) {
			LOGE("grid_obj_rec == NULL");
			continue;
		}

		contaiment = _rectangle_containment_check(grid_obj_rec,
			repo_obj_rec); /*moved obj in grid obj*/

		if (!contaiment) {
			contaiment = _rectangle_containment_check(repo_obj_rec,
				grid_obj_rec); /*grid obj in moved obj*/
		}

		if (contaiment) {
			grid_obj_new_pos = _reposition_grid_item_pos_new_get(
				grid_obj_rec, repo_obj_rec);

			if (grid_obj_new_pos &&
				!eina_rectangles_intersect(grid_obj_new_pos,
					repo_obj_rec)) {
				intersection_in_grid =
				livebox_utils_check_rect_list_grid_interesction(
					grid_obj_new_pos, obj, items_in_grid);

				/*TODO: This call is not required when we assume
				 that no livebox can be put side by side*/
				if (!intersection_in_grid) {
					elm_grid_pack_set(obj,
						grid_obj_new_pos->x,
						grid_obj_new_pos->y,
						grid_obj_new_pos->w,
						grid_obj_new_pos->h);
					can_space_make = true;
				} else {
					can_space_make = false;
					eina_rectangle_free(grid_obj_new_pos);
					eina_rectangle_free(grid_obj_rec);
					break;
				}

				eina_rectangle_free(grid_obj_new_pos);
			}
		}

		eina_rectangle_free(grid_obj_rec);
	}

	if (can_space_make)
		__grid_reposition_update_item_position(repo_obj_rec);

	eina_rectangle_free(repo_obj_rec);
	eina_list_free(items_in_grid);
}

static void __grid_reposition_save_item_start_data(Evas_Object *object)
{
	int x = -1, y = -1, w = -1, h = -1;

	if (!object) {
		LOGE("object == NULL");
		return;
	}

	s_info.repositioned_object_data.obj = object;
	elm_grid_pack_get(object, &x, &y, &w, &h);

	s_info.repositioned_object_data.pos_in_grid = eina_rectangle_new(x, y,
							w, h);
}

static void __grid_reposition_update_item_position(Eina_Rectangle *new_geometry)
{
	if (!new_geometry) {
		LOGE("new_geometry == NULL");
		return;
	}

	if (!s_info.repositioned_object_data.pos_in_grid) {
		LOGE("s_info.repositioned_object_data.pos_in_grid == NULL");
		return;
	}

	s_info.repositioned_object_data.pos_in_grid->x = new_geometry->x;
	s_info.repositioned_object_data.pos_in_grid->y = new_geometry->y;
	s_info.repositioned_object_data.pos_in_grid->w = new_geometry->w;
	s_info.repositioned_object_data.pos_in_grid->h = new_geometry->h;
}

static bool __grid_reposition_check_grid_intersection(Evas_Object *moved_item,
	Evas_Object *grid)
{
	Eina_List *grid_items = NULL;
	Eina_List *l = NULL;
	Evas_Object *grid_item = NULL;

	if (!moved_item) {
		LOGE("moved_item == NULL");
		return false;
	}

	if (!grid) {
		LOGE("grid == NULL");
		return false;
	}

	grid_items = livebox_utils_get_liveboxes_on_gird(grid);
	if (!grid_items) {
		LOGE("grid_items == NULL");
		return false;
	}

	EINA_LIST_FOREACH(grid_items, l, grid_item) {
		if (!grid_item) {
			LOGE("item == NULL");
			continue;
		}

		if (__grid_reposition_check_item_intersection(moved_item,
			grid_item) == true) {
			eina_list_free(grid_items);
			return true;
		}
	}

	eina_list_free(grid_items);
	return false;
}

static Eina_Rectangle *_normalized_rectangle_get(Evas_Object *obj)
{
	int x = -1;
	int y = -1;
	int w = -1;
	int h = -1;
	Eina_Rectangle *rect = NULL;

	if (!s_info.current_grid) {
		LOGE("s_info.current_grid == NULL");
		return NULL;
	}

	if (!obj) {
		LOGE("obj == NULL");
		return NULL;
	}

	livebox_utils_convert_size_to_grid_coord(obj, s_info.current_grid,
		&x, &y, &w, &h);
	livebox_utils_normalize_grid_pos(x, y, w, h, &x, &y);
	rect = livebox_utils_convert_virtual_grid_geo_to_screen(s_info.current_grid,
		x, y, w, h);

	if (!rect) {
		LOGE("rect == NULL");
		return NULL;
	}

	return rect;
}

static bool __grid_reposition_check_item_intersection(Evas_Object *moved_item,
	Evas_Object *grid_item)
{
	Eina_Rectangle *moved_item_rec = NULL;
	Eina_Rectangle *grid_item_rec  = NULL;
	bool ret = false;

	if (!moved_item) {
		LOGE("moved_item == NULL");
		return false;
	}

	if (!grid_item) {
		LOGE("grid_item == NULL");
		return false;
	}

	moved_item_rec = _normalized_rectangle_get(moved_item);
	if (!moved_item_rec) {
		LOGE("moved_item_rec == NULL");
		return false;
	}

	grid_item_rec =  livebox_utils_get_widget_rectangle(grid_item);
	if (!grid_item_rec) {
		LOGE("grid_item_rec == NULL");
		eina_rectangle_free(moved_item_rec);
		return false;
	}

	ret = eina_rectangle_intersection(moved_item_rec, grid_item_rec);

	eina_rectangle_free(moved_item_rec);
	eina_rectangle_free(grid_item_rec);

	return ret;
}

static void _grid_reposition_update_data_model(Evas_Object *page)
{
	Eina_List *items = NULL;
	Tree_node_t *node = NULL;
	Tree_node_t *page_node = NULL;
	int x = 0, y = 0, w = 0, h = 0;
	Eina_List *l;
	Evas_Object *livebox;

	LOGI("DATA MODEL REPOSITION START");

	elm_grid_pack_get(s_info.repositioned_object, &x, &y, &w, &h);
	node = evas_object_data_get(s_info.repositioned_object, KEY_ICON_DATA);
	if (!node) {
		LOGE("node == NULL");
		return;
	}

	page_node = evas_object_data_get(page, KEY_ICON_DATA);
	if (!page_node) {
		LOGE("obj_node == NULL");
		return;
	}

	data_model_reposition_widget(page_node, node);

	items = livebox_utils_get_liveboxes_on_page_list(page);
	if (!items) {
		LOGD("items == NULL");
		return;
	}

	EINA_LIST_FOREACH(items, l, livebox) {
		if (!livebox) {
			LOGE("livebox == NULL");
			continue;
		}

		elm_grid_pack_get(livebox, &x, &y, &w, &h);
		node = evas_object_data_get(livebox, KEY_ICON_DATA);
		if (!node) {
			LOGE("node == NULL");
			continue;
		}

		data_model_resize_widget(node, x, y, w, h);
	}

	home_screen_print_tree();
}

static void __grid_reposition_drop_anim_cb(Anim_Data_t **ad)
{
	Evas_Object *page = NULL;

	LOGI("DROP ANIM END");

	if (!s_info.current_grid || !s_info.origination_grid ||
		!s_info.repositioned_object) {
		LOGE("s_info.current_grid == %p; s_info.origination_grid == %p;\
			s_info.repositioned_object == %p", s_info.current_grid,
			s_info.origination_grid, s_info.repositioned_object);

		s_info.repositioned_object = NULL;
		s_info.current_grid     = NULL;
		livebox_animator_del_grid(ad);
		s_info.intersection_found = false;
		eina_rectangle_free(
			s_info.repositioned_object_data.pos_in_grid);
	}

	livebox_utils_repack_grid_object(NULL, livebox_utils_get_shadow(),
		s_info.current_grid, NULL);

	if (!s_info.intersection_found) {
		livebox_utils_repack_grid_object(s_info.repositioned_object,
				s_info.repositioned_object,
				NULL,
				s_info.current_grid);

		page = livebox_scroller_get_page();
	} else {
		elm_grid_pack(s_info.origination_grid,
			s_info.repositioned_object,
			s_info.repositioned_object_data.pos_in_grid->x,
			s_info.repositioned_object_data.pos_in_grid->y,
			s_info.repositioned_object_data.pos_in_grid->w,
			s_info.repositioned_object_data.pos_in_grid->h);

		page = livebox_utils_get_prev_livebox_layout();
	}

	if (!page) {
		LOGE("page == NULL");
		s_info.repositioned_object = NULL;
		s_info.current_grid     = NULL;

		livebox_animator_del_grid(ad);
		s_info.intersection_found = false;
		eina_rectangle_free(
			s_info.repositioned_object_data.pos_in_grid);
		return;
	}

	_grid_reposition_update_data_model(page);

	s_info.repositioned_object = NULL;
	s_info.current_grid     = NULL;

	livebox_animator_del_grid(ad);
	s_info.intersection_found = false;
	eina_rectangle_free(s_info.repositioned_object_data.pos_in_grid);
}
