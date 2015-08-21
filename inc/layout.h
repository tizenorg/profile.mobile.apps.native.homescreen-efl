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

#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <Elementary.h>
#include <stdbool.h>

/**
 * @brief Sets text in caption menu
 * If a parameter is NULL, that parameter won't be set
 */
extern void layout_main_caption_set_menu_texts(
		const char *label_text,
		const char *left_text,
		const char *right_text);


#endif /* LAYOUT_H_ */
