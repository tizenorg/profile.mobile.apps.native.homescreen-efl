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

#ifndef __HOME_SCREEN_UTIL_H__
#define __HOME_SCREEN_UTIL_H__

#include <dlog.h>
#include <Elementary.h>
#include <stdbool.h>

#define BUF_SIZE 64

/* Multi-language */
#ifndef _
#define _(str) gettext(str)
#endif

/* Build */
#define HAPI __attribute__((visibility("hidden")))

#define COUNT_OF(x) \
((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))


/**
 * @brief Creates elm_layout widget and load edje file to it.
 *
 * @param win Homescreen efl window pointer.
 * @param edje_filename  name of the edje file
 * @param edje_group group name from the edje file
 * @return elm_layout Evas_Object pointer.
 */
extern Evas_Object *util_create_edje_layout(
	Evas_Object *win, const char* edje_filename, const char* edje_group);

/*DBG callbacks set. Please use it if new objects are not visible .etc*/
extern void tst_resize_cb(void *data, Evas *e, Evas_Object *obj, void *ei);
extern void tst_show_cb(void *data, Evas *e, Evas_Object *obj, void *ei);
extern void tst_hide_cb(void *data, Evas *e, Evas_Object *obj, void *ei);
extern void tst_move_cb(void *data, Evas *e, Evas_Object *obj, void *ei);
extern void tst_del_cb(void *data, Evas *e, Evas_Object *obj, void *ei);



/**
 * @brief Function returns Eina list of evas objects visible in gengrid
 *
 * @param gengrid Evas_Object pointer to elm_gengrid
 * @return Eina_List pointer which Evas_Objects
 */
extern Eina_List *elm_gengrid_get_evas_objects_from_items(Evas_Object *gengrid);

/**
 * @brief Function returns pointer to elm_object_item placed in gengrid
 * at specified position
 * @param gengrid Pointer to elm_gengrid_widget;
 * @param idx
 * @return Pointer to elm_object_item
 */
Elm_Object_Item *elm_gengrid_get_item_at_index(Evas_Object *gengrid, int idx);

#endif /* __HOME_SCREEN_UTIL_H__ */
