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
#include <widget_errno.h>

#include "add_widget_viewer/add_widget_data.h"
#include "util.h"

static struct {
    Eina_List *data_list;
} add_widget_data_s = {
    .data_list = NULL
};

static void __add_widget_data_list_delete(void);
static void __add_widget_data_free(add_widget_data_t *item);
static int __add_widget_data_widget_list_cb(const char *app_id, const char *widget_id, int is_prime, void *data);
static int __add_widget_data_widget_compare_cb(const void *data1, const void *data2);

bool add_widget_data_init(void)
{
    int ret = 0;
    ret = widget_service_get_widget_list(__add_widget_data_widget_list_cb, NULL);
    if (ret < 0 || add_widget_data_s.data_list == NULL) {
        LOGE("Can not create widget list, ret: %d", ret);
        return false;
    }

    return true;
}

void add_widget_data_fini(void)
{
    __add_widget_data_list_delete();
}

Eina_List *add_widget_data_get_widget_list(void)
{
    return add_widget_data_s.data_list;
}

Eina_List *add_widget_data_get_widget_preview_list(add_widget_data_t *widget)
{
    return widget->preview_list;
}

static int __add_widget_data_widget_list_cb(const char *app_id, const char *widget_id, int is_prime, void *data)
{
    LOGD("App id: %s", app_id);
    LOGD("Widget id: %s", widget_id);

    add_widget_data_t *widget = NULL;
    add_widget_data_preview_t *widget_preview_type = NULL;

    int *type = NULL;
    int types_count = 0;
    int ret = WIDGET_ERROR_NONE;

    ret = widget_service_get_supported_size_types(widget_id, &types_count, &type);
    if (ret != WIDGET_ERROR_NONE || types_count <= 0) {
        LOGE("Can not get widgets supported size types");
        return WIDGET_ERROR_FAULT;
    }

    widget = calloc(1, sizeof(add_widget_data_t));
    if (!widget) {
        LOGE("Can not allocate memory for list item");
        return WIDGET_ERROR_FAULT;
    }

    int i;
    for (i = 0; i < types_count; ++i) {
        widget_preview_type = calloc(1, sizeof(add_widget_data_preview_t));
        if (!widget_preview_type) {
            LOGE("Can not allocate memory for list item preview type");
            __add_widget_data_list_delete();
            return WIDGET_ERROR_FAULT;
        }

        widget_preview_type->path = widget_service_get_preview_image_path(widget_id, type[i]);
        if (!widget_preview_type->path) {
            free(widget_preview_type);
            continue;
        }

        widget_preview_type->type = type[i];

        widget->size_types |= type[i];
        widget->preview_list = eina_list_append(widget->preview_list, widget_preview_type);
    }

    widget->size_types_count = types_count;
    widget->app_id = strdup(app_id);
    widget->widget_id = strdup(widget_id);
    widget->label = widget_service_get_name(widget->widget_id, NULL);
    if (!widget->label || strlen(widget->label) == 0)
        widget->label = strdup(widget_id);

    add_widget_data_s.data_list = eina_list_sorted_insert(add_widget_data_s.data_list,
            __add_widget_data_widget_compare_cb, widget);

    free(type);

    return WIDGET_ERROR_NONE;
}

static int __add_widget_data_widget_compare_cb(const void *data1, const void *data2)
{
    int res = 0;
    add_widget_data_t *w1 = (add_widget_data_t *)data1;
    add_widget_data_t *w2 = (add_widget_data_t *)data2;

    res = strcmp(w1->label, w2->label);

    if (res < 0) return -1;
    if (res > 0) return 1;
    return 0;
}

static void __add_widget_data_list_delete(void)
{
    Eina_List *list;
    add_widget_data_t *item;

    EINA_LIST_FOREACH(add_widget_data_s.data_list, list, item)
        __add_widget_data_free(item);

    eina_list_free(add_widget_data_s.data_list);
    add_widget_data_s.data_list = NULL;
}

static void __add_widget_data_free(add_widget_data_t *item)
{
    Eina_List *list;
    add_widget_data_preview_t *preview;

    EINA_LIST_FOREACH(item->preview_list, list, preview) {
        free(preview->path);
        free(preview);
    }

    eina_list_free(item->preview_list);
    free(item->app_id);
    free(item->widget_id);
    free(item->label);
    free(item);
}
