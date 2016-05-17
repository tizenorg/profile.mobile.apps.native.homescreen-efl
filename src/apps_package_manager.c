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

#include "apps_package_manager.h"
#include "apps_data.h"
#include "util.h"
#include "conf.h"

static package_manager_h pkg_mgr = NULL;

static void __apps_package_manager_event_cb(const char *type, const char *package,
        package_manager_event_type_e event_type, package_manager_event_state_e event_state, int progress,
        package_manager_error_e error, void *user_data);
static bool __apps_data_pkg_get_apps_info(app_info_h app_handle, app_data_t **item);
static bool __apps_package_manager_get_item(app_info_h app_handle, void *data);
static void __apps_package_manager_install(const char *package);
static void __apps_package_manager_uninstall(const char *package);

void apps_package_manager_init()
{
    int ret;
    if (pkg_mgr != NULL)
        return;

    ret = package_manager_create(&pkg_mgr);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        LOGE("package_manager_create : failed[%d]", ret);
    }

    ret = package_manager_set_event_status(pkg_mgr, PACKAGE_MANAGER_STATUS_TYPE_INSTALL|PACKAGE_MANAGER_STATUS_TYPE_UNINSTALL);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        LOGE("package_manager_set_event_status : failed[%d]", ret);
    }

    ret = package_manager_set_event_cb(pkg_mgr, __apps_package_manager_event_cb, NULL);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        LOGE("package_manager_set_event_cb : failed[%d]", ret);
    }
}

static void __apps_package_manager_event_cb(const char *type, const char *package,
        package_manager_event_type_e event_type, package_manager_event_state_e event_state, int progress,
        package_manager_error_e error, void *user_data)
{
    if (event_state == PACKAGE_MANAGER_EVENT_STATE_STARTED) {
        LOGI("pkg:%s type:%d state:PACKAGE_MANAGER_EVENT_STATE_STARTED", package, event_type);
    } else if (event_state == PACKAGE_MANAGER_EVENT_STATE_PROCESSING) {
        LOGI("pkg:%s type:%d PACKAGE_MANAGER_EVENT_STATE_PROCESSING", package, event_type);
    } else if (event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED) {
        LOGI("pkg:%s type:%d PACKAGE_MANAGER_EVENT_STATE_COMPLETED", package, event_type);
        if (event_type == PACKAGE_MANAGER_EVENT_TYPE_INSTALL) {
            __apps_package_manager_install(package);
        } else if (event_type == PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL) {
            __apps_package_manager_uninstall(package);
        } else { //PACKAGE_MANAGER_EVENT_TYPE_UPDATE
            LOGD("UPDATE - %s", package);
        }
    } else {
        LOGE("pkg:%s type:%d state:PACKAGE_MANAGER_EVENT_STATE_COMPLETED: FAILED", package, event_type);
    }
}

Eina_Bool apps_package_manger_get_list(Eina_List **list)
{
    int ret;
    ret = app_manager_foreach_app_info(__apps_package_manager_get_item, list);
    if (ret != PACKAGE_MANAGER_ERROR_NONE)
        LOGE("package_manager_foreach_package_info failed[%d]", ret);
    return true;
}

static bool __apps_package_manager_get_item(app_info_h app_handle, void *data)
{
    Eina_List **list = (Eina_List **)data;
    app_data_t *item = NULL;
    if (__apps_data_pkg_get_apps_info(app_handle, &item)) {
        *list = eina_list_append(*list, item);
    }
    return true;
}

static bool __apps_data_pkg_get_apps_info(app_info_h app_handle, app_data_t **item)
{
    char *pkg = NULL;
    bool nodisplay = false;
    int ret;
    package_info_h p_handle = NULL;

    app_data_t *new_item = (app_data_t *)malloc(sizeof(app_data_t));
    memset(new_item, 0, sizeof(app_data_t));
    *item = new_item;

    app_info_is_nodisplay(app_handle, &nodisplay);
    if (nodisplay)
        goto ERROR;

    new_item->db_id = INIT_VALUE;
    new_item->parent_db_id = APPS_ROOT;
    new_item->owner = strdup(TEMP_OWNER);
    new_item->is_folder = false;
    new_item->position = INIT_VALUE;
    new_item->uri = NULL;
    new_item->type = APPS_DATA_TYPE_APP;

    ret = app_info_get_app_id(app_handle, &new_item->pkg_str);
    if (APP_MANAGER_ERROR_NONE != ret) {
        LOGE("app_info_get_app_id return [%d] %s", ret, new_item->pkg_str);
        goto ERROR;
    }

    ret = app_info_get_label(app_handle, &new_item->label_str);
    if (APP_MANAGER_ERROR_NONE != ret) {
        LOGE("app_info_get_label return [%d] %s", ret, new_item->label_str);
        goto ERROR;
    }

    ret = app_info_get_icon(app_handle, &new_item->icon_path_str);
    if (APP_MANAGER_ERROR_NONE != ret) {
        LOGE("app_info_get_icon return [%d]", ret);
        goto ERROR;
    }

    ret = app_info_get_package(app_handle, &pkg);
    if (APP_MANAGER_ERROR_NONE != ret) {
        LOGE("app_info_get_icon return [%d]", ret);
        goto ERROR;
    }

    LOGD("%s", pkg);

    ret = package_manager_get_package_info(pkg, &p_handle);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        LOGE("Failed to inialize package handle for item : %s",
                pkg);
        goto ERROR;
    }

    ret = package_info_is_removable_package(p_handle, &new_item->is_removable);
    if (PACKAGE_MANAGER_ERROR_NONE != ret) {
        LOGE("package_info_is_removable_package  return [%d]", ret);
        goto ERROR;
    }

    ret = package_info_is_system_package(p_handle, &new_item->is_system);
    if (PACKAGE_MANAGER_ERROR_NONE != ret) {
        LOGE("package_info_is_system_package return [%d]", ret);
        goto ERROR;
    }

    if (!new_item->icon_path_str || !ecore_file_can_read(new_item->icon_path_str)) {
        if (new_item->icon_path_str) free(new_item->icon_path_str);

        new_item->icon_path_str = malloc(MAX_FILE_PATH_LEN);
        sprintf(new_item->icon_path_str, "%s", util_get_res_file_path(IMAGE_DIR"/default_app_icon.png"));
    }
    if (pkg)
        free(pkg);
    return true;

ERROR:
    if (pkg)
        free(pkg);
    if (new_item && new_item->label_str)
        free(new_item->label_str);
    if (new_item && new_item->icon_path_str)
        free(new_item->icon_path_str);
    if (new_item && new_item->pkg_str)
        free(new_item->pkg_str);
    if (new_item && new_item->owner)
        free(new_item->owner);
    if (new_item) free(new_item);
    return false;

}

static void __apps_package_manager_install(const char *package)
{
    app_info_h app_info = NULL;
    app_data_t *item = NULL;

    app_manager_get_app_info(package, &app_info);
    if (__apps_data_pkg_get_apps_info(app_info, &item)) {
        apps_data_install(item);
    }
    app_info_destroy(app_info);

}

static void __apps_package_manager_uninstall(const char *package)
{
    apps_data_uninstall(package);
}
