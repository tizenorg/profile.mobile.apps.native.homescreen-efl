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

#include <app_manager.h>
#include <package_manager.h>
#include "homescreen-efl.h"
#include "app_mgr.h"
#include "util.h"
#include "data_model.h"
#include "app_icon.h"

static struct
{
	package_manager_h pack_mgr;
} s_app_mgr_info = {
	.pack_mgr = NULL,
};

static bool __app_info_cb(app_info_h ai, void *ud);
static void __app_mgr_event_cb(const char *type, const char *package,
		package_manager_event_type_e event_type,
		package_manager_event_state_e event_state,
		int progress,
		package_manager_error_e error,
		void *user_data);

static app_mgr_item_t *__app_mgr_load_item_info(app_info_h ai_h);

HAPI void app_mgr_init(void)
{
	package_manager_error_e res_stat = PACKAGE_MANAGER_ERROR_NONE;
	package_manager_error_e res_cb = PACKAGE_MANAGER_ERROR_NONE;

	if (s_app_mgr_info.pack_mgr)
		return;

	if (PACKAGE_MANAGER_ERROR_NONE ==
		package_manager_create(&s_app_mgr_info.pack_mgr)) {
		LOGD("PACKAGE_EVENT_MGR - handle obtained");

		res_stat = package_manager_set_event_status(
			s_app_mgr_info.pack_mgr,
			PACKAGE_MANAGER_STATUS_TYPE_INSTALL |
			PACKAGE_MANAGER_STATUS_TYPE_UNINSTALL);

		res_cb = package_manager_set_event_cb(
		s_app_mgr_info.pack_mgr, __app_mgr_event_cb, NULL);

		if (res_stat != PACKAGE_MANAGER_ERROR_NONE ||
			res_cb != PACKAGE_MANAGER_ERROR_NONE)
			LOGE("App mgr initialization failed!");
	} else {
		s_app_mgr_info.pack_mgr = NULL;
	}
}

HAPI void app_mgr_deinit(void)
{
	if (s_app_mgr_info.pack_mgr) {
		LOGD("PACKAGE_EVENT_MGR - freeing handle");
		package_manager_unset_event_cb(s_app_mgr_info.pack_mgr);
		package_manager_destroy(s_app_mgr_info.pack_mgr);
	}
}

HAPI Eina_List *app_mgr_get_apps(void)
{
	Eina_List *apps = NULL;
	app_manager_foreach_app_info(__app_info_cb, &apps);
	return apps;
}

HAPI void app_mgr_free_apps(Eina_List *apps)
{
	app_mgr_item_t *app_mgr_item;
	EINA_LIST_FREE(apps, app_mgr_item)
		free(app_mgr_item);
}

HAPI Eina_Bool app_mgr_uninstall_app(const char *appid)
{
	LOGI("");

	package_manager_request_h request;
	int id = 0;

	if (package_manager_request_create(&request)
		!= PACKAGE_MANAGER_ERROR_NONE) {
		LOGE("Could not create unistall request. App: %s", appid);
		return EINA_FALSE;
	}

	int ret = package_manager_request_set_mode(request, PACKAGE_MANAGER_REQUEST_MODE_DEFAULT);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		LOGE("Could not set request mode. App: %s", appid);
		return EINA_FALSE;
	}

	if (package_manager_request_uninstall(request, appid, &id) !=
		PACKAGE_MANAGER_ERROR_NONE) {
		LOGE("Could not uninstall application. App: %s", appid);
		return EINA_FALSE;
	}

	if (package_manager_request_destroy(request) !=
		PACKAGE_MANAGER_ERROR_NONE) {
		LOGE("Could not destroy unistall request. App: %s", appid);
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

HAPI Eina_Bool app_mgr_app_get_badge_count(const char *app_id,
	unsigned int *count)
{
	unsigned int to_be_displayed = 0;
	int result = BADGE_ERROR_NONE;

	if (!app_id) {
		LOGE("app_item is NULL in badge count");
		return EINA_FALSE;
	}

	*count = 0;
	result = badge_get_display(app_id, &to_be_displayed);

	if (result != BADGE_ERROR_NONE) {
		if ((result == BADGE_ERROR_SERVICE_NOT_READY ||
			result == BADGE_ERROR_NOT_EXIST))
			return true;

		LOGE("badge_get_display error %d", result);
		return false;
	}

	if (!to_be_displayed)
		return EINA_TRUE;

	result = badge_get_count(app_id, count);

	if (result != BADGE_ERROR_NONE) {
		*count = 0;
		if ((result == BADGE_ERROR_SERVICE_NOT_READY ||
			result == BADGE_ERROR_NOT_EXIST))
			return true;

		LOGE("badge_get_count error %d", result);
		return false;
	}

	return EINA_TRUE;
}

HAPI void app_mgr_register_badge_callback(badge_change_cb callback)
{
	int ret = BADGE_ERROR_NONE;
	ret = badge_register_changed_cb(callback, NULL);

	if (ret != BADGE_ERROR_NONE)
		LOGE("Could not register badge callback");
}

HAPI void app_mgr_unregister_badge_callback(badge_change_cb callback)
{
	int ret = BADGE_ERROR_NONE;
	ret = badge_unregister_changed_cb(callback);

	if (ret != BADGE_ERROR_NONE)
		LOGE("Could not unregister badge callback");
}

HAPI char *app_mgr_get_app_label(const char *app_id)
{
	app_info_h ai_handle;
	char *label = NULL;

	if (!app_id) {
		LOGE("Invalid parameter");
		return NULL;
	}

	if (app_info_create(app_id, &ai_handle) != APP_MANAGER_ERROR_NONE) {
		LOGE("Failed to load application id");
		return NULL;
	}

	if (app_info_get_label(ai_handle, &label) != APP_MANAGER_ERROR_NONE) {
		LOGE("Failed to get label!");
		app_info_destroy(ai_handle);
		return NULL;
	}

	app_info_destroy(ai_handle);
	return label;
}

static void __app_mgr_event_cb(const char *type, const char *package,
	package_manager_event_type_e event_type,
	package_manager_event_state_e event_state, int progress,
	package_manager_error_e error, void *user_data)
{
	if (PACKAGE_MANAGER_ERROR_NONE != error) {
		LOGE("PACKAGE_EVENT_MGR error in cb");
		return;
	}

	if (PACKAGE_MANAGER_EVENT_STATE_FAILED == event_state)
		progress = 0;

	if (PACKAGE_MANAGER_EVENT_TYPE_INSTALL == event_type && progress == 100) {
		LOGD("%s", package);
		app_info_h ai_handle = NULL;
		app_mgr_item_t *app_mgr_item = NULL;
		Tree_node_t *item = NULL;

		app_info_create(package, &ai_handle);
		if (!ai_handle) {
			LOGE("Failed to get app info handle");
			return;
		}

		app_mgr_item = __app_mgr_load_item_info(ai_handle);
		item = data_model_install_application(app_mgr_item);
		if (!item) {
			LOGE("Failed to create app item");
			app_info_destroy(ai_handle);
			return;
		}

		item->data->layout = app_icon_create(item, APP_ICON_TYPE_APP);
		app_info_destroy(ai_handle);
		home_screen_mvc_update_view();

	}

	if (PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL == event_type && progress == 100)
		LOGI("%s", package);
}

static bool __app_info_cb(app_info_h ai, void *ud)
{
	app_mgr_item_t *app_mgr_item = NULL;
	bool nodisplay = false;
	Eina_List **list = (Eina_List **)ud;

	if (app_info_is_nodisplay(ai, &nodisplay)) {
		LOGE("[FAILED][app_info_is_nodisplay]");
		return false;
	}

	#ifndef HOME_SCREEN_EFL_TEST_RUN
		if (nodisplay)
			return true;
	#endif

	app_mgr_item = __app_mgr_load_item_info(ai);

	if (!app_mgr_item) {
		LOGE("[Failed to initialie app_mgr_item]");
		return false;
	}


	*list = eina_list_append(*list, app_mgr_item);
	return true;
}

static app_mgr_item_t *__app_mgr_load_item_info(app_info_h ai_h)
{
	package_info_h p_handle = NULL;
	package_manager_error_e ret = PACKAGE_MANAGER_ERROR_NONE;

	if (!ai_h) {
		LOGE("[INVALID_PARAMS]");
		return NULL;
	}

	app_mgr_item_t *item = (app_mgr_item_t *) calloc(1, sizeof(*item));
	if (!item) {
		LOGE("Failed to create app_mgr_item");
		return NULL;
	}

	if (app_info_get_label(ai_h, &item->label)) {
		LOGE("[FAILED][app_info_get_label]");
		free(item);
		return NULL;
	}

	if (app_info_get_exec(ai_h, &item->exec)) {
		LOGE("[FAILED][app_info_get_exec]");
		free(item->label);
		free(item);
		return NULL;
	}

	if (app_info_get_icon(ai_h, &item->icon)) {
		LOGE("[FAILED][app_info_get_icon]");
		free(item->label);
		free(item->exec);
		free(item);
		return NULL;
	}

	if (app_info_get_app_id(ai_h, &item->appid)) {
		LOGE("[FAILED][app_info_get_app_id]");
		free(item->label);
		free(item->exec);
		free(item->icon);
		free(item);
		return NULL;
	}

	if (app_info_get_package(ai_h, &item->package)) {
		LOGE("[FAILED][app_info_get_package]");
		free(item->label);
		free(item->exec);
		free(item->icon);
		free(item->appid);
		free(item);
		return NULL;
	}

	ret = package_manager_get_package_info(item->package, &p_handle);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		LOGW("Failed to inialize package handle for item : %s",
			item->package);
		item->removable = false;
		return item;
	}

	ret = package_info_is_removable_package(p_handle, &item->removable);

	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		LOGE("Failed to get pacakge removeable flag");
		free(item->label);
		free(item->exec);
		free(item->icon);
		free(item->appid);
		free(item->package);
		free(item);
		return NULL;
	}

	package_info_destroy(p_handle);

	return item;
}
