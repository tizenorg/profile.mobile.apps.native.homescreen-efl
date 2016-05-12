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

#include "widget_viewer.h"
#include "util.h"
#include "conf.h"
#include "edc_conf.h"

static void __widget_viewer_delete_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

void widget_viewer_init(Evas_Object *win)
{
    widget_viewer_evas_init(win);
}

void widget_viewer_fini(void)
{
    widget_viewer_evas_fini();
}

Evas_Object *widget_viewer_add_widget(Evas_Object *parent, widget_data_t *item, int *widget_width, int *widget_height)
{
    /* case 1 */
    Evas_Object *widget_layout;

    widget_layout = elm_layout_add(parent);
    elm_layout_file_set(widget_layout, util_get_res_file_path(EDJE_DIR"/widget.edj"), GROUP_WIDGET_LY);
    evas_object_size_hint_weight_set(widget_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

#if 1
    Evas_Object *widget = widget_viewer_evas_add_widget(widget_layout, item->pkg_name, NULL, item->period);
    evas_object_size_hint_min_set(widget, CLUSTER_W, CLUSTER_H);
    evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    //evas_object_resize(widget, CLUSTER_W, CLUSTER_H);
    evas_object_color_set(widget, 255, 255, 255, 255);
#else //for TEST
    Evas_Object *widget = evas_object_rectangle_add(evas_object_evas_get(parent));
    evas_object_size_hint_min_set(widget, CLUSTER_W, CLUSTER_H);
    evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_color_set(widget, 255, 255, 255, 255);
    evas_object_show(widget);
#endif
    elm_object_part_content_set(widget_layout, WIDGET_CONTENT, widget);

    elm_object_signal_callback_add(widget_layout, SIGNAL_DELETE_BUTTON_CLICKED, SIGNAL_SOURCE, __widget_viewer_delete_btn_clicked_cb, (void *)item);

    evas_object_show(widget);
    evas_object_show(widget_layout);
    /*********/

    widget_service_get_size(item->type, widget_width, widget_height);
    return widget_layout;
}

static void __widget_viewer_delete_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    widget_data_t *item  = (widget_data_t *)data;
    cluster_data_delete(item);
}
