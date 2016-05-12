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

#ifndef __ADD_WIDGET_DATA_H__
#define __ADD_WIDGET_DATA_H__

typedef struct {
    char* app_id;
    char* widget_id;
    int is_prime;
} add_widget_data_t;

bool add_widget_data_init(void);
void add_widget_data_fini(void);

#endif /* __ADD_WIDGET_DATA_H__ */
