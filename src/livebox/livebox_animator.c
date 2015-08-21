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

#include "livebox/livebox_animator.h"
#include "util.h"
#include "livebox/livebox_panel.h"
#include "homescreen-efl.h"
#include "page_scroller.h"
#include "popup.h"

#define FRAME (1.0/30.0)
#define FPS 30

static Eina_Bool __livebox_animator_set_grid_frame(void *data, double pos);
static Eina_Bool __livebox_animator_set_geometry_frame(void *data, double pos);
static void __livebox_animator_create(Evas_Object *obj,
		float anim_time,
		Ecore_Pos_Map map, float map_var_1, float map_var_2,
		Eina_Rectangle *geometry_start, Eina_Rectangle *geometry_end,
		Anim_Grid_End on_end,
		Anim_Data_t **ad_out);


void livebox_animator_del_grid(Anim_Data_t **ad)
{
	if (!(*ad)) {
		LOGE("(*ad) == NULL");
		return;
	}

	(*ad)->on_end = NULL;
	ecore_animator_del((*ad)->animator);
	(*ad)->animator = NULL;
	(*ad)->anim_is_running = false;
	eina_rectangle_free((*ad)->geometry_start);
	eina_rectangle_free((*ad)->geometry_end);
	free((*ad));

	(*ad) = NULL;
}

void livebox_animator_play_grid_pack_set(Evas_Object *obj,
		float anim_time,
		Ecore_Pos_Map map, float map_var_1, float map_var_2,
		Eina_Rectangle *geometry_start, Eina_Rectangle *geometry_end,
		Anim_Grid_End on_end,
		Anim_Data_t **ad_out)
{
	__livebox_animator_create(obj, anim_time, map, map_var_1, map_var_2,
			geometry_start, geometry_end, on_end, ad_out);

	if (!(*ad_out)) {
		LOGE("Failed to create animation");
		return;
	}

	(*ad_out)->animator = ecore_animator_timeline_add(anim_time,
		__livebox_animator_set_grid_frame, ad_out);

	if (!(*ad_out)->animator) {
		LOGE("Failed to create animator");
		livebox_animator_del_grid(ad_out);
		return;
	}
}

void livebox_animator_play_geometry_set(Evas_Object *obj,
		float anim_time,
		Ecore_Pos_Map map, float map_var_1, float map_var_2,
		Eina_Rectangle *geometry_start, Eina_Rectangle *geometry_end,
		Anim_Grid_End on_end,
		Anim_Data_t **ad_out)
{
	__livebox_animator_create(obj, anim_time, map, map_var_1, map_var_2,
			geometry_start, geometry_end, on_end, ad_out);

	if (!(*ad_out)) {
		LOGE("Failed to create animation");
		return;
	}

	(*ad_out)->animator = ecore_animator_timeline_add(anim_time,
		__livebox_animator_set_geometry_frame, ad_out);

	if (!(*ad_out)->animator) {
		LOGE("Failed to create animator");
		livebox_animator_del_grid(ad_out);
		return;
	}
}

static void __livebox_animator_create(Evas_Object *obj,
		float anim_time,
		Ecore_Pos_Map map, float map_var_1, float map_var_2,
		Eina_Rectangle *geometry_start, Eina_Rectangle *geometry_end,
		Anim_Grid_End on_end,
		Anim_Data_t **ad_out)
{
	*ad_out = (Anim_Data_t *) calloc(1, sizeof(Anim_Data_t));
	if (!*ad_out) {
		LOGE("Failed to allocate ad_out");
		return;
	}

	(*ad_out)->anim_time = anim_time;
	(*ad_out)->obj = obj;
	(*ad_out)->map = map;
	(*ad_out)->map_var_1 = map_var_1;
	(*ad_out)->map_var_2 = map_var_2;
	(*ad_out)->geometry_start = geometry_start;
	(*ad_out)->geometry_end   = geometry_end;
	(*ad_out)->on_end = on_end;
	(*ad_out)->anim_is_running = true;
}


static Eina_Bool __livebox_animator_set_geometry_frame(void *data, double pos)
{
	double frame = pos;

	Anim_Data_t **ad = (Anim_Data_t **)data;
	if (!(*ad)) {
		LOGE("(*ad) == NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	int xs = (*ad)->geometry_start->x;
	int ys = (*ad)->geometry_start->y;
	int ws = (*ad)->geometry_start->w;
	int hs = (*ad)->geometry_start->h;

	int xe = (*ad)->geometry_end->x;
	int ye = (*ad)->geometry_end->y;
	int we = (*ad)->geometry_end->w;
	int he = (*ad)->geometry_end->h;

	int xd = xe - xs;
	int yd = ye - ys;
	int wd = we - ws;
	int hd = he - hs;

	frame = ecore_animator_pos_map(pos, (*ad)->map, (*ad)->map_var_1,
		(*ad)->map_var_2);

	evas_object_move((*ad)->obj, xs + xd * frame, ys + yd * frame);
	evas_object_resize((*ad)->obj, ws + wd * frame, hs + hd * frame);

	if (pos >= 1.0) {
		/*last frame*/
		if ((*ad)->on_end) {
			(*ad)->on_end(ad);
		} else {
			livebox_animator_del_grid(ad);
		}

		return ECORE_CALLBACK_CANCEL;
	}

	return ECORE_CALLBACK_RENEW;
}


static Eina_Bool __livebox_animator_set_grid_frame(void *data, double pos)
{
	double frame = pos;

	Anim_Data_t **ad = (Anim_Data_t **)data;
	if (!(*ad)) {
		LOGE("(*ad) == NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	int xs = (*ad)->geometry_start->x;
	int ys = (*ad)->geometry_start->y;
	int ws = (*ad)->geometry_start->w;
	int hs = (*ad)->geometry_start->h;

	int xe = (*ad)->geometry_end->x;
	int ye = (*ad)->geometry_end->y;
	int we = (*ad)->geometry_end->w;
	int he = (*ad)->geometry_end->h;

	int xd = xe - xs;
	int yd = ye - ys;
	int wd = we - ws;
	int hd = he - hs;

	frame = ecore_animator_pos_map(pos, (*ad)->map, (*ad)->map_var_1,
		(*ad)->map_var_2);

	elm_grid_pack_set((*ad)->obj,
		xs + xd * frame,
		ys + yd * frame,
		ws + wd * frame,
		hs + hd * frame);


	if (pos >= 1.0) {
		/*last frame*/
		if ((*ad)->on_end) {
			(*ad)->on_end(ad);
		} else {
			livebox_animator_del_grid(ad);
		}

		return ECORE_CALLBACK_CANCEL;
	}

	return ECORE_CALLBACK_RENEW;
}
