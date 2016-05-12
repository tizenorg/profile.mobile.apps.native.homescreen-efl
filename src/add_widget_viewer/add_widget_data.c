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

#include <Elementary.h>
#include <widget_service.h>

#include "add_widget_viewer/add_widget_data.h"

static struct {
    Eina_List *data_list;
} add_widget_data_s = {
    .data_list = NULL
};

static void __add_widget_data_list_delete(void);
static void __add_widget_data_free(add_widget_data_t *item);

bool add_widget_data_init(void)
{
    int ret = 0;
    /*ret = widget_service_get_widget_list(__widget_list_cb, NULL);
    LOGD("ERROR: %s", get_error_message(ret));
    if (ret < 0 || add_widget_data_s.data_list == NULL) {
        LOGE("Can not create widget list, ret: %d", ret);
        return false;
    }*/

    return true;
}

void add_widget_data_fini(void)
{
    __add_widget_data_list_delete();
}

static void __add_widget_data_list_delete(void)
{
    Eina_List *list;
    add_widget_data_t *item;

    EINA_LIST_FOREACH(add_widget_data_s.data_list, list, item)
        __add_widget_data_free(item);

    eina_list_free(add_widget_data_s.data_list);
    add_widget_data_s.data_list = NULL;
}

static void __add_widget_data_free(add_widget_data_t *item)
{
    //free data
}
