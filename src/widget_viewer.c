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

#include "widget_viewer.h"
#include "util.h"
#include "conf.h"

void widget_viewer_init(Evas_Object *win)
{
    widget_viewer_evas_init(win);
}

void widget_viewer_fini(void)
{
    widget_viewer_evas_fini();
}

Evas_Object *widget_viewer_add_widget(Evas_Object *parent, const char *pkgid, const char *sub_id
        , double period, int *widget_width, int *widget_height)
{
    Evas_Object *widget = widget_viewer_evas_add_widget(parent, pkgid, NULL, period);

    //Evas_Object *widget = evas_object_rectangle_add(parent);

    /*const char *bg_path = util_get_res_file_path(IMAGE_DIR"/home_button_apps.png");
    Evas_Object *widget = evas_object_image_filled_add(evas_object_evas_get(parent));
    evas_object_image_file_set(widget, bg_path, "bg");*/

    /*Evas_Object *widget = elm_button_add(parent);
    elm_object_text_set(widget, "HOME");*/

    widget_service_get_size(WIDGET_SIZE_TYPE_4x4, widget_width, widget_height);

    return widget;
}
