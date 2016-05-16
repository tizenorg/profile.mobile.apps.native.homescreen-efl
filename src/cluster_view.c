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
#include <efl_extension.h>

#include "homescreen-efl.h"
#include "cluster_view.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"
#include "page_indicator.h"
#include "widget_viewer.h"
#include "menu.h"
#include "popup.h"
#include "cluster_page.h"
#include "cluster_data.h"
#include "add_widget_viewer/add_widget_viewer.h"
#include "mouse.h"

static struct {
    Evas_Object *win;
    Evas_Object *scroller;
    Evas_Object *box;
    Evas_Object *allpage;
    Evas_Object *allpage_add;
    page_indicator_t *indicator;
    int page_count;
    int current_page;
    view_state_t view_state;
    Eina_List *page_list;
    cluster_page_t *picked_page;
    widget_data_t *picked_widget;
    Ecore_Timer *edit_mode_scroll_timer;
    bool is_srolling;
} cluster_view_s = {
    .scroller = NULL,
    .box = NULL,
    .allpage = NULL,
    .allpage_add = NULL,
    .indicator = NULL,
    .page_count = 0,
    .current_page = INIT_VALUE,
    .view_state = VIEW_STATE_NORMAL,
    .page_list = NULL,
    .picked_page = NULL,
    .picked_widget = NULL,
    .edit_mode_scroll_timer = NULL,
    .is_srolling = false,
};

static int cluster_menu_list[4] = {
        MENU_CLUSTER_EDIT,
        MENU_CLUSTER_ADD_WIDGET,
        MENU_CLUSTER_CHANGE_WALLPAPER,
        MENU_CLUSTER_ALL_PAGES
};

static mouse_info_t cluster_mouse_info = {
        .pressed = false,
        .long_pressed = false,
        .down_x = 0,
        .down_y = 0,
        .move_x = 0,
        .move_y = 0,
        .up_x = 0,
        .up_y = 0,
        .long_press_timer = NULL,
        .offset_x = 0,
        .offset_y = 0,
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
static void __cluster_view_scroll_to_page(int page_idx);
static void __cluster_view_add_widget_content(widget_data_t *item);
static void __cluster_view_create_all_page(void);
static void __cluster_view_destroy_all_page(void);
static cluster_page_t * __cluster_view_page_new(void);
static void __cluster_view_page_delete(cluster_page_t *page);
static void __cluster_view_allpage_delete_clicked (void *data, Evas_Object *obj, const char *emission, const char *source);
static void __cluster_view_allpage_add_clicked (void *data, Evas_Object *obj, const char *emission, const char *source);
static void __cluster_view_allpage_delete_page_cb(void *data, Evas_Object *obj, void *event_info);

static void __cluster_view_reorder_pages(void);

static void __clsuter_view_thumbnail_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_thumbnail_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_thumbnail_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool __cluster_view_thumbnail_time_cb(void *data);

static void __clsuter_view_edit_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_edit_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_edit_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool __cluster_view_edit_long_press_time_cb(void *data);

static void __cluster_view_edit_pick_up_widget(void *data);
static void __cluster_view_edit_drag_widget(void *data);
static void __cluster_view_edit_drop_widget(void *data);


static Eina_Bool __cluster_view_scroll_timer_cb(void *data);

static void __cluster_view_scroll_anim_stop_cb(void *data, Evas_Object *obj, void *event_info);

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

    cluster_view_s.indicator = page_indictor_create(cluster_view_s.scroller);
    page_indicator_scroller_resize(cluster_view_s.indicator, CLUSTER_VIEW_W , CLUSTER_VIEW_H);

    __cluster_view_create_cluster();
    __cluster_view_create_menu();

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
    evas_object_resize(cluster_view_s.scroller, CLUSTER_VIEW_W, CLUSTER_VIEW_H);

    elm_scroller_loop_set(cluster_view_s.scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_page_size_set(cluster_view_s.scroller, CLUSTER_W , CLUSTER_H);

    evas_object_smart_callback_add(cluster_view_s.scroller, "scroll,anim,stop", __cluster_view_scroll_anim_stop_cb, NULL);

    cluster_view_s.box = elm_box_add(cluster_view_s.scroller);
    elm_box_horizontal_set(cluster_view_s.box, EINA_TRUE);
    evas_object_size_hint_weight_set(cluster_view_s.box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(cluster_view_s.box, 0, 0);
    evas_object_show(cluster_view_s.box);

    elm_object_content_set(cluster_view_s.scroller, cluster_view_s.box);

    return cluster_view_s.scroller;
}

static void __cluster_view_create_cluster(void)
{
    Eina_List *widget_list = cluster_data_get_widget_list();
    int page_count = cluster_data_get_page_count();
    cluster_view_s.current_page = CLUSTER_HOME_PAGE;

    while (page_count--){
        __cluster_view_page_new();
    }

    Eina_List *find_list;
    widget_data_t *item = NULL;
    EINA_LIST_FOREACH(widget_list, find_list, item) {
        __cluster_view_add_widget_content(item);
        cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, item->page_idx);
        if (page == NULL) {
            LOGE("page is NULL");
        } else {
            if (!cluster_page_set_widget(page, item)) {
                LOGE("failed cluster_page_set_widget page %d", item->page_idx);
            }
        }
    }

    page_indicator_set_current_page(cluster_view_s.indicator, cluster_view_s.current_page);
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

    cluster_view_set_state(VIEW_STATE_EDIT);
}

static void __cluster_view_menu_add_widget_cb(void *data, Evas_Object *obj, void *event_info)
{
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
    menu_hide();

    cluster_view_set_state(VIEW_STATE_ADD_VIEWER);
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

    cluster_view_set_state(VIEW_STATE_ALL_PAGE);
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
    } else if(cluster_view_s.view_state == VIEW_STATE_EDIT) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    } else if(cluster_view_s.view_state == VIEW_STATE_ADD_VIEWER) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    } else if(cluster_view_s.view_state == VIEW_STATE_ALL_PAGE) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    }

    return true;
}

bool cluster_view_hw_back_key(void)
{
    if (cluster_view_s.view_state == VIEW_STATE_NORMAL) {
        return true;
    } else if(cluster_view_s.view_state == VIEW_STATE_EDIT) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    } else if(cluster_view_s.view_state == VIEW_STATE_ADD_VIEWER) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    } else if(cluster_view_s.view_state == VIEW_STATE_ALL_PAGE) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    }

    return true;
}

view_state_t cluster_view_get_state(void)
{
    return cluster_view_s.view_state;
}

void cluster_view_set_state(view_state_t state)
{
    if (state == VIEW_STATE_EDIT) {
        homescreen_efl_btn_hide(HOMESCREEN_VIEW_HOME);

        Eina_List *data_list = cluster_data_get_widget_list();
        Eina_List *find_list = NULL;

        cluster_page_t *page_item = NULL;
        EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
            if (page_item->page_layout) {
                elm_object_signal_emit(page_item->page_layout, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);
            }
        }

        widget_data_t *item = NULL;
        EINA_LIST_FOREACH(data_list, find_list, item) {
            if (item->widget_layout) {
                elm_object_signal_emit(item->widget_layout, SIGNAL_DELETE_BUTTON_SHOW_ANI, SIGNAL_SOURCE);
                elm_object_signal_emit(item->widget_layout, SIGNAL_CLUSTER_EDIT_STATE, SIGNAL_SOURCE);

                evas_object_event_callback_add(item->widget_layout, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_edit_down_cb, item);
                evas_object_event_callback_add(item->widget_layout, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_edit_move_cb, item);
                evas_object_event_callback_add(item->widget_layout, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_edit_up_cb, item);
            }
        }
    } else if (state == VIEW_STATE_NORMAL) {
        if (cluster_view_s.view_state == VIEW_STATE_EDIT) {
            homescreen_efl_btn_show(HOMESCREEN_VIEW_HOME);

            Eina_List *data_list = cluster_data_get_widget_list();
            Eina_List *find_list = NULL;

            cluster_page_t *page_item = NULL;
            EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
                if (page_item->page_layout) {
                    elm_object_signal_emit(page_item->page_layout, SIGNAL_EDIT_MODE_OFF, SIGNAL_SOURCE);
                }
            }

            widget_data_t *item = NULL;
            EINA_LIST_FOREACH(data_list, find_list, item) {
                if (item->widget_layout) {
                    elm_object_signal_emit(item->widget_layout, SIGNAL_DELETE_BUTTON_HIDE_ANI, SIGNAL_SOURCE);
                    elm_object_signal_emit(item->widget_layout, SIGNAL_CLUSTER_NORMAL_STATE, SIGNAL_SOURCE);

                    evas_object_event_callback_del(item->widget_layout, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_edit_down_cb);
                    evas_object_event_callback_del(item->widget_layout, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_edit_move_cb);
                    evas_object_event_callback_del(item->widget_layout, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_edit_up_cb);
                }
            }
        } else if (cluster_view_s.view_state == VIEW_STATE_ADD_VIEWER) {
            add_widget_viewer_win_destroy();
        } else if (cluster_view_s.view_state == VIEW_STATE_ALL_PAGE) {
            elm_win_indicator_mode_set(homescreen_efl_get_win(), ELM_WIN_INDICATOR_SHOW);
            homescreen_efl_btn_show(HOMESCREEN_VIEW_HOME);
            page_indicator_show(cluster_view_s.indicator);
            evas_object_show(cluster_view_s.scroller);
            __cluster_view_destroy_all_page();

            Eina_List *data_list = cluster_data_get_widget_list();
            Eina_List *find_list = NULL;

            cluster_page_t *page_item = NULL;
            EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
             /*   if (page_item->page_layout) {
                    elm_object_signal_emit(page_item->page_layout, CLUSTER_ALL_PAGE_MODE_OFF, SIGNAL_SOURCE);
                }
                */
                if (page_item->page_layout)
                    elm_box_pack_end(cluster_view_s.box, page_item->page_layout);
            }
        }
    } else if (state == VIEW_STATE_ADD_VIEWER) {
        add_widget_viewer_win_create();
    } else if (state == VIEW_STATE_ALL_PAGE) {
        elm_win_indicator_mode_set(homescreen_efl_get_win(), ELM_WIN_INDICATOR_HIDE);
        homescreen_efl_btn_hide(HOMESCREEN_VIEW_HOME);
        page_indicator_hide(cluster_view_s.indicator);

        __cluster_view_create_all_page();
    }

    cluster_view_s.view_state = state;
}

bool cluster_view_add_widget(widget_data_t *item, bool scroll)
{
    LOGD("");
    if (!item->widget_layout)
        __cluster_view_add_widget_content(item);

    int page_idx = INIT_VALUE;
    bool set_on = false;

    if (item->page_idx >= 0) {
        page_idx = item->page_idx;
        LOGD("idx %d", page_idx);
    } else {
        page_idx = cluster_view_s.current_page;
        LOGD("idx %d", page_idx);
    }

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, page_idx);
    set_on = cluster_page_set_widget(page, item);

    if (!set_on && !cluster_page_set_widget(page, item)) {
        Eina_List *find_list = NULL;
        cluster_page_t *page_item = NULL;
        bool set_on = false;
        EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
            if (page_item == NULL) {
                LOGE("page is NULL");
            } else {
                if (cluster_page_set_widget(page_item, item)) {
                    page_idx = page_item->page_index;
                    LOGD("idx %d", page_idx);
                    set_on = true;
                    break;
                }
            }
        }

        if (!set_on) {
            if (cluster_view_s.page_count >= CLUSTER_MAX_PAGE) {
                LOGE("cluster page is max.");

                Evas_Smart_Cb func[3] = { NULL, NULL, NULL };
                void *data[3] = { NULL, NULL, NULL };
                popup_show(POPUP_CLUSTER_PAGE_FULL, 1, func, data);
                return false;
            } else {
                cluster_page_t *page_t = __cluster_view_page_new();
                if (!page_t || !cluster_page_set_widget(page_t, item)) {
                    LOGE("Cannot add widget");
                    return false;
                }
                page_idx = page_t->page_index;
                LOGD("idx %d", page_idx);
            }
        }
    }

    if (scroll) {
        cluster_view_s.current_page = page_idx;
        __cluster_view_scroll_to_page(cluster_view_s.current_page);
    }

    return true;
}

void cluster_view_delete_widget(widget_data_t *item)
{
    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, item->page_idx);
    if (page)
        cluster_page_unset(page, item);
    else
        LOGE("Page is NULL");
    evas_object_del(item->widget_layout);
    item->widget_layout = NULL;
}

static void __cluster_view_scroll_to_home(void)
{
    __cluster_view_scroll_to_page(CLUSTER_HOME_PAGE);
}

static void __cluster_view_scroll_to_page(int page_idx)
{
    cluster_view_s.is_srolling = true;
    elm_scroller_page_bring_in(cluster_view_s.scroller, page_idx, 0);
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

static void __cluster_view_add_widget_content(widget_data_t *item)
{
    int w, h;
    LOGD("Create Widget: pkg[%s], type[%d]", item->pkg_name, item->type);
    item->widget_layout = widget_viewer_add_widget(cluster_view_s.win, item, &w, &h);
    LOGD("widget size : %d %d", w, h);
}

static void __cluster_view_create_all_page(void)
{
    cluster_view_s.allpage = elm_layout_add(homescreen_efl_get_win());
    elm_layout_file_set(cluster_view_s.allpage, util_get_res_file_path(EDJE_DIR"/cluster_allpage.edj"), GROUP_CLUSTER_ALLPAGE_LY);

    evas_object_size_hint_align_set(cluster_view_s.allpage, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(cluster_view_s.allpage, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_move(cluster_view_s.allpage, 0, 0);
    evas_object_show(cluster_view_s.allpage);

    Evas_Object *page_bg = evas_object_rectangle_add(evas_object_evas_get(cluster_view_s.allpage));
    evas_object_size_hint_min_set(page_bg, WINDOW_W, WINDOW_H);
    evas_object_size_hint_align_set(page_bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(page_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_color_set(page_bg, 0, 0, 0, 50);
    evas_object_show(page_bg);
    elm_object_part_content_set(cluster_view_s.allpage, SIZE_SETTER, page_bg);

    Eina_List *find_list = NULL;
    cluster_page_t *page_item = NULL;
    char icon_container[PATH_MAX_LEN];
    int idx=0;
    elm_box_unpack_all(cluster_view_s.box);
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        //HIDE page
        evas_object_move(page_item->page_layout, -720, 0);
        //make thumbnail ly
        if (page_item->page_layout) {
            page_item->thumbnail_ly = elm_layout_add(page_item->page_layout);
            elm_layout_file_set(page_item->thumbnail_ly, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);
            evas_object_event_callback_add(page_item->thumbnail_ly, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_thumbnail_down_cb, page_item);
            evas_object_event_callback_add(page_item->thumbnail_ly, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_thumbnail_move_cb, page_item);
            evas_object_event_callback_add(page_item->thumbnail_ly, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_thumbnail_up_cb, page_item);

            evas_object_size_hint_align_set(page_item->thumbnail_ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
            evas_object_size_hint_weight_set(page_item->thumbnail_ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_show(page_item->thumbnail_ly);

            Evas_Object *thumbnail = cluster_page_thumbnail(page_item);
            elm_object_part_content_set(page_item->thumbnail_ly, CLUSTER_ALLPAGE_THUMBNAIL_IMAGE, thumbnail);
            elm_object_signal_emit(page_item->thumbnail_ly, SIGNAL_ALLPAGE_DELETE_BUTTON_SHOW, SIGNAL_SOURCE);
            elm_object_signal_callback_add(page_item->thumbnail_ly,
                    SIGNAL_ALLPAGE_DELETE_BUTTON_CLICKED, SIGNAL_SOURCE,
                    __cluster_view_allpage_delete_clicked, page_item);

            sprintf(icon_container, "page_%d_%d", idx % 2, idx / 2);
            elm_object_part_content_set(cluster_view_s.allpage, icon_container, page_item->thumbnail_ly);
        }
        idx++;
    }
    if (idx < CLUSTER_MAX_PAGE) {
        cluster_view_s.allpage_add = elm_layout_add(cluster_view_s.allpage);
        elm_layout_file_set(cluster_view_s.allpage_add, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);

        evas_object_size_hint_align_set(cluster_view_s.allpage_add, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(cluster_view_s.allpage_add, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_show(cluster_view_s.allpage_add);

        elm_object_signal_emit(cluster_view_s.allpage_add, SIGNAL_ALLPAGE_ADD_BUTTON_SHOW, SIGNAL_SOURCE);
        elm_object_signal_callback_add(cluster_view_s.allpage_add,
                SIGNAL_ALLPAGE_ADD_BUTTON_CLICKED, SIGNAL_SOURCE,
                __cluster_view_allpage_add_clicked, NULL);

        sprintf(icon_container, "page_%d_%d", idx % 2, idx / 2);
        elm_object_part_content_set(cluster_view_s.allpage, icon_container, cluster_view_s.allpage_add);
    }
}

static void __cluster_view_destroy_all_page(void)
{
    Eina_List *find_list = NULL;
    cluster_page_t *page_item = NULL;
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        if (page_item) {
            evas_object_del(page_item->thumbnail_ly);
            page_item->thumbnail_ly = NULL;
        }
    }
    if (cluster_view_s.allpage_add)
        evas_object_del(cluster_view_s.allpage_add);
    if (cluster_view_s.allpage)
        evas_object_del(cluster_view_s.allpage);

    cluster_view_s.allpage_add = NULL;
    cluster_view_s.allpage = NULL;
}

static cluster_page_t *__cluster_view_page_new(void)
{
    if (cluster_view_s.page_count >= CLUSTER_MAX_PAGE) {
        LOGE("cluster page is max.");
        return NULL;
    }

    cluster_page_t *page_t = cluster_page_new(cluster_view_s.box);
    cluster_view_s.page_list = eina_list_append(cluster_view_s.page_list, page_t);
    elm_box_pack_end(cluster_view_s.box, page_t->page_layout);
    cluster_view_s.page_count = eina_list_count(cluster_view_s.page_list);
    page_t->page_index = cluster_view_s.page_count - 1;
    cluster_data_set_page_count(cluster_view_s.page_count);

    page_indicator_set_page_count(cluster_view_s.indicator, cluster_view_s.page_count);
    //page_indicator_set_current_page(cluster_view_s.indicator, cluster_view_s.current_page);

    return page_t;
}

static void __cluster_view_page_delete(cluster_page_t *page)
{
    cluster_view_s.page_list = eina_list_remove(cluster_view_s.page_list,page);
    elm_box_unpack(cluster_view_s.box, page->page_layout);
    cluster_view_s.page_count--;
    cluster_data_set_page_count(cluster_view_s.page_count);
    page_indicator_set_page_count(cluster_view_s.indicator, cluster_view_s.page_count);
    //delete widgets on page.
    Eina_List *find_list = NULL;
    widget_data_t *widget = NULL;
    EINA_LIST_FOREACH(page->widget_list, find_list, widget) {
        cluster_data_delete(widget);
    }
    //delete page.
    cluster_page_delete(page);
    //reorder pages.
    __cluster_view_reorder_pages();
}
static void __cluster_view_allpage_delete_clicked (void *data, Evas_Object *obj, const char *emission, const char *source)
{
    cluster_page_t *page_item = (cluster_page_t *)data;
    if (eina_list_count(page_item->widget_list) > 0) {
        Evas_Smart_Cb func[3] = { __cluster_view_allpage_delete_page_cb, NULL, NULL };
        void *data[3] = { page_item, NULL, NULL };
        popup_show(POPUP_CLUSTER_DELETE_PAGE, 2, func, data);
    } else {
        __cluster_view_allpage_delete_page_cb(page_item, NULL, NULL);
    }
}

static void __cluster_view_allpage_add_clicked (void *data, Evas_Object *obj, const char *emission, const char *source)
{
    int page_count = eina_list_count(cluster_view_s.page_list);
    char icon_container[PATH_MAX_LEN];

    cluster_page_t *page_t = __cluster_view_page_new();
    if (!page_t) {
            LOGE("cluster page is max.");
            return ;
        }

    page_t->thumbnail_ly = elm_layout_add(page_t->page_layout);
    elm_layout_file_set(page_t->thumbnail_ly, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);
    evas_object_event_callback_add(page_t->thumbnail_ly, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_thumbnail_down_cb, page_t);
    evas_object_event_callback_add(page_t->thumbnail_ly, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_thumbnail_move_cb, page_t);
    evas_object_event_callback_add(page_t->thumbnail_ly, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_thumbnail_up_cb, page_t);

    evas_object_size_hint_align_set(page_t->thumbnail_ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(page_t->thumbnail_ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(page_t->thumbnail_ly);

    Evas_Object *thumbnail = cluster_page_thumbnail(page_t);
    elm_object_part_content_set(page_t->thumbnail_ly, CLUSTER_ALLPAGE_THUMBNAIL_IMAGE, thumbnail);
    elm_object_signal_emit(page_t->thumbnail_ly, SIGNAL_ALLPAGE_DELETE_BUTTON_SHOW, SIGNAL_SOURCE);
    elm_object_signal_callback_add(page_t->thumbnail_ly,
            SIGNAL_ALLPAGE_DELETE_BUTTON_CLICKED, SIGNAL_SOURCE,
            __cluster_view_allpage_delete_clicked, page_t);

    sprintf(icon_container, "page_%d_%d", page_count % 2, page_count / 2);
    elm_object_part_content_unset(cluster_view_s.allpage, icon_container);
    elm_object_part_content_set(cluster_view_s.allpage, icon_container, page_t->thumbnail_ly);

    page_count ++;
    if (page_count < CLUSTER_MAX_PAGE) {
        sprintf(icon_container, "page_%d_%d", page_count % 2, page_count / 2);
        elm_object_part_content_set(cluster_view_s.allpage, icon_container, cluster_view_s.allpage_add);
    }
    else {
        evas_object_del(cluster_view_s.allpage_add);
        cluster_view_s.allpage_add = NULL;
    }
}

static void __cluster_view_allpage_delete_page_cb(void *data, Evas_Object *obj, void *event_info)
{
    cluster_page_t *page = (cluster_page_t *)data;
    int page_count =0;
    __cluster_view_page_delete(page);
    page_count = eina_list_count(cluster_view_s.page_list);
    if (page_count < CLUSTER_MAX_PAGE) {
        char icon_container[PATH_MAX_LEN];
        if(cluster_view_s.allpage_add == NULL)
        {
            cluster_view_s.allpage_add = elm_layout_add(cluster_view_s.allpage);
            elm_layout_file_set(cluster_view_s.allpage_add, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);

            evas_object_size_hint_align_set(cluster_view_s.allpage_add, EVAS_HINT_FILL, EVAS_HINT_FILL);
            evas_object_size_hint_weight_set(cluster_view_s.allpage_add, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_show(cluster_view_s.allpage_add);

            elm_object_signal_emit(cluster_view_s.allpage_add, SIGNAL_ALLPAGE_ADD_BUTTON_SHOW, SIGNAL_SOURCE);
            elm_object_signal_callback_add(cluster_view_s.allpage_add,
                    SIGNAL_ALLPAGE_ADD_BUTTON_CLICKED, SIGNAL_SOURCE,
                    __cluster_view_allpage_add_clicked, NULL);
        }
        sprintf(icon_container, "page_%d_%d", page_count % 2, page_count / 2);
        elm_object_part_content_unset(cluster_view_s.allpage, icon_container);
        elm_object_part_content_set(cluster_view_s.allpage, icon_container, cluster_view_s.allpage_add);
    }
    popup_hide();
}

static void __cluster_view_reorder_pages(void)
{
    Eina_List *find_list = NULL;
    Eina_List *find_list2 = NULL;
    cluster_page_t *page_item = NULL;
    widget_data_t *widget_item = NULL;
    int page_index = 5;
    char icon_container[PATH_MAX_LEN];

    while (page_index >= 0){
        sprintf(icon_container, "page_%d_%d", page_index % 2, page_index / 2);
        LOGD("%s unset", icon_container);
        elm_object_part_content_unset(cluster_view_s.allpage, icon_container);
        page_index --;
    }
    page_index = 0;
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        EINA_LIST_FOREACH(page_item->widget_list, find_list2, widget_item) {
            LOGD("%s - %d -> %d", widget_item->pkg_name, widget_item->page_idx, page_index);
            widget_item->page_idx = page_index;
            cluster_data_update(widget_item);
        }
        sprintf(icon_container, "page_%d_%d", page_index % 2, page_index / 2);
        LOGD("%s set", icon_container);
        elm_object_part_content_set(cluster_view_s.allpage, icon_container, page_item->thumbnail_ly);
        page_index++;
    }
    if (page_index <= CLUSTER_MAX_PAGE) {
        sprintf(icon_container, "page_%d_%d", page_index % 2, page_index / 2);
        elm_object_part_content_set(cluster_view_s.allpage, icon_container, cluster_view_s.allpage_add);
    }
    //for test
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        EINA_LIST_FOREACH(page_item->widget_list, find_list2, widget_item) {
            LOGD("%s - %d", widget_item->pkg_name, widget_item->page_idx);
        }
    }

}

static void  __clsuter_view_thumbnail_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    LOGD("");
    /*if (!cluster_view_s.long_press_timer) {
        cluster_view_s.long_press_timer = ecore_timer_add(LONG_PRESS_TIME,
                __cluster_view_thumbnail_time_cb, data);
    }*/
}
static void  __clsuter_view_thumbnail_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    LOGD("");
    /*if (cluster_view_s.long_press_timer) {
        ecore_timer_del(cluster_view_s.long_press_timer);
        cluster_view_s.long_press_timer = NULL;
    }
    cluster_view_s.picked_page = NULL;*/
}
static void  __clsuter_view_thumbnail_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    LOGD("");
    /*if (cluster_view_s.picked_page != NULL)
    {
        Evas_Event_Mouse_Move* ev = event_info;
        LOGD("MOVE: (%d,%d)", ev->cur.output.x, ev->cur.output.y);
        evas_object_move(cluster_view_s.picked_page->thumbnail_ly, ev->cur.output.x-50, ev->cur.output.y-50);
    }*/
}
static Eina_Bool __cluster_view_thumbnail_time_cb(void *data)
{
    LOGD("");
    cluster_view_s.picked_page = (cluster_page_t *)data;

    Eina_List *find_list = NULL;
    cluster_page_t *page_item = NULL;
    char icon_container[PATH_MAX_LEN];
    int idx=0;
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        if (page_item == cluster_view_s.picked_page)
        {
            sprintf(icon_container, "page_%d_%d", idx % 2, idx / 2);
            elm_object_part_content_unset(cluster_view_s.allpage, icon_container);
        }
        idx++;
    }
    return EINA_FALSE;
}

static void __clsuter_view_edit_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Down* ev = event_info;
    LOGD("DOWN: (%d,%d)", ev->output.x, ev->output.y);

    cluster_mouse_info.pressed = true;
    cluster_mouse_info.down_x = cluster_mouse_info.move_x = ev->output.x;
    cluster_mouse_info.down_y = cluster_mouse_info.move_y = ev->output.y;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    cluster_mouse_info.long_press_timer = ecore_timer_add(LONG_PRESS_TIME,
            __cluster_view_edit_long_press_time_cb, data);
}

static void __clsuter_view_edit_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Up* ev = event_info;
    LOGD("UP: (%d,%d)", ev->output.x, ev->output.y);

    if (!cluster_mouse_info.pressed)
        return ;

    cluster_mouse_info.pressed = false;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    if (!cluster_mouse_info.long_pressed)
        return ;

    cluster_mouse_info.long_pressed = false;

    cluster_mouse_info.up_x = ev->output.x;
    cluster_mouse_info.up_y = ev->output.y;

    elm_scroller_movement_block_set(cluster_view_s.scroller, ELM_SCROLLER_MOVEMENT_NO_BLOCK);

    if (cluster_view_s.picked_widget) {
        __cluster_view_edit_drop_widget(data);

        cluster_view_s.picked_widget = NULL;
    }
}

static void __clsuter_view_edit_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Move* ev = event_info;
    LOGD("MOVE: (%d,%d)", ev->cur.output.x, ev->cur.output.y);

    if (!cluster_mouse_info.pressed)
        return ;

    cluster_mouse_info.move_x = ev->cur.output.x;
    cluster_mouse_info.move_y = ev->cur.output.y;

    if (!cluster_mouse_info.long_pressed) {
        int distance = (cluster_mouse_info.move_x - cluster_mouse_info.down_x) * (cluster_mouse_info.move_x - cluster_mouse_info.down_x);
        distance += (cluster_mouse_info.move_y - cluster_mouse_info.down_y) * (cluster_mouse_info.move_y - cluster_mouse_info.down_y);

        if (distance > MOUSE_MOVE_MIN_DISTANCE) {
            if (cluster_mouse_info.long_press_timer) {
                ecore_timer_del(cluster_mouse_info.long_press_timer);
                cluster_mouse_info.long_press_timer = NULL;
            }

            return ;
        }
    }

    if (cluster_view_s.picked_widget) {
        __cluster_view_edit_drag_widget(data);
    }
}

static Eina_Bool __cluster_view_edit_long_press_time_cb(void *data)
{
    if (!cluster_mouse_info.pressed)
        return ECORE_CALLBACK_CANCEL;

    cluster_mouse_info.long_pressed = true;

    elm_scroller_movement_block_set(cluster_view_s.scroller, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);

    __cluster_view_edit_pick_up_widget(data);

    return ECORE_CALLBACK_CANCEL;
}

static void __cluster_view_edit_pick_up_widget(void *data)
{
    int cx = -1, cy = -1;
    int gx = -1, gy = -1;
    Evas *e = NULL;
    cluster_view_s.picked_widget = (widget_data_t *)data;

    Evas_Object *widget_layout = cluster_view_s.picked_widget->widget_layout;
    elm_object_signal_emit(widget_layout, SIGNAL_DELETE_BUTTON_HIDE_ANI, SIGNAL_SOURCE);
    elm_object_signal_emit(widget_layout, SIGNAL_CLUSTER_PICKUP_STATE, SIGNAL_SOURCE);

    e = evas_object_evas_get(widget_layout);

    evas_pointer_canvas_xy_get(e, &cx, &cy);
    evas_object_geometry_get(widget_layout, &gx, &gy, NULL, NULL);

    cluster_mouse_info.offset_x = cx - gx;
    cluster_mouse_info.offset_y = cy - gy;

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, cluster_view_s.picked_widget->page_idx);
    elm_grid_unpack(page->grid, widget_layout);

    evas_object_move(widget_layout, cluster_mouse_info.move_x - cluster_mouse_info.offset_x,
            cluster_mouse_info.move_y - cluster_mouse_info.offset_y);

    cluster_page_unset(page, cluster_view_s.picked_widget);
}

static void __cluster_view_edit_drag_widget(void *data)
{
    int page_x, page_y;
    int bg_x, bg_y, bg_w, bg_h;
    int widget_x, widget_y;
    cluster_view_s.picked_widget = (widget_data_t *)data;

    if (!cluster_view_s.picked_widget)
        return ;

    Evas_Object *widget_layout = cluster_view_s.picked_widget->widget_layout;
    evas_object_move(widget_layout, cluster_mouse_info.move_x - cluster_mouse_info.offset_x,
            cluster_mouse_info.move_y - cluster_mouse_info.offset_y);

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, cluster_view_s.current_page);
    evas_object_geometry_get(page->page_layout, &page_x, &page_y, NULL, NULL);
    edje_object_part_geometry_get(elm_layout_edje_get(page->page_layout), CLUSTER_BG, &bg_x, &bg_y, &bg_w, &bg_h);
    evas_object_geometry_get(widget_layout, &widget_x, &widget_y, NULL, NULL);

    if (cluster_mouse_info.move_x > CLUSTER_EDIT_RIGHT_SCROLL_REGION) {
        LOGD("Move to next page");
        if (!cluster_view_s.edit_mode_scroll_timer)
            cluster_view_s.edit_mode_scroll_timer = ecore_timer_add(HOME_EDIT_SCROLL_MOVE_TIME, __cluster_view_scroll_timer_cb, NULL);

    } else if (cluster_mouse_info.move_x < CLUSTER_EDIT_LEFT_SCROLL_REGION) {
        LOGD("Move to prev page");
        if (!cluster_view_s.edit_mode_scroll_timer)
            cluster_view_s.edit_mode_scroll_timer = ecore_timer_add(HOME_EDIT_SCROLL_MOVE_TIME, __cluster_view_scroll_timer_cb, NULL);
    } else {
        widget_x -= (page_x + bg_x);
        widget_y -= (page_y + bg_y);
        if (cluster_view_s.edit_mode_scroll_timer) {
            ecore_timer_del(cluster_view_s.edit_mode_scroll_timer);
            cluster_view_s.edit_mode_scroll_timer = NULL;
        }

        if (!cluster_view_s.is_srolling) {
            cluster_page_drag_widget(page, cluster_view_s.picked_widget->type, widget_x / (bg_w / CLUSTER_COL), widget_y / (bg_h / CLUSTER_ROW));
        }
    }
}

static void __cluster_view_edit_drop_widget(void *data)
{
    LOGD("");
    Evas_Object *widget_layout = cluster_view_s.picked_widget->widget_layout;
    elm_object_signal_emit(widget_layout, SIGNAL_DELETE_BUTTON_SHOW_ANI, SIGNAL_SOURCE);
    elm_object_signal_emit(widget_layout, SIGNAL_CLUSTER_EDIT_STATE, SIGNAL_SOURCE);

    if (cluster_view_s.edit_mode_scroll_timer) {
        ecore_timer_del(cluster_view_s.edit_mode_scroll_timer);
        cluster_view_s.edit_mode_scroll_timer = NULL;
    }

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, cluster_view_s.current_page);

    if (!cluster_page_drop_widget(page, cluster_view_s.picked_widget)) {
        cluster_view_add_widget(cluster_view_s.picked_widget, false);
    }
}

static void __cluster_view_scroll_anim_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
    cluster_view_s.is_srolling = false;
    elm_scroller_current_page_get(obj, &cluster_view_s.current_page, NULL);
    cluster_view_s.current_page = cluster_view_s.current_page % cluster_view_s.page_count;
    LOGD("current page %d", cluster_view_s.current_page);
}

static Eina_Bool __cluster_view_scroll_timer_cb(void *data)
{
    cluster_page_t *current_page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, cluster_view_s.current_page);

    int next_page_idx = 0;
    if (cluster_mouse_info.move_x > CLUSTER_EDIT_RIGHT_SCROLL_REGION) {
        next_page_idx = current_page->page_index + 1;
    } else if (cluster_mouse_info.move_x < CLUSTER_EDIT_LEFT_SCROLL_REGION) {
        next_page_idx = current_page->page_index - 1;
    } else {
        return ECORE_CALLBACK_CANCEL;
    }
    cluster_page_drag_cancel(current_page);
    __cluster_view_scroll_to_page(next_page_idx);
    return ECORE_CALLBACK_RENEW;
}

