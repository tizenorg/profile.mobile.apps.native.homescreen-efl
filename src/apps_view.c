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
#include <package_manager.h>
#include <badge.h>
#include <efl_extension.h>

#include "apps_view.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"
#include "page_indicator.h"
#include "homescreen-efl.h"
#include "apps_data.h"
#include "apps_db.h"
#include "menu.h"

#define FOLDER_STR_LEN 256
#define MAX_BADGE_COUNT_STR_LEN 5
#define MAX_BADGE_DISPLAY_COUNT 999

static struct {
    Evas_Object *scroller;
    Evas_Object *bg;
    Evas_Object *box;
    Evas_Object *chooser_btn;
    Evas_Object *folder_popup_ly;
    Evas_Object *folder_title_entry;
    int width;
    int height;
    page_indicator_t *indicator;
    Eina_List *page_list;
    int page_count;
    int current_page;
    view_state_t view_state;
    view_state_t pre_view_state;
    Eina_List *selected_items;
    app_data_t *dest_folder;
    app_data_t *opened_folder;
    int selected_item_count;
    Ecore_Animator *animator;
} apps_view_s = {
    .scroller = NULL,
    .bg = NULL,
    .box = NULL,
    .chooser_btn = NULL,
    .folder_popup_ly = NULL,
    .folder_title_entry = NULL,
    .width = 0,
    .height = 0,
    .indicator = NULL,
    .page_list = NULL,
    .page_count = 0,
    .current_page = 0,
    .view_state = VIEW_STATE_NORMAL,
    .pre_view_state = VIEW_STATE_UNKNOWN,
    .selected_items = NULL,
    .selected_item_count = 0,
    .dest_folder = NULL,
    .opened_folder = NULL,
    .animator = NULL
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
static void __apps_view_icon_uninstall_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __apps_view_icon_check_changed_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __apps_view_create_chooser(void);
static void __apps_view_chooser_left_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __apps_view_chooser_right_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __apps_view_update_chooser_text(int item_count);
static void __apps_view_create_menu(void);
static void __apps_view_menu_edit_cb(void *data, Evas_Object *obj, void *event_info);
static void __apps_view_menu_create_folder_cb(void *data, Evas_Object *obj, void *event_info);
static void __apps_view_open_folder_popup(app_data_t *item);
static void __apps_view_close_folder_popup(app_data_t *item);
static void __apps_view_close_folder_popup_done(void);
static Eina_Bool __apps_view_show_folder_anim(void *data, double pos);
static Eina_Bool __apps_view_hide_folder_anim(void *data, double pos);
static void __apps_view_hide_folder_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __apps_view_badge_update_cb(unsigned int action, const char *app_id, unsigned int count, void *user_data);
static void __apps_view_badge_update_icon(app_data_t *item);
static void __apps_view_badge_update_count(app_data_t *item);
static void __apps_view_plus_icon_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __apps_view_create_toast_popup(char *str);
static void __apps_view_toast_popup_timeout_cb(void *data, Evas_Object *obj, void *event_info);
static void __apps_view_toast_popup_block_clicked_cb(void *data, Evas_Object *obj, void *event_info);

Evas_Object *apps_view_create(Evas_Object *win)
{
    int ret = BADGE_ERROR_NONE;
    elm_win_screen_size_get(win, NULL, NULL, &apps_view_s.width, &apps_view_s.height);

    apps_data_init();
    __apps_view_create_base_gui(win);
    __apps_view_fill_apps();
    __apps_view_create_chooser();
    __apps_view_create_menu();

    ret = badge_register_changed_cb(__apps_view_badge_update_cb, NULL);

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

    apps_view_unset_all();

    EINA_LIST_FOREACH(data_list, find_list, item) {
        if (apps_view_icon_set(item))
            item_count++;
    }
    //Delete empty page
    while (((item_count-1) / (APPS_VIEW_COL*APPS_VIEW_ROW) +1) < eina_list_count(apps_view_s.page_list)) {
        __apps_view_remove_page();
    }
}

void apps_view_folder_reroder(void)
{
    Eina_List *data_list = apps_data_get_list();
    app_data_t *item = NULL;
    Eina_List *find_list;
    int item_count = 0;
    if (!apps_view_s.opened_folder)
        return;

    EINA_LIST_FOREACH(data_list, find_list, item) {
        if (item->parent_db_id == apps_view_s.opened_folder->db_id) {
            apps_view_icon_set(item);
            item_count++;
        }
    }

    if (item_count < APPS_FOLDER_MAX_ITEM) {
        Edje_Message_Float_Set *msg = malloc(sizeof(*msg) + sizeof(double));
        msg->count = 2;
        msg->val[0] = item_count / APPS_FOLDER_COL;
        msg->val[1] = item_count % APPS_FOLDER_COL;

        if (apps_view_get_state() != VIEW_STATE_CHOOSER) {
            Evas_Object *edje = NULL;
            edje = elm_layout_edje_get(apps_view_s.folder_popup_ly);
            edje_object_message_send(edje, EDJE_MESSAGE_FLOAT_SET, 1, msg);
            edje_object_signal_emit(edje, SIGNAL_APPS_FOLDER_SET_PLUS_ICON, SIGNAL_SOURCE);
            elm_object_signal_callback_add(apps_view_s.folder_popup_ly, SIGNAL_APPS_FOLDER_CLICKED_PLUS_ICON, SIGNAL_SOURCE,
                __apps_view_plus_icon_clicked, (void *)apps_view_s.opened_folder);
        } else {
            elm_object_signal_emit(apps_view_s.folder_popup_ly, SIGNAL_APPS_FOLDER_HIDE_PLUS_ICON, SIGNAL_SOURCE);
        }
        free(msg);
    } else {
        elm_object_signal_emit(apps_view_s.folder_popup_ly, SIGNAL_APPS_FOLDER_HIDE_PLUS_ICON, SIGNAL_SOURCE);
        elm_object_signal_callback_del(apps_view_s.folder_popup_ly, SIGNAL_APPS_FOLDER_CLICKED_PLUS_ICON, SIGNAL_SOURCE,
                __apps_view_plus_icon_clicked);
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

        apps_view_update_folder_icon(item);
        elm_object_part_content_set(icon_layout, APPS_ICON_CONTENT, folder_layout);

        if (apps_view_s.view_state == VIEW_STATE_EDIT) {
            elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_ON_ANI, SIGNAL_SOURCE);
            elm_object_signal_emit(item->app_layout, SIGNAL_UNINSTALL_BUTTON_SHOW_ANI, SIGNAL_SOURCE);
        } else if (apps_view_s.view_state == VIEW_STATE_CHOOSER) {
            elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_ON_ANI, SIGNAL_SOURCE);
        }
    } else {
        if (ecore_file_can_read(item->icon_path_str)) {
            LOGD("Create Image: %s", item->icon_path_str);
            icon_image = elm_image_add(icon_layout);
            elm_image_file_set(icon_image, item->icon_path_str, NULL);
            evas_object_size_hint_min_set(icon_image, APPS_VIEW_ICON_IMAGE, APPS_VIEW_ICON_IMAGE);
            evas_object_size_hint_max_set(icon_image, APPS_VIEW_ICON_IMAGE, APPS_VIEW_ICON_IMAGE);
            elm_object_part_content_set(icon_layout, APPS_ICON_CONTENT, icon_image);
        } else {
            LOGE("Can not read : %s", item->icon_path_str);
        }

        if (item->parent_db_id != APPS_ROOT)
            elm_object_signal_emit(item->app_layout, SIGNAL_SET_BLACK_TEXT, SIGNAL_SOURCE);

        if (apps_view_s.view_state == VIEW_STATE_EDIT) {
            if (item->parent_db_id == APPS_ROOT)
                elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);

            if (item->is_removable)
                elm_object_signal_emit(item->app_layout, SIGNAL_UNINSTALL_BUTTON_SHOW, SIGNAL_SOURCE);
        } else if (apps_view_s.view_state == VIEW_STATE_CHOOSER) {
            if (item->parent_db_id == APPS_ROOT)
                elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);

            elm_object_signal_emit(item->app_layout, SIGNAL_CHOOSER_MODE_ON, SIGNAL_SOURCE);
        }
        __apps_view_badge_update_icon(item);
    }
    elm_object_signal_callback_add(icon_layout, SIGNAL_ICON_CLICKED, SIGNAL_SOURCE, __apps_view_icon_clicked_cb, (void *)item);
    elm_object_signal_callback_add(icon_layout, SIGNAL_UNINSTALL_BUTTON_CLICKED, SIGNAL_SOURCE, __apps_view_icon_uninstall_btn_clicked_cb, (void *)item);
    elm_object_signal_callback_add(icon_layout, SIGNAL_CHANGED_CHECK_BOX, SIGNAL_SOURCE, __apps_view_icon_check_changed_cb,  (void *)item);

    /*
    elm_object_signal_callback_add(icon_layout, SIGNAL_DEFAULT_TOUCH_DOWN , APPS_ICON_CONTENT_TOUCH, _all_apps_icon_touch_down_cb, (void *)item);
    elm_object_signal_callback_add(icon_layout, SIGNAL_ICON_CLICKED, SIGNAL_SOURCE, _all_apps_icon_clicked_cb, (void *)item);
    elm_object_signal_callback_add(icon_layout, SIGNAL_UNINSTALL_BUTTON_CLICKED, SIGNAL_SOURCE, _all_apps_icon_uninstall_button_clicked_cb, (void *)item);
    elm_object_signal_callback_add(icon_layout, SIGNAL_CHANGED_CHECK_BOX, SIGNAL_SOURCE, _all_apps_icon_checkbox_changed,  (void *)item);
*/
    //icon label
    elm_object_part_text_set(icon_layout, APPS_ICON_NAME, item->label_str);

    evas_object_show(icon_image);
    evas_object_show(icon_layout);

    LOGD("icon [%s] create", item->pkg_str);

    return icon_layout;
}

void apps_view_unset_all(void)
{
    int i, j;
    char icon_container[FOLDER_STR_LEN];
    Eina_List *find_list = NULL;
    Evas_Object * page;

    EINA_LIST_FOREACH(apps_view_s.page_list, find_list, page) {
        for (i = 0; i < APPS_VIEW_ROW; i++)
            for (j = 0; j < APPS_VIEW_COL; j++) {
                sprintf(icon_container, "icon_%d_%d", i, j);
                elm_object_part_content_unset(page, icon_container);
            }
    }
}

bool apps_view_icon_set(app_data_t *item)
{
    Evas_Object *page = NULL;
    int page_index;
    int col, row;
    char icon_container[FOLDER_STR_LEN];

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
        if (elm_object_part_content_get(page, icon_container) != NULL) {
            LOGE("unset %p", elm_object_part_content_get(page, icon_container));
            elm_object_part_content_unset(page, icon_container);
        }
        elm_object_part_content_set(page, icon_container, item->app_layout);
        LOGD("[%s] -> [%s], [%p] page : %d", item->pkg_str, icon_container, item->app_layout, page_index);
        return true;
    } else if (apps_view_s.opened_folder && item->parent_db_id == apps_view_s.opened_folder->db_id) {
        col = (item->position / APPS_FOLDER_COL)%APPS_FOLDER_ROW;
        row = item->position % APPS_FOLDER_COL;

        sprintf(icon_container, "icon_%d_%d", col, row);
        if (elm_object_part_content_get(apps_view_s.folder_popup_ly, icon_container) != NULL) {
            LOGE("unset %p", elm_object_part_content_get(apps_view_s.folder_popup_ly, icon_container));
            elm_object_part_content_unset(apps_view_s.folder_popup_ly, icon_container);
        }
        elm_object_part_content_set(apps_view_s.folder_popup_ly, icon_container, item->app_layout);
        LOGD("[%s] -> [%s], [%p]", item->pkg_str, icon_container, item->app_layout);
    }

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

static void __apps_view_folder_fill_apps(void)
{
    Eina_List *list = apps_data_get_list();
    app_data_t *item = NULL;
    Eina_List *find_list;

    EINA_LIST_FOREACH(list, find_list, item) {
        if (item != NULL && item->parent_db_id == apps_view_s.opened_folder->db_id) {
            item->app_layout = apps_view_create_icon(item);
        }
    }
    apps_view_folder_reroder();
}

static void __apps_view_icon_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    app_data_t *item  = (app_data_t *)data;
    app_control_h app_control_handle = NULL;
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);

    if (item->is_folder) {
        LOGD("%d(%s) folder clicked", item->db_id, item->label_str);
        __apps_view_open_folder_popup(item);
    } else if (apps_view_s.view_state == VIEW_STATE_NORMAL) {
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

static void __apps_view_icon_uninstall_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    app_data_t *item  = (app_data_t *)data;
    package_manager_request_h request;
    int id = 0;
    feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
    if (!item) {
        LOGE("item is NULL");

    }

    LOGD("Uninstall :: %s", item->pkg_str);
    if (item->is_folder) {
        //folder delete
        apps_data_delete_folder(item);
    } else {
        if (package_manager_request_create(&request) != PACKAGE_MANAGER_ERROR_NONE) {
            LOGE("Could not create unistall request. App: %s", item->pkg_str);
            return;
        }
        int ret = package_manager_request_set_mode(request, PACKAGE_MANAGER_REQUEST_MODE_DEFAULT);
        if (ret != PACKAGE_MANAGER_ERROR_NONE) {
            LOGE("Could not set request mode. App: %s", item->pkg_str);
            return;
        }
        if (package_manager_request_uninstall(request, item->pkg_str, &id) != PACKAGE_MANAGER_ERROR_NONE) {
            LOGE("Could not uninstall application. App: %s", item->pkg_str);
            return;
        }
        if (package_manager_request_destroy(request) != PACKAGE_MANAGER_ERROR_NONE) {
            LOGE("Could not destroy unistall request. App: %s", item->pkg_str);
            return;
        }
    }
}
static void __apps_view_icon_check_changed_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    app_data_t *item  = (app_data_t *)data;

    if (apps_view_s.view_state != VIEW_STATE_CHOOSER)
        return;

    if (item->is_checked) {
        LOGD("%s - uncheck", item->label_str);
        elm_object_signal_emit(item->app_layout, SIGNAL_UNCHECK_CHECK_BOX, SIGNAL_SOURCE);
        apps_view_s.selected_items = eina_list_remove(apps_view_s.selected_items, item);
        item->is_checked = !item->is_checked;
    } else {
        int item_count = eina_list_count(apps_view_s.selected_items);
        if (apps_view_s.selected_item_count + item_count >= APPS_FOLDER_MAX_ITEM) {
            LOGD("checked item : %d > HOME_FOLDER_MAX_ITEM(9)", item_count);
            char str[1024];
            sprintf(str, _("IDS_HS_TPOP_MAXIMUM_NUMBER_OF_APPLICATIONS_IN_FOLDER_HPD_REACHED"), APPS_FOLDER_MAX_ITEM);
            __apps_view_create_toast_popup(str);
        } else {
            LOGD("%s - check", item->label_str);
            elm_object_signal_emit(item->app_layout, SIGNAL_CHECK_CHECK_BOX, SIGNAL_SOURCE);
            apps_view_s.selected_items = eina_list_append(apps_view_s.selected_items, item);
            item->is_checked = !item->is_checked;
        }
    }
    __apps_view_update_chooser_text(eina_list_count(apps_view_s.selected_items));
}

void apps_view_hw_menu_key(void)
{
    if (apps_view_s.view_state == VIEW_STATE_NORMAL)
        menu_change_state_on_hw_menu_key(apps_menu_table);
}

bool apps_view_hw_home_key(void)
{
    if (apps_view_s.opened_folder != NULL) {
        __apps_view_close_folder_popup(apps_view_s.opened_folder);
    }

    apps_view_set_state(VIEW_STATE_NORMAL);

    return false;
}

bool apps_view_hw_back_key(void)
{
    LOGD("%d", apps_view_s.pre_view_state);
    if (apps_view_s.opened_folder != NULL) {
        __apps_view_close_folder_popup(apps_view_s.opened_folder);
        return true;
    }

    if (apps_view_s.view_state == VIEW_STATE_NORMAL)
        return false;

    if (apps_view_s.view_state == VIEW_STATE_CHOOSER) {
        apps_view_set_state(apps_view_s.pre_view_state);
    } else if (apps_view_s.view_state == VIEW_STATE_EDIT) {
        apps_view_set_state(VIEW_STATE_NORMAL);
    }

    return true;
}

view_state_t apps_view_get_state(void)
{
    return apps_view_s.view_state;
}

void apps_view_set_state(view_state_t state)
{
    if (state == VIEW_STATE_EDIT) {
        homescreen_efl_btn_hide(HOMESCREEN_VIEW_APPS);

        Evas_Object *edje = NULL;
        edje = elm_layout_edje_get(apps_view_s.bg);
        if (!edje) {
            LOGE("Failed to get edje from layout");
            return;
        }
        edje_object_signal_emit(edje, SIGNAL_EDIT_MODE_ON_ANI, SIGNAL_SOURCE);

        Eina_List *find_list = NULL;
        Evas_Object *page_ly;
        EINA_LIST_FOREACH(apps_view_s.page_list, find_list, page_ly) {
            elm_object_signal_emit(page_ly, SIGNAL_EDIT_MODE_ON_ANI, SIGNAL_SOURCE);
        }

        Eina_List *data_list = apps_data_get_list();
        app_data_t *item = NULL;
        EINA_LIST_FOREACH(data_list, find_list, item) {
            if (item->app_layout) {
                elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_ON_ANI, SIGNAL_SOURCE);
                if (item->is_removable)
                    elm_object_signal_emit(item->app_layout, SIGNAL_UNINSTALL_BUTTON_SHOW_ANI, SIGNAL_SOURCE);
                elm_object_signal_emit(item->app_layout, SIGNAL_CHOOSER_MODE_OFF, SIGNAL_SOURCE);
            }
        }
    } else if (state == VIEW_STATE_CHOOSER) {
        homescreen_efl_btn_hide(HOMESCREEN_VIEW_APPS);
        elm_win_indicator_mode_set(homescreen_efl_get_win(), ELM_WIN_INDICATOR_HIDE);

        Evas_Object *edje = NULL;
        edje = elm_layout_edje_get(apps_view_s.bg);
        if (!edje) {
            LOGE("Failed to get edje from layout");
            return;
        }
        edje_object_signal_emit(edje, SIGNAL_EDIT_MODE_ON_ANI, SIGNAL_SOURCE);

        Eina_List *find_list = NULL;
        Evas_Object *page_ly;
        EINA_LIST_FOREACH(apps_view_s.page_list, find_list, page_ly) {
            elm_object_signal_emit(page_ly, SIGNAL_EDIT_MODE_ON_ANI, SIGNAL_SOURCE);
        }

        Eina_List *data_list = apps_data_get_list();
        app_data_t *item = NULL;
        EINA_LIST_FOREACH(data_list, find_list, item) {
            if (item->app_layout) {
                elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_ON_ANI, SIGNAL_SOURCE);
                if (!item->is_folder)
                    elm_object_signal_emit(item->app_layout, SIGNAL_CHOOSER_MODE_ON, SIGNAL_SOURCE);
            }
        }
        elm_object_signal_emit(apps_view_s.chooser_btn, SIGNAL_CHOOSER_BUTTON_SHOW, SIGNAL_SOURCE);
    } else if (state == VIEW_STATE_NORMAL) {
        homescreen_efl_btn_show(HOMESCREEN_VIEW_APPS);
        elm_win_indicator_mode_set(homescreen_efl_get_win(), ELM_WIN_INDICATOR_SHOW);

        Evas_Object *edje = NULL;
        edje = elm_layout_edje_get(apps_view_s.bg);
        if (!edje) {
            LOGE("Failed to get edje from layout");
            return;
        }
        edje_object_signal_emit(edje, SIGNAL_EDIT_MODE_OFF_ANI, SIGNAL_SOURCE);

        Eina_List *find_list = NULL;
        Evas_Object *page_ly;
        EINA_LIST_FOREACH(apps_view_s.page_list, find_list, page_ly) {
            elm_object_signal_emit(page_ly, SIGNAL_EDIT_MODE_OFF_ANI, SIGNAL_SOURCE);
        }
        Eina_List *data_list = apps_data_get_list();
        app_data_t *item = NULL;
        EINA_LIST_FOREACH(data_list, find_list, item) {
            if (item->app_layout) {
                elm_object_signal_emit(item->app_layout, SIGNAL_EDIT_MODE_OFF_ANI, SIGNAL_SOURCE);
                elm_object_signal_emit(item->app_layout, SIGNAL_UNINSTALL_BUTTON_HIDE_ANI, SIGNAL_SOURCE);
                elm_object_signal_emit(item->app_layout, SIGNAL_CHOOSER_MODE_OFF, SIGNAL_SOURCE);
            }
            item->is_checked = false;
        }
        if (apps_view_s.dest_folder)
            elm_object_signal_emit(apps_view_s.dest_folder->app_layout, SIGNAL_ICON_ENABLE, SIGNAL_SOURCE);
        apps_view_s.selected_items = eina_list_free(apps_view_s.selected_items);
        apps_view_s.dest_folder = NULL;
        apps_view_s.selected_item_count = 0;
        elm_object_signal_emit(apps_view_s.chooser_btn, SIGNAL_CHOOSER_BUTTON_HIDE, SIGNAL_SOURCE);
    }

    apps_view_s.pre_view_state = apps_view_s.view_state;
    apps_view_s.view_state = state;
}

void apps_view_update_folder_icon(app_data_t* item)
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
    LOGD("item_count %d", item_count);

    for (i = 0, find_list = folder_list, temp_item = eina_list_data_get(find_list);
            i < 4 && find_list;
            i++, find_list = eina_list_next(find_list), temp_item = eina_list_data_get(find_list)) {
        LOGD("%s", temp_item->label_str);
        sprintf(folder_item_count_string, "icon_%d", i);
        LOGD("%s", folder_item_count_string);
        icon_image = elm_image_add(item->folder_layout);
        elm_image_file_set(icon_image, temp_item->icon_path_str, NULL);
        evas_object_size_hint_weight_set(icon_image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_object_part_content_set(item->folder_layout, folder_item_count_string, icon_image);
        evas_object_show(icon_image);
    }
    sprintf(folder_item_count_string, "set_item_count_%d", item_count > 4 ? 4 : item_count);
    LOGD("%s", folder_item_count_string);
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

    apps_view_s.dest_folder = apps_data_add_folder();
    //apps_view_set_state(VIEW_STATE_CHOOSER);
    __apps_view_open_folder_popup(apps_view_s.dest_folder);
    menu_hide();
}

void apps_view_icon_add(app_data_t *item)
{
    if (item->parent_db_id == APPS_ROOT)
        apps_view_create_icon(item);
    else
        LOGE("Parent is NOT APPS_ROOT");
}

static void __apps_view_open_folder_popup(app_data_t *item)
{
    char edj_path[PATH_MAX] = {0, };
    snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/apps_folder_popup.edj"));

    apps_view_s.opened_folder = item;

    apps_view_s.folder_popup_ly = elm_layout_add(homescreen_efl_get_win());
    elm_layout_file_set(apps_view_s.folder_popup_ly , edj_path, GROUP_APPS_FOLDER_POPUP_LY);
    evas_object_size_hint_weight_set(apps_view_s.folder_popup_ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_resize(apps_view_s.folder_popup_ly, apps_view_s.width, apps_view_s.height);

    evas_object_show(apps_view_s.folder_popup_ly);

    Evas_Object *entry = elm_entry_add(apps_view_s.folder_popup_ly);
    apps_view_s.folder_title_entry = entry;
    evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_entry_single_line_set(entry, EINA_FALSE);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    evas_object_show(entry);

    elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
    elm_entry_text_style_user_push(entry, "DEFAULT='font=Samsung Condensed:style=Regular align=center color=#4DE7FFFF font_size=40 wrap=none'");
    elm_entry_entry_set(entry, apps_view_s.opened_folder->label_str);
    elm_object_part_content_set(apps_view_s.folder_popup_ly, APPS_FOLDER_TITLE, entry);

    __apps_view_folder_fill_apps();

    evas_object_color_set(apps_view_s.folder_popup_ly, 255, 255, 255, 0);
    apps_view_s.animator = ecore_animator_timeline_add(HOME_FOLDR_ANIMATION_TIME, __apps_view_show_folder_anim, NULL);
    elm_object_signal_callback_add(apps_view_s.folder_popup_ly, SIGNAL_APPS_FOLDER_HIDE, SIGNAL_SOURCE,
            __apps_view_hide_folder_cb, NULL);
}

static Eina_Bool __apps_view_show_folder_anim(void *data, double pos)
{
    evas_object_color_set(apps_view_s.folder_popup_ly, 255, 255, 255, pos*255);

    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(apps_view_s.folder_popup_ly, 255, 255, 255, 255);
        apps_view_s.animator = NULL;
        return ECORE_CALLBACK_DONE;
    }

    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool __apps_view_hide_folder_anim(void *data, double pos)
{
    evas_object_color_set(apps_view_s.folder_popup_ly, 255, 255, 255, (1-pos)*255);
    if (pos >= (1.0 - (1e-10))) {
        evas_object_color_set(apps_view_s.folder_popup_ly, 255, 255, 255, 0);
        __apps_view_close_folder_popup_done();
        apps_view_s.animator = NULL;
        return ECORE_CALLBACK_DONE;
    }

    return ECORE_CALLBACK_RENEW;
}

static void __apps_view_close_folder_popup(app_data_t *item)
{
    apps_view_s.animator = ecore_animator_timeline_add(HOME_FOLDR_ANIMATION_TIME, __apps_view_hide_folder_anim, NULL);
}

static void __apps_view_close_folder_popup_done(void)
{
    const char *folder_text = elm_object_text_get(apps_view_s.folder_title_entry);
    if (!apps_view_s.opened_folder->label_str || strcmp(apps_view_s.opened_folder->label_str, folder_text)) {
        if (apps_view_s.opened_folder->label_str)
            free(apps_view_s.opened_folder->label_str);
        apps_view_s.opened_folder->label_str = strdup(folder_text);
        elm_object_part_text_set(apps_view_s.opened_folder->app_layout, APPS_ICON_NAME, apps_view_s.opened_folder->label_str);
        apps_data_update_folder(apps_view_s.opened_folder);
    }

    Eina_List *list = apps_data_get_list();
    app_data_t *item = NULL;
    Eina_List *find_list;
    EINA_LIST_FOREACH(list, find_list, item) {
        if (item != NULL && item->parent_db_id == apps_view_s.opened_folder->db_id && item->app_layout) {
            evas_object_del(item->app_layout);
        }
    }
    evas_object_del(apps_view_s.folder_popup_ly);
    apps_view_s.folder_title_entry = NULL;
    apps_view_s.folder_popup_ly = NULL;
    apps_view_s.opened_folder = NULL;
}
static void __apps_view_hide_folder_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    if (apps_view_s.opened_folder)
        __apps_view_close_folder_popup(apps_view_s.opened_folder);
}

static void __apps_view_create_chooser(void)
{
    char edj_path[PATH_MAX] = {0, };

    snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/apps_chooser_btn.edj"));

    apps_view_s.chooser_btn = elm_layout_add(homescreen_efl_get_win());
    elm_layout_file_set(apps_view_s.chooser_btn, edj_path, GROUP_APPS_CHOOSER_BTN_LY);
    evas_object_size_hint_weight_set(apps_view_s.chooser_btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(homescreen_efl_get_win(), apps_view_s.chooser_btn);

    evas_object_show(apps_view_s.chooser_btn);

    elm_object_signal_callback_add(apps_view_s.chooser_btn, SIGNAL_CHOOSER_LEFT_BTN_CLICKED, SIGNAL_SOURCE, __apps_view_chooser_left_btn_clicked, NULL);
    elm_object_signal_callback_add(apps_view_s.chooser_btn, SIGNAL_CHOOSER_RIGHT_BTN_CLICKED, SIGNAL_SOURCE, __apps_view_chooser_right_btn_clicked, NULL);
    __apps_view_update_chooser_text(apps_view_s.selected_item_count);
}

static void __apps_view_chooser_left_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    LOGD("Cancel button clicked");
    apps_view_set_state(apps_view_s.pre_view_state);
}

static void __apps_view_chooser_right_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    LOGD("Done button clicked");
    Eina_List *find_list;
    app_data_t *item = NULL;
    EINA_LIST_FOREACH(apps_view_s.selected_items, find_list, item) {
        LOGD("%s-> move to folder {%d}", item->label_str, apps_view_s.dest_folder->db_id);
        item->parent_db_id = apps_view_s.dest_folder->db_id;
        apps_db_update(item);
        if (item->app_layout) {
            evas_object_del(item->app_layout);
            item->app_layout = NULL;
        }
    }
    apps_data_sort();
    apps_view_reroder();

    Eina_List *list = apps_data_get_list();
    EINA_LIST_FOREACH(list, find_list, item) {
        if (item->is_folder)
            apps_view_update_folder_icon(item);
    }
    //apps_view_update_folder_icon(apps_view_s.dest_folder);
    apps_view_set_state(VIEW_STATE_NORMAL);
}

static void __apps_view_update_chooser_text(int item_count)
{
    char text[FOLDER_STR_LEN];
    if (apps_view_s.selected_item_count + item_count > 0) {
        sprintf(text, _("IDS_MEMO_HEADER_PD_SELECTED_ABB2"), apps_view_s.selected_item_count + item_count);
    } else
        sprintf(text, "");
    elm_object_part_text_set(apps_view_s.chooser_btn, APPS_CHOOSER_MIDDLE_LABEL, text);
}

static void __apps_view_badge_update_cb(unsigned int action, const char *app_id, unsigned int count, void *user_data)
{
    LOGD("%s updated", app_id);
    Eina_List *data_list = apps_data_get_list();
    Eina_List *find_list = NULL;
    app_data_t *item = NULL;
    if (app_id == NULL) {
        LOGD("app id is NULL");
        EINA_LIST_FOREACH(data_list, find_list, item) {
            __apps_view_badge_update_count(item);
        }
        return;
    }
    EINA_LIST_FOREACH(data_list, find_list, item) {
        if (strcmp(app_id, item->pkg_str) == 0) {
            __apps_view_badge_update_count(item);
        }
    }
}

static void __apps_view_badge_update_count(app_data_t *item)
{
    unsigned int to_be_displayed = 0;
    int result = BADGE_ERROR_NONE;

    if (!item->pkg_str) {
        LOGE("app_item is NULL in badge count");
        return;
    }

    result = badge_get_display(item->pkg_str, &to_be_displayed);
    if (result != BADGE_ERROR_NONE) {
        LOGE("badge_get_display error %d", result);
        item->badge_count = 0;
        return;
    }
    if (!to_be_displayed) {
        item->badge_count = 0;
        return;
    }

    result = badge_get_count(item->pkg_str, &item->badge_count);
    if (result != BADGE_ERROR_NONE) {
        LOGE("badge_get_count error %d", result);
        item->badge_count = 0;
        return;
    }
    __apps_view_badge_update_icon(item);
}

static void __apps_view_badge_update_icon(app_data_t *item)
{
    char number_str[FOLDER_STR_LEN];
    if (item->badge_count == 0) {
        elm_object_signal_emit(item->app_layout, SIGNAL_BADGE_HIDE, SIGNAL_SOURCE);
        return;
    }

    if (item->badge_count > MAX_BADGE_DISPLAY_COUNT) {
        sprintf(number_str, "%d+", MAX_BADGE_DISPLAY_COUNT);
    } else {
        sprintf(number_str, "%d", item->badge_count);
    }
    elm_layout_text_set(item->app_layout, APPS_ICON_BADGE_TEXT, number_str);
    elm_object_signal_emit(item->app_layout, SIGNAL_BADGE_SHOW, SIGNAL_SOURCE);
}

static void __apps_view_plus_icon_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    app_data_t *item  = (app_data_t *)data;
    Eina_List *folder_list = NULL;
    apps_view_set_state(VIEW_STATE_CHOOSER);
    __apps_view_close_folder_popup_done();
    apps_view_s.dest_folder = item;
    elm_object_signal_emit(item->app_layout, SIGNAL_ICON_DISABLE, SIGNAL_SOURCE);

    apps_data_get_folder_item_list(&folder_list, item);
    apps_view_s.selected_item_count = eina_list_count(folder_list);
    __apps_view_update_chooser_text(0);

    eina_list_free(folder_list);
}

static void __apps_view_create_toast_popup(char *str)
{
    Evas_Object *popup;

    popup = elm_popup_add(homescreen_efl_get_win());
    elm_object_style_set(popup, "toast");
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, str);
    elm_popup_timeout_set(popup, 2.0);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
    evas_object_smart_callback_add(popup, "block,clicked", __apps_view_toast_popup_block_clicked_cb, NULL);
    evas_object_smart_callback_add(popup, "timeout", __apps_view_toast_popup_timeout_cb, NULL);

    evas_object_show(popup);

}

static void __apps_view_toast_popup_timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}
static void __apps_view_toast_popup_block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}
