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
#include <widget_service.h>
#include <widget_viewer_evas.h>
#include <widget_errno.h>
#include <shortcut_manager.h>


#include "util.h"
#include "conf.h"

#include "add_viewer/add_viewer.h"
#include "add_viewer/list_view.h"
#include "add_viewer/view.h"
#include "add_viewer/widget.h"

static Evas_Object *__list_view_create_list(Evas_Object *content);
static void __list_view_genlist_theme_set(Evas_Object *genlist);
static char *__gl_text_get_cb(void *data, Evas_Object *obj, const char *part);
static Evas_Object *__gl_content_get_cb (void *data, Evas_Object *obj, const char *part);
static Evas_Object *__list_view_widget_box_create(Evas_Object *obj, widget_t *widget);
static Evas_Object *__list_view_preview_box_create(Evas_Object *main_box, widget_t *widget, preview_t *preview);
static void __list_view_item_realized_cb(void *data, Evas_Object *obj, void *event_info);
static void __list_view_preview_clicked_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

Evas_Object *list_view_create_content(Evas_Object *naviframe, Evas_Object *genlist)
{
	Elm_Object_Item *item;
	Evas_Object *content;

	content = view_content_create(naviframe, NULL);
	if (!content) {
		return NULL;
	}

	genlist = __list_view_create_list(content);
	elm_object_part_content_set(content, "content", genlist);

	item = elm_naviframe_item_push(naviframe, _("IDS_HS_HEADER_ADD_WIDGET"), NULL, NULL, content, NULL);
	view_content_show(content);
	view_content_show_index(content);

	if (!item) {
		LOGD("Failed to push an item\n");
		return NULL;
	}

	return content;
}

static Evas_Object *__list_view_create_list(Evas_Object *content)
{
	Elm_Genlist_Item_Class *itc_widget;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *widget_gl_item = NULL;
	Eina_List *widget_list = NULL, *l = NULL;
	widget_t *widget = NULL;

	genlist = elm_genlist_add(content);
	if (!genlist) {
		LOGE("Failed to create a genlist\n");
		return NULL;
	}

	elm_scroller_bounce_set(genlist, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_NONE);

	__list_view_genlist_theme_set(genlist);

	itc_widget = elm_genlist_item_class_new();
	itc_widget->item_style = "widget,leaf";
	itc_widget->func.text_get = __gl_text_get_cb;
	itc_widget->func.content_get = __gl_content_get_cb;

	widget_list = widget_get_widget_list();
	if (!widget_list) {
		LOGE("Can not get widget list");
		return NULL;
	}

	LOGD("Widget list length: %d", eina_list_count(widget_list));
	EINA_LIST_FOREACH(widget_list, l, widget) {
		LOGD("Genlist append");
		widget_gl_item = elm_genlist_item_append(genlist, itc_widget, widget, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		widget->genlist_item = widget_gl_item;
	}

	evas_object_smart_callback_add(genlist, "realized", __list_view_item_realized_cb, NULL);

	elm_genlist_item_class_free(itc_widget);

	return genlist;
}

static void __list_view_genlist_theme_set(Evas_Object *genlist)
{
	Elm_Theme *main_theme;

	main_theme = add_viewer_get_main_theme();
	if (main_theme)
		elm_object_theme_set(genlist, main_theme);
}

static char *__gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	widget_t *widget = data;

	if (!widget || !part)
		return NULL;

	if(!strcmp(part, "elm.text"))
		return strdup(widget->widget_id);

	return strdup("widget name not found");
}

static Evas_Object *__gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	LOGD("Part: %s", part);

	Evas_Object *main_box = NULL;
	Eina_List *widget_list = NULL;
	widget_t *widget = data;

	if (!widget || !part)
		return NULL;

	if (!strcmp(part, "bg")) {
		return NULL;
	}

	widget_list = widget_get_widget_list();
	if (!widget_list) {
		LOGE("Can not get widget list");
		return NULL;
	}

	main_box = __list_view_widget_box_create(obj, widget);
	if (!main_box) {
		LOGE("Can not create preview box");
		return NULL;
	}

	return main_box;
}

static Evas_Object *__list_view_widget_box_create(Evas_Object *obj, widget_t *widget)
{
	LOGD("Create box for %s", widget->widget_id);

	Evas_Object *main_box = NULL;
	Evas_Object *mid_box = NULL;
       	Evas_Object *small_box = NULL;
	Evas_Object *preview_layout = NULL;
	Eina_List *l;
	Eina_List *children;
	preview_t *preview;
	int tmp = 0;
	int h = 0;
	int filled = 0;
	int height = 0;

	main_box = elm_box_add(obj);
	if (!main_box) {
		LOGE("Can not create preview box");
		return NULL;
	}

	elm_box_horizontal_set(main_box, EINA_FALSE);
	elm_box_homogeneous_set(main_box, EINA_FALSE);
	elm_box_padding_set(main_box, 0, 30);
	elm_box_align_set(main_box, 0.0, 0.0);

	EINA_LIST_FOREACH(widget->preview_list, l, preview) {

		preview_layout = __list_view_preview_box_create(main_box, widget, preview);
		if(!preview_layout) {
			LOGE("Can not create preview layout");
			return NULL;
		}

		elm_object_part_text_set(preview_layout, "text", widget->widget_id);

		//elm_box_pack_end(main_box, preview_layout);

		evas_object_size_hint_min_get(preview_layout, NULL, &h);
		if (h <= 0) {
			LOGE("Preview layout height is unknown");
			continue;
		}

		evas_object_show(preview_layout);

		switch (preview->type) {
		case WIDGET_SIZE_TYPE_4x6:
		case WIDGET_SIZE_TYPE_4x5:
		case WIDGET_SIZE_TYPE_4x4:
		case WIDGET_SIZE_TYPE_4x3:
		case WIDGET_SIZE_TYPE_4x2:
		case WIDGET_SIZE_TYPE_4x1:
			if (!mid_box) {
				mid_box = elm_box_add(main_box);
				if (!mid_box) {
					LOGE("Failed to add mid box\n");
					return NULL;
				}

				elm_box_align_set(mid_box, 0.0, 0.0);
				elm_box_horizontal_set(mid_box, EINA_TRUE);
				elm_box_homogeneous_set(mid_box, EINA_FALSE);
				elm_box_padding_set(mid_box, 12, 0);
				evas_object_size_hint_align_set(mid_box, 0.0, 0.0);
				filled = 0;
			}

			if (tmp < h) {
				tmp = h;
			}
			filled++;
			evas_object_size_hint_align_set(preview_layout, 0.0, 0.0);
			elm_box_pack_end(mid_box, preview_layout);

			if (filled == 2) {
				elm_box_pack_end(main_box, mid_box);
				evas_object_show(mid_box);
				mid_box = NULL;
				height += (tmp + 30);
				tmp = 0;
			}
			break;
		case WIDGET_SIZE_TYPE_2x2:
		case WIDGET_SIZE_TYPE_2x1:
		case WIDGET_SIZE_TYPE_1x1:
			if (mid_box) {
				elm_box_pack_end(main_box, mid_box);
				evas_object_show(mid_box);
				height += (h + 30);
				filled = 0;
				tmp = 0;
			}

			if (!small_box) {
				small_box = elm_box_add(main_box);
				if (!small_box) {
					LOGD("Failed to add small box\n");
					evas_object_del(preview_layout);
					evas_object_del(main_box);
					return NULL;
				}

				elm_box_align_set(small_box, 0.0, 0.0);
				elm_box_horizontal_set(small_box, EINA_TRUE);
				elm_box_homogeneous_set(small_box, EINA_FALSE);
				elm_box_padding_set(small_box, 12, 0);
				evas_object_size_hint_align_set(small_box, 0.0, 0.0);
				filled = 0;
			}

			if (tmp < h) {
				tmp = h;
			}

			filled++;
			evas_object_size_hint_align_set(preview_layout, 0.0, 0.0);
			elm_box_pack_end(small_box, preview_layout);

			if (filled == 3) {
				evas_object_show(small_box);
				elm_box_pack_end(main_box, small_box);
				height += (tmp + 30);
				small_box = NULL;
				tmp = 0;
			}
			break;
			return NULL;
		default:
			LOGE("Unsupported size\n");
			break;
			return NULL;
		}

	}

	if (small_box) {
		evas_object_show(small_box);
		elm_box_pack_end(main_box, small_box);
		height += (tmp + 30);
	} else if (mid_box) {
		elm_box_pack_end(main_box, mid_box);
		evas_object_show(mid_box);
		height += (tmp + 30);
	}

	children = elm_box_children_get(main_box);
	if (eina_list_count(children) == 0) {
		evas_object_del(main_box);
		LOGE("Preview is not exists");
		main_box = NULL;
	} else {
		height += 20;

		evas_object_size_hint_min_set(main_box, 0, height);
		evas_object_size_hint_max_set(main_box, 0, height);
		evas_object_show(main_box);
	}

	eina_list_free(children);

	return main_box;
}

static Evas_Object *__list_view_preview_box_create(Evas_Object *main_box, widget_t *widget, preview_t *preview)
{
	LOGD("Create preview: type->%d path->%s", preview->type, preview->path);

	Evas_Object *preview_layout = NULL;
	Evas_Object *preview_img = NULL;
	char *size_str = NULL;
	int w = 0, h = 0;
	int ret = 0;

	if (!preview || !preview->path || !preview->type) {
		LOGE("Path to preview image is empty");
		return NULL;
	}

	switch (preview->type) {
	case WIDGET_SIZE_TYPE_1x1:
		size_str = "preview,1x1";
		break;
	case WIDGET_SIZE_TYPE_2x1:
		size_str = "preview,2x1";
		break;
	case WIDGET_SIZE_TYPE_2x2:
		size_str = "preview,2x2";
		break;
	case WIDGET_SIZE_TYPE_4x1:
		size_str = "preview,4x1";
		break;
	case WIDGET_SIZE_TYPE_4x2:
		size_str = "preview,4x2";
		break;
	case WIDGET_SIZE_TYPE_4x3:
		size_str = "preview,4x3";
		break;
	case WIDGET_SIZE_TYPE_4x4:
		size_str = "preview,4x4";
		break;
	case WIDGET_SIZE_TYPE_4x5:
		size_str = "preview,4x5";
		break;
	case WIDGET_SIZE_TYPE_4x6:
		size_str = "preview,4x6";
		break;
	case WIDGET_SIZE_TYPE_EASY_1x1:
		size_str = "easy,preview,1x1";
		break;
	case WIDGET_SIZE_TYPE_EASY_3x1:
		size_str = "easy,preview,3x1";
		break;
	case WIDGET_SIZE_TYPE_EASY_3x3:
		size_str = "easy,preview,3x3";
		break;
	default:
		return NULL;
	}

	preview_layout = elm_layout_add(main_box);
	if (!preview_layout) {
		LOGE("Can not create preview layout");
		return NULL;
	}

	LOGD("Widget size_str: %s", size_str);
	ret = elm_layout_file_set(preview_layout, util_get_res_file_path(EDJE_DIR"/preview.edj"), size_str);
	if (ret != EINA_TRUE) {
		LOGE("Can not set preview layout");
		evas_object_del(preview_layout);
		return NULL;
	}

	preview_img = evas_object_image_filled_add(evas_object_evas_get(preview_layout));
	if (!preview_img) {
		LOGE("Can not create image object");
		evas_object_del(preview_layout);
		return NULL;
	}

	int *preview_type = calloc(1, sizeof(int));
	if (!preview_type) {
		LOGE("Can not allocate memory for additional data");
	}

	*preview_type = preview->type;

	evas_object_data_set(preview_layout, "preview_type", preview_type);
	evas_object_event_callback_add(preview_layout, EVAS_CALLBACK_MOUSE_UP, __list_view_preview_clicked_cb, widget);

	evas_object_image_file_set(preview_img, preview->path, NULL);
	evas_object_image_size_get(preview_img, &w, &h);
	evas_object_image_fill_set(preview_img, 0, 0, w, h);

	edje_object_size_min_calc(elm_layout_edje_get(preview_layout), &w, &h);
	evas_object_size_hint_min_set(preview_layout, w, h);

	elm_object_part_content_set(preview_layout, "preview", preview_img);

	return preview_layout;
}

static void __list_view_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("item realized_cb");

	char ch[2] = {0, };
	int ret = 0;
	Elm_Object_Item *item = event_info;
	widget_t *widget = NULL;


	widget = (widget_t *)elm_object_item_data_get(item);

	LOGD("item data: %s", widget->widget_id);
	ch[0] = widget->widget_id[0];

	if (strlen(ch) == 0) {
		LOGE("Can not get first char of widget name");
		return;
	}

	ret = view_index_set_index(ch);
	if (ret != 0)
		LOGE("Can not set selected index item");
}

static void __list_view_preview_clicked_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	LOGD("Preview clicked");

	widget_t *widget = NULL;
	int w = 0, h = 0;
	int ret = -1;
	int *size = NULL;
	shortcut_widget_size_e size_type;

	size = evas_object_data_del(obj, "preview_type");
	if (*size == WIDGET_SIZE_TYPE_UNKNOWN) {
		LOGE("Can not get widgets size type");
		return;
	}

	widget = (widget_t *)data;
	if (!widget) {
		LOGE("Can not get widget");
		return;
	}


	ret = widget_service_get_size(*size, &w, &h);
	if (ret != 0) {
		LOGE("Can not get widget size");
		return;
	}

	switch (*size) {
	case WIDGET_SIZE_TYPE_1x1:
		size_type = WIDGET_SIZE_1x1;
		break;
	case WIDGET_SIZE_TYPE_2x1:
		size_type = WIDGET_SIZE_2x1;
		break;
	case WIDGET_SIZE_TYPE_2x2:
		size_type = WIDGET_SIZE_2x2;
		break;
	case WIDGET_SIZE_TYPE_4x1:
		size_type = WIDGET_SIZE_4x1;
		break;
	case WIDGET_SIZE_TYPE_4x2:
		size_type = WIDGET_SIZE_4x2;
		break;
	case WIDGET_SIZE_TYPE_4x3:
		size_type = WIDGET_SIZE_4x3;
		break;
	case WIDGET_SIZE_TYPE_4x4:
		size_type = WIDGET_SIZE_4x4;
		break;
	case WIDGET_SIZE_TYPE_4x5:
		size_type = WIDGET_SIZE_4x5;
		break;
	case WIDGET_SIZE_TYPE_4x6:
		size_type = WIDGET_SIZE_4x6;
		break;
	case WIDGET_SIZE_TYPE_EASY_1x1:
		size_type = WIDGET_SIZE_EASY_1x1;
		break;
	case WIDGET_SIZE_TYPE_EASY_3x1:
		size_type = WIDGET_SIZE_EASY_3x1;
		break;
	case WIDGET_SIZE_TYPE_EASY_3x3:
		size_type = WIDGET_SIZE_EASY_3x3;
		break;
	default:
		LOGE("Invalid size type\n");
		free(size);
		return;
	}

	ret = shortcut_add_to_home_widget(widget->widget_id, size_type, widget->widget_id, NULL, -1.0f, 1, NULL, NULL);
	if (ret != WIDGET_ERROR_NONE)
		LOGE("ERROR: %s", get_error_message(ret));

	free(size);

	add_viewer_window_delete();
}

