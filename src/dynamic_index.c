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

#include "dynamic_index.h"

#include <stdbool.h>
#include <Elementary.h>

#include "homescreen-efl.h"
#include "util.h"
#include "livebox/livebox_panel.h"
#include "layout.h"

static Evas_Object *__dynamic_index_create_indice(Evas_Object *box);
static Eina_Bool __dynamic_index_fill_box(dynamic_index_t *dynamic_index);

static void __dynamic_index_page_count_changed_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __dynamic_index_page_area_changed_cb(void *data, Evas_Object *obj, void *event_info);

static void __dynamic_index_current_page_changed_start_cb(void *data, Evas_Object *obj, void *event_info);
static void __dynamic_index_current_page_changed_stop_cb(void *data, Evas_Object *obj, void *event_info);
static void __dynamic_index_set_properties(Evas_Object *index, double percent);
static Evas_Object *__dynamic_index_box_create(void);
static void __dynamic_index_rotate(Evas_Object *element, double angle, double alpha);

HAPI dynamic_index_t *dynamic_index_new(Evas_Object *page_scroller)
{
	dynamic_index_t *dynamic_index = NULL;
	Evas_Object *box = NULL;

	if (!page_scroller) {
		LOGD("Invalid parameter : page_scroller is NULL");
		return NULL;
	}

	dynamic_index = (dynamic_index_t *) malloc (sizeof(dynamic_index_t));
	if (!dynamic_index)
		return NULL;

	box = __dynamic_index_box_create();

	dynamic_index->scroller = page_scroller;
	dynamic_index->box = box;
	dynamic_index->page_count = page_scroller_get_page_count(page_scroller);
	dynamic_index->page_current = page_scroller_get_current_page(page_scroller);

	elm_scroller_page_size_get(page_scroller, &dynamic_index->page_width, NULL);
	elm_scroller_region_get(page_scroller, &dynamic_index->x_current, NULL, NULL, NULL);

	if (!__dynamic_index_fill_box(dynamic_index)) {
		LOGD("Box fill failed");
		evas_object_del(box);
		free(dynamic_index);
		return NULL;
	}

	elm_object_signal_callback_add(page_scroller, "scroller,count,changed", "*", __dynamic_index_page_count_changed_cb, dynamic_index);
	evas_object_smart_callback_add(page_scroller, "scroll", __dynamic_index_page_area_changed_cb, dynamic_index);
	evas_object_smart_callback_add(page_scroller, "scroll,drag,start", __dynamic_index_current_page_changed_start_cb, dynamic_index);
	evas_object_smart_callback_add(page_scroller, "scroll,anim,stop", __dynamic_index_current_page_changed_stop_cb, dynamic_index);
	evas_object_data_set(page_scroller, KEY_INDEX, dynamic_index);

	return dynamic_index;
}

HAPI void dynamic_index_clear(dynamic_index_t *dynamic_index)
{
	int i = 0;

	if (!dynamic_index) {
		LOGE("Invalid parameter");
		return;
	}

	for (i = 0; i < dynamic_index->page_count; i++) {
		if (dynamic_index->indices[i]) {
			evas_object_del(dynamic_index->indices[i]);
			dynamic_index->indices[i] = NULL;
		}
	}

	elm_box_clear(dynamic_index->box);
}

HAPI void dynamic_index_del(dynamic_index_t *dynamic_index)
{
	if (!dynamic_index) {
		LOGE("[INVALID_PARAM][dynamic_index='%p']", dynamic_index);
		return;
	}

	elm_object_signal_callback_del(dynamic_index->scroller, "scroller,count,changed", "*", __dynamic_index_page_count_changed_cb);

	evas_object_smart_callback_del(dynamic_index->scroller, "scroll", __dynamic_index_page_area_changed_cb);
	evas_object_smart_callback_del(dynamic_index->scroller, "scroll,drag,start", __dynamic_index_current_page_changed_start_cb);
	evas_object_smart_callback_del(dynamic_index->scroller, "scroll,anim,stop", __dynamic_index_current_page_changed_stop_cb);

	dynamic_index_clear(dynamic_index);
	free(dynamic_index);
}

HAPI void dynamic_index_reload(dynamic_index_t *dynamic_index, int idx, int page_count)
{
	if (!dynamic_index) {
		LOGE("[INVALID_PARAMS]");
		return;
	}

	dynamic_index_clear(dynamic_index);
	dynamic_index->page_current = idx;
	dynamic_index->page_count = page_count;

	__dynamic_index_fill_box(dynamic_index);
}

static Evas_Object *__dynamic_index_create_indice(Evas_Object *box)
{
	Evas_Object *layout = NULL;
	Evas_Object *size_setter_rect = NULL;

	int idx_width = (INDEX_ELEMENT_CONTAINER_WIDTH / ROOT_WIN_W) * home_screen_get_root_width();
	int idx_height = (INDEX_ELEMENT_CONTAINER_HEIGHT / ROOT_WIN_H) * home_screen_get_root_height();

	if (!box) {
		LOGD("Invalid argument : box is NULL");
		return NULL;
	}

	layout = elm_layout_add(box);
	if (!layout) {
		LOGD("Layout creation failed");
		return NULL;
	}

	if (!elm_layout_file_set(layout, util_get_res_file_path(INDEX_EDJE), GROUP_INDEX_ELEMENT)) {
		evas_object_del(layout);
		return NULL;
	}

	size_setter_rect = evas_object_rectangle_add(evas_object_evas_get(home_screen_get_win()));
	if (!size_setter_rect) {
		LOGE("Failed to create size setter rectangle");
		return NULL;
	}

	elm_object_part_content_set(layout, INDICE_SIZE_SETTER, size_setter_rect);

	evas_object_color_set(size_setter_rect, 0, 0, 0, 0);
	evas_object_size_hint_min_set(size_setter_rect, idx_width, idx_height);
	evas_object_size_hint_max_set(size_setter_rect, idx_width, idx_height);
	evas_object_resize(size_setter_rect, idx_width, idx_height);

	evas_object_show(layout);
	elm_box_pack_end(box, layout);
	return layout;
}

static void __dynamic_index_set_properties(Evas_Object *index, double percent)
{
	double size = (percent*INDEX_CURRENT_SIZE+(1.0-percent)*INDEX_NORMAL_SIZE)*home_screen_get_root_width();
	evas_object_size_hint_min_set(index, size, size);
	evas_object_size_hint_max_set(index, size, size);
	evas_object_color_set(index, 255, 255, 255, percent*INDEX_CURRENT_ALPHA+(1.0-percent)*INDEX_NORMAL_ALPHA);
}

static Eina_Bool __dynamic_index_fill_box(dynamic_index_t *dynamic_index)
{
	int i = 0, c_page = -1;

	Evas_Object *layout = NULL;

	if (!dynamic_index)
		return EINA_FALSE;

	if (!dynamic_index->box || dynamic_index->page_count < 1) {
		LOGD("Invalid parameters : box is NULL or elements_count is < 1");
		return EINA_FALSE;
	}

	c_page = dynamic_index->page_current;
	if(c_page < 0 || c_page > dynamic_index->page_count)
	{
		LOGE("Failed to get current_page ");
		return EINA_FALSE;
	}

	for (i = 0 ; i < dynamic_index->page_count; i++) {
		layout = __dynamic_index_create_indice(dynamic_index->box);
		if (!layout) {
			LOGE("Failed to append index element into the box");
			dynamic_index_clear(dynamic_index);
			return EINA_FALSE;
		}

		dynamic_index->indices[i] = layout;
	}

	elm_layout_signal_emit(dynamic_index->indices[c_page], SIGNAL_SET_CURRENT, SIGNAL_SOURCE);
	__dynamic_index_set_properties(dynamic_index->indices[dynamic_index->page_current], 1.0);
	return EINA_TRUE;
}

static void __dynamic_index_page_count_changed_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	LOGD("The page count has changed");
	dynamic_index_t *dynamic_index = (dynamic_index_t *)data;
	dynamic_index_clear(dynamic_index);
	dynamic_index->page_count = page_scroller_get_page_count(dynamic_index->scroller);

	LOGD("page count : %d ", dynamic_index->page_count);

	if (!dynamic_index->page_count)
		return;

	dynamic_index->page_current = page_scroller_get_current_page(dynamic_index->scroller);

	if (!__dynamic_index_fill_box(dynamic_index)) {
		elm_object_signal_callback_del(dynamic_index->scroller, "scroller,count,changed", "*", __dynamic_index_page_count_changed_cb);
		evas_object_smart_callback_del(dynamic_index->scroller, "scroll", __dynamic_index_page_area_changed_cb);
		evas_object_smart_callback_del(dynamic_index->scroller, "scroll,drag,start", __dynamic_index_current_page_changed_start_cb);
		evas_object_smart_callback_del(dynamic_index->scroller, "scroll,anim,stop", __dynamic_index_current_page_changed_stop_cb);
	}
}

HAPI double _dist(int current, int page_x, int page_width)
{
	return 1.0-((double)fabs(page_x-current))/page_width;
}

static void __dynamic_index_page_area_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	int x = 0;
	double angle = 0.0;
	double color = 0.0;
	int next_page = -1;

	dynamic_index_t *dynamic_index = (dynamic_index_t*) data;
	if(!dynamic_index)
	{
		LOGE("Invalid dynamic_index");
		return;
	}
	elm_scroller_region_get(obj, &x, NULL, NULL, NULL);

	angle = (double)(x - dynamic_index->x_current)/dynamic_index->page_width * 90.0;

	if(fabs(x - dynamic_index->x_current) <= dynamic_index->page_width)
	{
		next_page = x > dynamic_index->x_current ? (dynamic_index->page_current+1) % dynamic_index->page_count : dynamic_index->page_current-1;
		color = fabs(angle) * 2;
	}
	else
	{
		if(x > dynamic_index->x_current)
		{
			next_page = dynamic_index->page_count-1;
			color = (double)(dynamic_index->page_width*dynamic_index->page_count - x)/dynamic_index->page_width * 180.0;
		}
		else
		{
			next_page = 0;
			color = 180.0;
		}
	}

	if (next_page == dynamic_index->page_count - 1 && dynamic_index->page_current == 0 && dynamic_index->page_count % 2 == 1)
		angle = angle - 270.0;

	__dynamic_index_rotate(dynamic_index->indices[dynamic_index->page_current], 90.0 + angle, 255.0 - color);
	__dynamic_index_rotate(dynamic_index->indices[next_page], angle, 75.0 + color);
}

static Evas_Object *__dynamic_index_box_create(void)
{
	Evas_Object *box = NULL;

	box = elm_box_add(home_screen_get_win());
	if (!box) {
		LOGD("Box creation failed");
		return NULL;
	}

	elm_box_homogeneous_set(box, EINA_TRUE);
	elm_box_horizontal_set(box, EINA_TRUE);

	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_box_align_set(box, 0.5, 0.5);
	evas_object_show(box);

	return box;
}

static void __dynamic_index_current_page_changed_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	dynamic_index_t *dynamic_index = (dynamic_index_t*) data;
	if(!dynamic_index)
	{
		LOGE("Invalid data");
		return;
	}

	elm_scroller_current_page_get(obj, &dynamic_index->page_current, NULL);
	elm_scroller_region_get(obj, &dynamic_index->x_current, NULL, NULL, NULL);
}

static void __dynamic_index_current_page_changed_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	dynamic_index_t *dynamic_index = (dynamic_index_t*)data;
	int i;

	if(!dynamic_index)
	{
		LOGE("Invalid data");
		return;
	}

	elm_scroller_current_page_get(obj, &dynamic_index->page_current, NULL);
	for(i = 0; i<dynamic_index->page_current; i++)
	{
		elm_object_signal_emit(dynamic_index->indices[i], SIGNAL_SET_DEFAULT, SIGNAL_SOURCE);
	}
	elm_object_signal_emit(dynamic_index->indices[dynamic_index->page_current], SIGNAL_SET_CURRENT, SIGNAL_SOURCE);
	for(i = dynamic_index->page_current+1; i<dynamic_index->page_count; i++)
	{
		elm_object_signal_emit(dynamic_index->indices[i], SIGNAL_SET_DEFAULT, SIGNAL_SOURCE);
	}
}

static void __dynamic_index_rotate(Evas_Object *element, double angle, double alpha)
{
	Evas_Object *edje = NULL;

	if(!element)
	{
		LOGE("Invalid argument : element is NULL");
		return;
	}

	edje = elm_layout_edje_get(element);
	if(!edje)
	{
		LOGE("Failed to get edje from layout");
		return;
	}

	Edje_Message_Float_Set *msg = malloc(sizeof(*msg) + sizeof(double));
	msg->count = 2;
	msg->val[0] = angle;
	msg->val[1] = alpha;

	edje_object_message_send(edje, EDJE_MESSAGE_FLOAT_SET, 1, msg);
	edje_object_signal_emit(edje, "rot_changed", "ly");

	free(msg);
}

