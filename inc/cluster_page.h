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

#ifndef __CLUSTER_PAGE_H__
#define __CLUSTER_PAGE_H__

#include <Elementary.h>
#include <widget_service.h>
#include "conf.h"
#include "cluster_data.h"

typedef struct {
    int page_index;
    Evas_Object *page_layout;
    int space[CLUSTER_ROW][CLUSTER_COL];

    Evas_Object *grid;
    Evas_Object *thumbnail_ly;
    Evas_Object *highlight;
    int highlight_pos_x;
    int highlight_pos_y;

    Eina_List *widget_list;
} cluster_page_t;

cluster_page_t *cluster_page_new(Evas_Object* parent);
void cluster_page_delete(cluster_page_t *page);
bool cluster_page_set_widget(cluster_page_t *page, widget_data_t *item);
bool cluster_page_unset(cluster_page_t *page, widget_data_t *item);
bool cluster_page_check_empty_space(cluster_page_t *page, widget_size_type_e type,
        int pos_x, int pos_y, int *empty_x, int *empty_y);
Evas_Object *cluster_page_thumbnail(cluster_page_t *page);
void cluster_page_drag_cancel(cluster_page_t *page);
void cluster_page_drag_widget(cluster_page_t *page, widget_size_type_e type, int pos_x, int pos_y);
bool cluster_page_drop_widget(cluster_page_t *page, widget_data_t *widget);

#endif /* __CLUSTER_VIEW_PAGE_H__ */
