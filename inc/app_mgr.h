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

#ifndef APP_MGR_H_
#define APP_MGR_H_

#include <badge.h>
#include "util.h"

typedef struct {
	char *label;
	char *exec;
	char *icon;
	char *appid;
	char *package;
	bool removable;
} app_mgr_item_t;

typedef void (*package_event_manager_event_callback_t)(const char *app_info,
	int progress);

/**
 * @brief Initialize application manager
 */
extern void app_mgr_init(void);

/**
 * @brief Deinintialize application manager
 */
extern void app_mgr_deinit(void);

/**
 * @brief Gets the application list
 */
extern Eina_List *app_mgr_get_apps(void);

/**
 * @brief Destroys application list
 */
extern void app_mgr_free_apps(Eina_List *apps);

/**
 * @brief Uninstall an application from the platform
 */
extern Eina_Bool app_mgr_uninstall_app(const char *appid);

/**
 * @brief gets badge count
 * @param app_id - application id
 * @param count - badges count
 * returns EINA_FALSE on failure
 */
extern Eina_Bool app_mgr_app_get_badge_count(const char *app_id,
	unsigned int *count);

/**
* @brief registers listener for badge status changes
* @param callback - callback function to call on badge change
*/
extern void app_mgr_register_badge_callback(badge_change_cb callback);

/**
* @brief uregisters listener for badge status changes
* @param callback - callback function previously registered
*/
extern void app_mgr_unregister_badge_callback(badge_change_cb callback);

/**
 * @brief Returns application label
 * @details Function is used only for update application label after language change.
 * @param app_id Application id.
 */
char *app_mgr_get_app_label(const char *app_id);

#endif /* APP_MGR_H_ */
