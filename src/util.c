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
#include <stdbool.h>

#include "homescreen-efl.h"
#include "util.h"
#include "data_model.h"

HAPI Evas_Object *util_create_edje_layout(Evas_Object *parent, const char *edje_filename, const char *edje_group)
{
	Evas_Object *layout = NULL;

	if (!edje_filename || !edje_group) {
		LOGE("Filename or groupname is NULL");
		return NULL;
	}

	if (!parent) {
		LOGE("Parent is NULL");
		return NULL;
	}

	layout = elm_layout_add(parent);
	if (!layout) {
		LOGE("Cannot create layout");
		return NULL;
	}

	if (!elm_layout_file_set(layout, edje_filename, edje_group)) {
		evas_object_del(layout);
		LOGE("Cannot load layout");
		return NULL;
	}

	evas_object_show(layout);

	return layout;
}

/* TEST FUNCTIONS : TO BE REMOVED*/
HAPI void tst_resize_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	Evas_Coord x, y, w, h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	LOGD("Obj_Type[%s] %d %d %d %d", evas_object_type_get(obj), x, y, w, h);
}

HAPI void tst_show_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	LOGD("MSZ ");

}

HAPI void tst_hide_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	LOGD("MSZ ");
}

HAPI void tst_move_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	Evas_Coord x, y, w, h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	LOGD("MSZ %d %d %d %d", x, y, w, h);
}

HAPI void tst_del_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	LOGD("tst_del_cb: obj type [%s]", evas_object_type_get(obj));

	Tree_node_t *t_data = evas_object_data_get(obj, KEY_ICON_DATA);
	if (t_data)
		LOGI("[obtained data [%s]]", t_data->data->label);
}

HAPI Eina_List *elm_gengrid_get_evas_objects_from_items(Evas_Object *gengrid)
{
	Eina_List *items = NULL;
	Eina_List *it = NULL;
	Elm_Object_Item *data = NULL;

	Eina_List *tmp = NULL;
	Evas_Object *obj = NULL;

	if (!gengrid) {
		LOGE("[INVALID_PARAM][gengrid='%p']", gengrid);
		return NULL;
	}

	tmp = elm_gengrid_realized_items_get(gengrid);
	if (!tmp) {
		LOGE("[FAILED][elm_gengrid_realized_items_get]");
		return NULL;
	}

	EINA_LIST_FOREACH(tmp, it, data) {
		if (!data)
			continue;

		obj = elm_object_item_part_content_get(data, GRID_CONTENT);
		if (!obj) {
			LOGE("[FAILED][elm_object_item_part_content_get]");
			eina_list_free(tmp);
			eina_list_free(items);
			return NULL;
		}

		items = eina_list_append(items, obj);

		if (!items) {
			LOGE("[FAILED][eina_list_append]");
			eina_list_free(tmp);
			return NULL;
		}
	}

	eina_list_free(tmp);

	return items;
}

HAPI Elm_Object_Item *elm_gengrid_get_item_at_index(Evas_Object *gengrid, int idx)
{
	Eina_List *items = NULL;
	Elm_Object_Item *found = NULL;

	if (!gengrid || idx < 0) {
		LOGE("[INVALID_PARAM][gengrid='%p'][idx='%d']", gengrid, idx);
		return NULL;
	}

	items = elm_gengrid_realized_items_get(gengrid);
	if (!items) {
		LOGE("[FAILED][elm_gengrid_realized_items_get]");
		return NULL;
	}

	if (idx > eina_list_count(items)) {
		LOGE("[INVALID_INDEX]");
		eina_list_free(items);
		return NULL;
	}

	found = eina_list_nth(items, idx);
	eina_list_free(items);

	return found;
}
