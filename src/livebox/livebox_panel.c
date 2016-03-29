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

#include <Elementary.h>

#include "util.h"
#include "homescreen-efl.h"
#include "page_scroller.h"
#include "option_menu.h"
#include "dynamic_index.h"
#include "data_model.h"

#include "layout.h"
#include "popup.h"
#include "layout.h"
#include "conf.h"

#include "livebox/livebox_panel.h"
#include "livebox/livebox_animator.h"
#include "livebox/grid_item_resize.h"
#include "livebox/grid_reposition.h"
#include "livebox/livebox_utils.h"
#include "livebox/livebox_widget.h"

static Evas_Object *__livebox_panel_create_page(Evas_Object *livebox_scroller);
static void __livebox_panel_freeze_scroller(void);
static bool __livebox_panel_is_page_empty(Evas_Object *page);

static void __livebox_panel_register_longpress_callbacks(Evas_Object *livebox_scroller,
	Evas_Object *page_layout, Evas_Object *livebox);
static void __livebox_panel_scrolled_cb(void *data, Evas_Object *obj,
	void *event_info);
static void __livebox_panel_page_changed_cb(void *data, Evas_Object *obj,
	void *event_info);
static void __livebox_panel_mouse_down_cb(void *data, Evas *evas, Evas_Object *object,
	void *event_info);
static void __livebox_panel_mouse_up_cb(void *data, Evas *evas, Evas_Object *object,
	void *event_info);
static Eina_Bool __livebox_panel_longpress_cb(void *data);

static void __livebox_panel_update_edit_mode_layout(Evas_Object *livebox_scroller_layout);
static void __livebox_panel_add_page_clicked_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source);

static void __libebox_panel_toggle_selected_livebox(Evas_Object *livebox, Evas_Object *page);
static void __livebox_panel_set_remove_buttons_visiblity(Evas_Object *page, bool visible);
static void __livebox_panel_set_reposition_bg_visibility(bool visible);
static void __livebox_panel_del_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source);
static Evas_Object *__livebox_panel_get_widget_layout(Evas_Object *obj);

static void __livebox_panel_scroll_start_cb(void *data, Evas_Object *obj, void *info);
static Eina_Bool __livebox_panel_change_page_timer_start_cb(void *data);

static void __livebox_panel_page_count_changed_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source);
static void __livebox_panel_set_remove_button_visibility(Evas_Object *livebox,
	bool visible);
static void __livebox_panel_hide_remove_button(Evas_Object *livebox);

static bool __livebox_panel_iterate_page_fill_cb(Tree_node_t *parent,
	Tree_node_t *page_node, void *data);
static bool __livebox_panel_iterate_node_fill_cb(Tree_node_t *page_node,
	Tree_node_t *node, void *data);

static void __livebox_panel_show_warning_popup(void);
static bool __livebox_panel_find_empty_space(Evas_Object *page,
	int span_w, int span_h, int *pos_x, int *pos_y);

static int __livebox_panel_get_mouse_move_distance_square(int x, int y);

static struct {
	Ecore_Timer *longpress_timer;
	int origination_page_num;
	Ecore_Timer *page_change_timer;
	int page_change;
} livebox_reposition_info_s = {
	.longpress_timer = NULL,
	.origination_page_num = 0,
	.page_change_timer = NULL,
	.page_change = LIVEBOX_PAGE_NO_CHANGE,
};


static struct {
	Evas_Object *livebox_page_scroller;
	Evas_Object *livebox_page_box;
	Evas_Object *add_page;
	dynamic_index_t *dynamic_index;

	Eina_List *livebox_page_list;

	int pages_count;
	int current_page;

	bool edit_mode;

	bool livebox_remove_button_state;

	Eina_Bool is_resizing;
	Eina_Bool is_longpress;

	Evas_Object *item_moved;
	Eina_Rectangle no_move_pos;
	Evas_Point mouse_press_pos;

} s_info = {
	.livebox_page_scroller = NULL,
	.livebox_page_box = NULL,
	.add_page = NULL,
	.dynamic_index = NULL,

	.livebox_page_list = NULL,

	.pages_count = 0,
	.current_page = 0,

	.edit_mode = false,

	.livebox_remove_button_state = false,
	.is_resizing = false,

	.is_longpress = false,

	.item_moved = NULL,
	.no_move_pos.x = 0,
	.no_move_pos.y = 0,
	.no_move_pos.w = 0,
	.no_move_pos.h = 0,

	.mouse_press_pos = {-1, -1},
};

void livebox_panel_update_dynamic_index(void)
{
	if (!s_info.livebox_page_scroller) {
		LOGE("s_info.livebox_page_scroller == NULL");
		return;
	}

	elm_object_signal_emit(s_info.livebox_page_scroller,
		SIGNAL_SCROLLER_PAGE_COUNT_CHANGED, SIGNAL_SOURCE);
}

void livebox_panel_create_dynamic_index(void)
{
	if (!s_info.livebox_page_scroller) {
		LOGE("s_info.livebox_page_scroller == NULL");
		return;
	}


	s_info.dynamic_index = dynamic_index_new(s_info.livebox_page_scroller);
	if (!s_info.dynamic_index) {
		LOGE("s_info.dynamic_index == NULL");
		return;
	}
}

void livebox_panel_fill(void)
{
	Tree_node_t *home = NULL;
	int widget_pos_h = 0;

	home = data_model_get_home();
	if (!home) {
		LOGE("home == NULL");
		return;
	}

	if (home->count == 0) {
		LOGW("home->count == NULL");
		return;
	} else {
		LOGI("home->count == %d", home->count);
	}

	data_model_iterate_pages(home, __livebox_panel_iterate_page_fill_cb,
		__livebox_panel_iterate_node_fill_cb, &widget_pos_h);
	livebox_panel_create_dynamic_index();
}

Evas_Object *livebox_panel_create(Evas_Object *parent)
{
	Evas_Object *liveboxes = NULL;

	if (!parent) {
		LOGE("parent == NULL");
		return NULL;
	}

	liveboxes = livebox_panel_create_scroller();
	if (!liveboxes) {
		LOGE("liveboxes == NULL");
		return NULL;
	}

	livebox_widget_init();

	return liveboxes;
}

void livebox_panel_set_content(Evas_Object *layout)
{
	if(s_info.dynamic_index->box)
		elm_layout_content_set(layout, PART_INDEX, s_info.dynamic_index->box);
}

void livebox_panel_unset_content(Evas_Object *layout)
{
	Evas_Object *dynamic_index = NULL;

	dynamic_index = elm_object_part_content_unset(layout, PART_INDEX);

	if (!dynamic_index) {
		LOGE("dyamic_index == NULL");
		return;
	}

	evas_object_hide(dynamic_index);
}


Evas_Object *livebox_scroller_get_page(void)
{
	int current_page_number = page_scroller_get_current_page(
		s_info.livebox_page_scroller);
	return page_scroller_get_page(s_info.livebox_page_scroller,
		current_page_number);
}

void livebox_panel_refresh_content_size(void)
{
	Evas_Object *page_container = NULL;
	Evas_Object *item = NULL;
	Eina_List *it = NULL;
	Eina_List *items_in_grid = NULL;
	Evas_Coord container_x = -1;
	Evas_Coord container_y = -1;
	Evas_Coord container_width = -1;
	Evas_Coord container_height = -1;
	Evas_Coord x = -1, y = -1, w = -1, h = -1;

	if (!s_info.livebox_page_scroller) {
		return;
	}

	page_container = page_scroller_get_page(
		s_info.livebox_page_scroller,
		page_scroller_get_current_page(s_info.livebox_page_scroller));

	if (!page_container) {
		LOGE("page_container == NULL");
		return;
	}

	evas_object_geometry_get(s_info.livebox_page_scroller,
		&container_x, &container_y, &container_width,
		&container_height);

	items_in_grid = livebox_utils_get_liveboxes_on_page_list(
		page_container);

	EINA_LIST_FOREACH(items_in_grid, it, item) {
		if (!item) {
			LOGE("item == NULL");
			continue;
		}

		elm_grid_pack_get(item, &x, &y, &w, &h);
		evas_object_move(item,
			(x*(container_width-LIVEBOX_HORIZONTAL_PADDING) / 100) +
			LIVEBOX_HORIZONTAL_PADDING / 2 + container_x,
			container_height * y / 100 + container_y);
		evas_object_resize(item,
			(int)((float) w/100 * (container_width)) -
			(w * LIVEBOX_HORIZONTAL_PADDING)/100,
			(int)((float)h/100 * (container_height)) -
			(y*LIVEBOX_HORIZONTAL_PADDING)/100);
	}
}

Evas_Object *livebox_panel_create_scroller(void)
{
	Evas_Object *livebox_page_scroller = NULL;
	Evas_Object *win = home_screen_get_win();
	Evas_Object *box = NULL;
	Eina_List   *lvb_page_list = NULL;

	int p_count = 0;
	int cur_page = 0;

	if (!win) {
		LOGE("Window is NULL");
		return NULL;
	}

	livebox_page_scroller =
		page_scroller_create(home_screen_get_root_width(),
		LIVEBOX_PAGE_HEIGHT_REL*home_screen_get_root_height());
	if (!livebox_page_scroller) {
		LOGE("Cannot create livebox page scroller");
		return NULL;
	}

	box = elm_object_content_get(livebox_page_scroller);
	if (!box) {
		LOGE("Cannot get the box");
		evas_object_del(livebox_page_scroller);
		return NULL;
	}

	livebox_utils_create_shadow(livebox_page_scroller);
	evas_object_smart_callback_add(livebox_page_scroller,
		"scroll,drag,start", __livebox_panel_scroll_start_cb, NULL);
	elm_object_signal_callback_add(livebox_page_scroller,
		SIGNAL_SCROLLER_PAGE_COUNT_CHANGED, SIGNAL_LAYOUT_SOURCE,
		__livebox_panel_page_count_changed_cb, NULL);

	/*After creation empty one empty space should be created.*/

	s_info.livebox_page_scroller = livebox_page_scroller;
	s_info.current_page = cur_page;
	s_info.pages_count = p_count;
	s_info.livebox_page_list = lvb_page_list;

	return livebox_page_scroller;
}


void livebox_panel_destroy(void)
{
	Eina_List *pages = s_info.livebox_page_list;

	Evas_Object *livebox_scroller_page = NULL;

	if (!pages) {
		LOGE("Pages pointer is NULL");
		return;
	}

	EINA_LIST_FREE(pages, livebox_scroller_page) {
		livebox_panel_destroy_page(livebox_scroller_page);
	}

	page_scroller_destroy(s_info.livebox_page_scroller);

	livebox_widget_fini();
}

void livebox_panel_paused(void)
{
	livebox_widget_viewer_paused();
}

void livebox_panel_resumed(void)
{
	livebox_widget_viewer_resumed();
}

void livebox_panel_add_livebox(Tree_node_t *node, Evas_Object *page,
	const char *livebox_pkgname, int pos_x, int pos_y,
	int width, int height,
	char *content_info)
{
	Evas_Object *livebox = NULL;
	Evas_Object *livebox_layout = NULL;
	Evas_Object *grid = NULL;
	int ret = 0;


	livebox_layout = elm_layout_add(s_info.livebox_page_scroller);
	if (!livebox_layout) {
		evas_object_del(page);
		LOGE("livebox_layout is not created");
		return;
	}

	if (!node) {
		LOGE("node == NULL");
		return;
	}

#ifdef LIVEBOX_RESIZE_TEST
	livebox = elm_image_add(livebox_layout);
#else
	livebox = livebox_widget_add(livebox_pkgname, livebox_layout, content_info);
#endif

	if (!livebox) {
		LOGE("Livebox is not created");
		return;
	}

	node->data->layout = livebox_layout;
	evas_object_data_set(livebox_layout, KEY_ICON_DATA, node);

	ret = elm_layout_file_set(livebox_layout, util_get_res_file_path(EDJE_LIVEBOX_LAYOUT_FILENAME),
		GROUP_LIVEBOX_LAYOUT);
	if (ret != EINA_TRUE) {
		LOGE("Can not set layout file");
		return;
	}

	ret = elm_layout_content_set(livebox_layout, PART_LIVEBOX, livebox);
	if (ret != EINA_TRUE) {
		LOGE("Can not set layout file");
		return;
	}

	evas_object_size_hint_weight_set(livebox_layout, EVAS_HINT_EXPAND,
		EVAS_HINT_EXPAND);

	evas_object_show(livebox);
	evas_object_show(livebox_layout);

	elm_layout_signal_callback_add(livebox_layout, SIGNAL_CLICKED,
		SIGNAL_REMOVE_SOURCE, __livebox_panel_del_cb, NULL);

	grid = elm_object_part_content_get(page, SIGNAL_CONTENT_SOURCE);
	if (!grid) {
		LOGE("grid == NULL");
		return;
	}

	elm_grid_pack(grid,
		livebox_layout, pos_x * LIVEBOX_GRID_ROWS_CELLS_MULTI,
		pos_y * LIVEBOX_GRID_ROWS_CELLS_MULTI,
		width * LIVEBOX_GRID_ROWS_CELLS_MULTI,
		height * LIVEBOX_GRID_ROWS_CELLS_MULTI);

	__livebox_panel_register_longpress_callbacks(NULL, page, livebox_layout);

	LOGD("Adding [%s] at: [%d, %d] <>[%d, %d]",
		livebox_pkgname, pos_x, pos_y, width, height);
}


Evas_Object *livebox_panel_add_page(Tree_node_t *node,
	Evas_Object *livebox_scroller)
{
	Evas_Object *page = NULL;

	page = livebox_panel_create_empty_page(node, livebox_scroller);

	if (!page) {
		LOGE("page == NULL");
		return NULL;
	}

	if (!page_scroller_add_page(livebox_scroller, page)) {
		LOGE("Can not create page");

		livebox_panel_destroy_page(page);
		return NULL;
	}

	/*update pages list*/
	s_info.livebox_page_list = page_scroller_get_all_pages(
		livebox_scroller);
	__livebox_panel_freeze_scroller();

	return page;
}

bool livebox_panel_add_empty_page(Evas_Object *livebox_scroller)
{
	/*@todo add check if page count will not be over the limit*/
	Evas_Object *page = __livebox_panel_create_page(livebox_scroller);
	return page_scroller_add_page(livebox_scroller, page);
}


Evas_Object *livebox_panel_create_empty_page(Tree_node_t *node,
	Evas_Object *livebox_scroller)
{
	Evas_Object *page = NULL;

	if (!livebox_scroller) {
		LOGE("Livebox panel is NULL");
		return NULL;
	}

	if (!node) {
		LOGE("node == NULL");
		return NULL;
	}

	page = __livebox_panel_create_page(livebox_scroller);
	if (!page) {
		LOGE("page == NULL");
		return NULL;
	}

	evas_object_data_set(page, KEY_ICON_DATA, node);

	return page;
}


void livebox_panel_change_edit_mode_state(bool close)
{
	Evas_Object *livebox_scroller = s_info.livebox_page_scroller;
	if (!livebox_scroller) {
		LOGE("Livebox panel is NULL");
		return;
	}

	if (!s_info.edit_mode && close) {
		/*if edit mode is disabled closing doesn't take any effect*/
		return;
	}

	if (s_info.edit_mode && close) {
		s_info.edit_mode = false;
		/*closing edit mode*/
		__livebox_panel_update_edit_mode_layout(livebox_scroller);
	} else if (!s_info.edit_mode) {
		s_info.edit_mode = true;
		__livebox_panel_update_edit_mode_layout(livebox_scroller);
	}
}

void livebox_panel_destroy_page(Evas_Object *livebox_scroller_page)
{
	Eina_List *lb_layouts_list = NULL;
	Eina_List *l               = NULL;
	Evas_Object *lb_layout     = NULL;
	Evas_Object *widget        = NULL;
	Tree_node_t *page_node = NULL;

	if (!livebox_scroller_page) {
		LOGE("Livebox panel is NULL");
		return;
	}

	page_node = evas_object_data_get(livebox_scroller_page, KEY_ICON_DATA);
	if (!page_node) {
		LOGE("page_node == NULL");
		return;
	}

	data_model_del_item(page_node);

	lb_layouts_list = livebox_utils_get_liveboxes_on_page_list(
		livebox_scroller_page);
	if (!lb_layouts_list) {
		LOGE("pages == NULL");
		evas_object_del(livebox_scroller_page);
		home_screen_print_tree();
		return;
	}

	EINA_LIST_FOREACH(lb_layouts_list, l, lb_layout) {
		widget = __livebox_panel_get_widget_layout(lb_layout);
		if (!widget) {
			LOGE("widget == NULL");
			continue;
		}

		livebox_widget_del(widget);
		evas_object_del(lb_layout);
	}

	eina_list_free(lb_layouts_list);
	evas_object_del(livebox_scroller_page);

	__livebox_panel_freeze_scroller();
	home_screen_print_tree();
}

bool livebox_panel_is_add_page_selected(void)
{
	int current_page_number = page_scroller_get_current_page(
		s_info.livebox_page_scroller);
	Evas_Object *page_container = page_scroller_get_page(
		s_info.livebox_page_scroller, current_page_number);

	return (page_container == s_info.add_page &&
		livebox_panel_check_if_add_page_is_visible());
}

void livebox_panel_move_mouse_cb(int x, int y)
{
	int mouse_move_distance = __livebox_panel_get_mouse_move_distance_square(x, y);

	if (mouse_move_distance > LIVEBOX_MIN_MOUSE_MOVE) {
		ecore_timer_del(livebox_reposition_info_s.longpress_timer);
		livebox_reposition_info_s.longpress_timer = NULL;

		if(!s_info.is_longpress)
			page_scroller_unfreeze(s_info.livebox_page_scroller);
	}

	if (s_info.is_longpress &&
		!livebox_reposition_info_s.page_change_timer) {
		if (x < LIVEBOX_PAGE_CHANGE_RECT_SIZE) {
			livebox_reposition_info_s.page_change_timer =
			ecore_timer_add(LIVEBOX_PAGE_CHANGE_PERIOD,
				__livebox_panel_change_page_timer_start_cb, (void *)false);
			LOGI("PAGE CHANGE TIMER CREATED <LEFT>");
		} else if (x > home_screen_get_root_width() -
			LIVEBOX_PAGE_CHANGE_RECT_SIZE) {
			livebox_reposition_info_s.page_change_timer =
				ecore_timer_add(LIVEBOX_PAGE_CHANGE_PERIOD,
					__livebox_panel_change_page_timer_start_cb, (void *)true);
			LOGI("PAGE CHANGE TIMER CREATED <RIGHT>");
		}

		grid_reposition_move(x, y);
	} else if (livebox_reposition_info_s.page_change_timer &&
			x > LIVEBOX_PAGE_CHANGE_RECT_SIZE &&
			x < home_screen_get_root_width() -
			LIVEBOX_PAGE_CHANGE_RECT_SIZE) {
		ecore_timer_del(livebox_reposition_info_s.page_change_timer);
		livebox_reposition_info_s.page_change_timer = NULL;
		LOGI("PAGE CHANGE TIMER DESTROYED");

		grid_reposition_move(x, y);
	}
}

Eina_List *livebox_panel_get_pages(void)
{
	return s_info.livebox_page_list;
}

Evas_Object *livebox_panel_get(void)
{
	return s_info.livebox_page_scroller;
}

Evas_Object *livebox_panel_get_index(void)
{
	if (!s_info.dynamic_index) {
		LOGE("s_info.dynamic_index == NULL");
		return NULL;
	}

	if (!s_info.dynamic_index->box) {
		LOGE("s_info.dynamic_index->box == NULL");
		return NULL;
	}

	return s_info.dynamic_index->box;
}

bool livebox_panel_check_if_add_page_is_visible(void)
{
	Evas_Object *lvb_scorller = NULL;
	Evas_Object *page = NULL;
	Evas_Object *page_edje = NULL;

	int p_count = -1;
	const char *state = NULL;

	lvb_scorller = s_info.livebox_page_scroller;
	if (!lvb_scorller) {
		LOGE("Faield to get livebox scroller");
		return false;
	}

	p_count = page_scroller_get_page_count(lvb_scorller);
	if (p_count < 0) {
		LOGE("Failed to get page count");
		return false;
	}

	LOGD("p_count is %d ", p_count);

	page = page_scroller_get_page(lvb_scorller, --p_count);
	if (!page) {
		LOGE("Failed to get page");
		return false;
	}

	page_edje = elm_layout_edje_get(page);
	if (!page_edje) {
		LOGE("Failed to get page edje");
		return false;
	}

	state = edje_object_part_state_get(page_edje,
		PART_ADD_PAGE_BUTTON, NULL);

	if (!state) {
		LOGE("Failed to get state");
		return false;
	}

	LOGD("state is : %s ", state);

	return strncmp(state, "default", LIVEBOX_MAX_STATE_LEN);
}

void livebox_panel_set_edit_mode_layout(bool turn_on)
{
	Evas_Object *lvb_scorller = NULL;
	Evas_Object *page = NULL;
	char *signal_to_send = NULL;
	Eina_List *all_pages = NULL;
	Eina_List *it = NULL;

	lvb_scorller = s_info.livebox_page_scroller;
	if (!lvb_scorller) {
		LOGE("Failed to get livebox scroller");
		return;
	}

	all_pages = page_scroller_get_all_pages(lvb_scorller);
	if (!all_pages) {
		LOGE("Failed to get page");
		return;
	}

	if (turn_on) {
		signal_to_send = SIGNAL_EDIT_START;
	} else {
		signal_to_send = SIGNAL_EDIT_STOP;
	}

	EINA_LIST_FOREACH(all_pages, it, page) {
		if (!page) {
			LOGE("page == NULL");
			continue;
		}

		elm_object_signal_emit(page, signal_to_send,
			SIGNAL_LAYOUT_SOURCE);
	}
}

void livebox_panel_add_page_visible_state_set(bool visible)
{
	Evas_Object *page = NULL;

	page = livebox_scroller_get_last_page();
	if (!page) {
		LOGE("page == NULL");
		return;
	}

	if (visible) {
		s_info.add_page = page;

		elm_object_signal_emit(page, SIGNAL_ADD_SHOW,
			SIGNAL_LAYOUT_SOURCE);
		elm_object_signal_emit(page, SIGNAL_EDIT_START,
			SIGNAL_LAYOUT_SOURCE);
	} else {
		elm_object_signal_emit(page, SIGNAL_ADD_HIDE,
			SIGNAL_LAYOUT_SOURCE);
	}
}

bool livebox_panel_can_add_page(void)
{
	int count = 0;
	Eina_List *pages = page_scroller_get_all_pages(livebox_panel_get());
	count = eina_list_count(pages);


	if (count < LIVEBOX_SCROLLER_MAX_PAGES) {
		return true;
	} else if (count == LIVEBOX_SCROLLER_MAX_PAGES &&
		livebox_panel_check_if_add_page_is_visible()) {
		return true;
	}

	__livebox_panel_show_warning_popup();
	return false;

}

int livebox_panel_find_empty_space(int span_w, int span_h,
	int *pos_x, int *pos_y)
{
	Evas_Object *current_page = NULL;
	Evas_Object *page = NULL;
	Eina_List *l = NULL;
	int page_index = -1;

	Eina_List *pages = page_scroller_get_all_pages(livebox_panel_get());
	if (!pages) {
		LOGE("pages == NULL");
		return -1;
	}


	current_page = livebox_scroller_get_page();
	if (!current_page) {
		LOGE("current_page == NULL");
		eina_list_free(pages);
		return -1;
	}

	if (__livebox_panel_find_empty_space(current_page,
		span_w, span_h, pos_x, pos_y)) {
		eina_list_free(pages);
		return page_scroller_get_current_page(livebox_panel_get());
	}

	EINA_LIST_FOREACH(pages, l, page) {
		++page_index;

		if (!page) {
			LOGE("page == NULL");
			continue;
		}

		if (page == current_page)
			continue;

		if (__livebox_panel_find_empty_space(page, span_w, span_h,
			pos_x, pos_y)) {
			eina_list_free(pages);
			return page_index;
		}
	}

	eina_list_free(pages);
	return -1;
}

static int __livebox_panel_get_mouse_move_distance_square(int x, int y)
{
	int dx = x - s_info.mouse_press_pos.x;
	int dy = y - s_info.mouse_press_pos.y;

	return (dx * dx) + (dy * dy);
}

static bool __livebox_panel_find_empty_space(Evas_Object *page, int span_w,
	int span_h, int *pos_x, int *pos_y)
{
	Evas_Object *livebox;
	Eina_List *l;
	Eina_List *livebox_list = NULL;
	Eina_Rectangle *grid_item_rec =  NULL;
	int y = 0;
	bool ret = true;

	livebox_list = livebox_utils_get_liveboxes_on_page_list(page);
	if (!livebox_list) {
		LOGE("livebox_list == NULL");
		*pos_y = 0;
		*pos_x = 0;
		return true;
	}

	if (eina_list_count(livebox_list) == 0) {
		*pos_y = 0;
		*pos_x = 0;
		eina_list_free(livebox_list);
		return true;
	}

	Eina_Rectangle *moved_item_rec = eina_rectangle_new(0, 0,
		span_w, span_h);
	if (!moved_item_rec) {
		LOGE("moved_item_rec == NULL");
		eina_list_free(livebox_list);
		return false;
	}

	for (y = 0; y + span_h <= LIVEBOX_GRID_ROWS; ++y) {
		moved_item_rec->y = y;

		EINA_LIST_FOREACH(livebox_list, l, livebox) {
			grid_item_rec = livebox_utils_get_grid_widget_rectangle(
				livebox);
			if (!grid_item_rec) {
				LOGE("grid_item_rec == NULL");
				eina_rectangle_free(grid_item_rec);
				continue;
			}

			if (eina_rectangle_intersection(moved_item_rec,
				grid_item_rec)) {
				ret = false;
				eina_rectangle_free(grid_item_rec);
				break;
			} else {
				*pos_y = y;
				*pos_x = 0;
				ret = true;
			}

			eina_rectangle_free(grid_item_rec);
		}

		if (ret) {
			eina_list_free(livebox_list);
			eina_rectangle_free(moved_item_rec);
			return true;
		}
	}

	eina_rectangle_free(moved_item_rec);
	eina_list_free(livebox_list);
	return false;
}

static void __livebox_panel_show_warning_popup(void)
{
	popup_data_t *data = popup_create_data();
	if (!data) {
		LOGE("data == NULL");
		return;
	}

	data->orientation        = ELM_POPUP_ORIENT_CENTER;
	data->popup_cancel_text  = NULL;
	data->popup_confirm_text = NULL;
	data->popup_content_text = strdup(_("IDS_HS_POP_THERE_IS_NOT_ENOUGH_SPACE_ON_THE_HOME_SCREEN_REMOVE_SOME_WIDGETS_AND_TRY_AGAIN"));
	data->popup_title_text   = NULL;
	data->type               = POPUP_INFO;
	data->visible_time       = 5.0;

	popup_show(data, NULL, NULL, NULL);
}

static void __livebox_panel_freeze_scroller(void)
{

	if (page_scroller_get_page_count(s_info.livebox_page_scroller) > 1) {
		page_scroller_unfreeze(s_info.livebox_page_scroller);
	} else {
		page_scroller_freeze(s_info.livebox_page_scroller);
	}
}

static void __livebox_panel_page_count_changed_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source)
{
	__livebox_panel_freeze_scroller();
}


static Evas_Object *__livebox_panel_create_page(Evas_Object *livebox_scroller)
{
	Evas_Object *grid = NULL;
	Evas_Object *rect = NULL;

	Evas_Object *livebox_container = util_create_edje_layout(
		livebox_scroller, util_get_res_file_path(EDJE_LIVEBOX_CONTAINER_FILENAME),
		GROUP_LIVEBOX_CONTAINER);

	if (!livebox_container) {
		LOGE("Cannot create scroller layout");
		return NULL;
	}

	grid = elm_grid_add(livebox_container);
	if (!grid) {
		LOGE("grid == NULL");
		evas_object_del(livebox_container);
		return NULL;
	}

	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND,
		EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL,
		EVAS_HINT_FILL);
	elm_grid_size_set(grid, LIVEBOX_TOTAL_COLUMNS, LIVEBOX_TOTAL_ROWS);

	elm_object_part_content_set(livebox_container, SIGNAL_CONTENT_SOURCE,
		grid);
	livebox_utils_create_grid_shadow_clipper(livebox_container);

	rect = evas_object_rectangle_add(evas_object_evas_get(
		livebox_container));
	if (!rect) {
		LOGE("rect == NULL");
		evas_object_del(grid);
		evas_object_del(livebox_container);
		return NULL;
	}


	evas_object_color_set(rect, 0, 0, 0, 0);
	evas_object_size_hint_min_set(rect, home_screen_get_root_width(),
		LIVEBOX_SIZE_REL * home_screen_get_root_height() -
			(2.0*LIVEBOX_X_MIN * home_screen_get_root_width()));
	evas_object_size_hint_max_set(rect, home_screen_get_root_width(),
		LIVEBOX_SIZE_REL * home_screen_get_root_height() -
			(2.0*LIVEBOX_X_MIN * home_screen_get_root_width()));

	elm_object_part_content_set(livebox_container, PART_PAGE_BG, rect);

	elm_object_signal_emit(livebox_container, SIGNAL_RESIZE_SLIDERS_HIDE,
		SIGNAL_LIVEBOX_SOURCE);
	elm_object_signal_callback_add(livebox_container,
		SIGNAL_ADD_PAGE_CLICKED, SIGNAL_LAYOUT_SOURCE,
		__livebox_panel_add_page_clicked_cb, NULL);

	grid_item_init_resize_callbacks(livebox_scroller, livebox_container);

	return livebox_container;
}


static void __livebox_panel_update_edit_mode_layout(Evas_Object *livebox_scroller_scroller)
{
	Eina_List *livebox_container_list;
	Eina_List *node;
	Evas_Object *livebox_container = NULL;

	if (!livebox_scroller_scroller) {
		LOGE("Livebox panel layout is NULL");
		return;
	}

	livebox_container_list = page_scroller_get_all_pages(
		livebox_scroller_scroller);
	if (s_info.edit_mode) {
		EINA_LIST_FOREACH(livebox_container_list, node,
			livebox_container)
		{
			if (!livebox_container) {
				LOGE("livebox_container == NULL");
				continue;
			}

			if (!__livebox_panel_is_page_empty(livebox_container)) {
				__livebox_panel_set_remove_buttons_visiblity(
					livebox_container, true);
			} else {
				elm_object_signal_emit(livebox_container,
					SIGNAL_EMPTY_PAGE_EDIT_START,
					SIGNAL_LIVEBOX_SOURCE);
			}
			elm_object_signal_emit(livebox_container,
				SIGNAL_EDIT_START, SIGNAL_LIVEBOX_SOURCE);
		}
	} else {
		EINA_LIST_FOREACH(livebox_container_list, node,
			livebox_container) {
			if (!livebox_container) {
				LOGE("livebox_container == NULL");
				continue;
			}

			__livebox_panel_set_remove_buttons_visiblity(livebox_container,
				false);

			elm_object_signal_emit(livebox_container,
				SIGNAL_EMPTY_PAGE_EDIT_STOP,
				SIGNAL_LIVEBOX_SOURCE);
			elm_object_signal_emit(livebox_container,
				SIGNAL_EDIT_STOP, SIGNAL_LIVEBOX_SOURCE);

			elm_object_signal_emit(livebox_container,
			 SIGNAL_RESIZE_SLIDERS_HIDE, SIGNAL_LIVEBOX_SOURCE);
		}
	}

	eina_list_free(livebox_container_list);
}

static bool __livebox_panel_is_page_empty(Evas_Object *page)
{
	Eina_List *lb_list = NULL;

	if (!page) {
		LOGE("page == NULL");
		return true;
	}

	if (page == s_info.add_page &&
		livebox_panel_check_if_add_page_is_visible()) {
		return false;
	}

	lb_list = livebox_utils_get_liveboxes_on_page_list(page);
	if (!lb_list) {
		LOGD("lb_list == NULL");
		return true;
	}

	if (eina_list_count(lb_list) > 0) {
		return false;
	}

	return true;
}


static void _livebox_remove_button_mouse_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source)
{
	LOGD("rm btn %s", emission);

	if (!obj) {
		LOGE("obj == NULL");
		return;
	}

	s_info.livebox_remove_button_state = (bool) strstr(emission,
		"mouse,down");
}

static void __livebox_panel_register_longpress_callbacks(Evas_Object *livebox_scroller,
	Evas_Object *page_layout, Evas_Object *livebox)
{
	Evas_Object *scroller = s_info.livebox_page_scroller;

	evas_object_event_callback_add(livebox, EVAS_CALLBACK_MOUSE_DOWN,
		__livebox_panel_mouse_down_cb, page_layout);
	evas_object_event_callback_add(livebox, EVAS_CALLBACK_MOUSE_UP,
		__livebox_panel_mouse_up_cb, livebox_scroller);
	elm_layout_signal_callback_add(livebox, SIGNAL_MOUSE_DOWN,
		SIGNAL_REMOVE_SOURCE, _livebox_remove_button_mouse_cb, NULL);
	elm_layout_signal_callback_add(livebox, SIGNAL_MOUSE_UP,
		SIGNAL_REMOVE_SOURCE, _livebox_remove_button_mouse_cb, NULL);

	if (scroller) {
		/*this callback should abort longpress*/
		evas_object_smart_callback_add(scroller,
			SIGNAL_SCROLLER_DRAG_START, __livebox_panel_scrolled_cb,
			NULL);
		evas_object_smart_callback_add(scroller,
			SIGNAL_SCROLLER_SCROLL, __livebox_panel_page_changed_cb,
			NULL);
	}
}


static void __livebox_panel_mouse_down_cb(void *data, Evas *evas,
	Evas_Object *object, void *event_info)
{
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;
	int current_page_number = page_scroller_get_current_page(
		s_info.livebox_page_scroller);
	Evas_Object *page_layout = page_scroller_get_page(
		s_info.livebox_page_scroller, current_page_number);
	int obj_x = 0, obj_y = 0, obj_w = 0, obj_h = 0;
	Evas_Object *widget = NULL;

	if (!ev || !page_layout) {
		LOGE("Invalid event info");
		return;
	}

	page_scroller_freeze(s_info.livebox_page_scroller);

	s_info.mouse_press_pos.x = ev->output.x;
	s_info.mouse_press_pos.y = ev->output.y;

	evas_object_geometry_get(object, &obj_x, &obj_y, &obj_w, &obj_h);

	if (s_info.edit_mode) {

		widget = elm_layout_content_get(object, PART_LIVEBOX);
		if (!widget) {
			LOGE("widget == NULL");
			return;
		}

		livebox_widget_set_event_blocking(widget);
	}

	if (livebox_reposition_info_s.longpress_timer) {
		LOGD("Longpress timer existed - this should not happen");
		ecore_timer_del(livebox_reposition_info_s.longpress_timer);
		livebox_reposition_info_s.longpress_timer = NULL;
	}

	if (!s_info.livebox_remove_button_state) {
		livebox_reposition_info_s.longpress_timer = ecore_timer_add(
			LIVEBOX_LONGPRESS_TIME, __livebox_panel_longpress_cb, object);
	} else {
		LOGD("Remove button pressed");
	}
}

static void __libebox_panel_toggle_selected_livebox(Evas_Object *livebox, Evas_Object *page)
{
	Evas_Object *prev_selected_page = NULL;

	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	if (!page) {
		LOGE("page == NULL");
		return;
	}

	livebox_utils_set_selected_livebox(livebox, page);
	prev_selected_page = livebox_utils_get_prev_livebox_layout();
	if (prev_selected_page && page != prev_selected_page) {
	/*page != prev_selected_page' when page is not change the sliders
	should not disappear */
		LOGD("Previous page found");
		elm_object_signal_emit(prev_selected_page,
			SIGNAL_RESIZE_SLIDERS_HIDE, SIGNAL_LIVEBOX_SOURCE);
		elm_object_signal_emit(prev_selected_page,
			SIGNAL_GRID_SHADOW_HIDE, PART_GRID_BG);
	}
}

static void __livebox_panel_mouse_up_cb(void *data, Evas *evas,
	Evas_Object *object, void *event_info)
{
	int current_page_number = page_scroller_get_current_page(
		s_info.livebox_page_scroller);
	Evas_Object *livebox_container = page_scroller_get_page(
		s_info.livebox_page_scroller, current_page_number);

	s_info.is_longpress = false;
	page_scroller_unfreeze(s_info.livebox_page_scroller);

	__livebox_panel_freeze_scroller();

	__livebox_panel_set_remove_button_visibility(
		grid_reposition_get_repositioned_item(), true);
	__livebox_panel_set_reposition_bg_visibility(false);
	elm_object_signal_emit(livebox_container, SIGNAL_GRID_SHADOW_HIDE, PART_GRID_BG);

	grid_reposition_end();
	livebox_utils_set_shadow_visibility(false);

	LOGD("PAGE CHANGE TIMER DEL: %p",
		livebox_reposition_info_s.page_change_timer);
	ecore_timer_del(livebox_reposition_info_s.page_change_timer);
	livebox_reposition_info_s.page_change_timer = NULL;

	if (livebox_reposition_info_s.longpress_timer) {
		LOGD("NOT LONGPRESS");
		ecore_timer_del(livebox_reposition_info_s.longpress_timer);

		if (home_screen_get_view_type() != HOMESCREEN_VIEW_HOME_EDIT) {
			LOGD("No edit mode exiting...");
			return;
		}

		livebox_reposition_info_s.longpress_timer = NULL;
		__libebox_panel_toggle_selected_livebox(object, livebox_container);
		grid_item_set_resize_livebox_sliders(object, livebox_container);
	}
}

static void __livebox_panel_scrolled_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	if (livebox_reposition_info_s.longpress_timer) {
		ecore_timer_del(livebox_reposition_info_s.longpress_timer);
		livebox_reposition_info_s.longpress_timer = NULL;
	}
}

static void __livebox_panel_page_changed_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	/*livebox_utils_set_shadow_visibility(false);*/
}


static Eina_Bool _longpress_timer_cb(void *data)
{
	grid_reposition_start();
	__livebox_panel_hide_remove_button(
		grid_reposition_get_repositioned_item());
	__livebox_panel_set_reposition_bg_visibility(true);

	return ECORE_CALLBACK_CANCEL;
}

/*@todo add check to proceed only if edit mode is ON*/
static Eina_Bool __livebox_panel_longpress_cb(void *data)
{
	LOGD("Longpress!");
	int current_page_number = page_scroller_get_current_page(
		s_info.livebox_page_scroller);
	Evas_Object *longpressed_page = page_scroller_get_page(
		s_info.livebox_page_scroller, current_page_number);

	livebox_reposition_info_s.longpress_timer = NULL;
	Evas_Object *livebox = (Evas_Object *) data;
	Evas_Object *widget = elm_layout_content_get(livebox, PART_LIVEBOX);

	if (!widget) {
		LOGE("widget == NULL");
		return EINA_FALSE;
	}
	livebox_widget_set_event_blocking(widget);

	if (!longpressed_page) {
		LOGE("No longpressed object stored");
		return EINA_FALSE;
	}

	if (s_info.is_resizing)
		return EINA_FALSE;


	if (s_info.livebox_remove_button_state)
		return EINA_FALSE;

	s_info.is_longpress = true;

	__libebox_panel_toggle_selected_livebox(livebox, longpressed_page);
	elm_object_signal_emit(longpressed_page, SIGNAL_GRID_SHADOW_SHOW, PART_GRID_BG);

	grid_item_set_resize_sliders_visibility(longpressed_page, false);
	grid_reposition_init(livebox_utils_get_livebox_container_grid(
		longpressed_page), livebox);
	page_scroller_freeze(s_info.livebox_page_scroller);

	if (home_screen_get_view_type() != HOMESCREEN_VIEW_HOME_EDIT) {
		home_screen_set_view_type(HOMESCREEN_VIEW_HOME_EDIT);
		livebox_panel_set_edit_mode_layout(true);
		livebox_panel_change_edit_mode_state(false);
		ecore_timer_add(0.1, _longpress_timer_cb, longpressed_page);
	} else {
		grid_reposition_start();
		__livebox_panel_hide_remove_button(livebox);
		__livebox_panel_set_reposition_bg_visibility(true);
	}

	return EINA_FALSE;
}

static void __livebox_panel_add_page_clicked_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source)
{
	Evas_Object *lvb_scorller = NULL;
	int page_count = -1;

	lvb_scorller = livebox_panel_get();
	if (!lvb_scorller) {
		LOGE("Failed to get scroller");
		return;
	}

	page_count = page_scroller_get_page_count(lvb_scorller);

	if (page_count < LIVEBOX_SCROLLER_MAX_PAGES) {
		/*remove add_page button from last page*/
		livebox_panel_add_page_visible_state_set(false);

		/*add new page*/
		livebox_panel_add_empty_page(lvb_scorller);

		/*show add button on last page*/
		livebox_panel_add_page_visible_state_set(true);
	} else {
		livebox_panel_add_page_visible_state_set(false);
		/*elm_object_signal_emit(dynamic_index->indices[5],
		EDJE_SIGNAL_ADD_PAGE_HIDE, EDJE_SIGNAL_LAYOUT_SOURCE);*/
	}

	elm_object_signal_emit(lvb_scorller, SIGNAL_SCROLLER_PAGE_COUNT_CHANGED,
			SIGNAL_LAYOUT_SOURCE);
}

Evas_Object *livebox_scroller_get_last_page(void)
{
	Evas_Object *lvb_scorller = NULL;
	Evas_Object *page = NULL;

	Eina_List *all_pages = NULL;
	int page_count = -1;

	lvb_scorller = s_info.livebox_page_scroller;
	if (!lvb_scorller) {
		LOGE("Failed to get livebox scroller");
		return NULL;
	}

	all_pages = page_scroller_get_all_pages(lvb_scorller);
	if (!all_pages) {
		LOGE("Failed to get page");
		return NULL;
	}

	page_count = eina_list_count(all_pages);
	if (!page_count) {
		LOGE("Failed to get page count!");
		return NULL;
	}

	page = eina_list_nth(all_pages, --page_count);
	if (!page) {
		LOGE("Failed to get page");
		return NULL;
	}

	return page;
}

static void __livebox_panel_scroll_start_cb(void *data, Evas_Object *obj, void *info)
{
	Evas_Object *page = livebox_utils_get_selected_livebox_layout();
	if (!page) {
		LOGE("page == NULL");
		return;
	}

	grid_item_set_resize_sliders_visibility(page, false);
}


static Eina_Bool __livebox_panel_change_page_timer_start_cb(void *data)
{
	bool is_right = (bool)data;
	int page_count =  -1;
	int current_page = -1;
	Evas_Object *grid = NULL;
	Evas_Object *page = NULL;
	int mx = -1, my = -1;

	if (!s_info.livebox_page_scroller) {
		LOGE("s_info.livebox_page_scroller == NULL");
		return false;
	}

	page_count =  page_scroller_get_page_count(
		s_info.livebox_page_scroller);
	current_page = page_scroller_get_current_page(
		s_info.livebox_page_scroller);

	LOGD("PAGE CHANGE... %s", is_right ? "Going right" : "Going left");

	livebox_utils_repack_grid_object(NULL, livebox_utils_get_shadow(),
		livebox_utils_get_livebox_container_grid(
			livebox_utils_get_selected_livebox_layout()), NULL);

	/*TODO: Check if this can be removed*/
	livebox_utils_repack_grid_object(NULL,
		livebox_utils_selected_livebox_get(),
			livebox_utils_get_livebox_container_grid(
				livebox_utils_get_selected_livebox_layout()),
					NULL);

	page_scroller_unfreeze(s_info.livebox_page_scroller);

	if (is_right) {
		++current_page;
	} else {
		--current_page;
	}

	current_page = current_page % page_count;
	LOGD("CURRENT PAGE: %d", current_page);

	page = page_scroller_get_page(s_info.livebox_page_scroller,
		current_page);
	if (!page) {
		LOGE("page == NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	grid = livebox_utils_get_livebox_container_grid(page);
	if (!grid) {
		LOGE("grid == NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	elm_scroller_page_bring_in(s_info.livebox_page_scroller,
		current_page, 0);

	page_scroller_freeze(s_info.livebox_page_scroller);
	__libebox_panel_toggle_selected_livebox(livebox_utils_selected_livebox_get(), page);
	grid_reposition_set_current_grid(grid);
	livebox_utils_repack_grid_object(livebox_utils_selected_livebox_get(),
		livebox_utils_get_shadow(), NULL, grid);

	livebox_utils_get_cursor_pos(&mx, &my);
	grid_reposition_move(mx, my);

	livebox_utils_set_shadow_visibility(true);

	return ECORE_CALLBACK_RENEW;
}

static void __livebox_panel_set_remove_button_visibility(
	Evas_Object *livebox, bool visible)
{
	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	if (visible) {
		elm_layout_signal_emit(livebox,
			SIGNAL_LIVEBOX_REMOVE_BUTTON_SHOW,
			SIGNAL_LIVEBOX_SOURCE);
	} else {
		elm_layout_signal_emit(livebox,
			SIGNAL_LIVEBOX_REMOVE_BUTTON_HIDE,
			SIGNAL_LIVEBOX_SOURCE);
	}
}

static void __livebox_panel_hide_remove_button(Evas_Object *livebox)
{
	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	elm_layout_signal_emit(livebox, SIGNAL_REMOVE_BUTTON_INSTANT_HIDE,
		SIGNAL_LIVEBOX_SOURCE);
}

static void __livebox_panel_set_remove_buttons_visiblity(Evas_Object *page, bool visible)
{
	Eina_List *l = NULL;
	Evas_Object *lb = NULL;

	if (!page) {
		LOGE("page == NULL");
		return;
	}

	Eina_List *list = livebox_utils_get_liveboxes_on_page_list(page);

	if (!list) {
		LOGE("list == NULL");
		return;
	}

	EINA_LIST_FOREACH(list, l, lb) {
		if (!lb) {
			LOGE("lb == NULL");
			continue;
		}
		__livebox_panel_set_remove_button_visibility(lb, visible);
	}
}

static void __livebox_panel_set_reposition_bg_visibility(bool visible)
{
	Evas_Object *livebox = grid_reposition_get_repositioned_item();
	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	if (visible) {
		elm_object_signal_emit(livebox,
			SIGNAL_LIVEBOX_REPOSITION_BG_SHOW,
			SIGNAL_LIVEBOX_SOURCE);
	} else {
		elm_object_signal_emit(livebox,
			SIGNAL_LIVEBOX_REPOSITION_BG_HIDE,
			SIGNAL_LIVEBOX_SOURCE);
	}
}

static Evas_Object *__livebox_panel_get_widget_layout(Evas_Object* obj)
{
	Evas_Object *widget = elm_layout_content_get(obj, PART_LIVEBOX);
	if (!widget) {
		LOGE("widget == NULL");
		return NULL;
	}

	return widget;
}

static void __livebox_panel_del_cb(void *data, Evas_Object *obj, const char *emission,
	const char *source)
{
	Evas_Object *grid = NULL;
	Evas_Object *widget = NULL;
	Tree_node_t *node = NULL;

	int current_page_number =
		page_scroller_get_current_page(s_info.livebox_page_scroller);

	Evas_Object *page_container =
		page_scroller_get_page(
			s_info.livebox_page_scroller, current_page_number);
	if (!page_container) {
		LOGE("page_container == NULL");
		return;
	}
	elm_object_signal_emit(page_container, SIGNAL_RESIZE_SLIDERS_HIDE,
		SIGNAL_LIVEBOX_SOURCE);

	LOGI("DELETE LIVEBOX BUTTON PRESSED");

	node = evas_object_data_get(obj, KEY_ICON_DATA);
	if (!node) {
		LOGE("node == NULL");
		return;
	}

	widget = __livebox_panel_get_widget_layout(obj);
	livebox_widget_del(widget);
	data_model_del_item(node);

	evas_object_del(obj);

	grid = livebox_utils_get_livebox_container_grid(page_container);
	if (!grid) {
		LOGE("page_container == NULL");
		return;
	}

	livebox_utils_repack_grid_object(NULL, livebox_utils_get_shadow(),
		grid, NULL);

	home_screen_print_tree();
}

static bool __livebox_panel_iterate_node_fill_cb(Tree_node_t *page_node, Tree_node_t *node,
	void *data)
{
	int *widget_pos_h = (int *)data;

	Evas_Object *page = page_node->data->layout;
	if (!page) {
		LOGE("page == NULL");
		return false;
	}

	livebox_panel_add_livebox(node, page, node->data->appid,
		node->data->col, node->data->row, node->data->col_span,
		node->data->row_span,
		node->data->content_info);

	*widget_pos_h += LIVEBOX_ON_CREATE_SIZE/2;
	return true;
}

static bool __livebox_panel_iterate_page_fill_cb(Tree_node_t *parent, Tree_node_t *page_node,
	void *data)
{
	Evas_Object *page = NULL;
	Evas_Object *liveboxes = s_info.livebox_page_scroller;
	int *widget_pos_h = (int *)data;
	*widget_pos_h = 0;

	page = livebox_panel_add_page(page_node, liveboxes);
	if (!page) {
		LOGE("[FAILED][page]");
		data_model_del_item(page_node);
		return false;
	}

	page_node->data->layout = page;
	return true;
}
