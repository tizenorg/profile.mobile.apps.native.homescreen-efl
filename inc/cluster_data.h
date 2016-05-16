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

#ifndef __CLUSTER_DATA_H__
#define __CLUSTER_DATA_H__

#include <app.h>
#include <Elementary.h>
#include <widget_service.h>

typedef struct {
    int widget_id;
    int page_idx;
    int pos_y;
    int pos_x;
    char *pkg_name;
    char *content_info;
    widget_size_type_e type;
    double period;
    int allow_duplicate;
    Evas_Object *widget_layout;
} widget_data_t;

void cluster_data_init(void);
Eina_List *cluster_data_get_widget_list(void);

int cluster_data_get_page_count();
void cluster_data_set_page_count(int count);

void cluster_data_insert_widget(const char* pkg_name, const char* content_info, int type);
void cluster_data_insert(widget_data_t *item);
void cluster_data_delete(widget_data_t *item);
void cluster_data_update(widget_data_t *item);

#endif /* __CLUSTER_DATA_H__ */
