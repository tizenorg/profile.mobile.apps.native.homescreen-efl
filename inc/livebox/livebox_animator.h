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

#ifndef LIVEBOX_ANIMATOR_H_
#define LIVEBOX_ANIMATOR_H_

struct _Anim_Data;

typedef void (*Anim_Grid_End)(struct _Anim_Data **ad);

typedef struct _Anim_Data {
	double anim_time;
	Evas_Object *obj;
	Ecore_Pos_Map map;
	double map_var_1;
	double map_var_2;
	Eina_Rectangle *geometry_start;
	Eina_Rectangle *geometry_end;
	Anim_Grid_End on_end;
	Ecore_Animator *animator;
	bool anim_is_running;
} Anim_Data_t;

/**
 * @brief Delete an animator
 * @param ad Animator to delete
 */
extern void livebox_animator_del_grid(Anim_Data_t **ad);

/**
 * @brief creates a grid animator and starts it
 * @details Animator will be performed on grid item. It works by packing
 * an item to grid's virtual resolution.
 * Notice that the animated item has to be packed to a grid before the
 * animation starts
 * @param obj object to animate
 * @param anim_time animation time
 * @param map animator mapping method
 * @param map_var_1 value used by map
 * @param map_var_2 value used by map
 * @param geometry_start animator start position
 * @param geometry_end animator end position
 * @param on_end callback called on end of animation
 * @param ad_out created animator object
 */
extern void livebox_animator_play_grid_pack_set(Evas_Object *obj,
		float anim_time,
		Ecore_Pos_Map map, float map_var_1, float map_var_2,
		Eina_Rectangle *geometry_start, Eina_Rectangle *geometry_end,
		Anim_Grid_End on_end,
		Anim_Data_t **ad_out);

/**
 * @brief creates a geometry animator and starts it
 * @details object position and size will be animated
 * @param obj object ot animate
 * @param anim_time animation time
 * @param map animator mapping method
 * @param map_var_1 value used by map
 * @param map_var_2 value used by map
 * @param geometry_start animator start position
 * @param geometry_end animator end position
 * @param on_end callback called on end of animation
 * @param ad_out created animator object
 */
extern void livebox_animator_play_geometry_set(Evas_Object *obj,
		float anim_time,
		Ecore_Pos_Map map, float map_var_1, float map_var_2,
		Eina_Rectangle *geometry_start, Eina_Rectangle *geometry_end,
		Anim_Grid_End on_end,
		Anim_Data_t **ad_out);

#endif /* LIVEBOX_ANIMATOR_H_ */
