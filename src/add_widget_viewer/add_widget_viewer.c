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

#include <app.h>
#include <Elementary.h>
#include <widget_service.h>

#include "add_widget_viewer/add_widget_viewer.h"
#include "add_widget_viewer/add_widget_data.h"
#include "cluster_data.h"
#include "cluster_view.h"
#include "util.h"
#include "conf.h"
#include "edc_conf.h"

static struct {
    Evas_Object *win;
    Evas_Object *conformant;
    Evas_Object *layout;
    Evas_Object *naviframe;
    Evas_Object *genlist;
    Elm_Theme *theme;
    Elm_Object_Item *navi_item;
    Evas_Object *index;
} add_widget_viewer_s = {
    .win = NULL,
    .conformant = NULL,
    .layout = NULL,
    .naviframe = NULL,
    .genlist = NULL,
    .theme = NULL,
    .navi_item = NULL,
    .index = NULL
};

static void __add_widget_viewer_win_del_cb(void *data, Evas_Object* obj, void* event_info);
static Elm_Theme *__add_widget_viewer_create_theme(void);
static Evas_Object *__add_widget_viewer_create_conformant(void);
static Evas_Object *__add_widget_viewer_create_layout();
static Evas_Object *__add_widget_viewer_create_naviframe();

static Evas_Object *__add_widget_viewer_create_content(Evas_Object *naviframe, Evas_Object *genlist);
static Evas_Object *__add_widget_viewer_create_index(Evas_Object *layout);
static void __add_widget_viewer_index_cb(void *data, Evas_Object *obj, void *event_info);
static int __add_widget_viewer_compare_index_cb(const void *data1, const void *data2);
static Evas_Object *__add_widget_viewer_create_list(Evas_Object *content);
static char *__add_widget_viewer_list_text_get_cb(void *data, Evas_Object *obj, const char *part);
static Evas_Object *__add_widget_viewer_list_content_get_cb(void *data, Evas_Object *obj, const char *part);
static Evas_Object *__add_widget_viewer_list_widget_box_create(Evas_Object *obj, add_widget_data_t *widget);
static void __add_widget_viewer_preview_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static Evas_Object *__add_widget_viewer_list_widget_preview_box_create(Evas_Object *main_box, add_widget_data_t *widget, add_widget_data_preview_t *preview);
static void __add_widget_viewer_item_realized_cb(void *data, Evas_Object *obj, void *event_info);

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

    add_widget_viewer_s.theme = __add_widget_viewer_create_theme();
    add_widget_viewer_s.conformant = __add_widget_viewer_create_conformant();
    add_widget_viewer_s.layout = __add_widget_viewer_create_layout();
    add_widget_viewer_s.naviframe = __add_widget_viewer_create_naviframe();

    add_widget_viewer_s.navi_item = __add_widget_viewer_create_content(add_widget_viewer_s.naviframe, add_widget_viewer_s.genlist);

    elm_object_signal_emit(add_widget_viewer_s.layout, "display,normal", "container");

    elm_win_indicator_mode_set(add_widget_viewer_s.win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(add_widget_viewer_s.win, ELM_WIN_INDICATOR_OPAQUE);
    elm_win_resize_object_add(add_widget_viewer_s.win, add_widget_viewer_s.conformant);
    elm_win_conformant_set(add_widget_viewer_s.win, EINA_TRUE);
    evas_object_resize(add_widget_viewer_s.win, WINDOW_W, WINDOW_H);

    evas_object_show(add_widget_viewer_s.naviframe);
    evas_object_show(add_widget_viewer_s.layout);
    evas_object_show(add_widget_viewer_s.conformant);

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

static Elm_Theme *__add_widget_viewer_create_theme(void)
{
    Elm_Theme *theme = elm_theme_new();
    if (!theme) {
        LOGE("Failed to create theme\n");
        return NULL;
    }

    elm_theme_ref_set(theme, NULL);
    elm_theme_extension_add(theme, util_get_res_file_path(EDJE_DIR"/add_widget_viewer.edj"));

    return theme;
}

static Evas_Object *__add_widget_viewer_create_conformant(void)
{
    Evas_Object *conformant;
    Evas_Object *bg;

    conformant = elm_conformant_add(add_widget_viewer_s.win);
    if (!conformant) {
        LOGE("Failed to create a conformant\n");
        return NULL;
    }

    evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    bg = elm_bg_add(conformant);
    if (bg) {
        elm_object_style_set(bg, "indicator/headerbg");
        elm_object_part_content_set(conformant, "elm.swallow.indicator_bg", bg);
        evas_object_show(bg);
    } else {
        LOGE("Failed to create a BG object\n");
    }

    return conformant;
}

static Evas_Object *__add_widget_viewer_create_layout()
{
    Evas_Object *layout;
    char edj_path[STR_PATH_LEN] = {0, };

    if (!add_widget_viewer_s.conformant) {
        return NULL;
    }

    layout = elm_layout_add(add_widget_viewer_s.conformant);
    if (!layout) {
        return NULL;
    }

    snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/add_widget_viewer.edj"));
    if (elm_layout_file_set(layout, edj_path, ADD_WIDGET_VIEWER_CONFORMANT_FRAME) != EINA_TRUE) {
        evas_object_del(layout);
        return NULL;
    }

    elm_object_part_content_set(add_widget_viewer_s.conformant, "elm.swallow.content", layout);

    return layout;
}

static Evas_Object *__add_widget_viewer_create_naviframe()
{
    Evas_Object *naviframe;

    naviframe = elm_naviframe_add(add_widget_viewer_s.conformant);
    if (!naviframe) {
        return NULL;
    }

    elm_naviframe_content_preserve_on_pop_set(naviframe, EINA_TRUE);
    elm_object_part_content_set(add_widget_viewer_s.layout, "content", naviframe);

    return naviframe;
}

static Evas_Object *__add_widget_viewer_create_content(Evas_Object *naviframe, Evas_Object *genlist)
{
    Elm_Object_Item *item;
    Evas_Object *content;

    content = elm_layout_add(naviframe);
    if (!content) {
        return NULL;
    }

    if (elm_layout_file_set(content, util_get_res_file_path(EDJE_DIR"/add_widget_viewer.edj"), ADD_WIDGET_VIEWER_CONTENT_FRAME) != EINA_TRUE) {
        LOGE("Can not set layout file");
        evas_object_del(content);
        return NULL;
    }
    add_widget_viewer_s.index = __add_widget_viewer_create_index(content);
    if (!add_widget_viewer_s.index) {
        evas_object_del(content);
        return NULL;
    }
    elm_object_part_content_set(content, "index", add_widget_viewer_s.index);

    genlist = __add_widget_viewer_create_list(content);
    elm_object_part_content_set(content, "content", genlist);

    item = elm_naviframe_item_push(naviframe, _("IDS_HS_HEADER_ADD_WIDGET"), NULL, NULL, content, NULL);
    elm_object_signal_emit(content, "display,normal", "container");
    elm_object_signal_emit(content, "display,index", "container");

    if (!item) {
        LOGD("Failed to push an item\n");
        return NULL;
    }

    return content;
}

static Evas_Object *__add_widget_viewer_create_index(Evas_Object *layout)
{
    Evas_Object *index;
    const char *idx_str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char ch[2] = {0, };

    index = elm_index_add(layout);
    elm_index_autohide_disabled_set(index, EINA_TRUE);
    elm_index_omit_enabled_set(index, EINA_TRUE);
    elm_index_item_append(index, "#", NULL, NULL);

    int i;
    for (i = 0; i < strlen(idx_str); ++i) {
        ch[0] = idx_str[i];
        ch[1] = '\0';
        elm_index_item_append(index, ch, __add_widget_viewer_index_cb, &ch[0]);
    }

    elm_index_level_go(index, 0);

    return index;
}

static void __add_widget_viewer_index_cb(void *data, Evas_Object *obj, void *event_info)
{
    char *idx_str = (char *)data;
    Eina_List *widget_list = NULL;
    add_widget_data_t *widget = NULL;

    widget_list = add_widget_data_get_widget_list();
    if (!widget_list) {
        LOGE("Can not get widget list");
        return;
    }

    widget = eina_list_search_unsorted(widget_list, __add_widget_viewer_compare_index_cb, idx_str);
    if (!widget) {
        LOGE("Can not find widget");
        return;
    }

    elm_genlist_item_bring_in(widget->genlist_item, ELM_GENLIST_ITEM_SCROLLTO_TOP);

}

static int __add_widget_viewer_compare_index_cb(const void *data1, const void *data2)
{
    add_widget_data_t *w = (add_widget_data_t *)data1;
    char *idx_str = (char *)data2;

    char c1 = 0;
        char c2 = 0;

    c1 = tolower(idx_str[0]);
    c2 = tolower(w->widget_id[0]);

    LOGD("Compare: %c == %c in %s, %s", c1, c2, idx_str, w->widget_id);

    if (c1 < c2) return -1;
    if (c1 > c2) return 1;
    return 0;
}

static Evas_Object *__add_widget_viewer_create_list(Evas_Object *content)
{
    Elm_Genlist_Item_Class *itc_widget;
    Evas_Object *genlist = NULL;
    Elm_Object_Item *widget_gl_item = NULL;
    Eina_List *widget_list = NULL, *l = NULL;
    add_widget_data_t *widget = NULL;

    genlist = elm_genlist_add(content);
    if (!genlist) {
        LOGE("Failed to create a genlist\n");
        return NULL;
    }

    elm_scroller_bounce_set(genlist, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_NONE);

    if (add_widget_viewer_s.theme)
        elm_object_theme_set(genlist, add_widget_viewer_s.theme);

    itc_widget = elm_genlist_item_class_new();
    itc_widget->item_style = "widget,leaf";
    itc_widget->func.text_get = __add_widget_viewer_list_text_get_cb;
    itc_widget->func.content_get = __add_widget_viewer_list_content_get_cb;

    widget_list = add_widget_data_get_widget_list();
    if (!widget_list) {
        LOGE("Can not get widget list");
        return NULL;
    }

    LOGD("Widget list length: %d", eina_list_count(widget_list));
    EINA_LIST_FOREACH(widget_list, l, widget) {
        LOGD("Genlist append %s", widget->app_id);
        widget_gl_item = elm_genlist_item_append(genlist, itc_widget, widget, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
        widget->genlist_item = widget_gl_item;
    }

    evas_object_smart_callback_add(genlist, "realized", __add_widget_viewer_item_realized_cb, NULL);

    elm_genlist_item_class_free(itc_widget);

    return genlist;
}

static void __add_widget_viewer_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
    LOGD("item realized_cb");

    char ch[2] = {0, };
    Elm_Object_Item *item = event_info;
    Elm_Index_Item *index_item = NULL;
    add_widget_data_t *widget = NULL;

    widget = (add_widget_data_t *)elm_object_item_data_get(item);

    LOGD("item data: %s", widget->widget_id);
    ch[0] = widget->widget_id[0];

    if (strlen(ch) == 0) {
        LOGE("Can not get first char of widget name");
        return;
    }

    index_item = elm_index_item_find(add_widget_viewer_s.index, ch);
    if (!index_item) {
        LOGE("Can not set selected index item");
        return ;
    }

    elm_index_item_selected_set(index_item, EINA_TRUE);
}

static char *__add_widget_viewer_list_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    add_widget_data_t *widget = data;

    if (!widget || !part)
        return NULL;

    if (!strcmp(part, "elm.text")) {
        char style_string[STR_MAX] = {0, };
        snprintf(style_string, sizeof(style_string), "<font_size=%d>%s</font_size>", (int)CLUSTER_ADDVIEWER_TEXT_SIZE, widget->label);
        return strdup(style_string);
    }

    return strdup("widget name not found");
}

static Evas_Object *__add_widget_viewer_list_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    LOGD("Part: %s", part);

    Evas_Object *main_box = NULL;
    add_widget_data_t *widget = data;

    if (!widget || !part)
        return NULL;

    if  (!strcmp(part, SIZE_SETTER)) {
        Evas_Object *rect = NULL;
        rect = evas_object_rectangle_add(evas_object_evas_get(obj));
        if (!rect) {
            LOGE("Can not create preview box");
            return NULL;
        }
        evas_object_color_set(rect, 255, 255, 255, 0);
        evas_object_size_hint_min_set(rect, ALLPAGE_PREVIEW_LIST_ITEM_W, ALLPAGE_PREVIEW_LIST_ITEM_H);
        evas_object_size_hint_max_set(rect, ALLPAGE_PREVIEW_LIST_ITEM_W, ALLPAGE_PREVIEW_LIST_ITEM_H);
        evas_object_show(rect);
        return rect;
    } else if (!strcmp(part, "elm.icon")) {
        main_box = __add_widget_viewer_list_widget_box_create(obj, widget);
        if (!main_box) {
            LOGE("Can not create preview box");
            return NULL;
        }
    }

    return main_box;
}

static Evas_Object *__add_widget_viewer_list_widget_box_create(Evas_Object *obj, add_widget_data_t *widget)
{
    LOGD("Create box for %s", widget->widget_id);

    Evas_Object *box = NULL;
    Evas_Object *preview_layout = NULL;
    Eina_List *l;
    Eina_List *children;
    add_widget_data_preview_t *preview;

    box = elm_box_add(obj);
    if (!box) {
        LOGE("Can not create preview box");
        return NULL;
    }

    elm_box_align_set(box, 0.0, 0.0);
    elm_box_horizontal_set(box, EINA_TRUE);
    elm_box_homogeneous_set(box, EINA_FALSE);
    elm_box_padding_set(box, ALLPAGE_PREVIEW_PADDING_MID, 0);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    EINA_LIST_FOREACH(widget->preview_list, l, preview) {
        preview_layout = __add_widget_viewer_list_widget_preview_box_create(box, widget, preview);
        if (!preview_layout) {
            LOGE("Can not create preview layout");
            continue;
        }
        evas_object_show(preview_layout);

        switch (preview->type) {
        case WIDGET_SIZE_TYPE_4x4:
        case WIDGET_SIZE_TYPE_4x2:
            evas_object_size_hint_align_set(preview_layout, 0.0, 0.0);
            elm_box_pack_end(box, preview_layout);
            break;
        default:
            LOGE("Unsupported size\n");
            break;
            return NULL;
        }
    }

    children = elm_box_children_get(box);
    if (eina_list_count(children) == 0) {
        evas_object_del(box);
        LOGE("Preview is not exists");
        box = NULL;
    } else {
        evas_object_show(box);
    }

    eina_list_free(children);

    return box;
}

static Evas_Object *__add_widget_viewer_list_widget_preview_box_create(Evas_Object *main_box, add_widget_data_t *widget, add_widget_data_preview_t *preview)
{
    LOGD("Create preview: type->%d path->%s", preview->type, preview->path);

    Evas_Object *preview_layout = NULL;
    Evas_Object *preview_img = NULL;
    int w = 0, h = 0;
    int ret = 0;

    if (!preview || !preview->path || !preview->type) {
        LOGE("Path to preview image is empty");
        return NULL;
    }

    preview_layout = elm_layout_add(main_box);
    if (!preview_layout) {
        LOGE("Can not create preview layout");
        return NULL;
    }

    ret = elm_layout_file_set(preview_layout, util_get_res_file_path(EDJE_DIR"/add_widget_viewer.edj"), ADD_WIDGET_VIEWER_PREVIEW);
    if (ret != EINA_TRUE) {
        LOGE("Can not set preview layout");
        evas_object_del(preview_layout);
        return NULL;
    }

    switch (preview->type) {
    case WIDGET_SIZE_TYPE_4x2:
        w = ALLPAGE_PREVIEW_4X2_W;
        h = ALLPAGE_PREVIEW_4X2_H;
        break;
    case WIDGET_SIZE_TYPE_4x4:
        w = ALLPAGE_PREVIEW_4X4_W;
        h = ALLPAGE_PREVIEW_4X4_H;
        break;
    default:
        return NULL;
    }

    Evas_Object *rect = NULL;
    rect = evas_object_rectangle_add(evas_object_evas_get(preview_layout));
    if (!rect) {
        LOGE("Can not create preview layout");
        return NULL;
    }
    evas_object_color_set(rect, 255, 255, 255, 0);
    evas_object_size_hint_min_set(rect, w, h);
    evas_object_size_hint_max_set(rect, w, h);
    elm_object_part_content_set(preview_layout, SIZE_SETTER, rect);

    int *preview_type = calloc(1, sizeof(int));
    if (!preview_type) {
        LOGE("Can not allocate memory for additional data");
        evas_object_del(preview_layout);
        return NULL;
    }
    *preview_type = preview->type;

    preview_img = evas_object_image_filled_add(evas_object_evas_get(preview_layout));
    if (!preview_img) {
        LOGE("Can not create image object");
        evas_object_del(preview_layout);
        return NULL;
    }

    evas_object_data_set(preview_layout, "preview_type", preview_type);

    elm_object_signal_callback_add(preview_layout, SIGNAL_PREVIEW_CLICKED, SIGNAL_SOURCE, __add_widget_viewer_preview_clicked_cb, widget);

    evas_object_image_file_set(preview_img, preview->path, NULL);
    evas_object_image_size_get(preview_img, &w, &h);
    evas_object_image_fill_set(preview_img, 0, 0, w, h);

    elm_object_part_content_set(preview_layout, ADD_WIDGET_VIEWER_PREVIEW_ICON, preview_img);

    return preview_layout;
}
static void __add_widget_viewer_preview_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    LOGD("Preview clicked");

    add_widget_data_t *widget = NULL;
    int *size = NULL;

    size = evas_object_data_del(obj, "preview_type");
    if (*size == WIDGET_SIZE_TYPE_UNKNOWN) {
        LOGE("Can not get widgets size type");
        return;
    }

    widget = (add_widget_data_t *)data;
    if (!widget) {
        LOGE("Can not get widget");
        return;
    }

    int type = *size;
    free(size);
    char *widget_id = strdup(widget->widget_id);

    cluster_view_set_state(VIEW_STATE_NORMAL);

    cluster_data_insert_widget(widget_id, widget_id, type);
    free(widget_id);
}
