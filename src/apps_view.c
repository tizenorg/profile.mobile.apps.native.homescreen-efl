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
#include "page_indicator.h"
#include "homescreen-efl.h"

static struct {
    Evas_Object *scroller;
    Evas_Object *bg;
    Evas_Object *box;
    int width;
    int height;
    page_indicator_t *indicator;
    Eina_List *page_list;
    int page_count;
    int current_page;
} apps_info = {
    .scroller = NULL,
    .bg = NULL,
    .box = NULL,
    .width = 0,
    .height = 0,
    .indicator = NULL,
    .page_list = NULL,
    .page_count = 0,
    .current_page = 0
};

void __apps_view_create_base_gui(Evas_Object *win);
void __apps_view_add_page(void);
void __apps_view_remove_page(void);

Evas_Object *apps_view_create(Evas_Object *win)
{
    elm_win_screen_size_get(win, NULL, NULL, &apps_info.width, &apps_info.height);

    __apps_view_create_base_gui(win);

    if (!apps_info.scroller) {
        LOGE("[FAILED][apps_info.scroller==NULL]");
        return NULL;
    }

    return apps_info.scroller;
}

void apps_view_show(void)
{
    page_indicator_show(apps_info.indicator);
}

void apps_view_hide(void)
{
    page_indicator_hide(apps_info.indicator);
}

void apps_view_show_anim(double pos)
{
    Evas_Object *edje = NULL;
    edje = elm_layout_edje_get(apps_info.bg);
    if (!edje) {
        LOGE("Failed to get edje from layout");
        return;
    }
    Edje_Message_Float_Set *msg = malloc(sizeof(*msg) + 2 * sizeof(double));
    msg->count = 3;
    msg->val[0] = ((APPS_VIEW_ANIMATION_DELTA * (1-pos)) / apps_info.height);
    msg->val[1] = 1+((APPS_VIEW_ANIMATION_DELTA * (1-pos)) / apps_info.height);
    msg->val[2] = pos*255;

    edje_object_message_send(edje, EDJE_MESSAGE_FLOAT_SET, 1, msg);
    edje_object_signal_emit(edje, SIGNAL_APPS_VIEW_ANIM, SIGNAL_SOURCE);

    evas_object_color_set(apps_info.box, 255, 255, 255, pos*255);
    evas_object_move(apps_info.scroller, 0, APPS_VIEW_PADDING_TOP + (APPS_VIEW_ANIMATION_DELTA * (1-pos)));
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(apps_info.box, 255, 255, 255, 255);
        evas_object_move(apps_info.scroller, 0, APPS_VIEW_PADDING_TOP);
        edje_object_signal_emit(edje, SIGNAL_APPS_VIEW_SHOW, SIGNAL_SOURCE);
    }
}

void apps_view_hide_anim(double pos)
{
    Evas_Object *edje = NULL;
    edje = elm_layout_edje_get(apps_info.bg);
    if (!edje) {
        LOGE("Failed to get edje from layout");
        return;
    }
    Edje_Message_Float_Set *msg = malloc(sizeof(*msg) + 2 * sizeof(double));
    msg->count = 3;
    msg->val[0] = ((APPS_VIEW_ANIMATION_DELTA * pos) / apps_info.height);
    msg->val[1] = 1+((APPS_VIEW_ANIMATION_DELTA * pos) / apps_info.height);
    msg->val[2] = (1-pos)*255;

    edje_object_message_send(edje, EDJE_MESSAGE_FLOAT_SET, 1, msg);
    edje_object_signal_emit(edje, SIGNAL_APPS_VIEW_ANIM, SIGNAL_SOURCE);

    evas_object_color_set(apps_info.box, 255, 255, 255, (1-pos)*255);
    evas_object_move(apps_info.scroller, 0, APPS_VIEW_PADDING_TOP + (APPS_VIEW_ANIMATION_DELTA * pos));
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(apps_info.box, 255, 255, 255, 0);
        evas_object_move(apps_info.scroller, 0, apps_info.height);
        edje_object_signal_emit(edje, SIGNAL_APPS_VIEW_HIDE, SIGNAL_SOURCE);
    }
}

void __apps_view_create_base_gui(Evas_Object *win)
{
    char edj_path[PATH_MAX] = {0, };
    apps_info.bg = elm_layout_add(win);
    if (!apps_info.bg) {
        LOGE("[FAILED][apps_info.bg==NULL]");
        return;
    }
    snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/apps_view_bg.edj"));
    elm_layout_file_set(apps_info.bg, edj_path, GROUP_APPS_BG_LY);
    evas_object_size_hint_weight_set(apps_info.bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, apps_info.bg);
    evas_object_show(apps_info.bg);

    Evas_Object *rect = evas_object_rectangle_add(homescreen_efl_get_win());
    evas_object_color_set(rect, 0, 0, 0, 70);
    evas_object_size_hint_min_set(rect, apps_info.width, apps_info.height);
    evas_object_size_hint_max_set(rect, apps_info.width, apps_info.height);
    evas_object_resize(rect, apps_info.width, apps_info.height);
    elm_object_part_content_set(apps_info.bg, PART_APPS_VIEW_BG, rect);
    evas_object_show(rect);

    apps_info.scroller = elm_scroller_add(win);
    if (!apps_info.scroller) {
        LOGE("[FAILED][apps_info.scroller==NULL]");
        return;
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

    apps_info.indicator = page_indictor_create(apps_info.scroller);
    page_indicator_scroller_resize(apps_info.indicator, apps_info.width, apps_info.height);
    page_indicator_hide(apps_info.indicator);

    __apps_view_add_page();
    page_indicator_set_current_page(apps_info.indicator, 0);
}

void __apps_view_add_page(void)
{
    char edj_path[PATH_MAX] = {0, };
    Evas_Object *page_ly = elm_layout_add(apps_info.box);

    snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/apps_view.edj"));
    elm_layout_file_set(page_ly, edj_path, GROUP_APPS_LY);
    evas_object_size_hint_weight_set(page_ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_resize(page_ly, APPS_VIEW_W, APPS_VIEW_H);

    Evas_Object *rect = evas_object_rectangle_add(homescreen_efl_get_win());
    evas_object_color_set(rect, 255, 255, 100, 100);
    evas_object_size_hint_min_set(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_size_hint_max_set(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_resize(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_show(rect);
    elm_object_part_content_set(page_ly, SIZE_SETTER, rect);

    elm_box_pack_end(apps_info.box, page_ly);
    evas_object_show(page_ly);

    apps_info.page_list = eina_list_append(apps_info.page_list, page_ly);
    apps_info.page_count += 1;
    page_indicator_set_page_count(apps_info.indicator, apps_info.page_count);
}

void __apps_view_remove_page(void)
{
    if (apps_info.page_count == 0) {
        apps_info.page_count = 1;
        return ;
    }

    Evas_Object *item = eina_list_nth(apps_info.page_list, apps_info.page_count-1);
    apps_info.page_list = eina_list_remove(apps_info.page_list, item);
    apps_info.page_count -= 1;
    if (apps_info.current_page >= apps_info.page_count) {
        apps_info.current_page = apps_info.page_count - 1;
    }

    elm_scroller_page_bring_in(apps_info.scroller, apps_info.current_page, 0);
    elm_box_unpack(apps_info.box, item);
    page_indicator_set_page_count(apps_info.indicator, apps_info.page_count);
    evas_object_del(item);
}
