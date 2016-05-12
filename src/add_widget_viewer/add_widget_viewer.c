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

#include "add_widget_viewer/add_widget_viewer.h"
#include "add_widget_viewer/add_widget_data.h"
#include "util.h"

static struct {
    Evas_Object *win;
    Evas_Object *conformant;
    Evas_Object *layout;
    Evas_Object *naviframe;
    Evas_Object *genlist;
    Elm_Theme *theme;
    Elm_Object_Item *navi_item;
} add_widget_viewer_s = {
    .win = NULL,
    .conformant = NULL,
    .layout = NULL,
    .naviframe = NULL,
    .genlist = NULL,
    .theme = NULL,
    .navi_item = NULL,
};

static void __add_widget_viewer_win_del_cb(void *data, Evas_Object* obj, void* event_info);

void add_widget_viewer_win_create(void)
{
    if (!add_widget_data_init()) {
        LOGE("Can not create widget list");
        return ;
    }

    add_widget_viewer_s.win = elm_win_add(NULL, "add_viewer", ELM_WIN_BASIC);
    if (!add_widget_viewer_s.win) {
        LOGE("Failed to create a new window");
        return;
    }

    elm_win_alpha_set(add_widget_viewer_s.win, EINA_TRUE);
    elm_win_autodel_set(add_widget_viewer_s.win, EINA_TRUE);
    evas_object_smart_callback_add(add_widget_viewer_s.win, "delete,request", __add_widget_viewer_win_del_cb, NULL);

    /*add_widget_viewer_s.theme = __add_viewer_create_theme();
    add_widget_viewer_s.conformant = __add_viewer_create_conformant();
    add_widget_viewer_s.layout = __add_viewer_create_layout();
    add_widget_viewer_s.naviframe = __add_viewer_create_naviframe();

    add_widget_viewer_s.navi_item = list_view_create_content(add_widget_viewer_s.naviframe, add_widget_viewer_s.genlist);

    elm_object_signal_emit(add_widget_viewer_s.layout, "display,normal", "container");

    elm_win_indicator_mode_set(add_widget_viewer_s.win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(add_widget_viewer_s.win, ELM_WIN_INDICATOR_OPAQUE);
    elm_win_resize_object_add(add_widget_viewer_s.win, add_widget_viewer_s.conformant);
    elm_win_conformant_set(add_widget_viewer_s.win, EINA_TRUE);

    evas_object_show(add_widget_viewer_s.naviframe);
    evas_object_show(add_widget_viewer_s.layout);
    evas_object_show(add_widget_viewer_s.conformant);*/


    evas_object_show(add_widget_viewer_s.win);
}

void add_widget_viewer_win_destroy(void)
{
    evas_object_del(add_widget_viewer_s.win);
    add_widget_data_fini();
}

static void __add_widget_viewer_win_del_cb(void *data, Evas_Object* obj, void* event_info)
{
    evas_object_del(add_widget_viewer_s.win);
}
