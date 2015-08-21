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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Elementary.h>

#include "livebox/livebox_panel.h"
#include "util.h"
#include "livebox_all_pages.h"
#include "folder_panel.h"


static struct {
	Ecore_Event_Handler *mouse_down;
	Ecore_Event_Handler *mouse_up;
	Ecore_Event_Handler *mouse_move;
	bool pressed;
	Evas_Coord pre_x;
	Evas_Coord pre_y;
	bool is_initialized;
	Evas_Coord down_x;
	Evas_Coord down_y;
	Evas_Coord move_x;
	Evas_Coord move_y;
	Evas_Coord up_x;
	Evas_Coord up_y;
} mouse_info = {
	.mouse_down = NULL,
	.mouse_up = NULL,
	.mouse_move = NULL,
	.pressed = false,
	.pre_x = 0,
	.pre_y = 0,
	.is_initialized = false,
	.down_x = 0,
	.down_y = 0,
	.move_x = 0,
	.move_y = 0,
	.up_x = 0,
	.up_y = 0,
};


static Eina_Bool __mouse_down_cb(void *data, int type, void *event);
static Eina_Bool __mouse_up_cb(void *data, int type, void *event);
static Eina_Bool __mouse_move_cb(void *data, int type, void *event);


HAPI void mouse_register(void)
{
	mouse_info.mouse_down = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, __mouse_down_cb, NULL);
	if (!mouse_info.mouse_down)
		LOGD("Failed to register the mouse down event callback");

	mouse_info.mouse_move = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, __mouse_move_cb, NULL);
	if (!mouse_info.mouse_move) {
		LOGD("Failed to register the mouse move event callback");
		ecore_event_handler_del(mouse_info.mouse_down);
		mouse_info.mouse_down = NULL;
	}

	mouse_info.mouse_up = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, __mouse_up_cb, NULL);
	if (!mouse_info.mouse_up) {
		LOGD("Failed to register the mouse up event callback");
		ecore_event_handler_del(mouse_info.mouse_down);
		ecore_event_handler_del(mouse_info.mouse_move);

		mouse_info.mouse_down = NULL;
		mouse_info.mouse_move = NULL;
	}
}


HAPI void mouse_unregister(void)
{
	if (mouse_info.mouse_down) {
		ecore_event_handler_del(mouse_info.mouse_down);
		mouse_info.mouse_down = NULL;
	}

	if (mouse_info.mouse_up) {
		ecore_event_handler_del(mouse_info.mouse_up);
		mouse_info.mouse_up = NULL;
	}

	if (mouse_info.mouse_move) {
		ecore_event_handler_del(mouse_info.mouse_move);
		mouse_info.mouse_move = NULL;
	}
}


static Eina_Bool __mouse_down_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Button *move = event;

	if (mouse_info.pressed)
		return ECORE_CALLBACK_RENEW;

	mouse_info.pressed = true;
	mouse_info.is_initialized = false;

	mouse_info.down_x = move->root.x;
	mouse_info.down_y = move->root.y;

	return ECORE_CALLBACK_RENEW;
}


static Eina_Bool __mouse_up_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Button *move = event;

	if (!mouse_info.pressed)
		return ECORE_CALLBACK_RENEW;

	mouse_info.pressed = false;
	mouse_info.pre_x = 0;
	mouse_info.pre_y = 0;

	mouse_info.up_x = move->root.x;
	mouse_info.up_y = move->root.y;

	livebox_all_pages_up_item();

	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool __mouse_move_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Move *move = event;

	mouse_info.move_x = move->root.x;
	mouse_info.move_y = move->root.y;

	if (mouse_info.pressed == false)
		return ECORE_CALLBACK_RENEW;

	livebox_all_pages_move_item(move->root.x, move->root.y);
	livebox_panel_move_mouse_cb(move->root.x, move->root.y);

	return ECORE_CALLBACK_RENEW;
}




