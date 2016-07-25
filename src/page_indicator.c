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
#include "cluster_view.h"
#include "homescreen-efl.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"

static void __page_indicator_scroll_cb(void *data, Evas_Object *obj, void *event_info);
static void __page_indicator_scroll_anim_stop_cb(void *data, Evas_Object *obj, void *event_info);
static void __page_indicator_set_current_page(page_indicator_t *page_indicator);
static void __page_indicator_unit_rotate(Evas_Object *unit, double angle, double alpha);
static void __page_indicator_unit_clicked_cb(void *data, Evas_Object *obj, void *event_info);

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

#if 0
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

		unit_button = elm_button_add(page_indicator->unit[i]);
		if (unit_button == NULL)
			return NULL;

		elm_object_theme_set(unit_button, cluster_view_get_theme());
		elm_object_style_set(unit_button, "transparent");
		elm_object_part_content_set(page_indicator->unit[i], PAGE_INDICATOR_UNIT_BUTTON, unit_button);
		evas_object_data_set(unit_button, "page_indicator_unit_button", page_indicator->unit[i]);
		evas_object_smart_callback_add(unit_button, "clicked", __page_indicator_unit_clicked_cb, page_indicator);
	}
#endif 

#if 1
	for (i = 0; i < PAGE_INDICATOR_MAX_PAGE_COUNT; i++) {
		page_indicator->unit[i] = elm_button_add(homescreen_efl_get_win());
		elm_object_theme_set(page_indicator->unit[i], cluster_view_get_theme());
		elm_object_style_set(page_indicator->unit[i], "page_indicator_unit");
		evas_object_smart_callback_add(page_indicator->unit[i], "clicked", __page_indicator_unit_clicked_cb, page_indicator);

		Evas_Object *rect = evas_object_rectangle_add(homescreen_efl_get_win());
		evas_object_color_set(rect, 255, 255, 255, 0);
		evas_object_size_hint_min_set(rect, PAGE_INDICATOR_UNIT, PAGE_INDICATOR_UNIT);
		evas_object_size_hint_max_set(rect, PAGE_INDICATOR_UNIT, PAGE_INDICATOR_UNIT);
		evas_object_resize(rect, PAGE_INDICATOR_UNIT, PAGE_INDICATOR_UNIT);
		evas_object_show(rect);
		elm_object_part_content_set(page_indicator->unit[i], SIZE_SETTER, rect);
	}
#endif

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
	LOGD("page count: %d", count);

	if (count > PAGE_INDICATOR_MAX_PAGE_COUNT) {
		for (i = 0 ; i < PAGE_INDICATOR_MAX_PAGE_COUNT; i++) {
			if (i == 3) {
				elm_object_style_set(page_indicator->unit[i], "page_indicator_unit_center");
			}
			elm_box_pack_end(page_indicator->box, page_indicator->unit[i]);
			evas_object_show(page_indicator->unit[i]);
		}
	} else {
		for (i = 0 ; i < PAGE_INDICATOR_MAX_PAGE_COUNT; i++) {
			if (i < count) {
				if (i == 3) {
					elm_object_style_set(page_indicator->unit[i], "page_indicator_unit");
				}
				elm_box_pack_end(page_indicator->box, page_indicator->unit[i]);
				evas_object_show(page_indicator->unit[i]);
			} else {
				evas_object_move(page_indicator->unit[i], 0, -100);
			}
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
	int x = 0;
	int current_x = 0;
	double angle = 0.0;
	double color = 0.0;
	int next_page = -1;

	page_indicator_t *page_indicator = (page_indicator_t*) data;
	if (!page_indicator) {
		LOGE("Invalid page_indicator");
		return;
	}
	elm_scroller_region_get(obj, &x, NULL, NULL, NULL);

	current_x = page_indicator->current_page * page_indicator->w;
	angle = (double)(x - current_x) / page_indicator->w * 90.0;
	if (angle > 90.0) {
		angle = (double)(x - page_indicator->w * page_indicator->page_count) / page_indicator->w * 90.0;
		next_page = page_indicator->page_count - 1;
	} else {
		next_page = page_indicator->current_page + (angle > 0 ? 1 : -1);
		next_page %= page_indicator->page_count;
	}

	color = fabs(angle) * 2;

	__page_indicator_unit_rotate(page_indicator->unit[page_indicator->current_page], 90 + angle, 255.0 - color);
	__page_indicator_unit_rotate(page_indicator->unit[next_page], (angle < 0 ? 360 + angle : angle), 75.0 + color);
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
	int cur_indicator = 0;
	int cur_page = 0;

	cur_page = page_indicator->current_page;

	if (page_indicator->page_count <= PAGE_INDICATOR_MAX_PAGE_COUNT) {
		cur_indicator = cur_page;
	}
	else {
		if (cur_page < 3) {
			cur_indicator = cur_page;
		}
		else if (page_indicator->page_count - 3 - 1 < cur_page) {
			cur_indicator = (PAGE_INDICATOR_MAX_PAGE_COUNT - (page_indicator->page_count - 1 - cur_page)) - 1;
		}
		else {
			cur_indicator = PAGE_INDICATOR_MAX_PAGE_COUNT / 2;
		} 
	}

	for (i = 0; i < PAGE_INDICATOR_MAX_PAGE_COUNT; i++)
	{
		Evas_Object *edje = NULL;
		edje = elm_layout_edje_get(page_indicator->unit[i]);

		if (i == cur_indicator) {
			if (i == 3) {
				char cur_page_num[1024];
				snprintf(cur_page_num, sizeof(cur_page_num), "%d", cur_page);
				LOGD("Current page is %s, set the number to the page indicator", cur_page_num);
				if (edje_object_part_text_set(edje, "page_num", cur_page_num) == EINA_FALSE) {
					LOGE("Failed to set text on the page indicator");
				}
			}
			edje_object_signal_emit(edje, SIGNAL_PAGE_INDICATOR_CURRENT, SIGNAL_SOURCE);
		}
		else {
			edje_object_signal_emit(edje, SIGNAL_PAGE_INDICATOR_DEFAULT, SIGNAL_SOURCE);
		}
	}
}

static void __page_indicator_unit_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	unsigned int clicked_indicator = 0;
	Eina_List *list = NULL;
	Eina_List *cur_list = NULL;
	Eina_List *next_list = NULL;
	Evas_Object *page_indicator_unit = NULL;
	page_indicator_t *page_indicator = data;


	list = elm_box_children_get(page_indicator->box);
	if (list == NULL) {
		LOGE("Failed to get the list from the box");
		return;
	}

	EINA_LIST_FOREACH_SAFE(list, cur_list, next_list, page_indicator_unit) {
		if (obj == page_indicator_unit) break;	
		clicked_indicator++;
	}

	if (page_indicator->page_count > PAGE_INDICATOR_MAX_PAGE_COUNT) {
		if (clicked_indicator < 3) {
			LOGD("Clicked idx is smaller than CENTER, Go to %dth page", clicked_indicator); 
			elm_scroller_page_bring_in(page_indicator->scroller, clicked_indicator, 0);
		} else if (clicked_indicator== 3) {
			LOGD("Clicked idx is CENTER, Go to %dth page", (page_indicator->page_count - 1) / 2); 
			elm_scroller_page_bring_in(page_indicator->scroller, (page_indicator->page_count - 1) / 2, 0);
		} else {
			LOGD("Clicked idx is bigger than CENTER, Go to %dth page", (page_indicator->page_count - PAGE_INDICATOR_MAX_PAGE_COUNT) + clicked_indicator); 
			elm_scroller_page_bring_in(page_indicator->scroller, (page_indicator->page_count - PAGE_INDICATOR_MAX_PAGE_COUNT + clicked_indicator), 0);
		}
	} else {
		LOGD("Page Indicator is CLICKED %dth page", clicked_indicator);
		elm_scroller_page_bring_in(page_indicator->scroller, clicked_indicator, 0);
	}
}

static void __page_indicator_unit_rotate(Evas_Object *unit, double angle, double alpha)
{
	Evas_Object *edje = NULL;
	LOGD("Rotate animation start");

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
	edje_object_signal_emit(edje, SIGNAL_PAGE_INDICATOR_ROTATION_CHANGE, SIGNAL_SOURCE);

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

