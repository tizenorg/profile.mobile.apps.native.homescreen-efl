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

#ifndef POPUP_H_
#define POPUP_H_

#include <Evas.h>

#include "app_icon.h"

typedef enum {
	ORDER_MANUALLY = 0,
	ORDER_ALPHABETICAL = 1
} order_mode_t;

typedef enum {
	POPUP_TWO_BUTTONS,
	POPUP_INFO,
} popup_type_t;

typedef struct {
	popup_type_t type;
	char *popup_title_text;
	char *popup_content_text;
	char *popup_confirm_text;
	char *popup_cancel_text;
	Elm_Popup_Orient orientation;
	double visible_time;
} popup_data_t;

typedef void (*popup_confirm_cb)(void *data);
typedef void (*popup_cancel_cb)(void *data);

/**
 * @brief Allocates a structure for initialze popup widet
 * @return Pointer to popup data. It must be freed manually.
 */
popup_data_t *popup_create_data(void);

/**
 * @brief Frees p_data structure memory and its members
 * @return
 */
void popup_data_free(popup_data_t *p_data);

/**
 * @brief Function shows popup window on the screen
 * @param data popup_data_t struct which is used for initialize popup with
 * its content.
 *popup_confirm_cb - function called when confirm button will be clicked.
 * popup_cancel_cb - function called when cancel button will be clicked.
 * cb_data - data used in callback functions.
 */
void popup_show(popup_data_t *data, popup_confirm_cb confirm_cb,
				popup_cancel_cb cancel_cb, void *cb_data);

/**
 * @brief Hide popup and remove all data
 * @returns EINA_TRUE if popup is visible otherwise EINA_FALSE
 */
Eina_Bool popup_destroy(void);

#endif /* POPUP_H_ */
