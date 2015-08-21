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

#include "homescreen-efl.h"
#include "layout.h"
#include "conf.h"
#include "util.h"


HAPI void layout_main_caption_set_menu_texts(const char *label_text,
	const char *left_text, const char *right_text)
{
	Evas_Object *layout = home_screen_get_layout();

	if (!layout)
		return;

	if (label_text)
		elm_layout_text_set(layout, PART_CAPTION_MENU_CENTER_LABEL_NAME, label_text);

	if (left_text)
		elm_layout_text_set(layout, PART_CAPTION_MENU_LEFT_LABEL, left_text);

	if (right_text)
		elm_layout_text_set(layout, PART_CAPTION_MENU_RIGHT_LABEL, right_text);
}
