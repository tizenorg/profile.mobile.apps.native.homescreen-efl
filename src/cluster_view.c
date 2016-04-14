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

static int scroller_width = 720;
static int scroller_height = 1280;

static struct {
    Evas_Object *scroller;
    Evas_Object *box;
    int width;
    int height;
} cluster_info = {
    .scroller = NULL,
    .box = NULL,
    .width = 0,
    .height = 0
};

Evas_Object *cluster_view_create(Evas_Object *parent)
{
    elm_win_screen_size_get(parent, NULL, NULL, &cluster_info.width, &cluster_info.height);
    scroller_width = cluster_info.width;
    scroller_height = cluster_info.height;

    LOGD("%d %d", cluster_info.width, cluster_info.height);

    cluster_info.scroller = elm_scroller_add(parent);
    if (!cluster_info.scroller) {
        LOGE("[FAILED][cluster_info.scroller==NULL]");
        return NULL;
    }
    elm_scroller_content_min_limit(cluster_info.scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_bounce_set(cluster_info.scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(cluster_info.scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_scroll_limit_set(cluster_info.scroller, 1, 1);
    evas_object_size_hint_weight_set(cluster_info.scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_scroller_loop_set(cluster_info.scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_page_size_set(cluster_info.scroller, scroller_width, scroller_width);
    evas_object_resize(cluster_info.scroller, scroller_width , scroller_width);

    cluster_info.box = elm_box_add(cluster_info.scroller);
    elm_box_horizontal_set(cluster_info.box, EINA_TRUE);
    elm_box_align_set(cluster_info.box, 0.5, 0.5);
    evas_object_show(cluster_info.box);

    elm_object_content_set(cluster_info.scroller, cluster_info.box);

    Evas_Object *rect = evas_object_rectangle_add(cluster_info.box);
    evas_object_color_set(rect, 0, 255, 255, 100);
    evas_object_size_hint_min_set(rect, scroller_width, scroller_width);
    evas_object_size_hint_max_set(rect, scroller_width, scroller_width);
    evas_object_resize(rect, scroller_width, scroller_width);
    evas_object_show(rect);

    elm_box_pack_end(cluster_info.box, rect);
    evas_object_show(cluster_info.scroller);

    return cluster_info.scroller;
}
