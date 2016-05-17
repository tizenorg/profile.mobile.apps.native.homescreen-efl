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

#ifndef __WIDGET_VIEWER_H__
#define __WIDGET_VIEWER_H__

#include <Elementary.h>
#include <widget_service.h>
#include <widget_viewer_evas.h>

#include "cluster_data.h"

void widget_viewer_init(Evas_Object *win);
void widget_viewer_fini(void);

Evas_Object *widget_viewer_add_widget(Evas_Object *parent, widget_data_t *item, int *widget_width, int *widget_height);

#endif /* __WIDGET_VIEWER_H__ */
