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
#include <efl_extension.h>

#include "homescreen-efl.h"
#include "popup.h"
#include "util.h"

static struct {
    Evas_Object *popup;
    bool is_visible;
} popup_info = {
    .popup = NULL,
    .is_visible = false
};

static void __popup_default_cb(void *data, Evas_Object *obj, void *event_info);
static void __popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);

static void __toast_timeout_cb(void *data, Evas_Object *obj, void *event_info);
static void __toast_block_clicked_cb(void *data, Evas_Object *obj, void *event_info);

void popup_show(int btn_count, Evas_Smart_Cb btn_func[3], void *func_data[3],
        const char (*btn_text)[STR_MAX], const char *title_text, const char *popup_text)
{
    if (popup_info.is_visible) {
        return ;
    }

    char part[3][10] = { "button1", "button2", "button3" };

    popup_info.is_visible = true;

    popup_info.popup = elm_popup_add(homescreen_efl_get_win());
    elm_popup_align_set(popup_info.popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    evas_object_size_hint_weight_set(popup_info.popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_text_set(popup_info.popup, "title,text", title_text);
    elm_object_part_text_set(popup_info.popup, "default", popup_text);
    evas_object_smart_callback_add(popup_info.popup, "dismissed", __popup_dismissed_cb, NULL);

    int idx = 0;
    for (idx = 0; idx < btn_count; idx++) {
        Evas_Object *btn;
        btn = elm_button_add(popup_info.popup);
        elm_object_style_set(btn, "popup");
        elm_object_text_set(btn, btn_text[idx]);
        elm_object_part_content_set(popup_info.popup, part[idx], btn);
        evas_object_smart_callback_add(btn, "clicked",
                (btn_func[idx] == NULL ? __popup_default_cb : btn_func[idx]), func_data[idx]);
    }
    evas_object_show(popup_info.popup);
}

void popup_hide(void)
{
    if (!popup_info.is_visible) {
        return ;
    }

    popup_info.is_visible = false;

    evas_object_hide(popup_info.popup);
    evas_object_del(popup_info.popup);
    popup_info.popup = NULL;
}

bool popup_is_show(void)
{
    return popup_info.is_visible;
}

static void __popup_default_cb(void *data, Evas_Object *obj, void *event_info)
{
    popup_hide();
}

static void __popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
    popup_info.is_visible = false;
    if (popup_info.popup) {
        evas_object_del(popup_info.popup);
        popup_info.popup = NULL;
    }
}

void toast_show(char* str)
{
    Evas_Object *popup;

    popup = elm_popup_add(homescreen_efl_get_win());
    elm_object_style_set(popup, "toast");
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, str);
    elm_popup_timeout_set(popup, 2.0);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
    evas_object_smart_callback_add(popup, "block,clicked", __toast_block_clicked_cb, NULL);
    evas_object_smart_callback_add(popup, "timeout", __toast_timeout_cb, NULL);

    evas_object_show(popup);
}

static void __toast_timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}
static void __toast_block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}
