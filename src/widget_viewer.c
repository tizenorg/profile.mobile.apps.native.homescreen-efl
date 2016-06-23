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
#include <widget_service.h>
#include <widget_errno.h>

#include "widget_viewer.h"
#include "util.h"
#include "conf.h"
#include "edc_conf.h"

//#define _TEST_
#ifdef _TEST_
static Eina_Bool __widget_viewer_test_timer_cb(void *data)
{
	Evas_Object *widget = (Evas_Object*) data;
	evas_object_color_set(widget, rand()%255, rand()%255, rand()%255, 100);
	return ECORE_CALLBACK_RENEW;
}
static int count = 0;
#endif
static void __widget_viewer_delete_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

static void __widget_viewer_created_cb(void *data, Evas_Object *obj, void *event_info);
static void __widget_viewer_faulted_cb(void *data, Evas_Object *obj, void *event_info);
static void __widget_viewer_update_cb(void *data, Evas_Object *obj, void *event_info);
static void __widget_viewer_extra_info_updated_cb(void *data, Evas_Object *obj, void *event_info);

void widget_viewer_init(Evas_Object *win)
{
	widget_viewer_evas_init(win);
}

void widget_viewer_fini(void)
{
	widget_viewer_evas_fini();
}

void widget_viewer_add_widget(Evas_Object *parent, widget_data_t *widget_data, int *widget_width, int *widget_height)
{
	Evas_Object *widget_layout;

	LOGD("pkg : %s , Content INfo : %s", widget_data->pkg_name, widget_data->content_info);
	widget_layout = elm_layout_add(parent);
	elm_layout_file_set(widget_layout, util_get_res_file_path(EDJE_DIR"/widget.edj"), GROUP_WIDGET_LY);
	evas_object_size_hint_weight_set(widget_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	widget_service_get_size(widget_data->type, widget_width, widget_height);

	Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(widget_layout));
	evas_object_size_hint_min_set(rect, CLUSTER_W / 4, CLUSTER_H / 4);
	evas_object_size_hint_align_set(rect, 0, 0);
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(rect);
	elm_object_part_content_set(widget_layout, SIZE_SETTER, rect);

#ifndef _TEST_
	int w, h;
	Evas_Object *widget = widget_viewer_evas_add_widget(widget_layout, widget_data->pkg_name, widget_data->content_info, widget_data->period);
	evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	//Notify widget_size_type_e to widget application.
	widget_service_get_size(widget_data->type, &w, &h);
	evas_object_resize(widget, w, h);

	evas_object_smart_callback_add(widget, WIDGET_SMART_SIGNAL_WIDGET_CREATED, __widget_viewer_created_cb, widget_data);
	evas_object_smart_callback_add(widget, WIDGET_SMART_SIGNAL_WIDGET_FAULTED, __widget_viewer_faulted_cb, widget_data);
	evas_object_smart_callback_add(widget, WIDGET_SMART_SIGNAL_UPDATED, __widget_viewer_update_cb, widget_data);
	evas_object_smart_callback_add(widget, WIDGET_SMART_SIGNAL_EXTRA_INFO_UPDATED, __widget_viewer_extra_info_updated_cb, widget_data);

#else // for TEST
/*	Evas_Object *widget = evas_object_rectangle_add(evas_object_evas_get(widget_layout));
	evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(widget, 0, 0, 255, 100);
	evas_object_show(widget);
*/
	char number[1000];
	snprintf(number, sizeof(number), "<font_size=50>%s_%d</font_size>", widget_data->pkg_name, count++);
	Evas_Object *widget = elm_label_add(evas_object_evas_get(widget_layout));
	elm_object_style_set(widget, "popup/default");
	evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(widget, number);
	evas_object_color_set(widget, 0, 0, 255, 100);
	evas_object_show(widget);

	Ecore_Timer *timer = ecore_timer_add(1, __widget_viewer_test_timer_cb, widget);
#endif
	elm_object_part_content_set(widget_layout, WIDGET_CONTENT, widget);
	elm_object_signal_callback_add(widget_layout, SIGNAL_DELETE_BUTTON_CLICKED, SIGNAL_SOURCE, __widget_viewer_delete_btn_clicked_cb, (void *)widget_data);

	evas_object_show(widget);
	evas_object_show(widget_layout);

	widget_data->widget_content = widget;
	widget_data->widget_layout = widget_layout;
}

static void __widget_viewer_delete_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	widget_data_t *widget_data  = (widget_data_t *)data;
	cluster_data_delete(widget_data);
}

void widget_viewer_send_cancel_click_event(widget_data_t *widget_data)
{
	LOGD("Send to : %s", widget_data->pkg_name);
	Evas_Object *widget_obj = NULL;
	widget_obj = elm_object_part_content_get(widget_data->widget_layout, WIDGET_CONTENT);
	widget_viewer_evas_cancel_click_event(widget_obj);
}

void widget_viewer_thumbnail_add(widget_data_t *widget_data)
{
	Evas_Object *image = NULL;

	image = evas_object_image_filled_add(evas_object_evas_get(widget_data->widget_layout));
	if (!image) {
		LOGE("Could not create image proxy object");
		return;
	}

	if (!evas_object_image_source_set(image, widget_data->widget_content)) {
		evas_object_del(image);
		LOGE("Could not set proxy image source");
		return;
	}

	evas_object_image_source_visible_set(image, EINA_FALSE);

	int gw, gh;
	evas_object_geometry_get(widget_data->widget_layout, NULL, NULL, &gw, &gh);
	evas_object_image_fill_set(image, 0, 0, gw, gh);
	evas_object_show(image);

	elm_object_part_content_set(widget_data->widget_layout, WIDGET_THUMBNAIL, image);
	elm_object_part_content_unset(widget_data->widget_layout, WIDGET_CONTENT);
}

void widget_viewer_thumbnail_delete(widget_data_t *widget_data)
{
	if (widget_data && widget_data->widget_layout && widget_data->widget_content) {
		Evas_Object *image = NULL;
		image = elm_object_part_content_get(widget_data->widget_layout, WIDGET_THUMBNAIL);
		evas_object_image_source_visible_set(image, EINA_TRUE);
		elm_object_part_content_unset(widget_data->widget_layout, WIDGET_THUMBNAIL);
		evas_object_del(image);
		elm_object_part_content_set(widget_data->widget_layout, WIDGET_CONTENT, widget_data->widget_content);
	}
}

static void __widget_viewer_created_cb(void *data, Evas_Object *obj, void *event_info)
{
	widget_data_t *widget_data = (widget_data_t*) data;
	LOGD("Pkg : %s ", widget_data->pkg_name);
	const char *content_info = widget_viewer_evas_get_content_info(widget_data->widget_content);
	if (content_info) {
		if (widget_data->content_info) {
			free(widget_data->content_info);
			widget_data->content_info = NULL;
		}
		widget_data->content_info = strdup(content_info);
		LOGD("content info :%s", widget_data->content_info);
		cluster_data_update(widget_data);
	}
}

static void __widget_viewer_faulted_cb(void *data, Evas_Object *obj, void *event_info)
{
	widget_data_t *widget = (widget_data_t*) data;
	LOGD("Pkg : %s ", widget->pkg_name);
}

static void __widget_viewer_update_cb(void *data, Evas_Object *obj, void *event_info)
{
	widget_data_t *widget_data = (widget_data_t*) data;
	LOGD("Pkg : %s ", widget_data->pkg_name);
	const char *content_info = widget_viewer_evas_get_content_info(widget_data->widget_content);
	if (content_info) {
		if (widget_data->content_info) {
			free(widget_data->content_info);
			widget_data->content_info = NULL;
		}
		widget_data->content_info = strdup(content_info);
		LOGD("content info :%s", widget_data->content_info);
		cluster_data_update(widget_data);
	}
}

static void __widget_viewer_extra_info_updated_cb(void *data, Evas_Object *obj, void *event_info)
{
	widget_data_t *widget_data = (widget_data_t*) data;
	LOGD("Pkg : %s ", widget_data->pkg_name);
	const char *content_info = widget_viewer_evas_get_content_info(widget_data->widget_content);
	if (content_info) {
		if (widget_data->content_info) {
			free(widget_data->content_info);
			widget_data->content_info = NULL;
		}
		widget_data->content_info = strdup(content_info);
		LOGD("content info :%s", widget_data->content_info);
		cluster_data_update(widget_data);
	}
}

void widget_viewer_pause_widget(widget_data_t *widget_data)
{
	int ret = WIDGET_ERROR_NONE;

	if (!widget_data) {
		LOGE("widget_data is NULL");
		return;
	}

	ret = widget_viewer_evas_pause_widget(widget_data->widget_content);
	if (ret != WIDGET_ERROR_NONE) {
		LOGE("Failed to pause widget(%d)", ret);
	}
}

void widget_viewer_resume_widget(widget_data_t *widget_data)
{
	int ret = WIDGET_ERROR_NONE;

	if (!widget_data) {
		LOGE("widget_data is NULL");
		return;
	}

	ret = widget_viewer_evas_resume_widget(widget_data->widget_content);
	if (ret != WIDGET_ERROR_NONE) {
		LOGE("Failed to resume widget(%d)", ret);
	}
}
