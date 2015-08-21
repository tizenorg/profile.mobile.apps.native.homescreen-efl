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

#include "livebox/grid_item_resize.h"
#include "homescreen-efl.h"
#include "layout.h"
#include "livebox/livebox_panel.h"
#include "livebox/livebox_widget.h"
#include "page_scroller.h"
#include "conf.h"
#include "data_model.h"

typedef enum {
	GRID_RESIZE_LEFT,
	GRID_RESIZE_RIGHT,
	GRID_RESIZE_UP,
	GRID_RESIZE_DOWN,
} gird_item_resize_dir_t;

static struct {
	int x_at_drag_start;
	int y_at_drag_start;
	int w_at_drag_start;
	int h_at_drag_start;
	Eina_Bool is_resizing;
} s_info = {
	.x_at_drag_start = 0,
	.y_at_drag_start = 0,
	.w_at_drag_start = 0,
	.h_at_drag_start = 0,
	.is_resizing = false
};

static void __gird_item_resize_button_pressed_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source);
static int __grid_item_resize_drag_pos_get(Evas_Object *obj, const char *part,
	Eina_Bool is_horizontal);
static void __grid_item_resize_finalize(Evas_Object *obj);
static Eina_Bool __grid_item_resize_get_available_sizes(Evas_Object *livebox, int w, int h);
static void __grid_item_resize_drag(Evas_Object *livebox, int new_x, int new_y, int new_w,
	int new_h);
static void __grid_item_resize_drag_stop_cb(void *data, Evas_Object *obj, const char *emission,
	const char *source);
static void __grid_item_resize_get_sliders_geometry(Evas_Object *page, int *out_x,
	int *out_y, int *out_w, int *out_h);
static void __grid_item_resize_drag_cb(void *data, Evas_Object *obj, const char *emission,
	const char *source);
static void __grid_item_resize_page_background_clicked_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source);

#ifdef LIVEBOX_RESIZE_ENABLED
	static void __grid_item_resize_set_slider_pos(Evas_Object *page, const char *part,
		int pos, Eina_Bool is_horizontal);
#endif


void grid_item_set_resize_livebox_sliders(Evas_Object *livebox,
	Evas_Object *page)
{
#ifdef LIVEBOX_RESIZE_ENABLED
	Evas_Coord x = -1;
	Evas_Coord y = -1;
	Evas_Coord w = -1;
	Evas_Coord h = -1;

	if (!page) {
		LOGE("page == NULL");
		return;
	}

	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	elm_grid_pack_get(livebox, &x, &y, &w, &h);
	LOGD("LIVEBOX NEW POS: %d %d %d %d", x, y, w, h);

	__grid_item_resize_set_slider_pos(page, PART_RESIZE_BUTTON_UP, y, EINA_FALSE);
	__grid_item_resize_set_slider_pos(page, PART_RESIZE_BUTTON_DOWN, y + h, EINA_FALSE);
	__grid_item_resize_set_slider_pos(page, PART_RESIZE_BUTTON_LEFT, x, EINA_TRUE);
	__grid_item_resize_set_slider_pos(page, PART_RESIZE_BUTTON_RIGHT, x + w, EINA_TRUE);

	elm_layout_signal_emit(page, SIGNAL_RESIZE_SLIDERS_SHOW,
		SIGNAL_LIVEBOX_SOURCE);

#endif
}

bool grid_item_init_resize_callbacks(Evas_Object *livebox_container_scroller,
	Evas_Object *livebox_container)
{
	Evas_Object *edje = NULL;

	if (!livebox_container_scroller) {
		LOGE("livebox_container_scroller == NULL");
		return false;
	}

	if (!livebox_container) {
		LOGE("livebox_container == NULL");
		return false;
	}

	edje = elm_layout_edje_get(livebox_container);
	if (!edje) {
		LOGE("edje == NULL");
		return false;
	}

	edje_object_signal_callback_add(edje, SIGNAL_MOUSE_DOWN,
		PART_RESIZE_BUTTON_DOWN, __gird_item_resize_button_pressed_cb,
		livebox_container_scroller);
	edje_object_signal_callback_add(edje, SIGNAL_MOUSE_DOWN,
		PART_RESIZE_BUTTON_LEFT, __gird_item_resize_button_pressed_cb,
		livebox_container_scroller);
	edje_object_signal_callback_add(edje, SIGNAL_MOUSE_DOWN,
		PART_RESIZE_BUTTON_UP, __gird_item_resize_button_pressed_cb,
		livebox_container_scroller);
	edje_object_signal_callback_add(edje,
		SIGNAL_MOUSE_DOWN, PART_RESIZE_BUTTON_RIGHT,
		__gird_item_resize_button_pressed_cb, livebox_container_scroller);
	edje_object_part_drag_value_set(edje,
		PART_RESIZE_BUTTON_DOWN, 0.0, 0.999);
	edje_object_part_drag_value_set(edje,
		PART_RESIZE_BUTTON_UP, 0.0, 0.001);
	edje_object_signal_callback_add(edje,
		SIGNAL_RESIZING, PART_RESIZE_BUTTON_DOWN,
		__grid_item_resize_drag_cb, livebox_container);
	edje_object_signal_callback_add(edje,
		SIGNAL_RESIZE_END, PART_RESIZE_BUTTON_DOWN,
		__grid_item_resize_drag_stop_cb, NULL);
	edje_object_signal_callback_add(edje, SIGNAL_RESIZING,
		PART_RESIZE_BUTTON_UP, __grid_item_resize_drag_cb, livebox_container);
	edje_object_signal_callback_add(edje,
		SIGNAL_RESIZE_END, PART_RESIZE_BUTTON_UP, __grid_item_resize_drag_stop_cb, NULL);
	edje_object_part_drag_value_set(edje,
		PART_RESIZE_BUTTON_LEFT, 0.001, 0.0);
	edje_object_part_drag_value_set(edje,
		PART_RESIZE_BUTTON_RIGHT, 0.999, 0.0);
	edje_object_signal_callback_add(edje,
		SIGNAL_RESIZING, PART_RESIZE_BUTTON_LEFT, __grid_item_resize_drag_cb,
		livebox_container);
	edje_object_signal_callback_add(edje,
		SIGNAL_RESIZE_END, PART_RESIZE_BUTTON_LEFT,
		__grid_item_resize_drag_stop_cb, NULL);
	edje_object_signal_callback_add(edje,
		SIGNAL_RESIZING, PART_RESIZE_BUTTON_RIGHT,
		__grid_item_resize_drag_cb, livebox_container);
	edje_object_signal_callback_add(edje,
		SIGNAL_RESIZE_END, PART_RESIZE_BUTTON_RIGHT,
		__grid_item_resize_drag_stop_cb, NULL);

	elm_layout_signal_callback_add(livebox_container, SIGNAL_CLICKED,
		PART_PAGE_BG, __grid_item_resize_page_background_clicked_cb, NULL);

	return true;
}

void grid_item_set_resize_sliders_visibility(Evas_Object *page, bool visible)
{
	if (!page) {
		LOGE("page == NULL");
		return;
	}

	if (visible) {
		elm_object_signal_emit(page, SIGNAL_RESIZE_SLIDERS_SHOW,
			SIGNAL_LIVEBOX_SOURCE);
	} else {
		elm_object_signal_emit(page, SIGNAL_RESIZE_SLIDERS_HIDE,
			SIGNAL_LIVEBOX_SOURCE);
	}
}

static void __gird_item_resize_button_pressed_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source)
{
	Evas_Object *livebox = NULL;
	Evas_Object *livebox_scroller = NULL;

	if (!obj || !emission || !source) {
		LOGE("Wrong arguments provided: %p %p %p", obj,
			emission, source);
		return;
	}

	livebox_scroller = data;
	if (!livebox_scroller) {
		LOGE("livebox_scroller == NULL");
		return;

	}

	page_scroller_freeze(livebox_scroller);

	livebox = livebox_utils_selected_livebox_get();
	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	elm_grid_pack_get(livebox, &s_info.x_at_drag_start,
		&s_info.y_at_drag_start,
		&s_info.w_at_drag_start,
		&s_info.h_at_drag_start);

	s_info.is_resizing = EINA_TRUE;

	LOGD("BUTTON PRESSED: %p !!! x = %d; y = %d; w = %d; h = %d", livebox,
		s_info.x_at_drag_start,
		s_info.y_at_drag_start,
		s_info.w_at_drag_start,
		s_info.h_at_drag_start);
}

static int __grid_item_resize_drag_pos_get(Evas_Object *obj, const char *part,
	Eina_Bool is_horizontal)
{
	double val;
	int pos;

	if (!obj || !part) {
		LOGE("Wrong arguments provided: %p %p", obj, part);
		return -1;
	}

	if (is_horizontal) {
		edje_object_part_drag_value_get(obj, part, &val, NULL);
		pos = (int)(val * LIVEBOX_GRID_COLUMNS);
	} else {
		edje_object_part_drag_value_get(obj, part, NULL, &val);
		pos = (int)(val * LIVEBOX_GRID_ROWS);
	}


	LOGD("DRAG UP STOP: %f POS: %d", val, pos);
	return pos;
}


static void __grid_item_resize_finalize(Evas_Object *obj)
{
	LOGD("Resize");
	Evas_Object *lb_scroller = NULL;
	if (!obj) {
		LOGE("o == NULL");
		return;
	}

	edje_object_signal_emit(obj, SIGNAL_RESIZE_SLIDER_DRAG_OK,
	 SIGNAL_RESIZE_SLIDER_DRAG_OK);

	s_info.is_resizing = EINA_FALSE;

	lb_scroller = livebox_panel_get();
	if (!lb_scroller) {
		LOGE("lb_scroller == NULL");
		return;
	}

	page_scroller_unfreeze(lb_scroller);
}

static Eina_Bool __grid_item_resize_get_available_sizes(Evas_Object *livebox, int w, int h)
{
	int cnt = 0;
	int *size_list = NULL;
	Evas_Object *widget = NULL;
	int test_h = 0;
	int test_w = 0;
	int i = 0;

	if (!livebox) {
		LOGE("livebox == NULL");
		return EINA_FALSE;
	}

	widget = elm_layout_content_get(livebox, PART_LIVEBOX);
	if (!widget) {
		LOGE("widget == NULL");
		return EINA_FALSE;
	}

	livebox_widget_get_size_available(widget, &cnt, &size_list);

	LOGD("Size count: %d", cnt);

	if (!size_list) {
		LOGE("size_list == NULL");
		return EINA_FALSE;
	}

	for (i = 0; i < cnt; ++i) {
		livebox_widget_get_dimensions(size_list[i], &test_w, &test_h);

		if (w == test_w * LIVEBOX_GRID_ROWS_CELLS_MULTI &&
			h == test_h * LIVEBOX_GRID_ROWS_CELLS_MULTI) {
			LOGD("RESIZE AVAILABLE SIZE FOUND: %d %d",
				test_w, test_h);
			free(size_list);
			return EINA_TRUE;
		}
	}

	free(size_list);
	return EINA_FALSE;
}


static void __grid_item_resize_drag(Evas_Object *livebox,
	int new_x, int new_y, int new_w, int new_h)
{
	LOGD("resize2");
	Evas_Object *page = NULL;
	Eina_Rectangle *rect = NULL;
	Eina_List *list;
	int out_x = -1, out_y = -1, out_w = -1, out_h = -1;

	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	page = livebox_utils_get_selected_livebox_layout();
	if (!page) {
		LOGE("page == NULL");
		return;
	}

	list = livebox_utils_get_liveboxes_on_page_list(page);
	if (!list) {
		LOGE("list == NULL");
		return;

	}

	__grid_item_resize_get_sliders_geometry(page, &out_x, &out_y, &out_w, &out_h);
	rect = eina_rectangle_new(out_x, out_y, out_w, out_h);

	if (__grid_item_resize_get_available_sizes(livebox, new_w, new_h) &&
		!livebox_utils_check_rect_list_grid_interesction(rect,
			livebox, list)) {
		elm_grid_pack_set(livebox, new_x, new_y, new_w, new_h);
		LOGD("RESIZE OK: %d %d %d %d", new_x, new_y, new_w, new_h);
	} else {
		elm_grid_pack_set(livebox, s_info.x_at_drag_start,
			s_info.y_at_drag_start,
			s_info.w_at_drag_start,
			s_info.h_at_drag_start);
		LOGD("RESIZE FAIL: %d %d %d %d; ", new_x, new_y, new_w, new_h);
	}

	eina_rectangle_free(rect);
}

static void _drag_size_new_get(gird_item_resize_dir_t dir, Evas_Object *livebox,
	int pos, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
	Evas_Coord x2, y2;

	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	if (!x || !y || !w || !h) { /*Other arguments can be NULL*/
		LOGE("Wrong arguments: x = %p; y = %p; w = %p; h = %p",
			x, y, w, h);
		return;
	}

	elm_grid_pack_get(livebox, x, y, w, h);

	switch (dir) {
	case GRID_RESIZE_LEFT:
		x2 = *x + *w;
		*w = x2 - (pos * LIVEBOX_GRID_ROWS_CELLS_MULTI);
		*x = (pos * LIVEBOX_GRID_ROWS_CELLS_MULTI);
		break;
	case GRID_RESIZE_RIGHT:
		*w = (pos - *x) * LIVEBOX_GRID_ROWS_CELLS_MULTI;
		break;
	case GRID_RESIZE_UP:
		y2 = *y + *h;
		*h = y2 - (pos * LIVEBOX_GRID_ROWS_CELLS_MULTI);
		*y = (pos * LIVEBOX_GRID_ROWS_CELLS_MULTI);
		break;
	case GRID_RESIZE_DOWN:
		*h = (pos - *y) * LIVEBOX_GRID_ROWS_CELLS_MULTI;
		break;
	}
}

static void __grid_item_resize_drag_stop_cb(void *data, Evas_Object *obj, const char *emission,
	const char *source)
{
	int pos = -1;
	Evas_Coord x = -1;
	Evas_Coord y = -1;
	Evas_Coord w = -1;
	Evas_Coord h = -1;
	Evas_Object *livebox = NULL;
	Tree_node_t *item_node = NULL;
	gird_item_resize_dir_t resize_dir;
	bool horizontal = false;


	if (!obj || !emission || !source) {
		LOGE("Wrong arguments provided: %p %p %p",
			obj, emission, source);
		return;
	}

	livebox = livebox_utils_selected_livebox_get();
	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	__grid_item_resize_finalize(obj);

	if (!strncmp(PART_RESIZE_BUTTON_DOWN, source,
		LIVEBOX_MAX_BUTTON_NAME_LENGTH)) {
		horizontal = EINA_FALSE;
		resize_dir = GRID_RESIZE_DOWN;
	} else if (!strncmp(PART_RESIZE_BUTTON_UP, source,
		LIVEBOX_MAX_BUTTON_NAME_LENGTH)) {
		horizontal = EINA_FALSE;
		resize_dir = GRID_RESIZE_UP;
	} else if (!strncmp(PART_RESIZE_BUTTON_LEFT, source,
		LIVEBOX_MAX_BUTTON_NAME_LENGTH)) {
		horizontal = EINA_TRUE;
		resize_dir = GRID_RESIZE_LEFT;
	} else if (!strncmp(PART_RESIZE_BUTTON_RIGHT, source,
		LIVEBOX_MAX_BUTTON_NAME_LENGTH)) {
		horizontal = EINA_TRUE;
		resize_dir = GRID_RESIZE_RIGHT;
	} else {
		LOGE("UNKNOWN PART");
		return;
	}

	pos = __grid_item_resize_drag_pos_get(obj, source, horizontal);
	_drag_size_new_get(resize_dir, livebox, pos, &x, &y, &w, &h);

	__grid_item_resize_drag(livebox, x, y, w, h);
	elm_grid_pack_get(livebox, &x, &y, &w, &h);

	edje_object_part_drag_value_set(obj, PART_RESIZE_BUTTON_DOWN, 0,
			(y + h) * LIVEBOX_RESIZE_POS_MULTI_ROWS);

	edje_object_part_drag_value_set(obj, PART_RESIZE_BUTTON_UP, 0,
			(float) y * LIVEBOX_RESIZE_POS_MULTI_ROWS);

	edje_object_part_drag_value_set(obj, PART_RESIZE_BUTTON_LEFT,
			(float) x * LIVEBOX_RESIZE_POS_MULTI_COLS, 0);

	edje_object_part_drag_value_set(obj, PART_RESIZE_BUTTON_RIGHT,
			(float) (x + w) * LIVEBOX_RESIZE_POS_MULTI_COLS, 0);

	item_node = evas_object_data_get(livebox, KEY_ICON_DATA);
	if (!item_node) {
		LOGE("item_node == NULL");
		return;
	}

	data_model_resize_widget(item_node, x, y, w, h);
	home_screen_print_tree();
}

static void __grid_item_resize_get_sliders_geometry(Evas_Object *page, int *out_x,
	int *out_y, int *out_w, int *out_h)
{
	Evas_Object *edje = elm_layout_edje_get(page);
	Evas_Object *grid = NULL;
	double val = 0;

	if (!edje) {
		LOGE("edje == NULL");
		return;
	}

	grid = edje_object_part_swallow_get(edje, SIGNAL_CONTENT_SOURCE);
	if (!grid) {
		LOGE("grid == NULL");
		return;
	}

	if (!out_x || !out_y || !out_w || !out_h) {
		LOGE("x = %p; y = %p; w = %p; h = %p",
			out_x, out_y, out_w, out_h);
		return;
	}

	edje_object_part_drag_value_get(edje, PART_RESIZE_BUTTON_LEFT,
		&val, NULL);
	*out_x = round(val * LIVEBOX_TOTAL_COLUMNS);

	edje_object_part_drag_value_get(edje, PART_RESIZE_BUTTON_UP,
		NULL, &val);
	*out_y = round(val * LIVEBOX_TOTAL_ROWS);

	edje_object_part_drag_value_get(edje, PART_RESIZE_BUTTON_RIGHT,
		&val, NULL);
	*out_w = round(val * LIVEBOX_TOTAL_COLUMNS - *out_x);

	edje_object_part_drag_value_get(edje, PART_RESIZE_BUTTON_DOWN,
		NULL, &val);
	*out_h = round(val * LIVEBOX_TOTAL_ROWS - *out_y);
}

static void __grid_item_resize_drag_cb(void *data, Evas_Object *obj, const char *emission,
	const char *source)
{
	int pos = -1;
	Evas_Coord x = -1;
	Evas_Coord y = -1;
	Evas_Coord w = -1;
	Evas_Coord h = -1;
	Evas_Object *livebox = NULL;
	Eina_List   *list = NULL;
	Eina_Rectangle *rect;
	int out_x = -1, out_y = -1, out_w = -1, out_h = -1;

	Evas_Object *page_container = data;
	if (!page_container) {
		LOGE("page_container == NULL");
		return;
	}

	if (!obj || !emission || !source) {
		LOGE("Wrong arguments provided: obj == %p; \
			emission == %p; source == %p", obj, emission, source);
		return;
	}

	livebox = livebox_utils_selected_livebox_get();
	if (!livebox) {
		LOGE("livebox == NULL");
		return;
	}

	list = livebox_utils_get_liveboxes_on_page_list(page_container);
	if (!list) {
		LOGE("list == NULL");
		return;

	}

	if (!strncmp(source, PART_RESIZE_BUTTON_DOWN,
		LIVEBOX_MAX_BUTTON_NAME_LENGTH)) {
		pos = __grid_item_resize_drag_pos_get(obj, source, EINA_FALSE);
		_drag_size_new_get(GRID_RESIZE_DOWN, livebox, pos,
			&x, &y, &w, &h);

		LOGD("pos = %d", pos);
	} else if (!strncmp(source, PART_RESIZE_BUTTON_UP,
		LIVEBOX_MAX_BUTTON_NAME_LENGTH)) {
		pos = __grid_item_resize_drag_pos_get(obj, source, EINA_FALSE);
		_drag_size_new_get(GRID_RESIZE_UP, livebox,
			pos, &x, &y, &w, &h);

		LOGD("pos = %d", pos);
	} else if (!strncmp(source, PART_RESIZE_BUTTON_LEFT,
		LIVEBOX_MAX_BUTTON_NAME_LENGTH)) {
		pos = __grid_item_resize_drag_pos_get(obj, source, EINA_TRUE);
		_drag_size_new_get(GRID_RESIZE_LEFT, livebox,
			pos, &x, &y, &w, &h);
	} else if (!strncmp(source, PART_RESIZE_BUTTON_RIGHT,
		LIVEBOX_MAX_BUTTON_NAME_LENGTH)) {
		pos = __grid_item_resize_drag_pos_get(obj, source, EINA_TRUE);
		_drag_size_new_get(GRID_RESIZE_RIGHT, livebox,
			pos, &x, &y, &w, &h);
	} else {
		pos = 0;
	}

	__grid_item_resize_get_sliders_geometry(page_container, &out_x,
		&out_y, &out_w, &out_h);
	rect = eina_rectangle_new(out_x, out_y, out_w, out_h);

	if (!__grid_item_resize_get_available_sizes(livebox, w, h) ||
		livebox_utils_check_rect_list_grid_interesction(rect,
			livebox, list)) {
		edje_object_signal_emit(obj, SIGNAL_RESIZE_SLIDER_DRAG_ERR,
			SIGNAL_LIVEBOX_SOURCE);
	} else {
		edje_object_signal_emit(obj, SIGNAL_RESIZE_SLIDER_DRAG_OK,
			SIGNAL_LIVEBOX_SOURCE);
	}

	eina_rectangle_free(rect);
}

#ifdef LIVEBOX_RESIZE_ENABLED
static void __grid_item_resize_set_slider_pos(Evas_Object *page, const char *part,
	int pos, Eina_Bool is_horizontal)
{
	Evas_Object *edje = NULL;

	if (!page) {
		LOGE("page == NULL");
		return;
	}

	edje = elm_layout_edje_get(page);
	if (!edje) {
		LOGE("page == NULL");
		return;
	}

	if (!part) {
		LOGE("part == NULL");
		return;
	}

	if (is_horizontal) {
		if (pos < 0 || pos > LIVEBOX_TOTAL_COLUMNS) {
			LOGE("Wrong value of pos = %d", pos);
			return;
		}

		LOGD("NEW POS OF %s is %f", part,
			(float)pos / LIVEBOX_TOTAL_COLUMNS);
		edje_object_part_drag_value_set(edje, part,
			(float)pos / LIVEBOX_TOTAL_COLUMNS, 0);
	} else {
		if (pos < 0 || pos > LIVEBOX_TOTAL_ROWS) {
			LOGE("Wrong value of pos = %d", pos);
			return;
		}

		LOGD("NEW POS OF %s is %f", part,
			(float)pos / LIVEBOX_TOTAL_ROWS);
		edje_object_part_drag_value_set(edje, part, 0,
			(float)pos / LIVEBOX_TOTAL_ROWS);
	}
}
#endif

static void __grid_item_resize_page_background_clicked_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source)
{
	if (!obj) {
		LOGE("obj == NULL");
		return;
	}

	LOGD("Background clicked");
	elm_layout_signal_emit(obj, SIGNAL_RESIZE_SLIDERS_HIDE,
		SIGNAL_LIVEBOX_SOURCE);
}

