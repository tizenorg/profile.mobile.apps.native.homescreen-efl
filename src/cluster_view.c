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
    Evas_Object *allpage_add_page;
    Evas_Object *allpage_grid;
    page_indicator_t *indicator;
    int page_count;
    int current_page;
    view_state_t view_state;
    Eina_List *page_list;
    cluster_page_t *picked_page;
    widget_data_t *picked_widget;
    Ecore_Timer *edit_mode_scroll_timer;
    bool is_srolling;
    Ecore_Animator *edit_animator;
    int animation_from_x;
    int animation_from_y;
    int animation_to_x;
    int animation_to_y;
} cluster_view_s = {
    .scroller = NULL,
    .box = NULL,
    .allpage = NULL,
    .allpage_add_page = NULL,
    .allpage_grid = NULL,
    .indicator = NULL,
    .page_count = 0,
    .current_page = INIT_VALUE,
    .view_state = VIEW_STATE_NORMAL,
    .page_list = NULL,
    .picked_page = NULL,
    .picked_widget = NULL,
    .edit_mode_scroll_timer = NULL,
    .is_srolling = false,
    .edit_animator = NULL,
    .animation_from_x = INIT_VALUE,
    .animation_from_y = INIT_VALUE,
    .animation_to_x = INIT_VALUE,
    .animation_to_y = INIT_VALUE,
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
    .pressed_obj = NULL,
};

static Eina_Hash *cluster_menu_table = NULL;

Evas_Object *__cluster_view_create_base_gui(Evas_Object *win);

static void __clsuter_view_scroller_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_scroller_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_scroller_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool __cluster_view_scroller_long_press_time_cb(void *data);

static void __cluster_view_create_cluster(void);
static void __cluster_view_create_menu(void);
static void __cluster_view_menu_edit_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_menu_add_widget_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_menu_change_wallpaper_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_menu_all_pages_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_app_launch_request_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data);
static void __cluster_view_scroll_to_home(void);
static void __cluster_view_scroll_to_page(int page_idx, bool animation);
static void __cluster_view_add_widget_content(widget_data_t *item);
static void __cluster_view_create_all_page(void);
static void __cluster_view_destroy_all_page(void);
static cluster_page_t * __cluster_view_page_new(void);
static void __cluster_view_page_delete(cluster_page_t *page);
static void __cluster_view_allpage_delete_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __cluster_view_allpage_add_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __cluster_view_allpage_delete_page_cb(void *data, Evas_Object *obj, void *event_info);

static void __clsuter_view_thumbnail_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_thumbnail_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_thumbnail_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool __cluster_view_thumbnail_long_press_time_cb(void *data);
static void __cluster_view_allpage_drag_page(void *data);
static void __cluster_view_allpage_pick_up_page(void *data);
static void __cluster_view_allpage_drop_page(void *data);

static void __clsuter_view_widget_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_widget_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __clsuter_view_widget_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool __cluster_view_widget_long_press_time_cb(void *data);

static void __cluster_view_edit_pick_up_widget(void *data);
static void __cluster_view_edit_drag_widget(void *data);
static void __cluster_view_edit_drop_widget(void *data);
static Eina_Bool __cluster_view_edit_move_anim(void *data, double pos);
static void __cluster_view_edit_move_anim_done(void *data);

static Eina_Bool __cluster_view_scroll_timer_cb(void *data);

static void __cluster_view_scroll_anim_start_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_scroll_anim_stop_cb(void *data, Evas_Object *obj, void *event_info);
static void __cluster_view_allpage_get_page_pos(int page_idx, int *w, int *h);
static int __cluster_view_allpage_get_page_index(int x, int y);
static void __cluster_view_allpage_reposition(void);
static int __cluster_view_page_sort_cb(const void *a , const void *b);

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
    evas_object_event_callback_del(cluster_view_s.scroller, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_scroller_down_cb);
    evas_object_event_callback_del(cluster_view_s.scroller, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_scroller_move_cb);
    evas_object_event_callback_del(cluster_view_s.scroller, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_scroller_up_cb);

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

    evas_object_event_callback_add(cluster_view_s.scroller, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_scroller_down_cb, NULL);
    evas_object_event_callback_add(cluster_view_s.scroller, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_scroller_move_cb, NULL);
    evas_object_event_callback_add(cluster_view_s.scroller, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_scroller_up_cb, NULL);

    evas_object_smart_callback_add(cluster_view_s.scroller, "scroll,anim,stop", __cluster_view_scroll_anim_stop_cb, NULL);
    evas_object_smart_callback_add(cluster_view_s.scroller, "scroll,anim,start", __cluster_view_scroll_anim_start_cb, NULL);

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

    while (page_count--) {
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
    menu_hide();

    cluster_view_set_state(VIEW_STATE_EDIT);
}

static void __cluster_view_menu_add_widget_cb(void *data, Evas_Object *obj, void *event_info)
{
    menu_hide();

    cluster_view_set_state(VIEW_STATE_ADD_VIEWER);
}

static void __cluster_view_menu_change_wallpaper_cb(void *data, Evas_Object *obj, void *event_info)
{
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
    } else if (cluster_view_s.view_state == VIEW_STATE_EDIT) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    } else if (cluster_view_s.view_state == VIEW_STATE_ADD_VIEWER) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    } else if (cluster_view_s.view_state == VIEW_STATE_ALL_PAGE) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    }

    return false;
}

bool cluster_view_hw_back_key(void)
{
    if (cluster_view_s.view_state == VIEW_STATE_EDIT) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    } else if (cluster_view_s.view_state == VIEW_STATE_ADD_VIEWER) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    } else if (cluster_view_s.view_state == VIEW_STATE_ALL_PAGE) {
        cluster_view_set_state(VIEW_STATE_NORMAL);
    }

    return true;
}

view_state_t cluster_view_get_state(void)
{
    return cluster_view_s.view_state;
}

bool cluster_view_set_state(view_state_t state)
{
    if (cluster_view_s.is_srolling) {
        LOGE("cannot change view-state");
        return false;
    }

    if (cluster_view_s.edit_animator) {
        ecore_animator_del(cluster_view_s.edit_animator);
        cluster_view_s.edit_animator = NULL;
        __cluster_view_edit_move_anim_done(cluster_view_s.picked_widget);
    }

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
                if (!cluster_view_s.picked_widget || item->widget_layout != cluster_view_s.picked_widget->widget_layout)
                    elm_object_signal_emit(item->widget_layout, SIGNAL_DELETE_BUTTON_SHOW_ANI, SIGNAL_SOURCE);
                elm_object_signal_emit(item->widget_layout, SIGNAL_CLUSTER_EDIT_STATE, SIGNAL_SOURCE);
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

            Eina_List *find_list = NULL;

            cluster_page_t *page_item = NULL;
            elm_box_unpack_all(cluster_view_s.box);
            EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
                if (page_item->page_layout) {
                    elm_box_pack_end(cluster_view_s.box, page_item->page_layout);
                }
            }

            if (cluster_view_s.current_page >= cluster_view_s.page_count) {
                cluster_view_s.current_page = CLUSTER_HOME_PAGE;
            }
            __cluster_view_scroll_to_page(cluster_view_s.current_page, false);
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

    return true;
}

bool cluster_view_add_widget(widget_data_t *item, bool scroll)
{
    if (!item->widget_layout)
        __cluster_view_add_widget_content(item);

    int page_idx = INIT_VALUE;
    bool set_on = false;

    if (item->page_idx >= 0) {
        page_idx = item->page_idx;
    } else {
        page_idx = cluster_view_s.current_page;
    }

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, page_idx);
    set_on = cluster_page_set_widget(page, item);

    if (!set_on) {
        Eina_List *find_list = NULL;
        cluster_page_t *page_item = NULL;
        bool set_on = false;
        EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
            if (page_item == NULL) {
                LOGE("page is NULL");
            } else {
                if (cluster_page_set_widget(page_item, item)) {
                    page_idx = page_item->page_index;
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
                char btn_text[3][STR_MAX] = { "", "", "" };
                char title_text[STR_MAX] = { "" };
                char popup_text[STR_MAX] = { "" };
                snprintf(btn_text[0], sizeof(btn_text[0]), "%s", _("IDS_CAM_SK_OK"));
                snprintf(title_text, sizeof(title_text), "%s", _("IDS_HS_HEADER_UNABLE_TO_ADD_WIDGET_ABB"));
                snprintf(popup_text, sizeof(popup_text), "%s", _("IDS_HS_POP_UNABLE_TO_ADD_THIS_HOME_BOX_TO_THE_HOME_SCREEN_THERE_IS_NOT_ENOUGH_SPACE_ON_THE_HOME_SCREEN_MSG"));
                popup_show(1, func, data, btn_text, title_text, popup_text);
                return false;
            } else {
                cluster_page_t *page_t = __cluster_view_page_new();
                if (!page_t || !cluster_page_set_widget(page_t, item)) {
                    LOGE("Cannot add widget");
                    return false;
                }
                page_idx = page_t->page_index;
            }
        }
    }

    if (scroll) {
        cluster_view_s.current_page = page_idx;
        __cluster_view_scroll_to_page(cluster_view_s.current_page, true);
    }

    return true;
}

void cluster_view_delete_widget(widget_data_t *item)
{
    if (item == NULL)
        return ;

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, item->page_idx);
    if (page)
        cluster_page_unset(page, item);
    else
        LOGE("Page is NULL");

    if (item->widget_layout) {
        evas_object_event_callback_del(item->widget_layout, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_widget_down_cb);
        evas_object_event_callback_del(item->widget_layout, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_widget_move_cb);
        evas_object_event_callback_del(item->widget_layout, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_widget_up_cb);

        evas_object_del(item->widget_layout);
        item->widget_layout = NULL;
    }
}

static void __cluster_view_scroll_to_home(void)
{
    __cluster_view_scroll_to_page(CLUSTER_HOME_PAGE, true);
}

static void __cluster_view_scroll_to_page(int page_idx, bool animation)
{
    if (animation) {
        elm_scroller_page_bring_in(cluster_view_s.scroller, page_idx, 0);
    } else {
        page_indicator_set_current_page(cluster_view_s.indicator, page_idx);
        elm_scroller_page_show(cluster_view_s.scroller, page_idx, 0);
    }
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
    evas_object_event_callback_add(item->widget_layout, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_widget_down_cb, item);
    evas_object_event_callback_add(item->widget_layout, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_widget_move_cb, item);
    evas_object_event_callback_add(item->widget_layout, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_widget_up_cb, item);
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

    Evas_Object *grid = elm_grid_add(cluster_view_s.allpage);
    evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_grid_size_set(grid, WINDOW_W, WINDOW_H);
    evas_object_show(grid);
    elm_layout_content_set(cluster_view_s.allpage, CLUSTER_ALLPAGE_GRID, grid);
    cluster_view_s.allpage_grid = grid;

    Eina_List *find_list = NULL;
    cluster_page_t *page_item = NULL;
    elm_box_unpack_all(cluster_view_s.box);
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        evas_object_move(page_item->page_layout, -720, 0);
        if (page_item->page_layout) {
            page_item->thumbnail_ly = elm_layout_add(page_item->page_layout);
            elm_layout_file_set(page_item->thumbnail_ly, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);

            evas_object_size_hint_align_set(page_item->thumbnail_ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
            evas_object_size_hint_weight_set(page_item->thumbnail_ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_show(page_item->thumbnail_ly);

            Evas_Object *thumbnail = cluster_page_thumbnail(page_item);
            elm_object_part_content_set(page_item->thumbnail_ly, CLUSTER_ALLPAGE_THUMBNAIL_IMAGE, thumbnail);
            elm_object_signal_callback_add(page_item->thumbnail_ly,
                    SIGNAL_ALLPAGE_DELETE_BUTTON_CLICKED, SIGNAL_SOURCE,
                    __cluster_view_allpage_delete_clicked, page_item);

            evas_object_event_callback_add(thumbnail, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_thumbnail_down_cb, page_item);
            evas_object_event_callback_add(thumbnail, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_thumbnail_move_cb, page_item);
            evas_object_event_callback_add(thumbnail, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_thumbnail_up_cb, page_item);
        }
    }

    __cluster_view_allpage_reposition();

    if (cluster_view_s.page_count < CLUSTER_MAX_PAGE) {
        int x = 0, y = 0;
        cluster_view_s.allpage_add_page = elm_layout_add(cluster_view_s.allpage_grid);
        elm_layout_file_set(cluster_view_s.allpage_add_page, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);

        evas_object_size_hint_align_set(cluster_view_s.allpage_add_page, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(cluster_view_s.allpage_add_page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_show(cluster_view_s.allpage_add_page);

        elm_object_signal_emit(cluster_view_s.allpage_add_page, SIGNAL_ALLPAGE_ADD_BUTTON_SHOW, SIGNAL_SOURCE);
        elm_object_signal_callback_add(cluster_view_s.allpage_add_page,
                SIGNAL_ALLPAGE_ADD_BUTTON_CLICKED, SIGNAL_SOURCE,
                __cluster_view_allpage_add_clicked, NULL);

        __cluster_view_allpage_get_page_pos(cluster_view_s.page_count, &x, &y);
        elm_grid_pack(cluster_view_s.allpage_grid, cluster_view_s.allpage_add_page, x, y, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
    }
}

static void __cluster_view_allpage_reposition(void)
{
    Eina_List *find_list = NULL;
    cluster_page_t *page_item = NULL;
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        int x = 0, y = 0;
        __cluster_view_allpage_get_page_pos(page_item->page_index, &x, &y);
        elm_grid_pack(cluster_view_s.allpage_grid, page_item->thumbnail_ly, x, y, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
        if (cluster_view_s.page_count > 1)
            elm_object_signal_emit(page_item->thumbnail_ly, SIGNAL_ALLPAGE_DELETE_BUTTON_SHOW, SIGNAL_SOURCE);
        else {
            elm_object_signal_emit(page_item->thumbnail_ly, SIGNAL_ALLPAGE_DELETE_BUTTON_HIDE, SIGNAL_SOURCE);
        }
    }
}

static void __cluster_view_allpage_get_page_pos(int page_idx, int *w, int *h)
{
    int row, col;
    int start_y = 0;
    if (cluster_view_s.page_count < 2) {
        start_y = (WINDOW_H - CLUSTER_ALL_PAGE_H) / 2;
    } else if (cluster_view_s.page_count < 4) {
        start_y = (WINDOW_H - (CLUSTER_ALL_PAGE_H * 2 + CLUSTER_ALL_PAGE_GAP_H)) / 2;
    } else {
        start_y = (WINDOW_H - (CLUSTER_ALL_PAGE_H * 3 + CLUSTER_ALL_PAGE_GAP_H * 2)) / 2;
    }
    row = page_idx / 2;
    col = page_idx % 2;

    *w = CLUSTER_ALL_PAGE_PADDING_SIDE + (col * (CLUSTER_ALL_PAGE_W + CLUSTER_ALL_PAGE_GAP_W));
    *h = start_y + (row * (CLUSTER_ALL_PAGE_H + CLUSTER_ALL_PAGE_GAP_H));
}

static int __cluster_view_allpage_get_page_index(int x, int y)
{
    int idx = 0;
    int sx = CLUSTER_ALL_PAGE_PADDING_SIDE;
    int sy = 0;
    if (cluster_view_s.page_count < 2) {
        sy = (WINDOW_H - CLUSTER_ALL_PAGE_H) / 2;
    } else if (cluster_view_s.page_count < 4) {
        sy = (WINDOW_H - (CLUSTER_ALL_PAGE_H * 2 + CLUSTER_ALL_PAGE_GAP_H)) / 2;
    } else {
        sy = (WINDOW_H - (CLUSTER_ALL_PAGE_H * 3 + CLUSTER_ALL_PAGE_GAP_H * 2)) / 2;
    }
    int w = CLUSTER_ALL_PAGE_W + CLUSTER_ALL_PAGE_GAP_W;
    int h = CLUSTER_ALL_PAGE_H + CLUSTER_ALL_PAGE_GAP_H;

    for (idx=0; idx < cluster_view_s.page_count; idx++) {
        int row = idx / 2;
        int col = idx % 2;
        int ny = sy + (row * h);
        int nx = sx + (col * w);

        int d = (ny - y) * (ny - y) + (nx - x) * (nx - x);
        if (d < CLUSTER_VIEW_ALLPAGE_MOVE_GAP) {
            return (row * 2) + col;
        }
    }

    return INIT_VALUE;
}

static void __cluster_view_destroy_all_page(void)
{
    Eina_List *find_list = NULL;
    cluster_page_t *page_item = NULL;
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        if (page_item) {
            Evas_Object *image = elm_object_part_content_get(page_item->thumbnail_ly, CLUSTER_ALLPAGE_THUMBNAIL_IMAGE);
            evas_object_event_callback_del(image, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_thumbnail_down_cb);
            evas_object_event_callback_del(image, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_thumbnail_move_cb);
            evas_object_event_callback_del(image, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_thumbnail_up_cb);

            evas_object_del(page_item->thumbnail_ly);
            page_item->thumbnail_ly = NULL;
        }
    }
    if (cluster_view_s.allpage_add_page)
        evas_object_del(cluster_view_s.allpage_add_page);
    if (cluster_view_s.allpage)
        evas_object_del(cluster_view_s.allpage);

    cluster_view_s.allpage_add_page = NULL;
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

    return page_t;
}

static void __cluster_view_page_delete(cluster_page_t *page)
{
    elm_grid_unpack(cluster_view_s.allpage_grid, page->thumbnail_ly);

    cluster_view_s.page_list = eina_list_remove(cluster_view_s.page_list, page);
    elm_box_unpack(cluster_view_s.box, page->page_layout);

    Eina_List *find_list = NULL;
    widget_data_t *widget = NULL;
    EINA_LIST_FOREACH(page->widget_list, find_list, widget) {
        cluster_data_delete(widget);
    }
    cluster_page_delete(page);

    cluster_view_s.page_count--;

    cluster_data_set_page_count(cluster_view_s.page_count);
    page_indicator_set_page_count(cluster_view_s.indicator, cluster_view_s.page_count);
}

static void __cluster_view_allpage_delete_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    if (cluster_view_s.page_count <= 1)
        return ;

    cluster_page_t *page_item = (cluster_page_t *)data;
    if (eina_list_count(page_item->widget_list) > 0) {
        Evas_Smart_Cb func[3] = { __cluster_view_allpage_delete_page_cb, NULL, NULL };
        void *data[3] = { page_item, NULL, NULL };
        char btn_text[3][STR_MAX] = { "", "", "" };
        char title_text[STR_MAX] = { "" };
        char popup_text[STR_MAX] = { "" };
        snprintf(btn_text[0], sizeof(btn_text[0]), "%s", _("IDS_HS_OPT_DELETE"));
        snprintf(btn_text[1], sizeof(btn_text[1]), "%s", _("IDS_CAM_SK_CANCEL"));
        snprintf(title_text, sizeof(title_text), "%s", _("IDS_HS_HEADER_DELETE_PAGE_ABB2"));
        snprintf(popup_text, sizeof(popup_text), "%s", _("IDS_HS_POP_THIS_PAGE_AND_ALL_THE_ITEMS_IT_CONTAINS_WILL_BE_DELETED"));
        popup_show(2, func, data, btn_text, title_text, popup_text);
    } else {
        __cluster_view_allpage_delete_page_cb(page_item, NULL, NULL);
    }
}

static void __cluster_view_allpage_add_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    cluster_page_t *page_t = __cluster_view_page_new();
    if (!page_t) {
        LOGE("cluster page is max.");
        return ;
    }

    elm_grid_unpack(cluster_view_s.allpage_grid, cluster_view_s.allpage_add_page);

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

    evas_object_event_callback_add(thumbnail, EVAS_CALLBACK_MOUSE_DOWN, __clsuter_view_thumbnail_down_cb, page_t);
    evas_object_event_callback_add(thumbnail, EVAS_CALLBACK_MOUSE_MOVE, __clsuter_view_thumbnail_move_cb, page_t);
    evas_object_event_callback_add(thumbnail, EVAS_CALLBACK_MOUSE_UP, __clsuter_view_thumbnail_up_cb, page_t);

    __cluster_view_allpage_reposition();

    if (cluster_view_s.page_count < CLUSTER_MAX_PAGE) {
        int x = 0, y = 0;
        __cluster_view_allpage_get_page_pos(cluster_view_s.page_count, &x, &y);
        elm_grid_pack(cluster_view_s.allpage_grid, cluster_view_s.allpage_add_page, x, y, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
    } else {
        evas_object_del(cluster_view_s.allpage_add_page);
        cluster_view_s.allpage_add_page = NULL;
    }
}

static void __cluster_view_allpage_delete_page_cb(void *data, Evas_Object *obj, void *event_info)
{
    cluster_page_t *page = (cluster_page_t *)data;

    Eina_List *find_list = NULL;
    cluster_page_t *page_item = NULL;
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        if (page_item->page_index > page->page_index) {
            page_item->page_index -= 1;
        }
    }

    __cluster_view_page_delete(page);
    __cluster_view_allpage_reposition();

    if (cluster_view_s.page_count < CLUSTER_MAX_PAGE) {
        if (cluster_view_s.allpage_add_page == NULL) {
            cluster_view_s.allpage_add_page = elm_layout_add(cluster_view_s.allpage);
            elm_layout_file_set(cluster_view_s.allpage_add_page, util_get_res_file_path(EDJE_DIR"/cluster_allpage_thumbnail.edj"), GROUP_CLUSTER_ALLPAGE_THUMBNAIL_LY);

            evas_object_size_hint_align_set(cluster_view_s.allpage_add_page, EVAS_HINT_FILL, EVAS_HINT_FILL);
            evas_object_size_hint_weight_set(cluster_view_s.allpage_add_page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_show(cluster_view_s.allpage_add_page);

            elm_object_signal_emit(cluster_view_s.allpage_add_page, SIGNAL_ALLPAGE_ADD_BUTTON_SHOW, SIGNAL_SOURCE);
            elm_object_signal_callback_add(cluster_view_s.allpage_add_page,
                    SIGNAL_ALLPAGE_ADD_BUTTON_CLICKED, SIGNAL_SOURCE,
                    __cluster_view_allpage_add_clicked, NULL);
        }
        int x = 0, y = 0;
        __cluster_view_allpage_get_page_pos(cluster_view_s.page_count, &x, &y);
        elm_grid_pack(cluster_view_s.allpage_grid, cluster_view_s.allpage_add_page, x, y, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
    }
    popup_hide();
}

static void  __clsuter_view_thumbnail_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Down* ev = event_info;
    LOGD("DOWN: (%d,%d)", ev->output.x, ev->output.y);

    cluster_mouse_info.pressed = true;
    cluster_mouse_info.pressed_obj = obj;

    cluster_mouse_info.down_x = cluster_mouse_info.move_x = ev->output.x;
    cluster_mouse_info.down_y = cluster_mouse_info.move_y = ev->output.y;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    cluster_mouse_info.long_press_timer = ecore_timer_add(LONG_PRESS_TIME,
            __cluster_view_thumbnail_long_press_time_cb, data);
}

static void  __clsuter_view_thumbnail_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    cluster_page_t *page_t = (cluster_page_t *)data;
    Evas_Event_Mouse_Up* ev = event_info;
    LOGD("UP: (%d,%d)", ev->output.x, ev->output.y);

    if (!cluster_mouse_info.pressed)
        return ;

    cluster_mouse_info.pressed = false;
    cluster_mouse_info.pressed_obj = NULL;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    cluster_mouse_info.up_x = ev->output.x;
    cluster_mouse_info.up_y = ev->output.y;

    if (!cluster_mouse_info.long_pressed) {
        int distance = (cluster_mouse_info.down_x - cluster_mouse_info.up_x) * (cluster_mouse_info.down_x - cluster_mouse_info.up_x);
        distance += (cluster_mouse_info.down_y - cluster_mouse_info.up_y) * (cluster_mouse_info.down_y - cluster_mouse_info.up_y);

        if (distance <= MOUSE_MOVE_MIN_DISTANCE) {
            cluster_view_s.current_page = page_t->page_index;
            cluster_view_set_state(VIEW_STATE_NORMAL);
        }

        return ;
    }

    cluster_mouse_info.long_pressed = false;

    if (cluster_view_s.picked_page) {
        __cluster_view_allpage_drop_page(data);
    }
}

static int __cluster_view_page_sort_cb(const void *a , const void *b)
{
    cluster_page_t *item1 = (cluster_page_t *)a;
    cluster_page_t *item2 = (cluster_page_t *)b;

    return item1->page_index - item2->page_index;
}

static void  __clsuter_view_thumbnail_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Move* ev = event_info;

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

    if (cluster_view_s.picked_page) {
        __cluster_view_allpage_drag_page(data);
    }
}

static Eina_Bool __cluster_view_thumbnail_long_press_time_cb(void *data)
{
    if (!cluster_mouse_info.pressed)
        return ECORE_CALLBACK_CANCEL;

    cluster_mouse_info.long_pressed = true;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    __cluster_view_allpage_pick_up_page(data);

    return ECORE_CALLBACK_CANCEL;
}

static void __cluster_view_allpage_drag_page(void *data)
{
    if (cluster_view_s.picked_page) {
        int move_x = cluster_mouse_info.move_x - cluster_mouse_info.offset_x;
        int move_y = cluster_mouse_info.move_y - cluster_mouse_info.offset_y;
        int new_index = INIT_VALUE;
        evas_object_move(cluster_view_s.picked_page->thumbnail_ly, move_x, move_y);
        new_index = __cluster_view_allpage_get_page_index(move_x, move_y);

        int move = 0;
        int start, end;
        if (new_index != cluster_view_s.picked_page->page_index && new_index != INIT_VALUE) {
            if (cluster_view_s.picked_page->page_index < new_index) {
                move = -1;
                start = cluster_view_s.picked_page->page_index;
                end = new_index;
            } else if (cluster_view_s.picked_page->page_index > new_index) {
                move = +1;
                end = cluster_view_s.picked_page->page_index;
                start = new_index;
            }

            Eina_List *find_list = NULL;
            cluster_page_t *page_item = NULL;
            EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
                if (page_item == cluster_view_s.picked_page) {
                    page_item->page_index = new_index;
                } else if (page_item->page_index >= start && page_item->page_index <= end) {
                    page_item->page_index += move;
                }
                elm_grid_unpack(cluster_view_s.allpage_grid, page_item->thumbnail_ly);
            }

            int x, y;
            EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
                if (page_item != cluster_view_s.picked_page) {
                    __cluster_view_allpage_get_page_pos(page_item->page_index, &x, &y);
                    elm_grid_pack(cluster_view_s.allpage_grid, page_item->thumbnail_ly, x, y, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
                }
            }
        }
    }
}

static void __cluster_view_allpage_pick_up_page(void *data)
{
    cluster_view_s.picked_page = (cluster_page_t *)data;
    elm_grid_unpack(cluster_view_s.allpage_grid, cluster_view_s.picked_page->thumbnail_ly);

    elm_object_signal_emit(cluster_view_s.picked_page->thumbnail_ly, SIGNAL_ALLPAGE_DELETE_BUTTON_HIDE, SIGNAL_SOURCE);
    elm_object_signal_emit(cluster_view_s.picked_page->thumbnail_ly, SIGNAL_ALLPAGE_DRAG_BG_SHOW, SIGNAL_SOURCE);

    int x, y;
    __cluster_view_allpage_get_page_pos(cluster_view_s.picked_page->page_index, &x, &y);
    cluster_mouse_info.offset_x = cluster_mouse_info.down_x - x;
    cluster_mouse_info.offset_y = cluster_mouse_info.down_y - y;
}

static void __cluster_view_allpage_drop_page(void *data)
{
    if (cluster_view_s.picked_page) {
        if (cluster_view_s.page_count > 1)
            elm_object_signal_emit(cluster_view_s.picked_page->thumbnail_ly, SIGNAL_ALLPAGE_DELETE_BUTTON_SHOW, SIGNAL_SOURCE);
        elm_object_signal_emit(cluster_view_s.picked_page->thumbnail_ly, SIGNAL_ALLPAGE_DRAG_BG_HIDE, SIGNAL_SOURCE);

        int x, y;
        __cluster_view_allpage_get_page_pos(cluster_view_s.picked_page->page_index, &x, &y);
        elm_grid_pack(cluster_view_s.allpage_grid, cluster_view_s.picked_page->thumbnail_ly, x, y, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
        cluster_view_s.picked_page = NULL;
    }
    cluster_view_s.page_list = eina_list_sort(cluster_view_s.page_list,
            eina_list_count(cluster_view_s.page_list), __cluster_view_page_sort_cb);

    Eina_List *find_list = NULL, *widget_list = NULL;
    cluster_page_t *page_item = NULL;
    widget_data_t *widget_item = NULL;
    EINA_LIST_FOREACH(cluster_view_s.page_list, find_list, page_item) {
        EINA_LIST_FOREACH(page_item->widget_list, widget_list, widget_item) {
            widget_item->page_idx = page_item->page_index;
            cluster_data_update(widget_item);
        }
    }
}

static void __clsuter_view_widget_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Down* ev = event_info;
    LOGD("DOWN: (%d,%d)", ev->output.x, ev->output.y);

    if (cluster_view_s.edit_animator) {
        LOGE("edit animator is running");
        return ;
    }

    cluster_mouse_info.pressed = true;
    cluster_mouse_info.pressed_obj = obj;

    cluster_mouse_info.down_x = cluster_mouse_info.move_x = ev->output.x;
    cluster_mouse_info.down_y = cluster_mouse_info.move_y = ev->output.y;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    cluster_mouse_info.long_press_timer = ecore_timer_add(LONG_PRESS_TIME,
            __cluster_view_widget_long_press_time_cb, data);
}

static void __clsuter_view_widget_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Up* ev = event_info;
    LOGD("UP: (%d,%d)", ev->output.x, ev->output.y);

    if (!cluster_mouse_info.pressed)
        return ;

    cluster_mouse_info.pressed = false;
    cluster_mouse_info.pressed_obj = NULL;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    cluster_mouse_info.up_x = ev->output.x;
    cluster_mouse_info.up_y = ev->output.y;

    if (!cluster_mouse_info.long_pressed)
        return ;

    cluster_mouse_info.long_pressed = false;

    elm_scroller_movement_block_set(cluster_view_s.scroller, ELM_SCROLLER_MOVEMENT_NO_BLOCK);

    if (cluster_view_s.picked_widget) {
        __cluster_view_edit_drop_widget(data);
    }
}

static void __clsuter_view_widget_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
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

static Eina_Bool __cluster_view_widget_long_press_time_cb(void *data)
{
    widget_data_t *widget = (widget_data_t *)data;
    if (!cluster_mouse_info.pressed)
        return ECORE_CALLBACK_CANCEL;

    cluster_mouse_info.long_pressed = true;

    elm_scroller_movement_block_set(cluster_view_s.scroller, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);

    __cluster_view_edit_pick_up_widget(data);

    widget_viewer_send_cancel_click_event(widget);

    return ECORE_CALLBACK_CANCEL;
}

static void __cluster_view_edit_pick_up_widget(void *data)
{
    int cx = -1, cy = -1;
    int gx = -1, gy = -1, gw = 0, gh = 0;
    Evas *e = NULL;
    cluster_view_s.picked_widget = (widget_data_t *)data;

    Evas_Object *widget_layout = cluster_view_s.picked_widget->widget_layout;

    e = evas_object_evas_get(widget_layout);

    evas_pointer_canvas_xy_get(e, &cx, &cy);
    evas_object_geometry_get(widget_layout, &gx, &gy, &gw, &gh);
    LOGD("cx, cy (%d, %d )", cx, cy);
    LOGD("gx, gy, gw, gh (%d, %d, %d, %d)", gx, gy, gw, gh);

    cluster_mouse_info.offset_x = cx - gx;
    cluster_mouse_info.offset_y = cy - gy;

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, cluster_view_s.picked_widget->page_idx);

    cluster_page_unset(page, cluster_view_s.picked_widget);

    if (cluster_view_s.view_state == VIEW_STATE_NORMAL) {
        evas_object_resize(widget_layout, gw * 0.9, gh * 0.9);

        cluster_mouse_info.offset_x *= 0.9;
        cluster_mouse_info.offset_y *= 0.9;

        cluster_view_set_state(VIEW_STATE_EDIT);
    }

    elm_object_signal_emit(widget_layout, SIGNAL_DELETE_BUTTON_HIDE_ANI, SIGNAL_SOURCE);
    elm_object_signal_emit(widget_layout, SIGNAL_CLUSTER_PICKUP_STATE, SIGNAL_SOURCE);

    evas_object_move(widget_layout, cluster_mouse_info.move_x - cluster_mouse_info.offset_x,
            cluster_mouse_info.move_y - cluster_mouse_info.offset_y);
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
    if (cluster_view_s.edit_mode_scroll_timer) {
        ecore_timer_del(cluster_view_s.edit_mode_scroll_timer);
        cluster_view_s.edit_mode_scroll_timer = NULL;
    }

    if (cluster_view_s.edit_animator) {
        ecore_animator_del(cluster_view_s.edit_animator);
        cluster_view_s.edit_animator = NULL;
    }

    int to_x, to_y;
    cluster_view_s.animation_from_x = cluster_mouse_info.move_x - cluster_mouse_info.offset_x;
    cluster_view_s.animation_from_y = cluster_mouse_info.move_y - cluster_mouse_info.offset_y;

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, cluster_view_s.current_page);
    cluster_page_get_highlight_xy(page, &to_x, &to_y);
    if (to_x == INIT_VALUE || to_y == INIT_VALUE) {
        cluster_page_check_empty_space_pos(page, cluster_view_s.picked_widget, &to_x, &to_y);
    }

    cluster_view_s.animation_to_x = to_x;
    cluster_view_s.animation_to_y = to_y;

    cluster_view_s.edit_animator = ecore_animator_timeline_add(HOME_ANIMATION_TIME, __cluster_view_edit_move_anim, NULL);
}
static Eina_Bool __cluster_view_edit_move_anim(void *data, double pos)
{
    evas_object_move(cluster_view_s.picked_widget->widget_layout,
            (cluster_view_s.animation_to_x - cluster_view_s.animation_from_x) * pos + cluster_view_s.animation_from_x,
            (cluster_view_s.animation_to_y - cluster_view_s.animation_from_y) * pos + cluster_view_s.animation_from_y);

    if (pos >= 1.0 - (1e-10)) {
        __cluster_view_edit_move_anim_done(data);

        cluster_view_s.edit_animator = NULL;
        return ECORE_CALLBACK_DONE;
    }

    return ECORE_CALLBACK_RENEW;
}

static void __cluster_view_edit_move_anim_done(void *data)
{
    Evas_Object *widget_layout = cluster_view_s.picked_widget->widget_layout;
    elm_object_signal_emit(widget_layout, SIGNAL_DELETE_BUTTON_SHOW_ANI, SIGNAL_SOURCE);
    elm_object_signal_emit(widget_layout, SIGNAL_CLUSTER_EDIT_STATE, SIGNAL_SOURCE);

    cluster_page_t *page = (cluster_page_t *)eina_list_nth(cluster_view_s.page_list, cluster_view_s.current_page);

    if (!cluster_page_drop_widget(page, cluster_view_s.picked_widget)) {
        cluster_view_s.current_page = cluster_view_s.picked_widget->page_idx;
        __cluster_view_scroll_to_page(cluster_view_s.current_page, true);

        cluster_view_add_widget(cluster_view_s.picked_widget, true);
    }

    cluster_view_s.picked_widget = NULL;
}

static void __clsuter_view_scroller_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Down* ev = event_info;

    if (cluster_view_s.view_state != VIEW_STATE_NORMAL)
        return ;

    if (cluster_view_s.edit_animator) {
        LOGE("edit animator is running");
        return ;
    }

    LOGD("DOWN: (%d,%d)", ev->output.x, ev->output.y);

    cluster_mouse_info.pressed = true;
    cluster_mouse_info.pressed_obj = obj;

    cluster_mouse_info.down_x = cluster_mouse_info.move_x = ev->output.x;
    cluster_mouse_info.down_y = cluster_mouse_info.move_y = ev->output.y;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    cluster_mouse_info.long_press_timer = ecore_timer_add(LONG_PRESS_TIME,
            __cluster_view_scroller_long_press_time_cb, obj);
}

static void __clsuter_view_scroller_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Up* ev = event_info;

    if (!cluster_mouse_info.pressed || cluster_mouse_info.pressed_obj != obj)
        return ;

    LOGD("UP: (%d,%d)", ev->output.x, ev->output.y);

    cluster_mouse_info.pressed = false;

    if (cluster_mouse_info.long_press_timer) {
        ecore_timer_del(cluster_mouse_info.long_press_timer);
        cluster_mouse_info.long_press_timer = NULL;
    }

    cluster_mouse_info.up_x = ev->output.x;
    cluster_mouse_info.up_y = ev->output.y;

    cluster_mouse_info.long_pressed = false;
}

static void __clsuter_view_scroller_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Move* ev = event_info;

    if (!cluster_mouse_info.pressed || cluster_mouse_info.pressed_obj != obj)
        return ;

    LOGD("MOVE: (%d,%d)", ev->cur.output.x, ev->cur.output.y);

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
}

static Eina_Bool __cluster_view_scroller_long_press_time_cb(void *data)
{
    if (!cluster_mouse_info.pressed || cluster_mouse_info.pressed_obj != data)
        return ECORE_CALLBACK_CANCEL;

    cluster_mouse_info.long_pressed = true;

    cluster_view_set_state(VIEW_STATE_EDIT);

    return ECORE_CALLBACK_CANCEL;
}
static void __cluster_view_scroll_anim_start_cb(void *data, Evas_Object *obj, void *event_info)
{
    cluster_view_s.is_srolling = true;
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
    __cluster_view_scroll_to_page(next_page_idx, true);
    return ECORE_CALLBACK_RENEW;
}
