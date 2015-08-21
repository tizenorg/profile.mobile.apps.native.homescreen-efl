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

#include <stdio.h>
#include <stdbool.h>

#include <Elementary.h>
#include <Eina.h>
#include <system_settings.h>

#include "conf.h"

#include "livebox_all_pages.h"
#include "homescreen-efl.h"
#include "page_scroller.h"
#include "livebox/livebox_panel.h"
#include "livebox/livebox_utils.h"
#include "util.h"
#include "dynamic_index.h"
#include "mouse.h"
#include "popup.h"
#include "layout.h"
#include "data_model.h"

static Evas_Object *__livebox_all_pages_create_grid(Evas_Object *parent_layout);
static Evas_Object *__livebox_all_pages_create_thumb(Evas_Object *page, bool is_fake, bool show_remove_button);
static Evas_Object *__livebox_all_pages_create_bg(Evas_Object *parent);
static Evas_Object *__livebox_all_pages_create_add_icon_layout(void);
static Evas_Object *__livebox_all_pages_insert_thumb(void *data, Evas_Object *obj, const char *part);
static Evas_Object *__livebox_all_pages_get_thumb_page(Elm_Object_Item *it);

static Eina_Bool __livebox_all_pages_item_up_cb(void *data);
static Eina_Bool __livebox_all_pages_get_grid_state(void *data, Evas_Object *obj, const char *part);
static Eina_Bool __livebox_all_pages_new_item_created_cb(void *data);

static void __livebox_all_pages_del_grid(void *data, Evas_Object *obj);
static void __livebox_all_pages_grid_item_longpressed_cb(void *data, Evas_Object *obj, void *ei);
static void __livebox_all_pages_grid_item_moved_cb(void *data, Evas_Object *obj, void *ei);
static void __livebox_all_pages_grid_changed_cb(void *data, Evas_Object *obj, void *ei);
static void __livebox_all_pages_hide_remove_button(Elm_Object_Item *it);
static void __livebox_all_pages_realized_cb(void *data, Evas_Object *obj, void *ei);
static void __livebox_all_pages_remove_button_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __livebox_all_pages_remove_button_clicked_liveboxes_cb(void *data);
static void __livebox_all_pages_add_page_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __livebox_all_pages_page_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __libebox_all_pages_grid_item_pressed_cb(void *data, Evas_Object *obj, void *ei);

static char *__livebox_all_pages_get_grid_text(void *data, Evas_Object *obj, const char *part);
static bool __livebox_all_pages_fill_grid(Evas_Object *grid);
static bool __livebox_all_pages_check_add_page(Elm_Object_Item *it);
static bool __livebox_all_pages_is_page_empty(Evas_Object *page);
static Eina_List *__livebox_all_pages_unpack_grid(void);
static Elm_Gengrid_Item_Class *__livebox_all_pages_create_grid_item_class(void);


static struct {
	Evas_Object *item;
	Elm_Object_Item *g_item;

	int cur_x;
	int cur_y;

	bool is_created;
} s_info_reposition = {
	.item = NULL,
	.g_item = NULL,

	.cur_x = -1,
	.cur_y = -1,

	.is_created = false,
};

static struct {
	Evas_Object *grid;
	Evas_Object *layout;
	Elm_Gengrid_Item_Class *gic;
	Elm_Object_Item *add_page_item;
	bool is_add_page_visible;
	int current_page;
	int pressed_item;
	bool is_item_longpressed;
} s_info = {
	.grid = NULL,
	.layout = NULL,
	.gic = NULL,
	.add_page_item = NULL,
	.is_add_page_visible = true,
	.current_page = -1,
	.pressed_item = -1,
	.is_item_longpressed = false,
};

/*======================================== PUBLIC FUNCTIONS IMPLEMENTATION ======================================*/
HAPI Evas_Object *livebox_all_pages_show(void)
{
	Evas_Object *all_pages_layout = NULL;
	Evas_Object *win = home_screen_get_win();
	Evas_Object *bg = NULL;
	Evas_Object *grid = NULL;


	int g_count;
	int w = home_screen_get_root_width();
	int h = home_screen_get_root_height();

	if (!win) {
		LOGE("[INVALID_STATE][win='%p']", win);
		return NULL;
	}

	all_pages_layout = util_create_edje_layout(win, EDJE_DIR"/page_edit.edj", GROUP_ALL_PAGES);
	if (!all_pages_layout) {
		LOGE("[FAILED][util_edje_layout_create]");
		return NULL;
	}

	evas_object_size_hint_min_set(all_pages_layout, w, h);
	evas_object_size_hint_max_set(all_pages_layout, w, h);
	evas_object_resize(all_pages_layout, w, h);
	evas_object_layer_set(all_pages_layout, EVAS_LAYER_MAX-1);
	evas_object_move(all_pages_layout, ALL_PAGES_X, ALL_PAGES_Y);

	bg = __livebox_all_pages_create_bg(all_pages_layout);
	if (!bg) {
		LOGE("[FAILED][__livebox_all_pages_create_bg]");
		evas_object_del(all_pages_layout);
		return NULL;
	}

	grid = __livebox_all_pages_create_grid(all_pages_layout);
	if (!grid) {
		LOGE("[FAILED][__livebox_all_pages_create_grid]");
		evas_object_del(all_pages_layout);
		return NULL;
	}

	__livebox_all_pages_fill_grid(grid);

	s_info.grid = grid;
	s_info.layout = all_pages_layout;
	g_count = elm_gengrid_items_count(grid);

	if (g_count == MAX_ITEMS)
		s_info.is_add_page_visible = false;

	evas_object_smart_callback_add(grid, "realized", __livebox_all_pages_realized_cb, NULL);

	return all_pages_layout;
}

HAPI void livebox_all_pages_hide(void)
{
	Evas_Object *all_pages_layout = s_info.layout;
	Evas_Object *lvb_scroller = NULL;

	Eina_List *page_list = NULL;
	int page_count = 0;

	if (!all_pages_layout) {
		LOGE("[FAILED][all_pages_layout]");
		return;
	}

	lvb_scroller = livebox_panel_get();
	if (!lvb_scroller) {
		LOGE("[FAILED][livebox_panel_get]");
		return;
	}

	page_list = __livebox_all_pages_unpack_grid();
	if (!page_list) {
		LOGE("[FAILED][__livebox_all_pages_unpack_grid]");
		return;
	}

	page_scroller_pack_pages(lvb_scroller, page_list);
	eina_list_free(page_list);
	evas_object_del(all_pages_layout);
	elm_gengrid_item_class_free(s_info.gic);

	page_count = page_scroller_get_page_count(lvb_scroller);

	if (s_info.current_page >= page_count)
		s_info.current_page = page_count - 1;

	if (s_info.pressed_item >= 0)
		s_info.current_page = s_info.pressed_item - 1;

	LOGI("CURRENT : [%d] PRESSED_PAGE: [%d]", s_info.current_page, s_info.pressed_item);

	page_scroller_show_page(lvb_scroller, s_info.current_page);

	dynamic_index_reload(evas_object_data_get(lvb_scroller, KEY_INDEX), s_info.current_page,
		page_count);


	s_info.layout = NULL;
	s_info.grid = NULL;
	s_info.gic = NULL;
}

HAPI void livebox_all_pages_move_item(int x, int y)
{
	int xp, yp;
	int m_idx = -1; /*moved item index*/
	int d_idx = -1; /*destination index*/

	Elm_Object_Item *it = NULL;
	Elm_Object_Item *n_it = NULL;

	Evas_Object *empty_ly = NULL;

	if (!s_info_reposition.item)
		return;

	xp = x - OFFSET;
	yp = y - OFFSET;
	evas_object_move(s_info_reposition.item, xp, yp);

	it = elm_gengrid_at_xy_item_get(s_info.grid, x, y, NULL, NULL);
	if (!it || (it == s_info_reposition.g_item) || !s_info_reposition.g_item || s_info_reposition.is_created)
		return;

	/*check if item is add page*/
	if (__livebox_all_pages_check_add_page(it))
		return;

	m_idx = elm_gengrid_item_index_get(s_info_reposition.g_item);
	d_idx = elm_gengrid_item_index_get(it);

	empty_ly = __livebox_all_pages_create_thumb(NULL, false, true);
	if (!empty_ly) {
		LOGE("[FAILED][__livebox_all_pages_create_thumb]");
		return;
	}

	if (d_idx > m_idx) {
		/*insert after*/
		s_info_reposition.is_created = true;
		n_it = elm_gengrid_item_insert_after(s_info.grid, s_info.gic, empty_ly, it, NULL, NULL);
	} else {
		/*insert before*/
		s_info_reposition.is_created = true;
		n_it = elm_gengrid_item_insert_before(s_info.grid, s_info.gic, empty_ly, it, NULL, NULL);
	}

	evas_object_smart_callback_add(s_info.grid, "index,update", __livebox_all_pages_grid_changed_cb, n_it);
	elm_object_item_del(s_info_reposition.g_item);
	s_info_reposition.g_item = n_it;
}

HAPI void livebox_all_pages_up_item(void)
{
	if (!s_info_reposition.item)
		return;

	ecore_timer_add(0.05, __livebox_all_pages_item_up_cb, NULL);
}

/*====================================== PRIVATE FUNCTIONS IMPLEMENTATION =======================================*/
static void __livebox_all_pages_remove_button_clicked_liveboxes_cb(void *data)
{
	Elm_Object_Item *clicked_item = (Elm_Object_Item *) data;
	Evas_Object *add_page = NULL;
	Eina_List *g_items = NULL;
	Elm_Object_Item *g_item = NULL;
	Eina_List *last_node = NULL;
	Evas_Object *clicked_page =  NULL;
	int g_count = 0;

	if (!clicked_item) {
		LOGE("[FAILED][__livebox_all_pages_remove_button_clicked_liveboxes_cb]");
		return;
	}

	clicked_page = __livebox_all_pages_get_thumb_page(clicked_item);
	if (!clicked_page) {
		LOGE("[FAILED][__livebox_all_pages_get_thumb_page]");
		return;
	}

	g_items = elm_gengrid_realized_items_get(s_info.grid);
	last_node = eina_list_last(g_items);
	if (last_node)
		g_item = eina_list_data_get(last_node);
	eina_list_free(g_items);

	/*is this the correct order ? :*/
	livebox_panel_destroy_page(clicked_page);
	elm_object_item_del(clicked_item);
	g_count = elm_gengrid_items_count(s_info.grid);


	if (g_count < MAX_ITEMS && !(__livebox_all_pages_check_add_page(g_item))) {
		add_page = __livebox_all_pages_create_add_icon_layout();
		if (!add_page) {
			LOGE("[FAILED][__livebox_all_pages_create_add_icon_layout]");
			return;
		}

		s_info.add_page_item = elm_gengrid_item_append(s_info.grid, s_info.gic, add_page, NULL, NULL);
		if (!s_info.add_page_item) {
			LOGE("[FAILED][elm_gengrid_item_append]");
			evas_object_del(add_page);
			return;
		}

		s_info.is_add_page_visible = true;
	}

	if (g_count == 2)
		__livebox_all_pages_hide_remove_button(elm_gengrid_first_item_get(s_info.grid));

	s_info.pressed_item = -1;
	popup_destroy();
}

static bool __livebox_all_pages_is_page_empty(Evas_Object *page)
{
	Eina_List *lb_list = NULL;

	if (!page) {
		LOGE("[INVALID_PARAM][page='%p']", page);
		return true;
	}

	lb_list = livebox_utils_get_liveboxes_on_page_list(page);
	if (!lb_list) {
		LOGE("[FAILED][livebox_utils_liveboxes_on_page_list_get]");
		return true;
	}

	if (eina_list_count(lb_list) > 0) {
		LOGE("[FAILED][eina_list_count]");
		eina_list_free(lb_list);
		return false;
	}

	eina_list_free(lb_list);
	return true;
}

static void __livebox_all_pages_add_page_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Tree_node_t *page_node = NULL;

	Elm_Object_Item *first_item = NULL;

	Evas_Object *layout = s_info.layout;
	Evas_Object *grid = s_info.grid;
	Evas_Object *new_page = NULL;
	Evas_Object *thumb = NULL;
	Evas_Object *first_item_layout = NULL;


	int g_count = 0; /*gengrid items count*/

	Elm_Gengrid_Item_Class *gic = s_info.gic;
	Elm_Object_Item *add_item = s_info.add_page_item;

	int w = home_screen_get_root_width();
	int h = home_screen_get_root_height();

	w = THUMB_W_REL * w;
	h = THUMB_H_REL * h;

	if (!gic || !layout || !grid || !add_item) {
		LOGE("[INVALID_PARAM][gic='%p'][layout='%p'][grid='%p'][add_item='%p']", gic, layout, grid, add_item);
		return;
	}

	g_count = elm_gengrid_items_count(grid);

	page_node = data_model_add_widget_page();
	if (!page_node) {
		LOGE("page_node == NULL");
		return;
	}

	new_page =  livebox_panel_create_empty_page(page_node, livebox_panel_get());
	if (!new_page) {
		LOGE("[FAILED][livebox_panel_empty_page_create]");
		data_model_del_item(page_node);
		return;
	}

	home_screen_print_tree();

	thumb = __livebox_all_pages_create_thumb(new_page, true, true);
	if (!thumb) {
		LOGE("[FAILED][__livebox_all_pages_create_thumb]");
		evas_object_del(new_page);
		return;
	}


	if (g_count < MAX_ITEMS) {
		elm_gengrid_item_insert_before(grid, gic, thumb, add_item, NULL, NULL);
		s_info.is_add_page_visible = true;
	} else if (g_count == MAX_ITEMS) {
		elm_object_item_del(s_info.add_page_item);
		s_info.add_page_item = NULL;

		elm_gengrid_item_append(grid, gic, thumb, NULL, NULL);
		s_info.is_add_page_visible = false;
	}

	/*Show remove button on first page*/
	first_item = elm_gengrid_first_item_get(s_info.grid);
	if (!first_item) {
		return;
	}

	first_item_layout = elm_object_item_part_content_get(first_item, GENGRID_ITEM_CONTENT);
	if (!first_item_layout)
		return;
	/*WHAT HAPPENS to previously allocated thumb (g_count > MAX_ITEMS)??*/
	thumb = elm_object_part_content_get(first_item_layout, PART_GRID_ITEM_CONTENT);
	if (!thumb)
		return;

	elm_object_signal_emit(thumb, SIGNAL_REMOVE_BUTTON_SHOW, SIGNAL_SOURCE);
}

static Evas_Object *__livebox_all_pages_create_bg(Evas_Object *parent)
{
	LOGI("");
	Evas_Object *bg = NULL;
	char *buf = NULL;
	int ret = -1;

	if (!parent) {
		LOGE("[FAILED][__livebox_all_pages_create_bg]");
		return NULL;
	}

	bg = elm_bg_add(parent);
	if (!bg) {
		LOGE("[FAILED][elm_bg_add]");
		return NULL;
	}

	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, &buf);

	if (ret != SYSTEM_SETTINGS_ERROR_NONE || !buf) {
		LOGE("[FAILED][failed to get bg path]");
		evas_object_del(bg);
		return NULL;
	}

	if (!elm_bg_file_set(bg, buf, NULL)) {
		LOGE("[FAILED][failed to set bg]");
		evas_object_del(bg);
		free(buf);
		return NULL;
	}

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(bg);
	elm_object_part_content_set(parent, "bg", bg);
	free(buf);
	return bg;
}

static Evas_Object *__livebox_all_pages_create_grid(Evas_Object *parent_layout)
{
	Evas_Object *grid = NULL;

	if (!parent_layout) {
		LOGE("[INVALID_PARAM][parent_layout='%p']", parent_layout);
		return NULL;
	}

	grid = elm_gengrid_add(parent_layout);
	if (!grid) {
		LOGE("Failed to create gengrid");
		return NULL;
	}

	elm_gengrid_item_size_set(grid, (ITEM_W_REL * home_screen_get_root_width()), (ITEM_H_REL * home_screen_get_root_height()));
	elm_gengrid_select_mode_set(grid, ELM_OBJECT_SELECT_MODE_NONE);
	elm_gengrid_filled_set(grid, EINA_TRUE);
	elm_gengrid_horizontal_set(grid, EINA_FALSE);

	/* elm_gengrid_reorder_mode_set(grid, EINA_TRUE);*/
	elm_gengrid_align_set(grid, 0.5, 0.5);

	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(parent_layout, PART_LAYOUT_CONTENT, grid);
	evas_object_show(grid);

	evas_object_smart_callback_add(grid, "longpressed", __livebox_all_pages_grid_item_longpressed_cb, NULL);
	evas_object_smart_callback_add(grid, "moved", __livebox_all_pages_grid_item_moved_cb, NULL);
	evas_object_smart_callback_add(grid, "pressed", __libebox_all_pages_grid_item_pressed_cb, NULL);

	__livebox_all_pages_create_grid_item_class();

	return grid;
}

static Elm_Gengrid_Item_Class *__livebox_all_pages_create_grid_item_class(void)
{
	Elm_Gengrid_Item_Class *gic = s_info.gic;
	if (!s_info.gic) {
		gic = elm_gengrid_item_class_new();
		if (!gic) {
			LOGE("Failed to create gengrid item class");
			return NULL;
		}

		gic->item_style = "default";
		gic->func.text_get = __livebox_all_pages_get_grid_text;
		gic->func.content_get = __livebox_all_pages_insert_thumb;
		gic->func.state_get = __livebox_all_pages_get_grid_state;
		gic->func.del = __livebox_all_pages_del_grid;

		s_info.gic = gic;
	}

	return gic;
}

static Evas_Object *__livebox_all_pages_create_thumb(Evas_Object *page, bool is_fake, bool show_remove_button)
{
	Evas_Object *win = NULL;
	Evas_Object *thumb = NULL;
	Evas_Object *ly_grid_item = NULL;
	Evas_Object *ly_thumb = NULL;
	Evas *e = NULL;

	win = home_screen_get_win();
	if (!win) {
		LOGE("[FAILED][home_screen_win_get]");
		return NULL;
	}

	int w = home_screen_get_root_width();
	int h = home_screen_get_root_height();

	w = THUMB_W_REL * w;
	h = THUMB_H_REL * h;

	ly_grid_item = util_create_edje_layout(win, EDJE_DIR"/page_edit.edj", GROUP_GRID_ITEM);
	if (!ly_grid_item) {
		LOGE("[FAILED][util_edje_layout_create]");
		return NULL;
	}

	ly_thumb = util_create_edje_layout(win, EDJE_DIR"/page_edit.edj", GROUP_THUMB);
	if (!ly_thumb) {
		LOGE("[FAILED][util_edje_layout_create]");
		evas_object_del(ly_grid_item);
		return NULL;
	}

	elm_object_signal_callback_add(ly_thumb, SIGNAL_THUMBNAIL_CLICKED, SIGNAL_SOURCE, __livebox_all_pages_page_clicked_cb, NULL);

	if (page && !is_fake) {
		thumb = page_scroller_get_page_thumbnail(page, w, h);
		if (!thumb) {
			LOGE("[FAILED][page_scroller_page_thumbnail_get]");
			evas_object_del(ly_grid_item);
			evas_object_del(ly_thumb);
			return NULL;
		}

		evas_object_resize(thumb, w, h);
		evas_object_size_hint_max_set(thumb, w, h);
		evas_object_data_set(thumb, KEY_THUMB_DATA, page);
	} else if (page) {
		e = evas_object_evas_get(win);
		if (!e) {
			LOGE("[FAILED][evas_object_evas_get]");
			return NULL;
		}

		thumb = evas_object_rectangle_add(e);
		if (!thumb) {
			LOGE("[FAILED][Failed to create thumb]");
			evas_object_del(ly_grid_item);
			evas_object_del(ly_thumb);
			return NULL;
		}

		evas_object_color_set(thumb, 0, 0, 0, 0);
		evas_object_resize(thumb, w, h);
		evas_object_size_hint_max_set(thumb, w, h);
		evas_object_data_set(thumb, KEY_THUMB_DATA, page);
	}

	if (!page && !is_fake) {
		elm_object_signal_emit(ly_thumb, SIGNAL_BG_HIDE, SIGNAL_SOURCE);
		elm_object_signal_emit(ly_thumb, SIGNAL_REMOVE_BUTTON_HIDE, SIGNAL_SOURCE);
	}
	elm_object_signal_callback_add(ly_thumb, SIGNAL_REMOVE_BUTTON_CLICKED, SIGNAL_SOURCE, __livebox_all_pages_remove_button_clicked_cb, ly_thumb);

	elm_object_part_content_set(ly_thumb, PART_THUMB_CONTENT, thumb);
	elm_object_part_content_set(ly_grid_item, PART_GRID_ITEM_CONTENT, ly_thumb);

	if (!show_remove_button)
		elm_object_signal_emit(ly_thumb, SIGNAL_REMOVE_BUTTON_HIDE_INSTANT, SIGNAL_SOURCE);

	return ly_grid_item;
}

static Evas_Object *__livebox_all_pages_create_add_icon_layout(void)
{
	Evas_Object *win = NULL;
	Evas *e = NULL;
	Evas_Object *add_icon = NULL;
	Evas_Object *ly_grid_item = NULL;
	Evas_Object *ly_thumb = NULL;
	int w = home_screen_get_root_width();
	int h = home_screen_get_root_height();

	win = home_screen_get_win();
	if (!win) {
		LOGE("[FAILED][home_screen_win_get]");
		return NULL;
	}

	e = evas_object_evas_get(win);
	if (!e) {
		LOGE("[FAILED][evas_object_evas_get]");
		return NULL;
	}

	add_icon = evas_object_rectangle_add(e);
	if (!add_icon) {
		LOGE("[FAILED][evas_object_rectangle_add]");
		return NULL;
	}
	evas_object_color_set(add_icon, 0, 0, 0, 0);

	ly_grid_item = util_create_edje_layout(win, EDJE_DIR"/page_edit.edj", GROUP_GRID_ITEM);
	if (!ly_grid_item) {
		LOGE("[FAILED][util_edje_layout_create]");
		evas_object_del(add_icon);
		return NULL;
	}

	ly_thumb = util_create_edje_layout(win, EDJE_DIR"/page_edit.edj", GROUP_THUMB);
	if (!ly_thumb) {
		LOGE("[FAILED][util_edje_layout_create]");
		evas_object_del(add_icon);
		evas_object_del(ly_grid_item);
		return NULL;
	}

	w = THUMB_W_REL * w;
	h = THUMB_H_REL * h;
	evas_object_size_hint_max_set(add_icon, w, h);

	elm_object_signal_emit(ly_thumb, SIGNAL_REMOVE_BUTTON_HIDE_INSTANT, SIGNAL_SOURCE);
	elm_object_part_content_set(ly_grid_item, PART_GRID_ITEM_CONTENT, ly_thumb);
	elm_object_part_content_set(ly_thumb, PART_THUMB_CONTENT, add_icon);

	evas_object_data_set(add_icon, KEY_THUMB_DATA, ly_thumb); /*if pointer returned from the data_key will be the same it would mean that we have add page.*/
	elm_object_signal_callback_add(ly_thumb, SIGNAL_THUMBNAIL_CLICKED, SIGNAL_SOURCE, __livebox_all_pages_add_page_clicked_cb, (void *) index);
	elm_object_signal_emit(ly_thumb, SIGNAL_ADD_ICON_SHOW, SIGNAL_SOURCE);

	return ly_grid_item;
}

static bool __livebox_all_pages_fill_grid(Evas_Object *grid)
{
	Eina_List *pages = NULL;
	Eina_List *it = NULL;

	Evas_Object *lvb_scroller = NULL;
	Evas_Object *page = NULL;
	Evas_Object *t_page = NULL;
	Evas_Object *add_page = NULL;

	bool show_remove_page = false;

	Elm_Gengrid_Item_Class *gic = s_info.gic;

	int g_count = 0;

	if (!grid) {
		LOGE("[INVALID_PARAM][grid='%p']", grid);
		return false;
	}

	lvb_scroller = livebox_panel_get();
	if (!lvb_scroller) {
		LOGE("[FAILED][lvb_scroller='%p']", lvb_scroller);
		return false;
	}

	s_info.current_page = page_scroller_get_current_page(lvb_scroller);

	LOGI("[%s]", evas_object_type_get(lvb_scroller));

	pages = page_scroller_unpack_pages(lvb_scroller);
	if (!pages) {
		LOGE("[FAILED][page_scroller_unpack_pages]");
		return false;
	}

	show_remove_page = (eina_list_count(pages) > 1);

	EINA_LIST_FOREACH(pages, it, page) {
		if (!page)
			continue;

		t_page = __livebox_all_pages_create_thumb(page, false, show_remove_page);
		if (!t_page) {
			LOGE("[FAILED][__livebox_all_pages_create_thumb]");
			/*TODO: livebox implementer, free pages + think of the way to do something with ui data inside the nodes !*/
			elm_gengrid_clear(grid);
			return false;
		}

		elm_gengrid_item_append(grid, gic, t_page, NULL, NULL);
	}

	eina_list_free(pages);

	g_count = elm_gengrid_items_count(grid);
	if (g_count < MAX_ITEMS) {
		add_page = __livebox_all_pages_create_add_icon_layout();
		if (!add_page) {
			LOGE("[FAILED][__livebox_all_pages_create_add_icon_layout]");
			elm_gengrid_clear(grid);
			return false;
		}

		s_info.add_page_item = elm_gengrid_item_append(grid, gic, add_page, NULL, NULL);
		if (!s_info.add_page_item) {
			LOGE("[FAILED][elm_gengrid_item_append]");
			evas_object_del(add_page);
			elm_gengrid_clear(grid);
			return false;
		}

		s_info.is_add_page_visible = true;
	}

	return true;
}

static Evas_Object *__livebox_all_pages_insert_thumb(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *thumb = (Evas_Object *) data;

	if (!thumb) {
		LOGE("[INVALID_DATA][__livebox_all_pages_insert_thumb]");
		return NULL;
	}

	if (!strncmp(part, GENGRID_ITEM_CONTENT, strlen(GENGRID_ITEM_CONTENT))) {
		return thumb;
	}

	return NULL;
}

static Eina_List *__livebox_all_pages_unpack_grid(void)
{
	Evas_Object *page = NULL;

	Elm_Object_Item *g_item = NULL;

	Eina_List *p_list = NULL;
	Eina_List *g_items = NULL; /*list of elm_gengrid items.*/
	Eina_List *it = NULL;

	if (!s_info.grid) {
		LOGE("[FAILED][s_info.grid='%p']", s_info.grid);
		return NULL;
	}

	g_items = elm_gengrid_realized_items_get(s_info.grid);
	if (!g_items) {
		LOGE("[FAILED][elm_gengrid_realized_items_get]");
		return NULL;
	}

	EINA_LIST_FOREACH(g_items, it, g_item) {
		if (!g_item)
			continue;

		page = __livebox_all_pages_get_thumb_page(g_item);
		if (!page) {
			LOGE("[FAILED][__livebox_all_pages_get_thumb_page]");
			eina_list_free(p_list);
			eina_list_free(g_items);
			return NULL;
		}

		if (!__livebox_all_pages_check_add_page(g_item)) {
			p_list = eina_list_append(p_list, page);
			if (!p_list) {
				LOGE("[FAILED][eina_list_append]");
				eina_list_free(g_items);
				return NULL;
			}
		}
	}
	eina_list_free(g_items);

	return p_list;
}

static void __livebox_all_pages_grid_item_longpressed_cb(void *data, Evas_Object *obj, void *ei)
{
	s_info.is_item_longpressed = true;
	s_info.pressed_item = -1;

	Elm_Object_Item *it = (Elm_Object_Item *) ei;

	if (__livebox_all_pages_check_add_page(it))
		return;

	Evas_Object *thumb_layout = elm_object_item_part_content_get(it, GENGRID_ITEM_CONTENT);
	Evas_Object *thumb = elm_object_part_content_unset(thumb_layout, PART_GRID_ITEM_CONTENT);

	evas_object_layer_set(thumb, EVAS_LAYER_MAX);

	elm_object_signal_emit(thumb, SIGNAL_REMOVE_BUTTON_HIDE, SIGNAL_SOURCE);
	elm_object_signal_emit(thumb, SIGNAL_BG_REPOSITON, SIGNAL_SOURCE);

	s_info_reposition.item = thumb;
	s_info_reposition.g_item = it;
}

static void __livebox_all_pages_grid_item_moved_cb(void *data, Evas_Object *obj, void *ei)
{
	LOGD("item moved");
}

static void __livebox_all_pages_grid_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	ecore_timer_add(ITEM_CREATE_TIME, __livebox_all_pages_new_item_created_cb, NULL);
	evas_object_smart_callback_del(obj, "index,update", __livebox_all_pages_grid_changed_cb);
	return;
}

/**
 * @brief Block new item creation during reposition process
 */
static Eina_Bool __livebox_all_pages_new_item_created_cb(void *data)
{
	s_info_reposition.is_created = false;
	return EINA_FALSE;
}

/**
 * @details Function check if elm_object item contain add page layout
 */
static bool __livebox_all_pages_check_add_page(Elm_Object_Item *it)
{
	Evas_Object *page = NULL;
	Evas_Object *ly = NULL;
	Evas_Object *ly_content = NULL;

	if (!it) {
		LOGE("[INVALID_PARAM][it='%p']", it);
		return false;
	}

	ly = elm_object_item_part_content_get(it, GENGRID_ITEM_CONTENT);
	if (!ly) {
		LOGE("[FAILED][elm_object_item_part_content_get]");
		return false;
	}

	ly_content = elm_object_part_content_get(ly, PART_GRID_ITEM_CONTENT);
	if (!ly_content) {
		LOGE("[FAILED][elm_object_part_content_get]");
		return false;
	}

	page = __livebox_all_pages_get_thumb_page(it);
	if (!page) {
		LOGE("[FAILED][__livebox_all_pages_get_thumb_page]");
		return false;
	}

	if (page == ly_content)
		return true;

	return false;
}

static Evas_Object *__livebox_all_pages_get_thumb_page(Elm_Object_Item *it)
{
	Evas_Object *ly = NULL;
	Evas_Object *ly_content = NULL;
	Evas_Object *thumb = NULL;
	Evas_Object *page = NULL;

	if (!it) {
		LOGE("[INVALID_PARAM][it='%p']", it);
		return NULL;
	}

	ly = elm_object_item_part_content_get(it, GENGRID_ITEM_CONTENT);
	if (!ly) {
		LOGE("[FAILED][elm_object_item_part_content_get]");
		return NULL;
	}

	ly_content = elm_object_part_content_get(ly, PART_GRID_ITEM_CONTENT);
	if (!ly_content) {
		LOGE("[FAILED][elm_object_part_content_get]");
		return NULL;
	}

	thumb = elm_object_part_content_get(ly_content, PART_THUMB_CONTENT);
	if (!thumb) {
		LOGE("[FAILED][elm_object_part_content_get]");
		return NULL;
	}

	page = (Evas_Object *) evas_object_data_get(thumb, KEY_THUMB_DATA);

	return page;
}

static Eina_Bool __livebox_all_pages_item_up_cb(void *data)
{
	int gengrid_items = 0;
	Evas_Object *page = NULL;
	Evas_Object *prev_page = NULL;
	Elm_Object_Item *prev_it = NULL;
	Tree_node_t *page_node = NULL;
	Tree_node_t *prev_node = NULL;

	Elm_Object_Item *d_it = s_info_reposition.g_item; /*destination item;*/
	if (!d_it) {
		LOGE("[FAILED][__livebox_all_pages_item_up_cb]");
		return ECORE_CALLBACK_CANCEL;
	}

	Evas_Object *ly_container = elm_object_item_part_content_get(d_it, GENGRID_ITEM_CONTENT);
	if (!ly_container) {
		LOGE("[FAILED][elm_object_item_part_content_get]");
		return ECORE_CALLBACK_CANCEL;
	}

	elm_object_part_content_set(ly_container, PART_GRID_ITEM_CONTENT, s_info_reposition.item);
	elm_object_signal_emit(s_info_reposition.item, SIGNAL_BG_NORMAL, SIGNAL_SOURCE);

	gengrid_items = elm_gengrid_items_count(s_info.grid);
	if (gengrid_items > 2)
		elm_object_signal_emit(s_info_reposition.item, SIGNAL_REMOVE_BUTTON_SHOW, SIGNAL_SOURCE);

	page = __livebox_all_pages_get_thumb_page(d_it);
	if (!page) {
		LOGE("page == NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	page_node = evas_object_data_get(page, KEY_ICON_DATA);
	if (!page_node) {
		LOGE("page_node == NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	prev_it = elm_gengrid_item_prev_get(d_it);
	if (prev_it) {
		LOGD("prev_it != NULL");

		prev_page = __livebox_all_pages_get_thumb_page(prev_it);

		if (!prev_page) {
			LOGE("prev_page == NULL");
			return ECORE_CALLBACK_CANCEL;
		}

		prev_node = evas_object_data_get(prev_page, KEY_ICON_DATA);
		if (!prev_node) {
			LOGE("prev_node == NULL");
			return ECORE_CALLBACK_CANCEL;
		}

		data_model_insert_after(prev_node, page_node);
	} else {
		LOGD("prev_it == NULL");

		data_model_insert_after(NULL, page_node);
	}

	s_info_reposition.cur_x = -1;
	s_info_reposition.cur_y = -1;
	s_info_reposition.g_item = NULL;
	s_info_reposition.item = NULL;
	s_info_reposition.is_created = false;
	s_info.is_item_longpressed = false;

	home_screen_print_tree();

	return ECORE_CALLBACK_CANCEL;
}

/*Gengrid Class item functions*/
static char *__livebox_all_pages_get_grid_text(void *data, Evas_Object *obj, const char *part)
{
	return NULL;
}

static Eina_Bool __livebox_all_pages_get_grid_state(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void __livebox_all_pages_del_grid(void *data, Evas_Object *obj)
{
}

static void __livebox_all_pages_hide_remove_button(Elm_Object_Item *it)
{
	Evas_Object *thumb_layout = NULL;
	Evas_Object *thumb = NULL;

	if (!it)
		LOGE("[INVALID_PARAM][it='%p']", it);

	thumb_layout = elm_object_item_part_content_get(it, GENGRID_ITEM_CONTENT);
	thumb = elm_object_part_content_get(thumb_layout, PART_GRID_ITEM_CONTENT);
	elm_object_signal_emit(thumb, SIGNAL_REMOVE_BUTTON_HIDE, SIGNAL_SOURCE);
}

static void __livebox_all_pages_realized_cb(void *data, Evas_Object *obj, void *ei)
{
	int g_count = 0;
	g_count = elm_gengrid_items_count(obj);

	if (g_count == 2)
		__livebox_all_pages_hide_remove_button(elm_gengrid_first_item_get(obj));
}

static void __livebox_all_pages_remove_button_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Elm_Object_Item *clicked_item = NULL;
	Evas_Object *clicked_page = NULL;
	Evas_Object *lvb = (Evas_Object *) data;

	if (!lvb) {
		LOGE("[FAILED][lvb='%p']", lvb);
		return;
	}

	int x = -1, y = -1;
	evas_object_geometry_get(lvb, &x, &y, NULL, NULL);

	clicked_item = elm_gengrid_at_xy_item_get(s_info.grid, x, y, NULL, NULL);
	if (!clicked_item) {
		LOGE("[FAILED][elm_gengrid_at_xy_item_get]");
		return;
	}

	clicked_page = __livebox_all_pages_get_thumb_page(clicked_item);
	if (!clicked_page) {
		LOGE("[FAILED][__livebox_all_pages_get_thumb_page]");
		return;
	}
	if (__livebox_all_pages_is_page_empty(clicked_page)) {
		__livebox_all_pages_remove_button_clicked_liveboxes_cb(clicked_item);
	} else {
		popup_data_t *p_data = popup_create_data();
		if (!p_data) {
			LOGE("[FAILED][failed to create popup data]");
			return;
		}

		p_data->type = POPUP_TWO_BUTTONS;
		p_data->popup_title_text = strdup(_("IDS_HS_HEADER_DELETE_PAGE_ABB2"));
		p_data->popup_content_text = strdup(_("IDS_HS_POP_THIS_PAGE_AND_ALL_THE_ITEMS_IT_CONTAINS_WILL_BE_DELETED"));
		p_data->popup_confirm_text = strdup(_("IDS_HS_OPT_DELETE"));
		p_data->popup_cancel_text = strdup(_("IDS_CAM_SK_CANCEL"));
		p_data->orientation = ELM_POPUP_ORIENT_CENTER;

		popup_show(p_data, __livebox_all_pages_remove_button_clicked_liveboxes_cb, NULL,
					(void *) clicked_item);
	}
}

static void __livebox_all_pages_page_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *lvb_scroller = NULL;

	if (s_info.is_item_longpressed)
		return;

	lvb_scroller = livebox_panel_get();
	if (!lvb_scroller) {
		LOGE("Failed to get scroller");
		return;
	}

	livebox_all_pages_hide();
	home_screen_set_view_type(HOMESCREEN_VIEW_HOME);
	s_info.pressed_item = -1;
}

static void __libebox_all_pages_grid_item_pressed_cb(void *data, Evas_Object *obj, void *ei)
{
	s_info.pressed_item = elm_gengrid_item_index_get(ei);
}

