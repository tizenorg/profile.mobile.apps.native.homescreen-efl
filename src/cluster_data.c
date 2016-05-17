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
#include <app_preference.h>

#include "cluster_data.h"
#include "cluster_db.h"
#include "cluster_view.h"
#include "util.h"
#include "conf.h"

#define CLUSTER_DATA_PAGE_COUNT "cluster_page_count"

static struct {
    Eina_List *data_list;
} cluster_data_s = {
    .data_list = NULL
};

static void __cluster_data_item_free(widget_data_t *item);
static void __cluster_data_insert_default_data(void);

void cluster_data_init(void)
{
    if (cluster_db_create()) {
        __cluster_data_insert_default_data();
        cluster_data_set_page_count(2);
    }
    cluster_db_get_list(&cluster_data_s.data_list);
}

Eina_List *cluster_data_get_widget_list(void)
{
    return cluster_data_s.data_list;
}

int cluster_data_get_page_count()
{
    int result = -1;
    if (preference_get_int(CLUSTER_DATA_PAGE_COUNT, &result) != PREFERENCE_ERROR_NONE) {
        LOGE("preference_get_int : Error");
        result = -1;
    }
    return result;
}

void cluster_data_set_page_count(int count)
{
    if (preference_set_int(CLUSTER_DATA_PAGE_COUNT, count) != PREFERENCE_ERROR_NONE) {
        LOGE("preference_set_int(CLUSTER_DATA_PAGE_COUNT, %d): Error", count);
    }
}

void cluster_data_insert_widget(const char* pkg_name, const char* content_info, int type)
{
    widget_data_t *new_item = (widget_data_t *)malloc(sizeof(widget_data_t));
    memset(new_item, 0, sizeof(widget_data_t));

    new_item->page_idx = INIT_VALUE;
    new_item->pos_y = INIT_VALUE;
    new_item->pos_x = INIT_VALUE;
    new_item->pkg_name = strdup(pkg_name);
    new_item->content_info = content_info == NULL ? NULL : strdup(content_info);
    new_item->type = type;
    new_item->period = 0.0;
    new_item->allow_duplicate = 1;

    if (!cluster_view_add_widget(new_item, true)) {
        __cluster_data_item_free(new_item);
        return ;
    }
    cluster_data_insert(new_item);
}

void cluster_data_insert(widget_data_t *item)
{
    LOGD("%s - inserted to paged[%d]", item->pkg_name, item->page_idx);
    cluster_db_insert(item);
    cluster_data_s.data_list = eina_list_append(cluster_data_s.data_list, item);
}

void cluster_data_delete(widget_data_t *item)
{
    LOGD("%s - delete", item->pkg_name);
    cluster_db_delete(item);
    cluster_data_s.data_list = eina_list_remove(cluster_data_s.data_list, item);
    cluster_view_delete_widget(item);
    __cluster_data_item_free(item);
}

void cluster_data_update(widget_data_t *item)
{
    cluster_db_update(item);
}

static void __cluster_data_insert_default_data(void)
{
    /* org.tizen.gallery.widget
     * org.tizen.calendar.widget
     * org.tizen.music-player.widget
     * org.tizen.contacts-widget
    */
    widget_data_t *new_item = (widget_data_t *)malloc(sizeof(widget_data_t));
    memset(new_item, 0, sizeof(widget_data_t));
    new_item->page_idx = 0;
    new_item->pos_y = 0;
    new_item->pos_x = 0;
    new_item->pkg_name = strdup("org.tizen.gallery.widget");
    new_item->content_info = NULL;
    new_item->type = WIDGET_SIZE_TYPE_4x4;
    new_item->period = 0.0;
    new_item->allow_duplicate = 1;

    cluster_db_insert(new_item);
/*
    new_item = (widget_data_t *)malloc(sizeof(widget_data_t));
    memset(new_item, 0, sizeof(widget_data_t));

    new_item->page_idx = 1;
    new_item->pos_y = 0;
    new_item->pos_x = 0;
    new_item->pkg_name = strdup("org.tizen.gallery.widget");
    new_item->content_info = NULL;
    new_item->type = WIDGET_SIZE_TYPE_4x4;
    new_item->period = 0.0;
    new_item->allow_duplicate = 1;

    cluster_data_insert(new_item);
    */
}

static void __cluster_data_item_free(widget_data_t *item)
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
