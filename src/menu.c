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

#include "homescreen-efl.h"
#include "menu.h"
#include "conf.h"

static struct {
	Evas_Object *menu;
	bool is_visible;
} menu_info = {
	.menu = NULL,
	.is_visible = false
};

/* menu_item_t */
static const char menu_text[MENU_MAX][STR_ID_LEN] = {
		"IDS_HS_OPT_EDIT",
		"IDS_HS_HEADER_ADD_WIDGET",
		"IDS_HS_OPT_CHANGE_WALLPAPER_ABB",
		"IDS_HS_OPT_ALL_PAGES",
		"IDS_HS_OPT_EDIT",
		"IDS_HS_OPT_CREATE_FOLDER_OPT_TTS"
};

static void __menu_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	menu_info.is_visible = false;

	if (menu_info.menu) {
		evas_object_del(menu_info.menu);
		menu_info.menu = NULL;
	}
}

void menu_show(Eina_Hash* hash_table)
{
	int menu_id;
	if (menu_info.is_visible) {
		return ;
	}

	menu_info.is_visible = true;

	menu_info.menu = elm_ctxpopup_add(homescreen_efl_get_win());
	elm_object_style_set(menu_info.menu, "more/default");
	elm_ctxpopup_horizontal_set(menu_info.menu, EINA_FALSE);
	elm_ctxpopup_auto_hide_disabled_set(menu_info.menu, EINA_TRUE);
	elm_ctxpopup_direction_priority_set(menu_info.menu, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN);

	evas_object_smart_callback_add(menu_info.menu, "dismissed", __menu_dismissed_cb, NULL);

	for (menu_id = MENU_DEFAULT+1; menu_id < MENU_MAX; menu_id++) {
		void *data = eina_hash_find(hash_table, &menu_id);
		if (data) {
			elm_ctxpopup_item_append(menu_info.menu, _(menu_text[menu_id]), NULL, data, NULL);
		}
	}

	evas_object_move(menu_info.menu, 0, WINDOW_H);
	evas_object_show(menu_info.menu);
}

void menu_hide(void)
{
	if (!menu_info.is_visible) {
		return ;
	}

	menu_info.is_visible = false;

	evas_object_hide(menu_info.menu);
	evas_object_del(menu_info.menu);
	menu_info.menu = NULL;
}

bool menu_is_show(void)
{
	return menu_info.is_visible;
}

void menu_change_state_on_hw_menu_key(Eina_Hash* hash_table)
{
	if (menu_info.is_visible) {
		menu_hide();
	} else {
		menu_show(hash_table);
	}
}
