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

#include <Elementary.h>
#include <Ecore.h>
#include <Eina.h>

#include "homescreen-efl.h"
#include "option_menu.h"
#include "livebox/livebox_panel.h"
#include "livebox_all_pages.h"
#include "folder_panel.h"
#include "popup.h"
#include "util.h"

static Eina_Bool __key_release_cb(void *data, int type, void *event);
static Eina_Bool __key_press_cb(void *data, int type, void *event);

typedef enum {
	HW_KEY_NONE = -1,
	HW_KEY_BACK,
	HW_KEY_HOME,
	HW_KEY_MENU,
} hw_key_t;

static struct {
	Eina_Bool pressed;
	Ecore_Event_Handler *press_handler;
	Ecore_Event_Handler *release_handler;
	Eina_Bool register_handler;
} key_info_s = {
	.pressed = EINA_FALSE,
	.press_handler = NULL,
	.release_handler = NULL,
	.register_handler = EINA_FALSE
};


HAPI void key_register(void)
{
	if (!key_info_s.release_handler) {
		key_info_s.release_handler = ecore_event_handler_add(ECORE_EVENT_KEY_UP, __key_release_cb, NULL);
		if (!key_info_s.release_handler)
			LOGE("Failed to register a key up event handler");
	}

	if (!key_info_s.press_handler) {
		key_info_s.press_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, __key_press_cb, NULL);
		if (!key_info_s.press_handler)
			LOGE("Failed to register a key down event handler");
	}

	key_info_s.pressed = EINA_FALSE;
	key_info_s.register_handler = EINA_TRUE;
}


HAPI void key_unregister(void)
{
	if (key_info_s.release_handler) {
		ecore_event_handler_del(key_info_s.release_handler);
		key_info_s.release_handler = NULL;
	}

	if (key_info_s.press_handler) {
		ecore_event_handler_del(key_info_s.press_handler);
		key_info_s.press_handler = NULL;
	}

	key_info_s.register_handler = EINA_FALSE;
}


static Eina_Bool __key_press_cb(void *data, int type, void *event)
{
	Evas_Event_Key_Down *ev = event;

	if (!key_info_s.register_handler || !ev)
		return ECORE_CALLBACK_RENEW;

	key_info_s.pressed = EINA_TRUE;

	return ECORE_CALLBACK_RENEW;
}


static Eina_Bool __key_release_cb(void *data, int type, void *event)
{
	Evas_Event_Key_Up *ev = event;
	hw_key_t pressed_key = HW_KEY_NONE;

	if (!key_info_s.register_handler || !ev)
		return ECORE_CALLBACK_RENEW;

	LOGD("Key(%s) released %d", ev->keyname, key_info_s.pressed);

	if (key_info_s.pressed == EINA_FALSE)
		return ECORE_CALLBACK_RENEW;

	if (!strcmp(ev->keyname, KEY_BACK))
		pressed_key = HW_KEY_BACK;
	else if (!strcmp(ev->keyname, KEY_MENU))
		pressed_key = HW_KEY_MENU;
	else if(!strcmp(ev->keyname, KEY_HOME))
		pressed_key = HW_KEY_HOME;
	else {
		key_info_s.pressed = EINA_FALSE;

		return ECORE_CALLBACK_RENEW;
	}

	if (pressed_key == HW_KEY_MENU) {
		LOGD("Menu pressed");
		option_menu_change_state_on_hw_menu_key();

		return ECORE_CALLBACK_RENEW;
	}

	homescreen_view_t view_t = home_screen_get_view_type();

	if (pressed_key == HW_KEY_BACK || pressed_key == HW_KEY_HOME) {

		option_menu_hide();
		if (popup_destroy()) {
			/*we need to close only popup if it is visible. Dont change the view type.*/
			return ECORE_CALLBACK_RENEW;
		}

		if (folder_panel_is_folder_visible()) {
			folder_panel_close_folder();
			return ECORE_CALLBACK_RENEW;
		}

		switch (view_t) {
		case HOMESCREEN_VIEW_HOME:
			break;
		case HOMESCREEN_VIEW_HOME_EDIT:
			home_screen_set_view_type(HOMESCREEN_VIEW_HOME);
			/*TODO: option menu should not have any info about livebox panel. This logic should be invoked in homescreen-efl.c*/
			livebox_panel_set_edit_mode_layout(false);
			livebox_panel_change_edit_mode_state(true);
			break;
		case HOMESCREEN_VIEW_HOME_ALL_PAGES:
			/*TODO: this should be invoked in homescreen-efl.c*/
			livebox_all_pages_hide();
			home_screen_set_view_type(HOMESCREEN_VIEW_HOME);
			break;
		case HOMESCREEN_VIEW_ALL_APPS:
			home_screen_set_view_type(HOMESCREEN_VIEW_HOME);
			break;
		case HOMESCREEN_VIEW_ALL_APPS_CHOOSE:
			home_screen_close_all_apps_choose_view();
			if(pressed_key == HW_KEY_HOME)
				home_screen_set_view_type(HOMESCREEN_VIEW_HOME);
			break;
		case HOMESCREEN_VIEW_ALL_APPS_EDIT:
			home_screen_set_view_type(HOMESCREEN_VIEW_ALL_APPS);
			if(pressed_key ==  HW_KEY_HOME)
				home_screen_set_view_type(HOMESCREEN_VIEW_HOME);
			break;
		case HOMESCREEN_VIEW_UNKNOWN:
			break;
		default:
			break;
		}
	}

	if (!strcmp(ev->keyname, KEY_RETURN))
		folder_panel_hide_input_panel();


	key_info_s.pressed = EINA_FALSE;

	return ECORE_CALLBACK_RENEW;
}


