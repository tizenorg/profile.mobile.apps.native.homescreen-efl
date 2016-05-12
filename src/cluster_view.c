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

#include "homescreen-efl.h"
#include "cluster_view.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"
#include "page_indicator.h"
#include "widget_viewer.h"
#include "menu.h"
#include "cluster_page.h"
#include "add_widget_viewer/add_widget_viewer.h"

#define FOLDER_STR_LEN 256

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
} cluster_view_s = {
    .scroller = NULL,
    .box = NULL,
    .allpage = NULL,
    .allpage_add = NULL,
    .indicator = NULL,
    .page_count = 0,
    .current_page = -1,
    .view_state = VIEW_STATE_NORMAL,
    .page_list = NULL
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
static void __cluster_view_scroll_to_page(int page_idx);
static void __cluster_view_add_widget(widget_data_t *item);
static void __cluster_view_create_all_page(void);
static void __cluster_view_destroy_all_page(void);
static void __cluster_view_allpage_delete_clicked (void *data, Evas_Object *obj, const char *emission, const char *source);
static void __cluster_view_allpage_add_clicked (void *data, Evas_Object *obj, const char *emission, const char *source);

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
    evas_object_resize(cluster_view_s.scroller, CLUSTER_VIEW_W, CLUSTER_VIEW_H);

    elm_scroller_loop_set(cluster_view_s.scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_page_size_set(cluster_view_s.scroller, CLUSTER_W , CLUSTER_H);

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
    cluster_view_s.page_count = page_count;
    cluster_view_s.current_page = CLUSTER_HOME_PAGE;

    while (page_count--){
        cluster_page_t *page_t = cluster_page_new(cluster_view_s.box);
        cluster_view_s.page_list = eina_list_append(cluster_view_s.page_list, page_t);
        elm_box_pack_end(cluster_view_s.box, page_t->page_layout);
    }
    Eina_List *find_list;
    widget_data_t *item = NULL;
    EINA_LIST_FOREACH(widget_list, find_list, item) {
        __cluster_view_add_widget(item);
        cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, item->page_idx);
        if (page == NULL) {
            LOGE("page is NULL");
        } else {
            cluster_page_set(page, item);
        }
    }
}

static void __cluster_view_change_to_all_pages_view()
{
    Eina_List *page_list = elm_box_children_get(cluster_view_s.box);
    int page_count = eina_list_count(page_list);
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

            /*
            Eina_List *data_list = cluster_data_get_widget_list();
            Eina_List *find_list = NULL;

            cluster_page_t *page_item = NULL;
            EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
                if (page_item->page_layout) {
                    elm_object_signal_emit(page_item->page_layout, CLUSTER_ALL_PAGE_MODE_OFF, SIGNAL_SOURCE);
                }
            }*/
        }
    } else if (state == VIEW_STATE_ADD_VIEWER) {
        add_widget_viewer_win_create();
    } else if (state == VIEW_STATE_ALL_PAGE) {
        elm_win_indicator_mode_set(homescreen_efl_get_win(), ELM_WIN_INDICATOR_HIDE);
        homescreen_efl_btn_hide(HOMESCREEN_VIEW_HOME);
        page_indicator_hide(cluster_view_s.indicator);
        evas_object_hide(cluster_view_s.scroller);

        __cluster_view_create_all_page();
/*
        Eina_List *data_list = cluster_data_get_widget_list();
        Eina_List *find_list = NULL;
        elm_box_unpack_all(cluster_view_s.box);
        cluster_page_t *page_item = NULL;
        int i = 0;
        EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
            if (page_item->page_layout) {
                Evas_Object *size_setter = elm_object_part_content_get(page_item->page_layout, SIZE_SETTER);
                if (size_setter)
                    evas_object_size_hint_min_set(size_setter, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
                evas_object_resize(page_item->page_layout, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
                elm_object_signal_emit(page_item->page_layout, CLUSTER_ALL_PAGE_MODE_ON, SIGNAL_SOURCE);
            }
        }
        */
    }

    cluster_view_s.view_state = state;
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

static void __cluster_view_add_widget(widget_data_t *item)
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
    char icon_container[FOLDER_STR_LEN];
    int idx=0;
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        if (page_item->page_layout) {
            page_item->thumbnail_ly = elm_layout_add(page_item->page_layout);
            elm_layout_file_set(page_item->thumbnail_ly, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);

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
    LOGD("zzz total page count %d", idx);
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

static void __cluster_view_allpage_delete_clicked (void *data, Evas_Object *obj, const char *emission, const char *source)
{
}

static void __cluster_view_allpage_add_clicked (void *data, Evas_Object *obj, const char *emission, const char *source)
{
    int page_count = eina_list_count(cluster_view_s.page_list);
    char icon_container[FOLDER_STR_LEN];

    cluster_page_t *page_t = cluster_page_new(cluster_view_s.box);
    cluster_view_s.page_list = eina_list_append(cluster_view_s.page_list, page_t);

    page_t->thumbnail_ly = elm_layout_add(page_t->page_layout);
    elm_layout_file_set(page_t->thumbnail_ly, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);

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

    cluster_data_set_page_count(page_count);
}
