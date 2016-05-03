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

#include "cluster_data.h"
#include "cluster_db.h"
#include "cluster_view.h"
#include "util.h"

static struct {
    Eina_List *data_list;
} cluster_data_s = {
    .data_list = NULL
};

static void __cluster_data_item_free(cluster_data_t *item);
static void __cluster_data_insert_default_data(void);

void cluster_data_init(void)
{
    if (cluster_db_create()) {
        __cluster_data_insert_default_data();
    }

    cluster_db_get_list(&cluster_data_s.data_list);
}

Eina_List *cluster_data_get_list(void)
{
    return cluster_data_s.data_list;
}

static void __cluster_data_insert_default_data(void)
{
    cluster_data_t *new_item = (cluster_data_t *)malloc(sizeof(cluster_data_t));
    memset(new_item, 0, sizeof(cluster_data_t));

    new_item->page_idx = 0;
    new_item->pos_y = 0;
    new_item->pos_x = 0;
    new_item->pkg_name = strdup("org.tizen.calendar.widget");
    new_item->content_info = NULL;
    new_item->type = 0;
    new_item->period = 0.0;
    new_item->allow_duplicate = 0;

    cluster_db_insert(new_item);
}

static void __cluster_data_item_free(cluster_data_t *item)
{
    if (item && item->widget_layout)
        evas_object_del(item->widget_layout);
    if (item && item->pkg_name)
        free(item->pkg_name);
    if (item && item->content_info)
        free(item->content_info);
    if (item)
        free(item);

}
