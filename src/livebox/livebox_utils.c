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

#include "livebox/livebox_utils.h"
#include "homescreen-efl.h"

static struct
{
	Evas_Object *selected_livebox;
	Evas_Object *selected_livebox_layout;
	Evas_Object *prev_livebox_layout;
	Evas_Object *shadow;
}
s_info = {
	.selected_livebox = NULL,
	.selected_livebox_layout = NULL,
	.prev_livebox_layout = NULL,
	.shadow = NULL,
};

static Evas_Object *__livebox_utils_create_grid_bg(Evas_Object *parent);
static Eina_Rectangle *__livebox_utils_get_grid_rectangle(Evas_Object *obj);
static bool __livebox_utils_check_intersection(Evas_Object *obj_a,
	Eina_Rectangle *rect);

Eina_Rectangle *livebox_utils_get_widget_rectangle(Evas_Object *obj)
{
	int x = -1;
	int y = -1;
	int w = -1;
	int h = -1;
	Eina_Rectangle *rect = NULL;

	if (!obj) {
		LOGE("obj == NULL");
		return NULL;
	}
	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rect = eina_rectangle_new(x, y, w, h);
	if (!rect) {
		LOGE("rect == NULL");
		return NULL;
	}

	return rect;
}

Eina_Rectangle *livebox_utils_get_grid_widget_rectangle(Evas_Object *obj)
{
	int x = -1;
	int y = -1;
	int w = -1;
	int h = -1;
	Eina_Rectangle *rect = NULL;

	if (!obj) {
		LOGE("obj == NULL");
		return NULL;
	}
	elm_grid_pack_get(obj, &x, &y, &w, &h);

	rect = eina_rectangle_new(x, y, w, h);
	if (!rect) {
		LOGE("rect == NULL");
		return NULL;
	}

	return rect;
}

Eina_Rectangle *livebox_utils_convert_virtual_grid_geo_to_screen(Evas_Object *grid,
	int x, int y, int w, int h)
{
	int gx = -1;
	int gy = -1;
	int gw = -1;
	int gh = -1;

	int vw = -1;
	int vh = -1;

	Eina_Rectangle *rec = NULL;

	if (!grid) {
		LOGE("grid == NULL");
		return NULL;
	}

	evas_object_geometry_get(grid, &gx, &gy, &gw, &gh);
	elm_grid_size_get(grid, &vw, &vh);

	rec = eina_rectangle_new(
		gx + (int)((float)x/vw * gw),
		gy + (int)((float)y/vh * gh),
		(int)((float)w/vw*gw),
		(int)((float)h/vh*gh)
	);

	if (!rec) {
		LOGE("rec == NULL");
		return NULL;
	}

	return rec;
}

bool livebox_utils_check_rect_list_grid_interesction(Eina_Rectangle *rect,
	Evas_Object *obj, Eina_List *list)
{
	Eina_List *l = NULL;
	Evas_Object *list_obj = NULL;

	if (!list) {
		LOGE("list == NULL");
		return true;
	}

	EINA_LIST_FOREACH(list, l, list_obj) {
		if (!list_obj) {
			LOGE("list_obj == NULL");
			continue;
		}

		if (list_obj != obj && __livebox_utils_check_intersection(
			list_obj, rect)) {
			LOGD("Intersection found");
			return true;
		}
	}

	return false;
}

Eina_List *livebox_utils_get_liveboxes_on_gird(Evas_Object *grid)
{
	Eina_List *livebox_list = NULL;

	if (!grid) {
		LOGE("grid == NULL");
		return NULL;
	}

	livebox_list = elm_grid_children_get(grid);
	if (!livebox_list) {
		LOGE("livebox_list == NULL");
		return NULL;
	}
	livebox_list = eina_list_remove(livebox_list, s_info.shadow);

	return livebox_list;
}


Eina_List *livebox_utils_get_liveboxes_on_page_list(Evas_Object *obj)
{
	Evas_Object *grid = NULL;

	if (!obj) {
		LOGE("o == NULL");
		return NULL;
	}

	grid = elm_layout_content_get(obj, SIGNAL_CONTENT_SOURCE);
	if (!grid) {
		LOGE("grid == NULL");
		return NULL;
	}

	return livebox_utils_get_liveboxes_on_gird(grid);
}
/* ---------------------------------------------------------------------------*/

Evas_Object *livebox_utils_get_selected_livebox_layout(void)
{
	return s_info.selected_livebox_layout;
}

Evas_Object *livebox_utils_get_prev_livebox_layout(void)
{
	return s_info.prev_livebox_layout;
}

Evas_Object *livebox_utils_selected_livebox_get(void)
{
	return s_info.selected_livebox;
}

void livebox_utils_set_selected_livebox(Evas_Object *livebox,
	Evas_Object *layout)
{
	LOGD("Selected_livebox_layout = %p; prev = %p",
		s_info.selected_livebox_layout, s_info.prev_livebox_layout);

	s_info.prev_livebox_layout = s_info.selected_livebox_layout;
	s_info.selected_livebox = livebox;
	s_info.selected_livebox_layout = layout;
}

Evas_Object *livebox_utils_get_livebox_container_grid(Evas_Object *container)
{
	Evas_Object *grid = elm_layout_content_get(container, "content");
	if (!grid) {
		LOGE("grid == NULL");
		return NULL;
	}

	return grid;
}


void livebox_utils_set_grid_object_pack(Evas_Object *livebox,
	Evas_Object *item_to_pack, Evas_Object *grid)
{
	int x = -1, y = -1, w = -1, h = -1;
	int out_x, out_y;

	if (!grid) {
		LOGE("grid == NULL");
		return;
	}

	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	if (!item_to_pack) {
		LOGE("item_to_pack == NULL");
		return;
	}

	livebox_utils_convert_size_to_grid_coord(livebox, grid, &x, &y, &w, &h);
	livebox_utils_normalize_grid_pos(x, y, w, h, &out_x, &out_y);
	elm_grid_pack_set(item_to_pack, out_x, out_y, w, h);
}

void livebox_utils_repack_grid_object(Evas_Object *livebox,
	Evas_Object *item_to_pack, Evas_Object *grid_from, Evas_Object *grid_to)
{
	int x = -1, y = -1, w = -1, h = -1;
	int out_x, out_y;

	if (!item_to_pack) {
		LOGE("item_to_pack == NULL");
		return;
	}

	if (grid_from) {
		elm_grid_pack_get(item_to_pack, &x, &y, &w, &h);
		LOGD("Unpacking from: %p -> %d %d %d %d",
			grid_from, x, y, w, h);

		elm_grid_unpack(grid_from, item_to_pack);
	}

	if (livebox && grid_to) {
		livebox_utils_convert_size_to_grid_coord(livebox,
			grid_to, &x, &y, &w, &h);
		livebox_utils_normalize_grid_pos(x, y, w, h, &out_x, &out_y);
		elm_grid_pack(grid_to, item_to_pack, out_x, out_y, w, h);
	}
}


void livebox_utils_convert_size_to_grid_coord(Evas_Object *livebox, Evas_Object *grid,
	int *out_x, int *out_y, int *out_w, int *out_h)
{
	int lx = 0, ly, lw = 0, lh = 0;
	int gx = 0, gy, gw = 0, gh = 0;
	float ratio_x = 0, ratio_y = 0;

	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	if (!grid) {
		LOGE("grid == NULL");
		return;
	}

	if (!out_w || !out_h) {
		LOGE("output: out_w == %p; out_h == %p", out_w, out_h);
		return;
	}

	evas_object_geometry_get(livebox, &lx, &ly, &lw, &lh);
	evas_object_geometry_get(grid,    &gx, &gy, &gw, &gh);

	if (gw == 0 || gh == 0) {
		LOGE("Grid wrong size gw == %d; gh == %d", gw, gh);
		return;
	}

	lx -= gx;
	ly -= gy;

	ratio_x = (float)lx / gw;
	ratio_y = (float)ly / gh;

	*out_x = round(ratio_x * LIVEBOX_TOTAL_COLUMNS);
	*out_y = round(ratio_y * LIVEBOX_TOTAL_ROWS);

	ratio_x = (float)lw / gw;
	ratio_y = (float)lh / gh;

	*out_w = round(ratio_x * LIVEBOX_TOTAL_COLUMNS);
	*out_h = round(ratio_y * LIVEBOX_TOTAL_ROWS);

	*out_x /= LIVEBOX_GRID_ROWS_CELLS_MULTI;
	*out_x *= LIVEBOX_GRID_ROWS_CELLS_MULTI;

	*out_y /= LIVEBOX_GRID_ROWS_CELLS_MULTI;
	*out_y *= LIVEBOX_GRID_ROWS_CELLS_MULTI;
}

void livebox_utils_normalize_grid_pos(int obj_x, int obj_y, int obj_w,
	int obj_h, int *out_x, int *out_y)
{
	int dx = obj_x + obj_w;
	int dy = obj_y + obj_h;

	if (dx > LIVEBOX_TOTAL_COLUMNS)
		obj_x = (LIVEBOX_TOTAL_COLUMNS - obj_w);

	if (dy > LIVEBOX_TOTAL_ROWS)
		obj_y = (LIVEBOX_TOTAL_ROWS - obj_h);

	if (obj_x < 0)
		obj_x = 0;

	if (obj_y < 0)
		obj_y = 0;

	*out_x = obj_x;
	*out_y = obj_y;
}

/* -------------------------------------- SHADOW -----------------------------*/

Evas_Object *livebox_utils_get_shadow()
{
	if (!s_info.shadow) {
		LOGE("s_info.shadow == NULL");
		return NULL;
	}

	return s_info.shadow;
}

void livebox_utils_shadow_unpack(void)
{
	Evas_Object *page = NULL;
	Evas_Object *grid = NULL;

	page = livebox_utils_selected_livebox_get();
	if (!page) {
		LOGE("page == NULL");
		return;
	}

	grid = livebox_utils_get_livebox_container_grid(page);
	if (!grid) {
		LOGE("grid == NULL");
		return;
	}

	livebox_utils_repack_grid_object(NULL, livebox_utils_get_shadow(),
		grid, NULL);
}

void livebox_utils_create_shadow(Evas_Object *parent)
{
	Evas *e = NULL;
	Evas_Object *shadow = NULL;

	if (!parent) {
		LOGE("parent == NULL");
		return;
	}

	e = evas_object_evas_get(parent);
	if (!e) {
		LOGE("parent == NULL");
		return;
	}


	shadow = evas_object_rectangle_add(e);
	if (!shadow) {
		LOGE("shadow == NULL");
		return;
	}

	s_info.shadow = shadow;
	evas_object_color_set(shadow, LIVEBOX_GRID_BG_ALPHA,
		LIVEBOX_GRID_BG_ALPHA, LIVEBOX_GRID_BG_ALPHA,
		LIVEBOX_GRID_BG_ALPHA);
	evas_object_pass_events_set(shadow, true);
}

void livebox_utils_set_shadow_visibility(bool visible)
{
	if (!s_info.shadow) {
		LOGE("s_info.shadow == NULL");
		return;
	}

	if (visible) {
		evas_object_color_set(s_info.shadow, LIVEBOX_GRID_BG_ALPHA,
			LIVEBOX_GRID_BG_ALPHA, LIVEBOX_GRID_BG_ALPHA,
			LIVEBOX_GRID_BG_ALPHA);
		evas_object_show(s_info.shadow);
	} else {
		evas_object_color_set(s_info.shadow, 0, 0, 0, 0);
		evas_object_hide(s_info.shadow);
	}
}

void livebox_utils_create_grid_shadow_clipper(Evas_Object *livebox_container)
{
	Evas_Object *grid_bg = NULL;

	if (!livebox_container) {
		LOGD("livebox_container == NULL");
		return;
	}

	grid_bg = __livebox_utils_create_grid_bg(livebox_container);
	if (!grid_bg) {
		LOGD("grid_bg == NULL");
		return;
	}

	elm_object_part_content_set(livebox_container, PART_GRID_BG, grid_bg);
	evas_object_clip_set(grid_bg, s_info.shadow);

	elm_layout_signal_emit(livebox_container,
		SIGNAL_GRID_SHADOW_HIDE, PART_GRID_BG);
}

static Evas_Object *__livebox_utils_create_grid_bg(Evas_Object *parent)
{
	Evas *e = NULL;
	Evas_Object *table = NULL;
	Evas_Object *item;
	int i = 0, j = 0;
	int w = -1, h = -1;

	if (!parent) {
		LOGE("parent == NULL");
		return NULL;
	}

	e = evas_object_evas_get(parent);
	if (!e) {
		LOGE("parent == NULL");
		return NULL;
	}

	table = elm_table_add(parent);
	if (!table) {
		LOGE("table == NULL");
		return NULL;
	}

	w = (LIVEBOX_GRID_BG_PADDING /
		(float)ROOT_WIN_W) * home_screen_get_root_width();
	h = (LIVEBOX_GRID_BG_PADDING /
		(float)ROOT_WIN_H) * home_screen_get_root_height();

	elm_table_padding_set(table, w, h);
	elm_table_homogeneous_set(table, EINA_TRUE);

	for (i = 0; i < LIVEBOX_GRID_COLUMNS; ++i) {
		for (j = 0; j < LIVEBOX_GRID_ROWS; ++j) {
			item = evas_object_rectangle_add(e);
			if (!item) {
				LOGE("item == NULL");
				evas_object_del(table);
				return NULL;
			}

			evas_object_color_set(item, 255, 255, 255, 255);
			evas_object_pass_events_set(item, true);
			evas_object_show(item);
			evas_object_size_hint_weight_set(item,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_fill_set(item,
				EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_table_pack(table, item, i, j, 1, 1);
		}
	}

	return table;
}

void livebox_utils_get_cursor_pos(int *mx, int *my)
{
	Evas *e = evas_object_evas_get(home_screen_get_win());
	if (!e) {
		LOGE("e == NULL");
		return;
	}
	evas_pointer_canvas_xy_get(e, mx, my);
}

static Eina_Rectangle *__livebox_utils_get_grid_rectangle(Evas_Object *obj)
{
	LOGD("");
	Eina_Rectangle *rect;
	int x = -1, y = -1, w = -1, h = -1;
	if (!obj) {
		LOGE("obj == NULL");
		return NULL;
	}

	elm_grid_pack_get(obj, &x, &y, &w, &h);
	rect = eina_rectangle_new(x, y, w, h);

	if (!rect) {
		LOGE("rect == NULL");
		return NULL;
	}

	return rect;
}

static bool __livebox_utils_check_intersection(Evas_Object *obj_a,
	Eina_Rectangle *rect)
{
	Eina_Rectangle *rect_o = NULL;
	bool out = true;

	if (!obj_a) {
		LOGE("obj == NULL");
		return true;
	}

	rect_o = __livebox_utils_get_grid_rectangle(obj_a);

	if (!rect_o || !rect) {
		LOGE("Rectangle not created: rect_o = %p; rect = %p",
			rect_o, rect);
		return true;
	}

	LOGD("Intersection: <%dx%d - %dx%d> - <%dx%d - %dx%d>",
		rect_o->x, rect_o->y, rect_o->w, rect_o->h,
		rect->x, rect->y, rect->w, rect->h);

	out = eina_rectangles_intersect(rect_o, rect);

	eina_rectangle_free(rect_o);

	return out;
}
