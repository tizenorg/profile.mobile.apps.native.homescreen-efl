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

#ifndef LIVEBOX_WIDGET_H_
#define LIVEBOX_WIDGET_H_

#include <Elementary.h>
#include <widget_service.h>

typedef struct {
	char *pkg_id;
	char *widget_id;
	int is_prime;
	int width;
	int height;
} widget_info_t;

/**
 * @brief Initializes the widget module.
 */
extern void livebox_widget_init(void);

/**
 * @brief Frees all data initialized for widget module.
 */
extern void livebox_widget_fini(void);

/**
 * @brief Gets the widgets list from the widget service
 * @return Eina_List pointer
 */
extern Eina_List *livebox_widget_get_list(void);

/**
 * @brief Cretes new widget and add it to the parent.
 *
 * @param pkg_name char pointer to package name of the widget
 * @param parent pointer to parent Evas_Object
 * @param content info
 *
 * @return pointer to created widget's Evas_Object
 */
extern Evas_Object *livebox_widget_add(const char *pkg_name, Evas_Object *parent, char *content_info);

/**
 * @brief Removes the widget and its data.
 *
 * @param Widget's Evas_Object pointer.
 */
extern void livebox_widget_del(Evas_Object *widget);

/**
 * @brief Gets the widget size.
 *
 * @param size type
 * @param x - x size in pixels.
 * @param y - y size in pixels.
 */
extern void livebox_widget_get_dimensions(int size, int *x, int *y);

/**
 * @brief Gets the available sizes of the widget
 * @param widget Evas_Object pointer to the widget object.
 * @param cnt number of available widget sizes
 * @param size_list sizes list.
 */
extern void livebox_widget_get_size_available(Evas_Object *widget, int *cnt, int **size_list);


/**
 * @brief Blocks the mouse_up event for given widget
 * @param widget widget to block events for
 */
extern void livebox_widget_set_event_blocking(Evas_Object *widget);

/**
 */
extern void livebox_widget_viewer_paused(void);

/**
 */
extern void livebox_widget_viewer_resumed(void);

#endif /* LIVEBOX_WIDGET_H_ */
