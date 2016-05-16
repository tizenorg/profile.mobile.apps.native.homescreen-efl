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

#include "homescreen-efl.h"
#include "popup.h"
#include "conf.h"
#include "util.h"

static struct {
    Evas_Object *popup;
    bool is_visible;
} popup_info = {
    .popup = NULL,
    .is_visible = false
};

/* popup_t
 * 0 : POPUP_CLUSTER_PAGE_FULL
 * 1 : POPUP_CLUSTER_DELETE_PAGE
 */
static const char popup_title_text[POPUP_MAX][PATH_MAX_LEN] = {
        "IDS_HS_HEADER_UNABLE_TO_ADD_WIDGET_ABB",
        "IDS_HS_HEADER_DELETE_PAGE_ABB2",
};

static const char popup_text[POPUP_MAX][PATH_MAX_LEN] = {
        "IDS_HS_POP_UNABLE_TO_ADD_THIS_HOME_BOX_TO_THE_HOME_SCREEN_THERE_IS_NOT_ENOUGH_SPACE_ON_THE_HOME_SCREEN_MSG",
        "IDS_HS_POP_THIS_PAGE_AND_ALL_THE_ITEMS_IT_CONTAINS_WILL_BE_DELETED",
};

static const char popup_button_text[POPUP_MAX][3][PATH_MAX_LEN] = {
        { "IDS_CAM_SK_OK", "", "" },
        { "IDS_HS_OPT_DELETE", "IDS_CAM_SK_CANCEL", "" }
};

static void __popup_default_cb(void *data, Evas_Object *obj, void *event_info);
static void __popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);

void popup_show(popup_t type, int btn_count, Evas_Smart_Cb btn_func[3], void *func_data[3])
{
    if (popup_info.is_visible) {
        return ;
    }

    popup_info.is_visible = true;

    popup_info.popup = elm_popup_add(homescreen_efl_get_win());
    elm_popup_align_set(popup_info.popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    evas_object_size_hint_weight_set(popup_info.popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_text_set(popup_info.popup, "title,text", _(popup_title_text[type]));
    elm_object_part_text_set(popup_info.popup, "default", _(popup_text[type]));
    evas_object_smart_callback_add(popup_info.popup, "dismissed", __popup_dismissed_cb, NULL);

    int idx = 0;
    for (idx = 0; idx < btn_count; idx++) {
        Evas_Object *btn;
        btn = elm_button_add(popup_info.popup);
        elm_object_style_set(btn, "popup");
        elm_object_text_set(btn, _(popup_button_text[type][idx]));
        elm_object_part_content_set(popup_info.popup, "button1", btn);
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
