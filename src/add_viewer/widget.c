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

#include <string.h>

#include <widget_service.h>
#include <widget_errno.h>
#include <app_manager.h>

#include "util.h"

#include "add_viewer/widget.h"

static Eina_List *widget_list;

static int __widget_list_cb(const char *appid, const char *lbid, int is_prime, void *data);
static int __widget_list_widget_append(const char *appid, const char *widget_id, void *data);
static void __widget_list_widget_remove(widget_t *widget);
static void __widget_list_delete(void);
static int __widget_compare_widget_id_cb(const void *data1, const void *data2);

int widget_init(void)
{

	int ret = 0;

	ret = widget_service_get_widget_list(__widget_list_cb, NULL);
	LOGD("ERROR: %s", get_error_message(ret));
	if (ret < 0 || widget_list == NULL) {
		LOGE("Can not create widget list, ret: %d", ret);
		return 1;
	}

	return 0;
}

void widget_fini(void)
{
	__widget_list_delete();
}

Eina_List *widget_get_widget_list(void)
{
	return widget_list;
}

Eina_List *widget_get_widget_preview_list(widget_t *widget)
{
	return widget->preview_list;
}

static int __widget_list_cb(const char *app_id, const char *widget_id, int is_prime, void *data) {
	LOGD("Widget list cb, app id = %s, widget id = %s", app_id, widget_id);
	return __widget_list_widget_append(app_id, widget_id, data);
}

static int __widget_compare_widget_id_cb(const void *data1, const void *data2)
{
	int res = 0;
	widget_t *w1 = (widget_t *)data1;
	widget_t *w2 = (widget_t *)data2;

	res = strcmp(w1->label, w2->label);

	if (res < 0) return -1;
	else if (res > 0) return 1;
	else return 0;
}

static void __widget_list_delete(void)
{
	Eina_List *l;
	widget_t *widget;

	EINA_LIST_FOREACH(widget_list, l, widget)
		__widget_list_widget_remove(widget);

	eina_list_free(widget_list);
	widget_list = NULL;
}

static int __widget_list_widget_append(const char *appid, const char *widget_id, void *data)
{
	LOGD("App id: %s", appid);
	LOGD("Widget id: %s", widget_id);

	widget_t *widget = NULL;
	preview_t *widget_preview_type = NULL;

	int *type = NULL;
	int types_count = 0;
	int ret = WIDGET_ERROR_NONE;

/*
	if (widget_service_get_nodisplay(widget_id)) {
		LOGE("Widget should not be displayed");
		return WIDGET_ERROR_NONE;
	}
*/
	ret = widget_service_get_supported_size_types(widget_id, &types_count, &type);
	if (ret != WIDGET_ERROR_NONE || types_count <= 0) {
		LOGE("Can not get widgets supported size types");
		return WIDGET_ERROR_FAULT; }

	widget = calloc(1, sizeof(widget_t));
	if (!widget) {
		LOGE("Can not allocate memory for list item");
		return WIDGET_ERROR_FAULT;
	}

	int i;
	for (i = 0; i < types_count; ++i) {

		widget_preview_type = calloc(1, sizeof(preview_t));
		if (!widget_preview_type) {
			LOGE("Can not allocate memory for list item preview type");
			__widget_list_delete();
			return WIDGET_ERROR_FAULT;
		}

		widget_preview_type->path = widget_service_get_preview_image_path(widget_id, type[i]);
		if(!widget_preview_type->path) {
			free(widget_preview_type);
			continue;
		}

		widget_preview_type->type = type[i];

		widget->size_types |= type[i];
		widget->preview_list = eina_list_append(widget->preview_list, widget_preview_type);
	}

	widget->size_types_count = types_count;
	widget->app_id = strdup(appid);
	widget->widget_id = strdup(widget_id);
	widget->label = widget_service_get_name(widget->widget_id, NULL);
	if (!widget->label)
		widget->label = strdup("");

	widget_list = eina_list_sorted_insert(widget_list, __widget_compare_widget_id_cb, widget);
	//widget_list = eina_list_append(widget_list, widget);

	free(type);

	return WIDGET_ERROR_NONE;
}

static void __widget_list_widget_remove(widget_t *widget)
{
	Eina_List *l;
	preview_t *preview;

	EINA_LIST_FOREACH(widget->preview_list, l, preview) {
		free(preview->path);
		free(preview);
	}

	eina_list_free(widget->preview_list);
	free(widget->app_id);
	free(widget->widget_id);
	free(widget->label);
	free(widget);
}


