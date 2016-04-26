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

#include "apps_view.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"
#include "page_indicator.h"
#include "homescreen-efl.h"
#include "apps_data.h"
#include "menu.h"

#define FOLDER_STR_LEN 256

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
    view_state_t view_state;
} apps_view_s = {
    .scroller = NULL,
    .bg = NULL,
    .box = NULL,
    .width = 0,
    .height = 0,
    .indicator = NULL,
    .page_list = NULL,
    .page_count = 0,
    .current_page = 0,
    .view_state = VIEW_STATE_NOMAL
};

static int apps_menu_list[2] = {
        MENU_APPS_EDIT,
        MENU_APPS_CREATE_FOLDER
};

static Eina_Hash *apps_menu_table = NULL;

static void __apps_view_create_base_gui(Evas_Object *win);
static Evas_Object * __apps_view_add_page(void);
static void __apps_view_remove_page(void);
static void __apps_view_fill_apps(void);
static void __apps_view_icon_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __apps_view_create_menu(void);
static void __apps_view_menu_edit_cb(void *data, Evas_Object *obj, void *event_info);
static void __apps_view_menu_create_folder_cb(void *data, Evas_Object *obj, void *event_info);
static void __apps_view_update_folder_icon(app_data_t* item);

Evas_Object *apps_view_create(Evas_Object *win)
{
    elm_win_screen_size_get(win, NULL, NULL, &apps_view_s.width, &apps_view_s.height);

    apps_data_init();
    __apps_view_create_base_gui(win);
    __apps_view_fill_apps();
    __apps_view_create_menu();

    if (!apps_view_s.scroller) {
        LOGE("[FAILED][apps_view_s.scroller==NULL]");
        return NULL;
    }

    return apps_view_s.scroller;
}

void apps_view_app_terminate(void)
{
    eina_hash_free(apps_menu_table);
}

void apps_view_show(void)
{
    page_indicator_show(apps_view_s.indicator);
}

void apps_view_hide(void)
{
    page_indicator_hide(apps_view_s.indicator);
}

void apps_view_show_anim(double pos)
{
    Evas_Object *edje = NULL;
    edje = elm_layout_edje_get(apps_view_s.bg);
    if (!edje) {
        LOGE("Failed to get edje from layout");
        return;
    }
    Edje_Message_Float_Set *msg = malloc(sizeof(*msg) + 2 * sizeof(double));
    msg->count = 3;
    msg->val[0] = ((APPS_VIEW_ANIMATION_DELTA * (1-pos)) / apps_view_s.height);
    msg->val[1] = 1+((APPS_VIEW_ANIMATION_DELTA * (1-pos)) / apps_view_s.height);
    msg->val[2] = pos*APPS_VIEW_BG_OPACITY;

    edje_object_message_send(edje, EDJE_MESSAGE_FLOAT_SET, 1, msg);
    edje_object_signal_emit(edje, SIGNAL_APPS_VIEW_ANIM, SIGNAL_SOURCE);

    evas_object_color_set(apps_view_s.box, 255, 255, 255, pos*255);
    evas_object_move(apps_view_s.scroller, 0, APPS_VIEW_PADDING_TOP + (APPS_VIEW_ANIMATION_DELTA * (1-pos)));
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(apps_view_s.box, 255, 255, 255, 255);
        evas_object_move(apps_view_s.scroller, 0, APPS_VIEW_PADDING_TOP);
        edje_object_signal_emit(edje, SIGNAL_APPS_VIEW_SHOW, SIGNAL_SOURCE);
    }
}

void apps_view_hide_anim(double pos)
{
    Evas_Object *edje = NULL;
    edje = elm_layout_edje_get(apps_view_s.bg);
    if (!edje) {
        LOGE("Failed to get edje from layout");
        return;
    }
    Edje_Message_Float_Set *msg = malloc(sizeof(*msg) + 2 * sizeof(double));
    msg->count = 3;
    msg->val[0] = ((APPS_VIEW_ANIMATION_DELTA * pos) / apps_view_s.height);
    msg->val[1] = 1+((APPS_VIEW_ANIMATION_DELTA * pos) / apps_view_s.height);
    msg->val[2] = (1-pos)*APPS_VIEW_BG_OPACITY;

    edje_object_message_send(edje, EDJE_MESSAGE_FLOAT_SET, 1, msg);
    edje_object_signal_emit(edje, SIGNAL_APPS_VIEW_ANIM, SIGNAL_SOURCE);

    evas_object_color_set(apps_view_s.box, 255, 255, 255, (1-pos)*255);
    evas_object_move(apps_view_s.scroller, 0, APPS_VIEW_PADDING_TOP + (APPS_VIEW_ANIMATION_DELTA * pos));
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(apps_view_s.box, 255, 255, 255, 0);
        evas_object_move(apps_view_s.scroller, 0, apps_view_s.height);
        edje_object_signal_emit(edje, SIGNAL_APPS_VIEW_HIDE, SIGNAL_SOURCE);
    }
}

void apps_view_reroder(void)
{
    Eina_List *data_list = apps_data_get_list();
    app_data_t *item = NULL;
    Eina_List *find_list;
    int item_count = 0;

    EINA_LIST_FOREACH(data_list, find_list, item) {
        if (apps_view_icon_set(item))
            item_count++;
    }
    //Delete empty page
    while (((item_count-1) / (APPS_VIEW_COL*APPS_VIEW_ROW) +1) < eina_list_count(apps_view_s.page_list)) {
        __apps_view_remove_page();
    }
}

Evas_Object* apps_view_create_icon(app_data_t *item)
{
    Evas_Object *icon_layout;
    Evas_Object *rect = NULL;
    Evas_Object *icon_image = NULL;

    icon_layout = elm_layout_add(apps_view_s.box);
    elm_layout_file_set(icon_layout, util_get_res_file_path(EDJE_DIR"/app_icon.edj"), GROUP_APP_ICON_LY);
    evas_object_size_hint_weight_set(icon_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    rect = evas_object_rectangle_add(evas_object_evas_get(icon_layout));
    if (!rect) {
        LOGE("[FAILED][rect=NULL]");
        return NULL;
    }
    evas_object_color_set(rect, 0, 0, 0, 0);
    evas_object_size_hint_min_set(rect, APPS_VIEW_W / APPS_VIEW_COL, APPS_VIEW_H / APPS_VIEW_ROW);
    evas_object_size_hint_max_set(rect, APPS_VIEW_W / APPS_VIEW_COL, APPS_VIEW_H / APPS_VIEW_ROW);
    elm_object_part_content_set(icon_layout, SIZE_SETTER, rect);

    item->app_layout = icon_layout;

    //icon image
    if (item->is_folder) {
        LOGD("Create Folder ICON");
        Evas_Object *folder_layout;
        Evas_Object *size_setter;

        folder_layout = elm_layout_add(icon_layout);
        elm_layout_file_set(folder_layout, util_get_res_file_path(EDJE_DIR"/apps_folder_icon.edj"), GROUP_FOLDER_ICON_LY);
        evas_object_size_hint_weight_set(folder_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

        size_setter = evas_object_rectangle_add(evas_object_evas_get(folder_layout));
        if (!rect) {
            LOGE("[FAILED][rect=NULL]");
            return NULL;
        }
        evas_object_color_set(size_setter, 0, 0, 0, 0);
        evas_object_size_hint_min_set(size_setter, APPS_VIEW_ICON_IMAGE, APPS_VIEW_ICON_IMAGE);
        evas_object_size_hint_max_set(size_setter, APPS_VIEW_ICON_IMAGE, APPS_VIEW_ICON_IMAGE);
        elm_object_part_content_set(folder_layout, SIZE_SETTER, size_setter);
        item->folder_layout = folder_layout;

        __apps_view_update_folder_icon(item);
        elm_object_part_content_set(icon_layout, PART_ICON_CONTENT, folder_layout);

    } else {
        if (ecore_file_can_read(item->icon_path_str)) {
            LOGD("Create Image: %s", item->icon_path_str);
            icon_image = elm_image_add(icon_layout);
            elm_image_file_set(icon_image, item->icon_path_str, NULL);
            evas_object_size_hint_min_set(icon_image, APPS_VIEW_ICON_IMAGE, APPS_VIEW_ICON_IMAGE);
            evas_object_size_hint_max_set(icon_image, APPS_VIEW_ICON_IMAGE, APPS_VIEW_ICON_IMAGE);
            elm_object_part_content_set(icon_layout, PART_ICON_CONTENT, icon_image);
        } else {
            LOGE("Can not read : %s", item->icon_path_str);
        }
    }
    elm_object_signal_callback_add(icon_layout, SIGNAL_ICON_CLICKED, SIGNAL_SOURCE, __apps_view_icon_clicked_cb, (void *)item);
    /*
    elm_object_signal_callback_add(icon_layout, SIGNAL_DEFAULT_TOUCH_DOWN , PART_ICON_CONTENT_TOUCH, _all_apps_icon_touch_down_cb, (void *)item);
    elm_object_signal_callback_add(icon_layout, SIGNAL_ICON_CLICKED, SIGNAL_SOURCE, _all_apps_icon_clicked_cb, (void *)item);
    elm_object_signal_callback_add(icon_layout, SIGNAL_UNINSTALL_BUTTON_CLICKED, SIGNAL_SOURCE, _all_apps_icon_uninstall_button_clicked_cb, (void *)item);
    elm_object_signal_callback_add(icon_layout, SIGNAL_CHANGED_CHECK_BOX, SIGNAL_SOURCE, _all_apps_icon_checkbox_changed,  (void *)item);
*/
    //icon label
    elm_object_part_text_set(icon_layout, PART_ICON_NAME, item->label_str);

    evas_object_show(icon_image);
    evas_object_show(icon_layout);
    LOGD("icon [%s] create", item->pkg_str);

    return icon_layout;
}

void apps_view_icon_unset(app_data_t *item)
{
    Evas_Object *page = NULL;
    int page_index;
    int col, row;
    char icon_container[255];

    if (item == NULL)
        return;

    page_index = item->position / (APPS_VIEW_COL*APPS_VIEW_ROW);
    col = (item->position / APPS_VIEW_COL)%APPS_VIEW_ROW;
    row = item->position % APPS_VIEW_COL;

    page = eina_list_nth(apps_view_s.page_list, page_index);

    sprintf(icon_container, "icon_%d_%d", col, row);
    LOGD("[%s] -> [%s], [%p]", item->pkg_str, icon_container, item->app_layout);
    elm_object_part_content_unset(page, icon_container);
}

bool apps_view_icon_set(app_data_t *item)
{
    Evas_Object *page = NULL;
    int page_index;
    int col, row;
    char icon_container[255];

    if (item == NULL)
        return false;

    LOGD("%s %d %d", item->pkg_str, item->parent_db_id, item->position);
    if (item->parent_db_id == APPS_ROOT) {
        page_index = item->position / (APPS_VIEW_COL*APPS_VIEW_ROW);
        col = (item->position / APPS_VIEW_COL)%APPS_VIEW_ROW;
        row = item->position % APPS_VIEW_COL;

        if (eina_list_count(apps_view_s.page_list) < page_index+1) {
            page = __apps_view_add_page();
        } else
            page = eina_list_nth(apps_view_s.page_list, page_index);

        sprintf(icon_container, "icon_%d_%d", col, row);
        elm_object_part_content_unset(page, icon_container);
        elm_object_part_content_set(page, icon_container, item->app_layout);
        LOGD("[%s] -> [%s], [%p]", item->pkg_str, icon_container, item->app_layout);
        return true;
    } else
        return false;
}

static void __apps_view_create_base_gui(Evas_Object *win)
{
    char edj_path[PATH_MAX] = {0, };
    apps_view_s.bg = elm_layout_add(win);
    if (!apps_view_s.bg) {
        LOGE("[FAILED][apps_view_s.bg==NULL]");
        return;
    }
    snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/apps_view_bg.edj"));
    elm_layout_file_set(apps_view_s.bg, edj_path, GROUP_APPS_BG_LY);
    evas_object_size_hint_weight_set(apps_view_s.bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, apps_view_s.bg);
    evas_object_show(apps_view_s.bg);

    Evas_Object *rect = evas_object_rectangle_add(homescreen_efl_get_win());
    evas_object_color_set(rect, 0, 0, 0, 0);
    evas_object_size_hint_min_set(rect, apps_view_s.width, apps_view_s.height);
    evas_object_size_hint_max_set(rect, apps_view_s.width, apps_view_s.height);
    evas_object_resize(rect, apps_view_s.width, apps_view_s.height);
    elm_object_part_content_set(apps_view_s.bg, SIZE_SETTER, rect);
    evas_object_show(rect);

    apps_view_s.scroller = elm_scroller_add(win);
    if (!apps_view_s.scroller) {
        LOGE("[FAILED][apps_view_s.scroller==NULL]");
        return;
    }
    elm_scroller_content_min_limit(apps_view_s.scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_bounce_set(apps_view_s.scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(apps_view_s.scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_scroll_limit_set(apps_view_s.scroller, 1, 1);
    evas_object_size_hint_weight_set(apps_view_s.scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_scroller_loop_set(apps_view_s.scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_page_size_set(apps_view_s.scroller, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_resize(apps_view_s.scroller, APPS_VIEW_W , APPS_VIEW_H);

    apps_view_s.box = elm_box_add(apps_view_s.scroller);
    elm_box_horizontal_set(apps_view_s.box, EINA_TRUE);
    elm_box_align_set(apps_view_s.box, 0.5, 0.5);
    evas_object_show(apps_view_s.box);

    elm_object_content_set(apps_view_s.scroller, apps_view_s.box);

    apps_view_s.indicator = page_indictor_create(apps_view_s.scroller);
    page_indicator_scroller_resize(apps_view_s.indicator, apps_view_s.width, apps_view_s.height);
    page_indicator_hide(apps_view_s.indicator);

    __apps_view_add_page();
    page_indicator_set_current_page(apps_view_s.indicator, 0);
}

static Evas_Object *__apps_view_add_page(void)
{
    char edj_path[PATH_MAX] = {0, };
    Evas_Object *page_ly = elm_layout_add(apps_view_s.box);

    snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/apps_view.edj"));
    elm_layout_file_set(page_ly, edj_path, GROUP_APPS_LY);
    evas_object_size_hint_weight_set(page_ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_resize(page_ly, APPS_VIEW_W, APPS_VIEW_H);

    Evas_Object *rect = evas_object_rectangle_add(homescreen_efl_get_win());
    evas_object_color_set(rect, 255, 255, 255, 0);
    evas_object_size_hint_min_set(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_size_hint_max_set(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_resize(rect, APPS_VIEW_W, APPS_VIEW_H);
    evas_object_show(rect);
    elm_object_part_content_set(page_ly, SIZE_SETTER, rect);

    elm_box_pack_end(apps_view_s.box, page_ly);
    evas_object_show(page_ly);

    apps_view_s.page_list = eina_list_append(apps_view_s.page_list, page_ly);
    apps_view_s.page_count += 1;
    page_indicator_set_page_count(apps_view_s.indicator, apps_view_s.page_count);
    return page_ly;
}

static void __apps_view_remove_page(void)
{
    if (apps_view_s.page_count == 0) {
        apps_view_s.page_count = 1;
        return ;
    }

    Evas_Object *item = eina_list_nth(apps_view_s.page_list, apps_view_s.page_count-1);
    apps_view_s.page_list = eina_list_remove(apps_view_s.page_list, item);
    apps_view_s.page_count -= 1;
    if (apps_view_s.current_page >= apps_view_s.page_count) {
        apps_view_s.current_page = apps_view_s.page_count - 1;
    }

    elm_scroller_page_bring_in(apps_view_s.scroller, apps_view_s.current_page, 0);
    elm_box_unpack(apps_view_s.box, item);
    page_indicator_set_page_count(apps_view_s.indicator, apps_view_s.page_count);
    evas_object_del(item);
}

static void __apps_view_fill_apps(void)
{
    Eina_List *list = apps_data_get_list();
    app_data_t *item = NULL;
    Eina_List *find_list;

    EINA_LIST_FOREACH(list, find_list, item) {
        if (item != NULL && item->parent_db_id == APPS_ROOT) {
            item->app_layout = apps_view_create_icon(item);
        }
    }
    apps_view_reroder();
}

static void __apps_view_icon_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    app_data_t *item  = (app_data_t *)data;
    app_control_h app_control_handle = NULL;
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);

    if (item->is_folder) {
        LOGD("%d(%s) folder clicked", item->db_id, item->label_str);
    } else {
        LOGD("%s(%s) applciation clicked", item->pkg_str, item->owner);
        if (app_control_create(&app_control_handle) != APP_CONTROL_ERROR_NONE) {
            LOGE("[FAILED][app_control_create]");
            return;
        }
        if (app_control_set_app_id(app_control_handle, item->pkg_str) != APP_CONTROL_ERROR_NONE) {
            LOGE("[FAILED][app_control_set_app_id]");
            app_control_destroy(app_control_handle);
            return;
        }
        if (app_control_send_launch_request(app_control_handle, NULL, NULL) != APP_CONTROL_ERROR_NONE) {
            LOGE("[FAILED][app_control_send_launch_request]");
            app_control_destroy(app_control_handle);
            return;
        }
        app_control_destroy(app_control_handle);
    }
}

void apps_view_hw_key_menu(void)
{
    menu_change_state_on_hw_menu_key(apps_menu_table);
}

view_state_t apps_view_get_state(void)
{
    return apps_view_s.view_state;
}

void apps_view_set_state(view_state_t state)
{
    if (state == VIEW_STATE_EDIT) {
        Evas_Object *edje = NULL;
        edje = elm_layout_edje_get(apps_view_s.bg);
        if (!edje) {
            LOGE("Failed to get edje from layout");
            return;
        }
        edje_object_signal_emit(edje, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);

        Eina_List *find_list = NULL;
        Evas_Object *page_ly;
        EINA_LIST_FOREACH(apps_view_s.page_list, find_list, page_ly) {
            elm_object_signal_emit(page_ly, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);
        }

        Eina_List *data_list = apps_data_get_list();
        app_data_t *item = NULL;
        EINA_LIST_FOREACH(data_list, find_list, item) {
            if (item->app_layout)
                elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);
        }

    } else if (state == VIEW_STATE_NOMAL) {
        Evas_Object *edje = NULL;
        edje = elm_layout_edje_get(apps_view_s.bg);
        if (!edje) {
            LOGE("Failed to get edje from layout");
            return;
        }
        edje_object_signal_emit(edje, SIGNAL_EDIT_MODE_OFF, SIGNAL_SOURCE);

        Eina_List *find_list = NULL;
        Evas_Object *page_ly;
        EINA_LIST_FOREACH(apps_view_s.page_list, find_list, page_ly) {
            elm_object_signal_emit(page_ly, SIGNAL_EDIT_MODE_OFF, SIGNAL_SOURCE);
        }
        Eina_List *data_list = apps_data_get_list();
        app_data_t *item = NULL;
        EINA_LIST_FOREACH(data_list, find_list, item) {
            if (item->app_layout)
                elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_OFF, SIGNAL_SOURCE);
        }
    }

    apps_view_s.view_state = state;
}

static void __apps_view_update_folder_icon(app_data_t* item)
{
    Eina_List *folder_list = NULL;
    Eina_List *find_list;
    app_data_t *temp_item;
    char folder_item_count_string[FOLDER_STR_LEN];
    Evas_Object *icon_image = NULL;

    int item_count = 0;
    int i;
    apps_data_get_folder_item_list(&folder_list, item);
    item_count = eina_list_count(folder_list);
/*
#define EINA_LIST_FOREACH(list, l, data) \
  for (l = list,                         \
       data = eina_list_data_get(l);     \
       l;                                \
       l = eina_list_next(l),            \
       data = eina_list_data_get(l))
*/
    for (i=0, find_list = folder_list, temp_item = eina_list_data_get(find_list);
            i<4 && find_list;
            i++, find_list = eina_list_next(find_list), temp_item = eina_list_data_get(find_list)) {
        sprintf(folder_item_count_string, "folder_icon_%d", i);
        icon_image = elm_image_add(item->folder_layout);
        elm_image_file_set(icon_image, temp_item->icon_path_str, NULL);
        evas_object_size_hint_weight_set(icon_image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_object_part_content_set(item->folder_layout, folder_item_count_string, icon_image);
        evas_object_show(icon_image);
    }
    sprintf(folder_item_count_string, "set_item_count_%d", item_count);
    elm_object_signal_emit(item->folder_layout, folder_item_count_string, SIGNAL_SOURCE);
    evas_object_show(item->folder_layout);
    eina_list_free(folder_list);
}

static void __apps_view_create_menu(void)
{
    apps_menu_table = eina_hash_int32_new(NULL);
    eina_hash_add(apps_menu_table, &apps_menu_list[0], __apps_view_menu_edit_cb);
    eina_hash_add(apps_menu_table, &apps_menu_list[1], __apps_view_menu_create_folder_cb);
}

static void __apps_view_menu_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
    menu_hide();

    apps_view_set_state(VIEW_STATE_EDIT);
}

static void __apps_view_menu_create_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
    menu_hide();
}

void apps_view_icon_add(app_data_t *item)
{
    if (item->parent_db_id == APPS_ROOT)
        apps_view_create_icon(item);
    else
        LOGE("Parent is NOT APPS_ROOT");
}
