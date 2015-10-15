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

#include "homescreen-efl.h"
#include "all_apps.h"
#include "page_scroller.h"
#include "app_grid.h"
#include "dynamic_index.h"

static struct {
	Evas_Object *all_apps;
	Evas_Object *parent;
	Evas_Coord w;
	Evas_Coord h;

	dynamic_index_t *index;

	bool anim_base_positions_actual;
	int anim_base_pos_x;
	int anim_base_pos_y;

} s_info = {
	.all_apps = NULL,
	.parent = NULL,
	.w = -1,
	.h = -1,

	.index = NULL,

	/*these variables are used during animation transitions:*/
	.anim_base_positions_actual = false,
	.anim_base_pos_x = -1,
	.anim_base_pos_y = -1
};


static bool __all_apps_fill(void);

HAPI void all_apps_detach_from_grid(Elm_Object_Item *grid_item)
{
	LOGI("");
	if (!grid_item) {
		LOGE("[INVALID_ARG][grid_item='%p']" , grid_item);
		return;
	}

	app_grid_unpack_item(grid_item);
	elm_object_item_del(grid_item);
}

HAPI Elm_Object_Item *all_apps_attach_to_grid(Evas_Object *icon, int grid_num)
{
	Evas_Object *grid = NULL;

	if (!icon) {
		LOGE("icon == NULL");
		return NULL;
	}

	grid = page_scroller_get_page(s_info.all_apps, grid_num);

	if (home_screen_get_view_type() == HOMESCREEN_VIEW_ALL_APPS_CHOOSE)
		elm_object_signal_emit(icon, SIGNAL_EDIT_MODE_OFF, SIGNAL_SOURCE);

	return app_grid_append_item(grid, icon);
}

HAPI Evas_Object *all_apps_create(Evas_Object *parent, void *data)
{
	Evas_Object *p_edje = NULL;

	/*Created before!*/
	if (!parent) {
		LOGE("[INVALID_ARG][parent='%p'][data='%p']" , parent, data);
		return NULL;
	}
	s_info.parent = parent;

	p_edje = elm_layout_edje_get(parent);
	edje_object_part_geometry_get(p_edje, PART_CONTENT , NULL , NULL, &s_info.w, &s_info.h);

	if (s_info.w < 0 || s_info.h < 0) {
		LOGE("[FAILED][edje_object_part_geometry_get][w='%d'][h='%d']", s_info.w, s_info.h);
		return NULL;
	}

	s_info.all_apps = page_scroller_create(s_info.w, s_info.h);
	if (!s_info.all_apps) {
		LOGE("[FAILED][page_scroller_create]");
		return NULL;
	}

	if (!__all_apps_fill()) {
		LOGE("[FAILED][__all_apps_fill]");
		page_scroller_destroy(s_info.all_apps);
		s_info.all_apps = NULL;
		return NULL;
	}

	s_info.index = dynamic_index_new(s_info.all_apps);
	if (!s_info.index) {
		LOGE("[FAILED][dynamic_index_new]");
		page_scroller_destroy(s_info.all_apps);
		s_info.all_apps = NULL;
		return NULL;
	}

	evas_object_size_hint_min_set(s_info.all_apps, home_screen_get_root_width(), home_screen_get_root_height());
	evas_object_resize(s_info.all_apps, home_screen_get_root_width(), home_screen_get_root_height());
	evas_object_move(s_info.all_apps, 0, home_screen_get_root_height());

	return s_info.all_apps;
}

HAPI void all_apps_show(void)
{
	 if (s_info.index) {
		elm_object_part_content_set(s_info.parent, PART_INDEX, s_info.index->box);
		evas_object_show(s_info.index->box);
 	}
}

HAPI void all_apps_hide(void)
{
	elm_object_part_content_unset(s_info.parent, PART_INDEX);
	if (s_info.index)
	 	evas_object_hide(s_info.index->box);
}

HAPI void all_apps_del(void)
{
	/*app_grids was added into all_apps so it should be removed
	when this object will be deleted.*/
	dynamic_index_del(s_info.index);
	s_info.index = NULL;

	evas_object_del(s_info.all_apps);
	s_info.all_apps = NULL;
}

HAPI void all_apps_set_view_mode(homescreen_view_t view)
{
	data_model_set_view_mode(view);
}

HAPI void all_apps_set_scrolling_blocked_state(bool is_blocked)
{
	if (is_blocked) page_scroller_freeze(s_info.all_apps);
	else page_scroller_unfreeze(s_info.all_apps);
}

HAPI bool all_apps_append_page(Tree_node_t *page_item)
{
	Evas_Object *app_grid = app_grid_create(s_info.all_apps, page_item, s_info.w, s_info.h, APP_GRID_TYPE_ALL_APPS);

	if (!app_grid) {
		LOGE("[FAILED][app_grid_create]");
		return false;
	}

	if (!page_scroller_add_page(s_info.all_apps, app_grid)) {
		LOGE("[FAILED][page_scroller_page_add]");
		evas_object_del(app_grid);
		return false;
	}

	return true;
}

HAPI void all_apps_remove_empty_pages(void)
{
	Evas_Object *empty_page_layout = NULL, *page_layout = NULL, *gengrid_inside_layout = NULL;
	int i = 0;

	do {
		i = 0;
		empty_page_layout = NULL;

		while ((page_layout = page_scroller_get_page(s_info.all_apps, i++))) {
			gengrid_inside_layout = elm_object_part_content_get(page_layout, PART_APP_GRID_CONTENT);

			if (elm_gengrid_items_count(gengrid_inside_layout) == 0) {
				empty_page_layout = page_layout;
				break;
			}
		}
		page_scroller_remove_page(s_info.all_apps, empty_page_layout);
	} while (empty_page_layout);
}

HAPI void all_apps_update_dynamic_index_count(void)
{
	elm_object_signal_emit(s_info.all_apps, "scroller,count,changed", "layout");
}

static bool __all_apps_fill(void)
{
	Tree_node_t *it = NULL;
	Tree_node_t *data_model = (Tree_node_t *)data_model_get_all_apps();

	if (!data_model) {
		LOGE("[FAILED][data_model_tree_get");
		return false;
	}

	TREE_NODE_FOREACH(data_model, it)
		if (!all_apps_append_page(it))
			return false;

	return true;
}