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

#include <feedback.h>

#include "cluster_view.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"
#include "page_indicator.h"
#include "widget_viewer.h"
#include "menu.h"
#include "cluster_data.h"

static struct {
    Evas_Object *win;
    Evas_Object *scroller;
    Evas_Object *box;
    page_indicator_t *indicator;
    int page_count;
    int current_page;
    view_state_t view_state;
} cluster_view_s = {
    .scroller = NULL,
    .box = NULL,
    .indicator = NULL,
    .page_count = 0,
    .current_page = -1,
    .view_state = VIEW_STATE_NORMAL
};

static int cluster_menu_list[4] = {
        MENU_CLUSTER_EDIT,
        MENU_CLUSTER_ADD_WIDGET,
        MENU_CLUSTER_CHANGE_WALLPAPER,
        MENU_CLUSTER_ALL_PAGES
};

static Eina_Hash *cluster_menu_table = NULL;

Evas_Object *__cluster_view_create_base_gui(Evas_Object *win);
static void __cluster_view_create_cluster(void);
static void __cluster_view_create_menu(void);
static void __cluster_view_menu_edit_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_menu_add_widget_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_menu_change_wallpaper_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_menu_all_pages_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_app_launch_request_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data);
static void __cluster_view_scroll_to_home(void);

Evas_Object *cluster_view_create(Evas_Object *win)
{
    cluster_view_s.win = win;

    widget_viewer_init(win);
    cluster_data_init();

    Evas_Object *base_layout = __cluster_view_create_base_gui(win);

    if (base_layout == NULL) {
        LOGE("[FAILED][base_layout == NULL]");
        return NULL;
    }

    __cluster_view_create_cluster();
    __cluster_view_create_menu();

    cluster_view_s.indicator = page_indictor_create(cluster_view_s.scroller);
    page_indicator_set_page_count(cluster_view_s.indicator, cluster_view_s.page_count);
    page_indicator_set_current_page(cluster_view_s.indicator, cluster_view_s.current_page);
    page_indicator_scroller_resize(cluster_view_s.indicator, CLUSTER_VIEW_W , CLUSTER_VIEW_H);

    return base_layout;
}

void cluster_view_app_terminate(void)
{
    eina_hash_free(cluster_menu_table);
    widget_viewer_fini();
}

void cluster_view_show(void)
{
    page_indicator_show(cluster_view_s.indicator);
}

void cluster_view_hide(void)
{
    page_indicator_hide(cluster_view_s.indicator);
}

void cluster_view_show_anim(double pos)
{
    evas_object_color_set(cluster_view_s.box, 255, 255, 255, pos*255);
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(cluster_view_s.box, 255, 255, 255, 255);
    }
}

void cluster_view_hide_anim(double pos)
{
    evas_object_color_set(cluster_view_s.box, 255, 255, 255, (1-pos)*255);
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(cluster_view_s.box, 255, 255, 255, 0);
    }
}

Evas_Object *__cluster_view_create_base_gui(Evas_Object *win)
{
    cluster_view_s.scroller = elm_scroller_add(win);
    if (cluster_view_s.scroller == NULL) {
        return NULL;
    }
    elm_scroller_content_min_limit(cluster_view_s.scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_bounce_set(cluster_view_s.scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(cluster_view_s.scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_scroll_limit_set(cluster_view_s.scroller, 1, 0);
    evas_object_size_hint_weight_set(cluster_view_s.scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_scroller_loop_set(cluster_view_s.scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_page_size_set(cluster_view_s.scroller, CLUSTER_W , CLUSTER_H);
    evas_object_resize(cluster_view_s.scroller, CLUSTER_W , CLUSTER_H);

    cluster_view_s.box = elm_box_add(cluster_view_s.scroller);
    elm_box_horizontal_set(cluster_view_s.box, EINA_TRUE);
    elm_box_align_set(cluster_view_s.box, 0.5, 0.5);
    evas_object_size_hint_weight_set(cluster_view_s.box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(cluster_view_s.box);

    elm_object_content_set(cluster_view_s.scroller, cluster_view_s.box);

    return cluster_view_s.scroller;
}

void __cluster_view_create_cluster(void)
{
    int w, h;
    Eina_List *list = cluster_data_get_list();
    cluster_data_t *item = eina_list_data_get(list);

    item->widget_layout = widget_viewer_add_widget(cluster_view_s.win, item->pkg_name, NULL, item->period, &w, &h);
    evas_object_size_hint_min_set(item->widget_layout, w, h);
    evas_object_size_hint_align_set(item->widget_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(item->widget_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_resize(item->widget_layout, w, h);
    evas_object_color_set(item->widget_layout, 255, 255, 255, 255);
    evas_object_show(item->widget_layout);

    elm_box_pack_end(cluster_view_s.box, item->widget_layout);

    cluster_view_s.page_count = 1;
    cluster_view_s.current_page = 0;
}

static void __cluster_view_create_menu(void)
{
    cluster_menu_table = eina_hash_int32_new(NULL);
    eina_hash_add(cluster_menu_table, &cluster_menu_list[0], __cluster_view_menu_edit_cb);
    eina_hash_add(cluster_menu_table, &cluster_menu_list[1], __cluster_view_menu_add_widget_cb);
    eina_hash_add(cluster_menu_table, &cluster_menu_list[2], __cluster_view_menu_change_wallpaper_cb);
    eina_hash_add(cluster_menu_table, &cluster_menu_list[3], __cluster_view_menu_all_pages_cb);
}

static void __cluster_view_menu_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
    menu_hide();
}

static void __cluster_view_menu_add_widget_cb(void *data, Evas_Object *obj, void *event_info)
{
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
    menu_hide();
}

static void __cluster_view_menu_change_wallpaper_cb(void *data, Evas_Object *obj, void *event_info)
{
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);

    const char *appid = "org.tizen.wallpaper-ui-service";
    app_control_h app_control_handle = NULL;

    if (app_control_create(&app_control_handle) != APP_CONTROL_ERROR_NONE) {
        LOGE("[FAILED][app_control_create]");
        return;
    }

    app_control_add_extra_data(app_control_handle, "from", "Homescreen-efl");
    app_control_add_extra_data(app_control_handle, "popup_type", "selection_popup");
    app_control_add_extra_data(app_control_handle, "setas-type", "Homescreen");

    if (app_control_set_app_id(app_control_handle, appid) != APP_CONTROL_ERROR_NONE) {
        LOGE("[FAILED][app_control_set_app_id]");
        app_control_destroy(app_control_handle);
        return;
    }

    if (app_control_send_launch_request(app_control_handle, __cluster_view_app_launch_request_cb, NULL) != APP_CONTROL_ERROR_NONE) {
        LOGE("[FAILED][app_control_send_launch_request]");
        app_control_destroy(app_control_handle);
        return;
    }

    app_control_destroy(app_control_handle);

    menu_hide();
}

static void __cluster_view_menu_all_pages_cb(void *data, Evas_Object *obj, void *event_info)
{
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
    menu_hide();
}

void cluster_view_hw_menu_key(void)
{
    if (cluster_view_s.view_state == VIEW_STATE_NORMAL)
        menu_change_state_on_hw_menu_key(cluster_menu_table);
}

bool cluster_view_hw_home_key(void)
{
    if (cluster_view_s.view_state == VIEW_STATE_NORMAL) {
        __cluster_view_scroll_to_home();
    } else {
        // go normal
    }

    return true;
}

bool cluster_view_hw_back_key(void)
{
    if (cluster_view_s.view_state == VIEW_STATE_NORMAL) {
        return true;
    }

    // go normal

    return true;
}

view_state_t cluster_view_get_state(void)
{
    return cluster_view_s.view_state;
}

void cluster_view_set_state(view_state_t state)
{

}

static void __cluster_view_scroll_to_home(void)
{

}


static void __cluster_view_app_launch_request_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data)
{
    switch (result) {
    case APP_CONTROL_RESULT_CANCELED:
        LOGE("[APP_CONTROL_RESULT_CANCELED]");
        break;
    case APP_CONTROL_RESULT_FAILED:
        LOGE("[APP_CONTROL_RESULT_FAILED]");
        break;
    case APP_CONTROL_RESULT_SUCCEEDED:
        LOGD("[APP_CONTROL_RESULT_SUCCEEDED]");
        break;
    default:
        LOGE("UNKNOWN ERROR");
        break;
    }
}


void cluster_view_hw_key_menu(void)
{
    menu_change_state_on_hw_menu_key(cluster_menu_table);
}

void cluster_view_scroll_to_home(void)
{

}
