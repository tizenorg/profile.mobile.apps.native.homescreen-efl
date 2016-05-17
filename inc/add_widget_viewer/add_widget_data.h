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

#ifndef __ADD_WIDGET_DATA_H__
#define __ADD_WIDGET_DATA_H__

typedef struct {
    char *widget_id;
    char *app_id;
    char *label;

    Eina_List *preview_list;
    Elm_Object_Item *genlist_item;

    int size_types_count;
    int size_types;
} add_widget_data_t;

typedef struct {
    int type;
    char *path;
} add_widget_data_preview_t;

bool add_widget_data_init(void);
void add_widget_data_fini(void);

Eina_List *add_widget_data_get_widget_list(void);
Eina_List *add_widget_data_get_widget_preview_list(add_widget_data_t *widget);

#endif /* __ADD_WIDGET_DATA_H__ */
