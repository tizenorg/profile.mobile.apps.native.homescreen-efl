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

#include "cluster_view.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"
#include "page_indicator.h"
#include "widget_viewer.h"

static struct {
    Evas_Object *win;
    Evas_Object *scroller;
    Evas_Object *box;
    page_indicator_t *indicator;
    int page_count;
    int current_page;
} cluster_info = {
    .scroller = NULL,
    .box = NULL,
    .indicator = NULL,
    .page_count = 0,
    .current_page = -1
};

Evas_Object *__cluster_view_create_base_gui(Evas_Object *win);
void __cluster_view_load_cluster_data(void);
void __cluster_view_create_cluster(void);

Evas_Object *cluster_view_create(Evas_Object *win)
{
    cluster_info.win = win;

    widget_viewer_init(win);

    Evas_Object *base_layout = __cluster_view_create_base_gui(win);

    if (base_layout == NULL) {
        LOGE("[FAILED][base_layout == NULL]");
        return NULL;
    }

    __cluster_view_load_cluster_data();
    __cluster_view_create_cluster();

    cluster_info.indicator = page_indictor_create(cluster_info.scroller);
    page_indicator_set_page_count(cluster_info.indicator, cluster_info.page_count);
    page_indicator_set_current_page(cluster_info.indicator, cluster_info.current_page);
    page_indicator_scroller_resize(cluster_info.indicator, CLUSTER_VIEW_W , CLUSTER_VIEW_H);

    return base_layout;
}

void cluster_view_app_terminate(void)
{
    widget_viewer_fini();
}

void cluster_view_show(void)
{
    page_indicator_show(cluster_info.indicator);
}

void cluster_view_hide(void)
{
    page_indicator_hide(cluster_info.indicator);
}

void cluster_view_show_anim(double pos)
{
    evas_object_color_set(cluster_info.box, 255, 255, 255, pos*255);
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(cluster_info.box, 255, 255, 255, 255);
    }
}

void cluster_view_hide_anim(double pos)
{
    evas_object_color_set(cluster_info.box, 255, 255, 255, (1-pos)*255);
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(cluster_info.box, 255, 255, 255, 0);
    }

}

Evas_Object *__cluster_view_create_base_gui(Evas_Object *win)
{
    cluster_info.scroller = elm_scroller_add(win);
    if (cluster_info.scroller == NULL) {
        return NULL;
    }
    elm_scroller_content_min_limit(cluster_info.scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_policy_set(cluster_info.scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_scroll_limit_set(cluster_info.scroller, 1, 0);
    evas_object_size_hint_weight_set(cluster_info.scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_scroller_loop_set(cluster_info.scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_page_size_set(cluster_info.scroller, CLUSTER_W , CLUSTER_H);
    evas_object_resize(cluster_info.scroller, CLUSTER_W , CLUSTER_H);

    evas_object_show(cluster_info.scroller);

    cluster_info.box = elm_box_add(cluster_info.scroller);
    elm_box_horizontal_set(cluster_info.box, EINA_TRUE);
    elm_box_align_set(cluster_info.box, 0.5, 0.5);
    evas_object_size_hint_weight_set(cluster_info.box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(cluster_info.box);

    elm_object_content_set(cluster_info.scroller, cluster_info.box);
    elm_scroller_bounce_set(cluster_info.scroller, EINA_FALSE, EINA_TRUE);

    return cluster_info.scroller;
}

void __cluster_view_load_cluster_data(void)
{

}

void __cluster_view_create_cluster(void)
{
    int w, h;
    Evas_Object *widget = widget_viewer_add_widget(cluster_info.win, NULL, NULL, 0, &w, &h);
    evas_object_size_hint_min_set(widget, w, h);
    evas_object_resize(widget, w, h);
    evas_object_color_set(widget, 100, 100, 100, 50);
    evas_object_show(widget);

    elm_box_pack_end(cluster_info.box, widget);

    cluster_info.page_count = 1;
    cluster_info.current_page = 0;
}
