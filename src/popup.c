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

#include "popup.h"

#include <stdbool.h>
#include <Elementary.h>

#include "homescreen-efl.h"
#include "util.h"
#include "livebox_all_pages.h"
#include "app_icon.h"

#define EDJE_LAYOUT_PATH "/usr/apps/com.samsung.home-lite/res/edje/popup.edj"
#define UNABLE_PART "unable_to_add_items"
#define UNABLE_POPUP_TITLE "<align=center>Unable to add</align>"
#define SORTED_POPUP "<align=center><font_size=24> Unable to rearrange icons <br> sorted by name (A-Z).</font> </align>"

#define POPUP_DATA_KEY "cb_data"

static struct
{
	Evas_Object *opened_popup;
} s_info = {
	.opened_popup = NULL
};

typedef struct _cb_data_t {
	popup_confirm_cb cb_ok;
	popup_cancel_cb cb_cancel;
	void *data;
} cb_data_t;

/*==========================PRIVATE FUNCTIONS DEFINITIONS======================================*/
static void __popup_init_two_buttons(Evas_Object *popup, popup_confirm_cb confirm_cb,
				popup_cancel_cb cancel_cb, popup_data_t *p_data, void *cb_data);
static void __popup_init_info(Evas_Object *popup, popup_data_t *p_data);
/*==========================POPUP CALLBACKS DEFINITIONS========================================*/
static void __popup_btn_ok_clicked_cb(void *data, Evas_Object *obj, void *ei);
static void __popup_btn_cancel_clicked_cb(void *data, Evas_Object *obj, void *ei);

/*==========================PUBLIC FUNCTIONS IMPLEMENTATION===================================*/
HAPI popup_data_t *popup_create_data(void)
{
	popup_data_t *data = (popup_data_t *) calloc(1, sizeof(popup_data_t));
	return data;
}

HAPI void popup_data_free(popup_data_t *p_data)
{
	if (!p_data) {
		LOGE("[INVALID_PARAM]");
		return;
	}

	if (p_data->popup_title_text)
		free(p_data->popup_title_text);
	if (p_data->popup_content_text)
		free(p_data->popup_content_text);
	if (p_data->popup_confirm_text)
		free(p_data->popup_confirm_text);
	if (p_data->popup_cancel_text)
		free(p_data->popup_cancel_text);

	free(p_data);
}

HAPI void popup_show(popup_data_t *p_data, popup_confirm_cb confirm_cb, popup_cancel_cb cancel_cb,
				void *cb_data)
{
	Evas_Object *popup = NULL;

	if (!p_data) {
		LOGE("[INVALID_DATA]");
		return;
	}

	popup = elm_popup_add(home_screen_get_win());
	if (!popup) {
		LOGE("[FAILED][elm_popup_add]");
		evas_object_del(popup);
		popup_data_free(p_data);
		return;
	}

	elm_popup_orient_set(popup, p_data->orientation);

	switch (p_data->type) {
	case POPUP_TWO_BUTTONS:
		__popup_init_two_buttons(popup, confirm_cb, cancel_cb, p_data, cb_data);
		break;
	case POPUP_INFO:
		__popup_init_info(popup, p_data);
	}

	s_info.opened_popup = popup;
	evas_object_layer_set(popup, EVAS_LAYER_MAX);
	evas_object_show(popup);
}

HAPI Eina_Bool popup_destroy(void)
{
	if (s_info.opened_popup) {
		evas_object_del(s_info.opened_popup);
		s_info.opened_popup = NULL;
		return EINA_TRUE;
	}

	return EINA_FALSE;
}
/*==========================PRIVATE FUNCTIONS IMPLEMENTATION==================================*/
static void __popup_init_two_buttons(Evas_Object *popup, popup_confirm_cb confirm_cb,
				popup_cancel_cb cancel_cb, popup_data_t *p_data, void *cb_data)
{
	Evas_Object *btn_ok = NULL;
	Evas_Object *btn_cancel = NULL;
	cb_data_t *data = NULL;

	if (!p_data) {
		LOGE("[INVALID_PARAM][p_data=NULL]");
		return;
	}

	if (!popup) {
		LOGE("[INVALID_PARAM]");
		popup_data_free(p_data);
		return;
	}

	data = (cb_data_t *) malloc(sizeof(cb_data_t));
	if (!data) {
		LOGE("[FAILED][malloc]");
		popup_data_free(p_data);
		return;
	}

	btn_ok = elm_button_add(popup);
	if (!btn_ok) {
		LOGE("[FAILED][elm_button_add]");
		evas_object_del(popup);
		popup_data_free(p_data);
		free(data);
		return;
	}

	btn_cancel = elm_button_add(popup);
	if (!btn_cancel) {
		LOGE("[FAILED][elm_button_add]");
		evas_object_del(popup);
		popup_data_free(p_data);
		free(data);
		return;
	}

	/*setting popup properties.*/
	elm_object_part_text_set(popup, "title,text", p_data->popup_title_text);
	elm_object_part_text_set(popup, "default", p_data->popup_content_text);

	elm_object_part_text_set(btn_ok, "default", p_data->popup_confirm_text);
	elm_object_part_text_set(btn_cancel, "default", p_data->popup_cancel_text);

	elm_object_part_content_set(popup, "button1", btn_cancel);
	elm_object_part_content_set(popup, "button2", btn_ok);

	data->cb_cancel = cancel_cb;
	data->cb_ok = confirm_cb;
	data->data = cb_data;

	evas_object_smart_callback_add(btn_ok, "clicked", __popup_btn_ok_clicked_cb, data);
	evas_object_smart_callback_add(btn_cancel, "clicked", __popup_btn_cancel_clicked_cb, data);

	popup_data_free(p_data);
}

static void __popup_init_info(Evas_Object *popup, popup_data_t *p_data)
{
	if (!p_data) {
		LOGE("[INVALID_PARAM][p_data=NULL]");
		return;
	}

	if (!popup) {
		LOGE("[INVALID_PARAM]");
		popup_data_free(p_data);
		return;
	}

	elm_object_style_set(popup, "toast");
	elm_object_part_text_set(popup, "default", p_data->popup_content_text);
	elm_popup_timeout_set(popup, p_data->visible_time);

	popup_data_free(p_data);
}

/*==================================POPUP CALLBACKS IMPLEMENTATION============================*/
static void __popup_btn_ok_clicked_cb(void *data, Evas_Object *obj, void *ei)
{
	LOGI("__popup_btn_ok_clicked_cb");
	cb_data_t *cb_data = (cb_data_t *) data;

	if (!cb_data) {
		LOGE("[INVALID_CB_DATA]");
		return;
	}

	if (cb_data->cb_ok) cb_data->cb_ok(cb_data->data);

	popup_destroy();
}

static void __popup_btn_cancel_clicked_cb(void *data, Evas_Object *obj, void *ei)
{
	LOGI("__popup_btn_cancel_clicked_cb");
	cb_data_t *cb_data = (cb_data_t *) data;

	if (!data) {
		LOGE("[INVALID_CB_DATA]");
		return;
	}

	if (cb_data->cb_cancel) cb_data->cb_cancel(cb_data->data);

	popup_destroy();
}
