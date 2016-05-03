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

#include "apps_data.h"
#include "apps_package_manager.h"
#include "apps_db.h"
#include "apps_view.h"
#include "util.h"

static struct {
    Eina_List *data_list;
} apps_data_s = {
    .data_list = NULL
};

void __apps_data_print(Eina_List *list);
static int __apps_sort_cb(const void *a , const void *b);
static void __apps_data_item_free(app_data_t *item);
app_data_t *__apps_data_find_item(int db_id);

void apps_data_init(void)
{
    Eina_List *pkg_list = NULL, *db_list = NULL;
    Eina_List *pkg_find_list, *db_find_list;
    app_data_t *pkg_item = NULL, *db_item = NULL;
    apps_package_manager_init();
    apps_package_manger_get_list(&pkg_list);

    if (!apps_db_create()) {
        apps_db_get_list(&db_list);
    }

    EINA_LIST_FOREACH(db_list, pkg_find_list, db_item) {
        if (db_item->is_folder) {
            apps_data_s.data_list = eina_list_append(apps_data_s.data_list, db_item);
        }
    }

    EINA_LIST_FOREACH(pkg_list, pkg_find_list, pkg_item) {
        EINA_LIST_FOREACH(db_list, db_find_list, db_item) {
            if (!strcmp(pkg_item->pkg_str, db_item->pkg_str) && !strcmp(pkg_item->owner, db_item->owner)) {
                pkg_item->db_id = db_item->db_id;
                pkg_item->parent_db_id = db_item->parent_db_id;
                db_item->temp = true;
                break;
            }
        }
        apps_data_s.data_list = eina_list_append(apps_data_s.data_list, pkg_item);
    }

    EINA_LIST_FOREACH(db_list, pkg_find_list, db_item) {
        if (!db_item->temp && !db_item->is_folder) {
            apps_db_delete(db_item);
        }
    }

    apps_data_sort();

    EINA_LIST_FOREACH(apps_data_s.data_list, pkg_find_list, pkg_item) {
        if (pkg_item->db_id == -1)
            apps_db_insert(pkg_item);
        else
            apps_db_update(pkg_item);
    }
    __apps_data_print(apps_data_s.data_list);
}

void apps_data_sort(void)
{
    int index = 0;
    int parent_id = APPS_ROOT;
    app_data_t *item = NULL;
    Eina_List *find_list;

    if (apps_data_s.data_list) {
        apps_data_s.data_list = eina_list_sort(apps_data_s.data_list, eina_list_count(apps_data_s.data_list), __apps_sort_cb);
    }
    EINA_LIST_FOREACH(apps_data_s.data_list, find_list, item) {
        if (item->parent_db_id == parent_id) {
            item->position = index++;
        } else {
            parent_id = item->parent_db_id;
            index = 0;
            item->position = index++;
        }
    }
}

Eina_List *apps_data_get_list(void)
{
    return apps_data_s.data_list;
}

void apps_data_get_folder_item_list(Eina_List **list, app_data_t *folder)
{
    app_data_t *item = NULL;
    Eina_List *find_list;

    EINA_LIST_FOREACH(apps_data_s.data_list, find_list, item) {
        if (item->parent_db_id == folder->db_id) {
            *list = eina_list_append(*list , item);
        }
    }
}

void apps_data_install(app_data_t *item)
{
    apps_data_s.data_list = eina_list_append(apps_data_s.data_list, item);
    apps_db_insert(item);
    apps_data_sort();
    apps_view_icon_add(item);
    apps_view_reroder();
}

void apps_data_uninstall(const char *package)
{
    Eina_List *find_list;
    app_data_t *item = NULL;

    EINA_LIST_FOREACH(apps_data_s.data_list, find_list, item) {
        if (item->pkg_str && (strcmp(item->pkg_str, package) == 0) &&
                item->owner && (strcmp(item->owner, TEMP_OWNER) == 0)) {
            apps_data_s.data_list = eina_list_remove_list(apps_data_s.data_list, find_list);
            apps_db_delete(item);
            apps_data_sort();
            apps_view_reroder();
            apps_view_folder_reroder();
            if (item->parent_db_id != APPS_ROOT) {
                app_data_t *parent = __apps_data_find_item(item->parent_db_id);
                if (parent)
                    apps_view_update_folder_icon(parent);
            }
            __apps_data_item_free(item);
            break;
        }
    }
}

static void __apps_data_item_free(app_data_t *item)
{
    if (item && item->app_layout)
        evas_object_del(item->app_layout);
    if (item && item->folder_layout)
        evas_object_del(item->folder_layout);
    if (item && item->label_str)
        free(item->label_str);
    if (item && item->icon_path_str)
        free(item->icon_path_str);
    if (item && item->pkg_str)
        free(item->pkg_str);
    if (item && item->owner)
        free(item->owner);
    if (item)
        free(item);

}

static int __apps_sort_cb(const void *a , const void *b)
{
    int i;
    app_data_t *item1 = (app_data_t *)a;
    app_data_t *item2 = (app_data_t *)b;
    if (item1->parent_db_id < item2->parent_db_id)
        return -1;
    else if (item1->parent_db_id > item2->parent_db_id)
        return 1;

    if (item1->label_str == NULL && item2->label_str == NULL)
        return 1;
    else if (item1->label_str == NULL)
        return -1;
    else if (item2->label_str == NULL)
        return 1;

    for (i = 0; item1->label_str[i]; i++) {
        if (tolower(item1->label_str[i]) != tolower(item2->label_str[i])) {
            int ret = tolower(item1->label_str[i]) - tolower(item2->label_str[i]);
            return ret;
        }
    }

    return item2->label_str[i] ? -1 : (item2->db_id - item1->db_id);
}

app_data_t *apps_data_add_folder(void)
{
    app_data_t *new_item = (app_data_t *)malloc(sizeof(app_data_t));
    memset(new_item, 0, sizeof(app_data_t));

    new_item->db_id = -1;
    new_item->parent_db_id = APPS_ROOT;
    new_item->owner = strdup(TEMP_OWNER);
    new_item->position = -1;
    new_item->label_str = strdup("");

    new_item->is_checked = false;
    new_item->is_folder = true;
    new_item->is_removable = true;
    new_item->is_system = false;

    apps_data_s.data_list = eina_list_append(apps_data_s.data_list, new_item);

    apps_db_insert(new_item);
    apps_view_icon_add(new_item);
    apps_data_sort();
    apps_view_reroder();

    return new_item;
}

void apps_data_delete_folder(app_data_t *folder_item)
{
    app_data_t *item = NULL;
    Eina_List *find_list;
    EINA_LIST_FOREACH(apps_data_s.data_list, find_list, item) {
        if (item->parent_db_id == folder_item->db_id) {
            item->parent_db_id = APPS_ROOT;
            apps_db_update(item);
            apps_view_icon_add(item);
        }
    }
    apps_data_s.data_list = eina_list_remove(apps_data_s.data_list, folder_item);
    apps_db_delete(folder_item);
    apps_data_sort();
    apps_view_reroder();
    __apps_data_item_free(folder_item);
}

void apps_data_update_folder(app_data_t *folder_item)
{
    apps_db_update(folder_item);
    apps_data_sort();
    apps_view_reroder();
}

void __apps_data_print(Eina_List *list)
{
    app_data_t *item = NULL;
    Eina_List *find_list;
    LOGD("========================================");
    EINA_LIST_FOREACH(list, find_list, item) {
        if (item != NULL)
            LOGD("%d [pkg: %s][name:%s][iconPath: %s][icon:%p]", item->position, item->pkg_str, item->label_str, item->icon_path_str, item->app_layout);
    }
    LOGD("========================================");
}

app_data_t *__apps_data_find_item(int db_id)
{
    app_data_t *item = NULL;
    Eina_List *find_list;
    EINA_LIST_FOREACH(apps_data_s.data_list, find_list, item) {
        if (item->db_id == db_id)
            return item;
    }
    return NULL;
}
