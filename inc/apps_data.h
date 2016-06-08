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

#ifndef __APPS_DATA_H__
#define __APPS_DATA_H__

#include <app.h>
#include <Elementary.h>

#define APPS_ROOT -1
#define TEMP_OWNER "owner"

typedef struct {
	int db_id;
	int parent_db_id;
	char* owner;
	Eina_Bool is_folder;
	int position;
	char* app_id;
	char* pkg_id;
	char* label_str;
	char* icon_path_str;
	char* uri;
	int type;
	bool is_removable;
	bool is_system;
	Evas_Object *app_layout;
	Evas_Object *folder_layout;
	unsigned int badge_count;
	Eina_Bool is_checked;
	bool temp;
} app_data_t;

enum {
	APPS_DATA_TYPE_APP = 0,
	APPS_DATA_TYPE_APP_SHORTCUT,
	APPS_DATA_TYPE_URI_SHORTCUT
};

void apps_data_init(void *data, Ecore_Thread *th);
void apps_data_sort(void);
Eina_List *apps_data_get_list(void);
void apps_data_get_folder_item_list(Eina_List **list, app_data_t *folder);
void apps_data_install(app_data_t *item);
void apps_data_uninstall(const char *package);
app_data_t *apps_data_add_folder(void);
void apps_data_delete_folder(app_data_t *folder_item);
void apps_data_update_folder(app_data_t *folder_item);
void apps_data_delete_item(app_data_t *item);
void apps_data_delete_list(Eina_List *list);
app_data_t *apps_data_find_item_by_index(int index);
int apps_data_get_folder_item_count(app_data_t *folder);
void apps_data_update_item_label(void);

#endif /* __APPS_DATA_H__ */
