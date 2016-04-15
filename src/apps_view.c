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

#include "apps_view.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"

static struct {
    Evas_Object *scroller;
    Evas_Object *box;
    int width;
    int height;
} apps_info = {
    .scroller = NULL,
    .box = NULL,
    .width = 0,
    .height = 0
};

Evas_Object *apps_view_create(Evas_Object *parent)
{
    elm_win_screen_size_get(parent, NULL, NULL, &apps_info.width, &apps_info.height);

    apps_info.scroller = elm_scroller_add(parent);
    if (!apps_info.scroller) {
        LOGE("[FAILED][apps_info.scroller==NULL]");
        return NULL;
    }
    elm_scroller_content_min_limit(apps_info.scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_bounce_set(apps_info.scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(apps_info.scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_scroll_limit_set(apps_info.scroller, 1, 1);
    evas_object_size_hint_weight_set(apps_info.scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_scroller_loop_set(apps_info.scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_page_size_set(apps_info.scroller, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_resize(apps_info.scroller, APPS_VIEW_W , APPS_VIEW_H);

    apps_info.box = elm_box_add(apps_info.scroller);
    elm_box_horizontal_set(apps_info.box, EINA_TRUE);
    elm_box_align_set(apps_info.box, 0.5, 0.5);
    evas_object_show(apps_info.box);

    elm_object_content_set(apps_info.scroller, apps_info.box);

    Evas_Object *rect = evas_object_rectangle_add(apps_info.box);
    evas_object_color_set(rect, 255, 0, 255, 100);
    evas_object_size_hint_min_set(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_size_hint_max_set(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_resize(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_show(rect);

    elm_box_pack_end(apps_info.box, rect);
    evas_object_show(apps_info.scroller);

    return apps_info.scroller;
}
