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

#ifndef OPTION_MENU_H_
#define OPTION_MENU_H_


#include <stdbool.h>

/**
 * @brief Shows option menu with proper entries.
 * @return true on success.
 */
extern bool option_menu_show(void);

/**
 * @brief Hides option menu popup.
 */
extern void option_menu_hide(void);

/**
 * @brief Handle hardware button actions.
 */
extern void option_menu_change_state_on_hw_menu_key(void);

/**
 * @brief Updates caption menu at the top of the screen.
 */
extern void option_menu_update_on_item_selected(void);


#endif /* OPTION_MENU_H_ */
