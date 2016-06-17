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

#include "hw_key.h"
#include "util.h"
#include "conf.h"
#include "homescreen-efl.h"
#include "menu.h"
#include "popup.h"

#define KEY_HOME "XF86Home"
#define KEY_MENU "XF86Menu"
#define KEY_BACK "XF86Back"
#define KEY_RETURN "Return"

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
} hw_key_info = {
	.pressed = EINA_FALSE,
	.press_handler = NULL,
	.release_handler = NULL,
	.register_handler = EINA_FALSE
};

static Eina_Bool __hw_key_release_cb(void *data, int type, void *event);
static Eina_Bool __hw_key_press_cb(void *data, int type, void *event);

void hw_key_register(void)
{
	if (!hw_key_info.release_handler) {
		hw_key_info.release_handler = ecore_event_handler_add(ECORE_EVENT_KEY_UP, __hw_key_release_cb, NULL);
		if (!hw_key_info.release_handler)
			LOGE("Failed to register a key up event handler");
	}

	if (!hw_key_info.press_handler) {
		hw_key_info.press_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, __hw_key_press_cb, NULL);
		if (!hw_key_info.press_handler)
			LOGE("Failed to register a key down event handler");
	}

	hw_key_info.pressed = EINA_FALSE;
	hw_key_info.register_handler = EINA_TRUE;
}

void hw_key_unregister(void)
{
	if (hw_key_info.release_handler) {
		ecore_event_handler_del(hw_key_info.release_handler);
		hw_key_info.release_handler = NULL;
	}

	if (hw_key_info.press_handler) {
		ecore_event_handler_del(hw_key_info.press_handler);
		hw_key_info.press_handler = NULL;
	}

	hw_key_info.register_handler = EINA_FALSE;
}

static Eina_Bool __hw_key_press_cb(void *data, int type, void *event)
{
	Evas_Event_Key_Down *ev = event;

	if (!hw_key_info.register_handler || !ev)
		return ECORE_CALLBACK_RENEW;

	hw_key_info.pressed = EINA_TRUE;

	return ECORE_CALLBACK_RENEW;
}


static Eina_Bool __hw_key_release_cb(void *data, int type, void *event)
{
	Evas_Event_Key_Up *ev = event;

	if (!hw_key_info.register_handler || !ev)
		return ECORE_CALLBACK_RENEW;

	LOGD("Key(%s) released %d", ev->keyname, hw_key_info.pressed);

	if (hw_key_info.pressed == EINA_FALSE)
		return ECORE_CALLBACK_RENEW;

	if (!strcmp(ev->keyname, KEY_BACK)) {
		if (menu_is_show()) {
			menu_hide();
		} else if (popup_is_show()) {
			popup_hide();
		} else {
			homescreen_efl_hw_back_key_release();
		}
	} else if (!strcmp(ev->keyname, KEY_MENU)) {
		homescreen_efl_hw_menu_key_release();
	} else {
		hw_key_info.pressed = EINA_FALSE;

		return ECORE_CALLBACK_RENEW;
	}

	hw_key_info.pressed = EINA_FALSE;

	return ECORE_CALLBACK_RENEW;
}
