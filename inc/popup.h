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

#ifndef __POPUP_H__
#define __POPUP_H__

typedef enum {
    POPUP_DEFAULT = -1,
    POPUP_CLUSTER_PAGE_FULL = 0,
    POPUP_CLUSTER_DELETE_PAGE,
    POPUP_MAX
} popup_t;

void popup_show(popup_t type, int btn_count, Evas_Smart_Cb btn_func[3], void *func_data[3]);
void popup_hide(void);
bool popup_is_show(void);

#endif /* __POPUP_H__ */
