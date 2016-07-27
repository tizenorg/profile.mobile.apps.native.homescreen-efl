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

#include "page_indicator.h"
#include "homescreen-efl.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"

static void __page_indicator_scroll_cb(void *data, Evas_Object *obj, void *event_info);
static void __page_indicator_scroll_anim_stop_cb(void *data, Evas_Object *obj, void *event_info);
static void __page_indicator_set_current_page(page_indicator_t *page_indicator);
static void __page_indicator_unit_rotate(Evas_Object *unit, double angle, double alpha);
static void __page_indicator_unit_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);

page_indicator_t * page_indictor_create(Evas_Object *scroller)
{
	int i;
	char edj_path[PATH_MAX] = {0, };
	page_indicator_t *page_indicator = NULL;

	snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/page_indicator_unit.edj"));

	if (scroller == NULL)
		return NULL;
	page_indicator = (page_indicator_t *)malloc(sizeof(page_indicator_t));
	page_indicator->scroller = scroller;
	page_indicator->page_count = 0;
	page_indicator->current_page = -1;

	page_indicator->box = elm_box_add(homescreen_efl_get_win());
	if (!page_indicator->box) {
		LOGE("page_index->box==NULL");
		free(page_indicator);
		return NULL;
	}
	elm_box_homogeneous_set(page_indicator->box, EINA_TRUE);
	elm_box_horizontal_set(page_indicator->box, EINA_TRUE);

	evas_object_size_hint_weight_set(page_indicator->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_box_align_set(page_indicator->box, 0.5, 0.5);
	elm_box_padding_set(page_indicator->box, PAGE_INDICATOR_GAP, 0);
	evas_object_show(page_indicator->box);
	evas_object_move(page_indicator->box, page_indicator->x, page_indicator->y);

	evas_object_smart_callback_add(scroller, "scroll", __page_indicator_scroll_cb, page_indicator);
	evas_object_smart_callback_add(scroller, "scroll,anim,stop", __page_indicator_scroll_anim_stop_cb, page_indicator);

	for (i = 0; i < PAGE_INDICATOR_MAX_PAGE_COUNT; i++) {
		page_indicator->unit[i] = elm_layout_add(homescreen_efl_get_win());
		elm_layout_file_set(page_indicator->unit[i], edj_path, GROUP_PAGE_INDICATOR_UNIT);
		evas_object_size_hint_weight_set(page_indicator->unit[i], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_resize(page_indicator->unit[i], PAGE_INDICATOR_UNIT, PAGE_INDICATOR_UNIT);

		Evas_Object *rect = evas_object_rectangle_add(homescreen_efl_get_win());
		evas_object_color_set(rect, 255, 255, 255, 0);
		evas_object_size_hint_min_set(rect, PAGE_INDICATOR_UNIT, PAGE_INDICATOR_UNIT);
		evas_object_size_hint_max_set(rect, PAGE_INDICATOR_UNIT, PAGE_INDICATOR_UNIT);
		evas_object_resize(rect, PAGE_INDICATOR_UNIT, PAGE_INDICATOR_UNIT);
		evas_object_show(rect);
		elm_object_part_content_set(page_indicator->unit[i], SIZE_SETTER, rect);

		elm_object_signal_callback_add(page_indicator->unit[i], SIGNAL_PAGE_INDICATOR_CLICKED, SIGNAL_SOURCE, __page_indicator_unit_clicked, page_indicator);
	}

	return page_indicator;
}

void page_indicator_set_page_count(page_indicator_t *page_indicator, int count)
{
	if (page_indicator == NULL) {
		LOGE("page indicator is NULL.");
		return ;
	}

	int i;
	int width = count * PAGE_INDICATOR_UNIT + (PAGE_INDICATOR_GAP * (count-1));
	if (page_indicator->page_count == count) {
		return ;
	}
	elm_box_unpack_all(page_indicator->box);
	for (i = 0 ; i < PAGE_INDICATOR_MAX_PAGE_COUNT; i++) {
		if (i < count) {
			elm_box_pack_end(page_indicator->box, page_indicator->unit[i]);
			evas_object_show(page_indicator->unit[i]);
		} else {
			evas_object_move(page_indicator->unit[i], 0, -100);
		}

	}

	page_indicator->x = (WINDOW_W - width) / 2;
	page_indicator->y = WINDOW_H - PAGE_INDICATOR_H - PAGE_INDICATOR_PADDING_BOTTON;
	page_indicator->page_count = count;

	evas_object_resize(page_indicator->box, width, PAGE_INDICATOR_H);
	evas_object_move(page_indicator->box, page_indicator->x, page_indicator->y);
}

void page_indicator_scroller_resize(page_indicator_t *page_indicator, int width, int height)
{
	LOGD("w:%d h:%d", width, height);
	page_indicator->w = width;
	page_indicator->h = height;
}

void page_indicator_set_current_page(page_indicator_t *page_indicator, int page_number)
{
	if (page_indicator == NULL) {
		LOGE("page indicator is NULL.");
		return ;
	}

	LOGD("Set Current :%d, old : %d", page_number, page_indicator->current_page);

	page_indicator->current_page = page_number;

	__page_indicator_set_current_page(page_indicator);
}

static void __page_indicator_scroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	int i = 0;
	int x = 0;
	int from_x = 0, to_x = 0;
	int from_page = 0;
	int to_page = 0;
	double from_page_angle = 0.0;
	double to_page_angle = 0.0;

	page_indicator_t *page_indicator = (page_indicator_t*) data;

	elm_scroller_region_get(obj, &x, NULL, NULL, NULL);
	from_page = x / page_indicator->w;
	to_page = (from_page + 1) % page_indicator->page_count;

	from_x = x;
	to_x = (from_x + page_indicator->w) % (page_indicator->w * page_indicator->page_count);

	to_page_angle = (double)(to_x - (to_page * page_indicator->w)) * 90.0 / page_indicator->w;
	from_page_angle = (to_page_angle + 90) - 180;

	for (i=0; i < page_indicator->page_count; i++) {
		double angle = 0.0;
		double color = 0.0;
		if (i == from_page) {
			angle = from_page_angle;
		} else if (i == to_page) {
			angle = to_page_angle;
		} else {
			angle = 0.0;
		}

		color = fabs(angle) * 2 + 75.0;

		__page_indicator_unit_rotate(page_indicator->unit[i], angle, color);
	}
}

static void __page_indicator_scroll_anim_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (data == NULL) {
		LOGE("Invalid data");
		return ;
	}

	page_indicator_t *page_indicator = (page_indicator_t*) data;

	elm_scroller_current_page_get(obj, &page_indicator->current_page, NULL);
	page_indicator->current_page %= page_indicator->page_count;

	__page_indicator_set_current_page(page_indicator);
}

static void __page_indicator_set_current_page(page_indicator_t *page_indicator)
{
	int i;
	for (i = 0; i < page_indicator->page_count && i < PAGE_INDICATOR_MAX_PAGE_COUNT; i++) {
		Evas_Object *edje = NULL;
		edje = elm_layout_edje_get(page_indicator->unit[i]);
		if (i == page_indicator->current_page) {
			edje_object_signal_emit(edje, SIGNAL_PAGE_IDICATOR_CURRENT, SIGNAL_SOURCE);
		}
		//else if { } // i == center_circle
		else {
			edje_object_signal_emit(edje, SIGNAL_PAGE_IDICATOR_DEFAULT, SIGNAL_SOURCE);
		}
	}
}

static void __page_indicator_unit_rotate(Evas_Object *unit, double angle, double alpha)
{
	Evas_Object *edje = NULL;

	if (!unit) {
		LOGE("Invalid argument : unit is NULL");
		return;
	}

	edje = elm_layout_edje_get(unit);
	if (!edje) {
		LOGE("Failed to get edje from layout");
		return;
	}

	Edje_Message_Float_Set *msg = malloc(sizeof(*msg) + 2 * sizeof(double));
	msg->count = 2;
	msg->val[0] = angle;
	msg->val[1] = alpha;

	edje_object_message_send(edje, EDJE_MESSAGE_FLOAT_SET, 1, msg);
	edje_object_signal_emit(edje, SIGNAL_PAGE_IDICATOR_ROTATION_CHANGE, SIGNAL_SOURCE);

	free(msg);
}

void page_indicator_show(page_indicator_t *page_indicator)
{
	evas_object_show(page_indicator->box);
}

void page_indicator_hide(page_indicator_t *page_indicator)
{
	evas_object_hide(page_indicator->box);
}

static void __page_indicator_unit_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	page_indicator_t *page_indicator = (page_indicator_t*) data;
	Evas_Object *unit = obj;
	int i;
	for (i = 0; i < page_indicator->page_count && i < PAGE_INDICATOR_MAX_PAGE_COUNT; i++) {
		if (unit == page_indicator->unit[i]) {
			elm_scroller_page_bring_in(page_indicator->scroller, i, 0);
			break;
		}
	}
}

