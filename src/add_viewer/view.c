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

#include "conf.h"
#include "util.h"

#include "add_viewer/add_viewer.h"
#include "add_viewer/widget.h"
#include "add_viewer/view.h"

static Evas_Object *__index_create(Evas_Object *layout, void *data);
static void __view_index_cb(void *data, Evas_Object *obj, void *event_info);
static int __view_compare_widget_idx(const void *data1, const void *data2);

static struct {
	Evas_Object *index;
} s_info = {
	.index = NULL
};

Evas_Object *view_content_create(Evas_Object *frame, void *data)
{
	Evas_Object *layout;

	layout = elm_layout_add(frame);
	if (!layout)
		return NULL;

	if (elm_layout_file_set(layout, util_get_res_file_path(EDJE_DIR"/preview.edj"), "content,frame") != EINA_TRUE) {
		LOGE("Can not set layout file");
		evas_object_del(layout);
		return NULL;
	}

	s_info.index = __index_create(layout, data);
	if (!s_info.index) {
		evas_object_del(layout);
		return NULL;
	}
	elm_object_part_content_set(layout, "index", s_info.index);

	return layout;
}

void view_content_show(Evas_Object *layout)
{
	if (/*model_is_easy_mode()*/0) {
		elm_object_signal_emit(layout, "display,easy", "container");
	} else {
		elm_object_signal_emit(layout, "display,normal", "container");
	}
}

void view_content_show_index(Evas_Object *layout)
{
	if (/*!model_is_easy_mode()*/1) {
		elm_object_signal_emit(layout, "display,index", "container");
	}
}

Evas_Object *view_get_index(void)
{
	return s_info.index;
}

int view_index_set_index(const char *idx)
{
	Elm_Index_Item *item = NULL;

	LOGD("Find item with idx: %s", idx);

	item = elm_index_item_find(s_info.index, idx);
	if(!item)
	{
		LOGE("Can not find index item");
		return 1;
	}
	elm_index_item_selected_set(item, EINA_TRUE);

	return 0;
}

static Evas_Object *__index_create(Evas_Object *layout, void *data)
{
	Evas_Object *index;
	const char *idx_str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char ch[2] = {0, };

	index = elm_index_add(layout);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_index_omit_enabled_set(index, EINA_TRUE);
       	elm_index_item_append(index, "#", NULL, NULL);

	int i;
	for (i = 0; i < strlen(idx_str); ++i)
	{
		ch[0] = idx_str[i];
		ch[1] = '\0';
		elm_index_item_append(index, ch, __view_index_cb, &ch[0]);
	}

	elm_index_level_go(index, 0);

	return index;
}

static void __view_index_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *idx_str = (char *)data;
	Eina_List *widget_list = NULL;
	widget_t *widget = NULL;

	widget_list = widget_get_widget_list();
	if (!widget_list) {
		LOGE("Can not get widget list");
		return;
	}

	widget = eina_list_search_unsorted(widget_list, __view_compare_widget_idx, idx_str);
	if (!widget) {
		LOGE("Can not find widget");
		return;
	}

	elm_genlist_item_bring_in(widget->genlist_item, ELM_GENLIST_ITEM_SCROLLTO_TOP);

}

static int __view_compare_widget_idx(const void *data1, const void *data2)
{
	widget_t *w = (widget_t *)data1;
	char *idx_str = (char *)data2;

	char c1 = 0;
       	char c2 = 0;

	c1 = tolower(idx_str[0]);
	c2 = tolower(w->widget_id[0]);

	LOGD("Compare: %c == %c in %s, %s", c1, c2, idx_str, w->widget_id);

	if(c1 < c2) return -1;
	else if(c1 > c2) return 1;
	else return 0;

}
