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

#ifndef __MOUSE_H__
#define __MOUSE_H__

#define MOUSE_MOVE_MIN_DISTANCE 100

typedef struct {
	bool pressed;
	bool long_pressed;
	Evas_Coord down_x;
	Evas_Coord down_y;
	Evas_Coord move_x;
	Evas_Coord move_y;
	Evas_Coord up_x;
	Evas_Coord up_y;
	int offset_x;
	int offset_y;
	Ecore_Timer *long_press_timer;
	Evas_Object *pressed_obj;
} mouse_info_t;

#endif /* __MOUSE_H__ */
