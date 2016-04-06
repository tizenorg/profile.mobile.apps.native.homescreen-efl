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

#include "app_grid.h"
#include "homescreen-efl.h"
#include "app_item.h"
#include "app_icon.h"
#include "page_scroller.h"
#include "all_apps.h"
#include "folder_panel.h"
#include "util.h"
#include "popup.h"
#include "data_model.h"

typedef enum {
	NEXT_PAGE_CB = 0,
	PREV_PAGE_CB,
	NONE_PAGE_CB
} page_change_action_t;

typedef enum {
	MODE_NORMAL = 0,
	MODE_EDIT,
	MODE_FOLDER,
} possible_view_modes_t;

typedef struct {
	page_change_action_t action;
	Evas_Object *parent;
} change_page_data_t;

static struct {
	Tree_node_t *repositioned_node;
	Tree_node_t *opened_folder_node;
	Tree_node_t *destination_folder;
	Evas_Object *repositioned_icon_layout;
	Evas_Object *src_grid;
	Evas_Object *repositioned_icon;
	Elm_Object_Item *repositioned_item;
	int src_idx;
	bool is_dnd_on;
	Evas_Coord dx;
	Evas_Coord dy;
	Elm_Gengrid_Item_Class *gic;
	Evas_Coord w;
	Evas_Coord h;
	int f_x;
	int f_y;
	int f_w;
	int f_h;
	Ecore_Timer *change_page_timer;
	Ecore_Timer *outside_folder_timer;
	change_page_data_t cp_data;
} s_info = {
	.repositioned_node = NULL,
	.opened_folder_node = NULL,
	.destination_folder = NULL,
	.repositioned_icon_layout = NULL,
	.src_grid = NULL,
	.repositioned_icon = NULL,
	.repositioned_item = NULL,
	.src_idx = 1,
	.is_dnd_on = false,
	.gic = NULL,
	.dx = -1,
	.dy = -1,
	.w = -1,
	.h = -1,
	.f_x = -1,
	.f_y = -1,
	.f_w = -1,
	.f_h = -1,
	.change_page_timer = NULL,
	.outside_folder_timer = NULL,
	.cp_data = {
		action: NONE_PAGE_CB,
		parent: NULL
	}
};

/*================================= PRIVATE FUNCTIONS ============================================*/
static bool __app_grid_fill(Evas_Object *grid, Tree_node_t *data);
static Evas_Object *__app_grid_create_ly(Evas_Object *parent);
static bool __app_grid_resize(Evas_Object *app_grid, Evas_Coord new_w, Evas_Coord new_h);
static Evas_Object *__app_grid_get_icon_container_ptr(Elm_Object_Item *it);
static Eina_Bool __app_grid_change_page_cb(void *data);
static void __app_grid_check_page_change(Evas_Object *scroller, int x, int y);
static void __app_grid_clear_reposition_structure(void);
static Eina_Bool __app_grid_close_folder_cb(void *data);
static Evas_Object *__app_grid_create_item_container(Evas_Object *parent);

/*================================= GENGRID CALLBACK FUNCTIONS ===================================*/
static void __app_grid_item_longpressed_cb(void *data, Evas_Object *obj, void *ei);
static Evas_Object *__app_grid_get_content(void *data, Evas_Object *obj, const char *part);
static Eina_Bool __app_grid_get_state(void *data, Evas_Object *obj, const char *part);
static void __app_grid_del_content(void *data, Evas_Object *obj);
static char *__app_grid_get_text(void *data, Evas_Object *obj, const char *part);
static void __app_grid_resize_cb(void *data, Evas *e, Evas_Object *obj, void *ei);

/*================================= MOUSE HANDLERS ===============================================*/
static Eina_Bool _app_grid_mouse_move_cb(void *data, int type, void *event);
static Eina_Bool __app_grid_mouse_up_cb(void *data, int type, void *event);

/*================================= DND FUNCTIONS ================================================*/
static void __app_grid_return_to_origin_pos(void);
static void __app_grid_icon_drop(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y,
		Elm_Xdnd_Action action);
static void __app_grid_drag_done(void *data, Evas_Object *obj);
static Evas_Event_Flags __app_grid_icon_move(void *data, void *event_info);
static Evas_Event_Flags __app_grid_icon_move_end(void *data, void *event_info);

/*================================= PUBLIC FUNCTIONS DEFS ========================================*/
HAPI Evas_Object *app_grid_create(Evas_Object *parent, Tree_node_t *data, Evas_Coord gengrid_width,
			Evas_Coord gengrid_height, app_grid_type_t type)
{
	Evas_Object *gengrid = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *rect = NULL;

	if (!parent) {
		LOGE("[INVALID_PARAM]['parent'='%p']", parent);
		return NULL;
	}

	rect = evas_object_rectangle_add(evas_object_evas_get(parent));
	if (!rect) {
		LOGE("[FAILED][rect=NULL]");
		return NULL;
	}

	layout = __app_grid_create_ly(parent);
	if (!layout) {
		LOGE("[FAILED][__app_grid_create_ly]");
		evas_object_del(rect);
		return NULL;
	}

	evas_object_color_set(rect, 0, 0, 0, 0);
	evas_object_size_hint_min_set(rect, gengrid_width, gengrid_height);
	evas_object_size_hint_max_set(rect, gengrid_width, gengrid_height);
	elm_object_part_content_set(layout, PART_APP_GRID_SIZE_SETTER, rect);

	gengrid = elm_gengrid_add(layout);
	if (!gengrid) {
		LOGE("[FAILED][elm_gengrid_add]");
		evas_object_del(layout);
		evas_object_del(rect);
		return NULL;
	}

	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(gengrid);

	if (!s_info.gic) {
		if (!(s_info.gic = elm_gengrid_item_class_new())) {
			LOGE("[FAILED][elm_gengrid_item_class_new]");
			evas_object_del(layout);
			evas_object_del(rect);
			return NULL;
		}

		s_info.gic->item_style 	= "default";
		s_info.gic->func.content_get = __app_grid_get_content;
		s_info.gic->func.del = __app_grid_del_content;
		s_info.gic->func.text_get = __app_grid_get_text;
		s_info.gic->func.state_get = __app_grid_get_state;

		ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _app_grid_mouse_move_cb, parent);
		ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, __app_grid_mouse_up_cb, NULL);
	}

	elm_object_scale_set(gengrid, 0.5);

	elm_gengrid_select_mode_set(gengrid, ELM_OBJECT_SELECT_MODE_NONE);
	elm_gengrid_reorder_mode_set(gengrid, EINA_FALSE);
	elm_gengrid_align_set(gengrid, 0.0, 0.0);

	if (!__app_grid_fill(gengrid, data)) {
		LOGE("[FAILED][__app_grid_fill]");
		evas_object_del(layout);
		evas_object_del(rect);
		return NULL;
	}

	if (type == APP_GRID_TYPE_ALL_APPS) {
		s_info.w = gengrid_width;
		s_info.h = gengrid_height;
	} else if (type == APP_GRID_TYPE_FOLDER) {
		elm_object_signal_emit(layout, SIGNAL_APP_GRID_BG_HIDE, SIGNAL_SOURCE);
	}

	elm_object_part_content_set(layout, PART_APP_GRID_CONTENT, gengrid);
	evas_object_smart_callback_add(gengrid, "longpressed", __app_grid_item_longpressed_cb, NULL);

	data->data->layout = layout;
	evas_object_event_callback_add(gengrid, EVAS_CALLBACK_RESIZE, __app_grid_resize_cb, (void *)type);

	/*It is used only internal.*/
	evas_object_data_set(gengrid, KEY_APP_GRID_DATA, layout);

	elm_scroller_movement_block_set(gengrid, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
	elm_scroller_movement_block_set(gengrid, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
	elm_gengrid_wheel_disabled_set(gengrid, EINA_TRUE);

	return layout;
}

HAPI Elm_Object_Item *app_grid_append_item(Evas_Object *layout, Evas_Object *icon_to_append)
{
	Elm_Object_Item *result = NULL;
	Evas_Object *container = NULL;

	if (!layout || !icon_to_append) {
		LOGE("[INVALID_PARAM][layout='%p'][icon_to_append='%p']", layout, icon_to_append);
		return NULL;
	}

	Evas_Object *grid = elm_object_part_content_get(layout, PART_APP_GRID_CONTENT);
	if (!grid) {
		LOGE("[FAILED][elm_object_part_content_get]");
		return NULL;
	}

	if (!(result = elm_gengrid_item_append(grid, s_info.gic,
			(container = app_grid_item_pack(grid, icon_to_append)), NULL, NULL))) {
		LOGE("Failed to append item");
		return NULL;
	}

	evas_object_show(icon_to_append);
	return result;
}

HAPI Elm_Object_Item *app_grid_insert_item_relative(Evas_Object *layout, Evas_Object *item,
				Elm_Object_Item *rel_it)
{
	Evas_Object *app_grid = NULL, *container = NULL;
	Elm_Object_Item *result = NULL;
	int dst_idx = -1;

	if (!layout || !item) {
		LOGE("[INVALID_PARAM][layout='%p'][item='%p']", layout, item);
		return NULL;
	}

	app_grid = elm_object_part_content_get(layout, PART_APP_GRID_CONTENT);
	if (!app_grid) {
		LOGE("[FAILED][elm_object_part_content_get]");
		return NULL;
	}

	if (rel_it) {
		dst_idx = elm_gengrid_item_index_get(rel_it);

		if ((s_info.src_grid == app_grid && s_info.src_idx > dst_idx)
				|| (s_info.src_grid != app_grid)) {
			if (!(result = elm_gengrid_item_insert_before(app_grid, s_info.gic,
				(container = app_grid_item_pack(app_grid, item)), rel_it, NULL, NULL))) {
				LOGE("[FAILED][elm_gengrid_item_insert_before]");
				evas_object_del(container);
				return NULL;
			}
		} else if (s_info.src_grid == app_grid && s_info.src_idx < dst_idx) {
			if (!(result = elm_gengrid_item_insert_after(app_grid, s_info.gic,
				(container = app_grid_item_pack(app_grid, item)), rel_it, NULL, NULL))) {
				LOGE("[FAILED][elm_gengrid_item_insert_before]");
				return NULL;
			}
		}
	} else {
		if (!(result = elm_gengrid_item_append(app_grid, s_info.gic,
				(container = app_grid_item_pack(app_grid, item)), NULL, NULL))) {
			LOGE("[FAILED][elm_gengrid_item_append]");
			return NULL;
		}
	}

	evas_object_show(item);
	return result;
}

HAPI Evas_Object *app_grid_item_pack(Evas_Object *p, Evas_Object *c)
{
	Evas_Object *container = NULL;

	if (!p) { /*When c is null it is not a big problem...it creates then empty gengrid item */
		LOGD("[ERR][INVALID_PARAM]['p'='%p']", p);
		return NULL;
	}

	/*This items will be removed when gengrid items will be deleted.
	It was checked in EVAS_CALLBACK_DEL function. */
	container = __app_grid_create_item_container(home_screen_get_win());
	if (!container) {
		LOGE("[FAILED][container=NULL]");
		return NULL;
	}

	if (c)
		elm_object_part_content_set(container, PART_APP_GRID_ITEM, c);

	return container;
}

HAPI void app_grid_unpack_item(Elm_Object_Item *item)
{
	Evas_Object *container = NULL;

	if (!item) {
		LOGE("[INVALID_PARAM][item=NULL]");
		return;
	}

	container = elm_object_item_part_content_get(item, GRID_CONTENT);
	if (!container) {
		LOGE("[FAILED][elm_object_item_part_content_get]");
		return;
	}

	elm_object_part_content_unset(container, PART_APP_GRID_ITEM);
}

HAPI Eina_Bool app_grid_get_item_content(Elm_Object_Item *item, Evas_Object **icon,
				Tree_node_t **data)
{
	Evas_Object *ic = NULL;
	Evas_Object *container = NULL;

	if (!item) {
		LOGE("[INVALID_PARAM][item='%p']", item);
		return EINA_FALSE;
	}

	container = elm_object_item_part_content_get(item, GRID_CONTENT);
	if (!container) {
		LOGE("[FAILED][elm_object_item_part_content_get]");
		return EINA_FALSE;
	}

	ic = elm_object_part_content_get(container, PART_APP_GRID_ITEM);
	if (!ic) {
		LOGE("[FAILED][eina_list_data_get]");
		return EINA_FALSE;
	}

	if (icon)
		*icon = ic;

	if (data) {
		*data = evas_object_data_get(ic, KEY_ICON_DATA);
		if (!(*data)) {
			LOGE("[FAILED][evas_object_data_get]");
			/*eina_list_free(box_content);*/
			return EINA_FALSE;
		}
	}
	return EINA_TRUE;
}

static Evas_Event_Flags __app_grid_icon_move(void *data, void *event_info)
{
	Elm_Gesture_Momentum_Info *pos_info = event_info;

	if(s_info.repositioned_icon == NULL)
		return EVAS_EVENT_FLAG_NONE;

	evas_object_move(s_info.repositioned_icon, pos_info->x2 - s_info.dx, pos_info->y2 - s_info.dy);

	if (folder_panel_is_folder_visible()) {
		LOGD("Folder is visible");
		if ((pos_info->y2 <= s_info.f_y || pos_info->y2 >= s_info.f_y + s_info.f_h) &&
				!s_info.outside_folder_timer) {

			LOGD("Icon is outside folder");
			s_info.outside_folder_timer = ecore_timer_add(1.0, __app_grid_close_folder_cb, NULL);
			if (!s_info.outside_folder_timer) {
				LOGE("[FAILED][timer=NULL]");
				return EVAS_EVENT_FLAG_ON_HOLD;
			}
		} else if (pos_info->y2 >= s_info.f_y && pos_info->y2 <= s_info.f_y + s_info.f_h) {
			LOGD("Icons position is on folder");
			ecore_timer_del(s_info.outside_folder_timer);
			s_info.outside_folder_timer = NULL;
		}
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Eina_Bool __app_grid_icon_move_cb_set(void *data)
{
	LOGD("Set icon move cb");
	int ret = 0;

	ret = home_screen_gesture_cb_set(ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_MOVE,
			__app_grid_icon_move, NULL);
	ret |= home_screen_gesture_cb_set(ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_END,
			__app_grid_icon_move_end, NULL);
	ret |= home_screen_gesture_cb_set(ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_ABORT,
			__app_grid_icon_move_end, NULL);

	if (ret) {
		LOGE("Can not set icon move callbacks");
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

static Eina_Bool __app_grid_icon_move_cb_del(void *data)
{
	LOGD("Delete icon move cb");
	int ret = 0;

	ret = home_screen_gesture_cb_unset(ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_MOVE,
			__app_grid_icon_move, NULL);
	ret |= home_screen_gesture_cb_unset(ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_END,
			__app_grid_icon_move_end, NULL);
	ret |= home_screen_gesture_cb_unset(ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_ABORT,
			__app_grid_icon_move_end, NULL);

	if (ret)
		LOGE("Can not unset callbacks");

	return ECORE_CALLBACK_CANCEL;
}

static Evas_Event_Flags __app_grid_icon_move_end(void *data, void *event_info)
{
	Elm_Gesture_Momentum_Info *pos_info = event_info;

	if(s_info.repositioned_icon != NULL) {

		if (pos_info != NULL) {
			LOGD("Drag end at: x,y=<%d,%d>", pos_info->x2, pos_info->y2);

			__app_grid_icon_drop(NULL, s_info.src_grid, pos_info->x2,
					pos_info->y2, ELM_XDND_ACTION_MOVE);
		}

		__app_grid_drag_done(NULL, NULL);
	}

	/* quickfix: idler adds delay so cb func might be deleted after it is finished
	 * removing cb inside its body causes memory errors
	*/
	ecore_idler_add(__app_grid_icon_move_cb_del, NULL);

	return EVAS_EVENT_FLAG_NONE;
}

/*=====================GENGRID CALLBACKS IMPLEMENTATION===========================================*/
static void __app_grid_item_longpressed_cb(void *data, Evas_Object *obj, void *ei)
{
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	Evas_Coord x, y, w, h, m_x, m_y;
	Evas_Object *icon_layout = NULL;
	Evas_Object *icon = NULL;
	Tree_node_t *icon_node = NULL;
	homescreen_view_t view_type = HOMESCREEN_VIEW_UNKNOWN;

	view_type = home_screen_get_view_type();

	if (view_type != HOMESCREEN_VIEW_ALL_APPS &&
			view_type != HOMESCREEN_VIEW_ALL_APPS_EDIT)
		return;

	if (!it || !app_grid_get_item_content(it, &icon_layout, NULL)) {
		LOGE("[FAILED][app_grid_item_content_get]");
		return;
	}

	if (!evas_object_data_get(icon_layout, KEY_IS_REPOSITIONABLE))
		return;

	if (!icon_layout) {
		LOGE("[FAILED][icon_layout=NULL]");
		return;
	}

	if (view_type == HOMESCREEN_VIEW_ALL_APPS)
		home_screen_set_view_type(HOMESCREEN_VIEW_ALL_APPS_EDIT);

	icon = elm_object_part_content_get(icon_layout, PART_ICON_CONTENT);
	if (!icon) {
		LOGE("[FAILED][icon=NULL]");
		return;
	}

	icon_node = evas_object_data_get(icon_layout, KEY_ICON_DATA);
	if (!icon_node) {
		LOGE("[FAILED][icon_node=NULL]");
		return;
	}

	/*check if the repositioned item is from a folder.
	It is necessary for removing item from the folder.*/
	if (icon_node->parent && icon_node->parent->parent && icon_node->parent->parent->data
				&& icon_node->parent->parent->data->type == APP_ITEM_FOLDER) {
		s_info.opened_folder_node = icon_node->parent->parent;
		folder_panel_get_folder_geometry(&s_info.f_x, &s_info.f_y, &s_info.f_w, &s_info.f_h);
	}

	evas_object_geometry_get(icon, &x, &y, &w, &h);
	evas_pointer_output_xy_get(evas_object_evas_get(obj), &m_x, &m_y);
	s_info.dx = abs(m_x - x);
	s_info.dy = abs(m_y - y);

	/*check if user perform longpress action on icon or on item.
	(item is a container for icon..is bigger because it must have place also for icon name)*/
	if ((m_x >= x && m_x <= x+w) && (m_y >= y && m_y <= y + h)) {
		s_info.repositioned_icon_layout = icon_layout;
		s_info.repositioned_icon = icon;
		s_info.repositioned_item = it;
		s_info.repositioned_node = icon_node;
		s_info.src_grid = obj;
		s_info.src_idx = elm_gengrid_item_index_get(s_info.repositioned_item);
		s_info.is_dnd_on = true;

		app_grid_unpack_item(it);

		if (__app_grid_icon_move_cb_set(NULL)) {
			LOGE("Gesture cb set failed");

			__app_grid_icon_move_cb_del(NULL);
		}

		evas_object_data_set(icon, KEY_REPOSITION_DATA, icon_layout);
		/*This is necessary for hide only box content in grid*/
		elm_object_part_content_unset(icon_layout, PART_ICON_CONTENT);
		evas_object_hide(icon_layout);
		all_apps_set_scrolling_blocked_state(true);
		folder_panel_enable_entry(false);
	}
}

static char *__app_grid_get_text(void *data, Evas_Object *obj, const char *part)
{
	return NULL;
}

static Evas_Object *__app_grid_get_content(void *data, Evas_Object *obj, const char *part)
{
	if (!data) {
		LOGE("[INVALID_PARAM][data='%p']", data);
		return NULL;
	}

	if (!strcmp(GRID_CONTENT, part))
		return (Evas_Object *) data;

	return NULL;
}

static Eina_Bool __app_grid_get_state(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void __app_grid_del_content(void *data, Evas_Object *obj)
{
	LOGD("DELETING :  ", evas_object_type_get(obj));

}

static void __app_grid_resize_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	int w = -1, h = -1, item_w = -1, item_h = -1;
	evas_object_geometry_get(obj, NULL, NULL, &w, &h);

	if ((app_grid_type_t)data == APP_GRID_TYPE_ALL_APPS) {
		item_w = w/APP_COLS;
		item_h = h/APP_ROWS;
	} else if ((app_grid_type_t)data == APP_GRID_TYPE_FOLDER) {
		item_w = (int)(w/FOLDER_COL_APPS - FOLDER_ITEM_MARGIN*home_screen_get_root_width());
		item_h = h/FOLDER_ROW_APPS;
	}

	elm_gengrid_item_size_set(obj, item_w, item_h);
}

/*=========================== MOUSE HANDLERS =====================================================*/
static Eina_Bool __app_grid_mouse_up_cb(void *data, int type, void *event)
{
	LOGD("");

	if (s_info.is_dnd_on)
		__app_grid_icon_move_end(NULL, NULL);

	home_screen_set_indice_state(INDICE_OFF);
	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _app_grid_mouse_move_cb(void *data, int type, void *event)
{
	if (s_info.is_dnd_on) {
		Evas_Object *scroller = (Evas_Object *) data;
		Ecore_Event_Mouse_Move *move = event;
		__app_grid_check_page_change(scroller, move->x, move->y);
	}

	return ECORE_CALLBACK_RENEW;
}

/*========================= PRIVATE FUNCTIONS ====================================================*/
static bool __app_grid_fill(Evas_Object *grid, Tree_node_t *data)
{
	Evas_Object *icon = NULL, *box = NULL;
	app_item_t *item = NULL;
	Tree_node_t *it = NULL;

	if (!data || !grid) {
		LOGE("[INVALID_PARAM][grid='%p'][data='%p']", grid, data);
		return false;
	}

	TREE_NODE_FOREACH(data, it) {
		item = it->data;
		if (!item) {
			LOGE("[FAILED][item=%p", item);
			continue;
		}

		if (!item->exec || !item->icon || !item->label || !item->appid) {
			LOGE("[FAILED][exec='%p'][icon='%p'][label='%p'][appid='%p']",
				item->exec, item->label, item->icon, item->appid);
			continue;
		}

		LOGI("add item: [%s]", item->label);

		if (item->type == APP_ITEM_ICON) {
			icon = app_icon_create(it, APP_ICON_TYPE_APP);
		} else if (item->type == APP_ITEM_FOLDER) {
			folder_panel_create(it);
			app_icon_update_folder_icon(it);
			icon = item->layout;
		}

		if (!icon) {
			LOGE("[FAILED][app_icon_create]");
			return false;
		}

		if (!(item->grid_item = elm_gengrid_item_append(grid, s_info.gic,
				(box = app_grid_item_pack(grid, icon)), NULL, NULL))) {
			LOGE("Failed to append an item!");
			evas_object_del(box);
			if (!box) {/*app_grid_item_pack had failed so the icon is still allocated:*/
				app_icon_destroy(icon);
			}
			return false;
		}

		item->layout = icon;
	}

	return true;
}

static Evas_Object *__app_grid_create_ly(Evas_Object *parent)
{
	Evas_Object *ly = NULL;

	if (!parent) {
		LOGE("[INVALID_PARAM][parent='%p']", parent);
		return NULL;
	}

	ly = elm_layout_add(parent);
	if (!ly) {
		LOGE("[FAILED][elm_layout_add]");
		return NULL;
	}

	if (!elm_layout_file_set(ly, util_get_res_file_path(EDJE_DIR"/app_grid.edj"), GROUP_APP_GRID)) {
		LOGE("[FAILED][elm_layout_file_set][%s]", util_get_res_file_path(EDJE_DIR"/app_grid.edj"));
		evas_object_del(ly);
		return NULL;
	}

	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(ly);

	return ly;
}

void app_grid_set_view_mode(Evas_Object *app_grid, homescreen_view_t view)
{
	if (!app_grid) {
		LOGE("[INVALID_PARAM]");
		return;
	}

	switch (view) {
	case HOMESCREEN_VIEW_ALL_APPS:
		elm_object_signal_emit(app_grid, SIGNAL_CHOOSE_MODE_OFF, SIGNAL_SOURCE);
		elm_object_signal_emit(app_grid, SIGNAL_EDIT_MODE_OFF, SIGNAL_SOURCE);
		__app_grid_resize(app_grid, s_info.w, s_info.h);
		break;
	case HOMESCREEN_VIEW_ALL_APPS_EDIT:
		elm_object_signal_emit(app_grid, SIGNAL_CHOOSE_MODE_OFF, SIGNAL_SOURCE);
		elm_object_signal_emit(app_grid, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);
		__app_grid_resize(app_grid, EDIT_MODE_SCALE*s_info.w, EDIT_MODE_SCALE*s_info.h);
		break;
	case HOMESCREEN_VIEW_ALL_APPS_CHOOSE:
		elm_object_signal_emit(app_grid, SIGNAL_EDIT_MODE_OFF, SIGNAL_SOURCE);
		elm_object_signal_emit(app_grid, SIGNAL_CHOOSE_MODE_ON, SIGNAL_SOURCE);
		__app_grid_resize(app_grid, CHOOSE_MODE_SCALE*s_info.w, CHOOSE_MODE_SCALE*s_info.h);
		break;
	default:
		LOGE("[FAILED][type='%d']", view);
		break;
	}
}

static bool __app_grid_resize(Evas_Object *app_grid, Evas_Coord new_w, Evas_Coord new_h)
{
	Evas_Object *gengrid = NULL;

	Evas_Coord new_item_w = new_w/APP_COLS;
	Evas_Coord new_item_h = new_h/APP_ROWS;

	if (!app_grid) {
		LOGE("[INVALID_PARAM][app_grid='%p']", app_grid);
		return false;
	}

	gengrid = elm_object_part_content_get(app_grid, PART_APP_GRID_CONTENT);
	if (!gengrid) {
		LOGE("[FAILED][elm_object_part_content_get]");
		return false;
	}

	if (new_item_w < 0 || new_item_h < 0) {
		LOGE("[INVALID_SIZE]");
		return false;
	}

	/*elm_gengrid_item_size_set(gengrid, new_item_w, new_item_h);*/

	return true;
}

static Eina_Bool __app_grid_change_page_cb(void *data)
{
	LOGI("__app_grid_change_page_cb");
	change_page_data_t *ch_page_data = (change_page_data_t *) data;
	Evas_Object *parent = NULL;

	s_info.change_page_timer = NULL;

	if (!ch_page_data) {
		LOGE("[INVALID_PARAM][ch_page_data='%p']", ch_page_data);
		return ECORE_CALLBACK_CANCEL;
	} else if (!ch_page_data->parent) {
		LOGE("[INVALID_PARAM][ch_page_data->parent='%p']", ch_page_data->parent);
		return ECORE_CALLBACK_CANCEL;
	}

	parent = ch_page_data->parent;

	if (ch_page_data->action == NEXT_PAGE_CB) {
		LOGI("next_page_signal");
		elm_object_signal_emit(parent, SIGNAL_NEXT_PAGE_CHANGE, SIGNAL_SOURCE);
	}

	if (ch_page_data->action == PREV_PAGE_CB) {
		LOGI("prev_page_signal");
		elm_object_signal_emit(parent, SIGNAL_PREV_PAGE_CHANGE, SIGNAL_SOURCE);
	}

	return ECORE_CALLBACK_CANCEL;
}

static void __app_grid_check_page_change(Evas_Object *scroller, int x, int y)
{
	if (!scroller) {
		LOGE("[INVALID_PARAM][scroller='%p']", scroller);
		return;
	}

	s_info.cp_data.parent = scroller;
	home_screen_set_indice_state(INDICE_OFF);

	if (x < CHANGE_PAGE_RIGHT_AREA * home_screen_get_root_width() &&
		x > CHANGE_PAGE_LEFT_AREA * home_screen_get_root_width()  &&
		s_info.change_page_timer) {
		ecore_timer_del(s_info.change_page_timer);
		s_info.change_page_timer = NULL;
		return;
	}

	if (x >= CHANGE_PAGE_RIGHT_AREA * home_screen_get_root_width()) {
		home_screen_set_indice_state(INDICE_NEXT_PAGE_ON);
		s_info.cp_data.action = NEXT_PAGE_CB;
	}

	if (x <= CHANGE_PAGE_LEFT_AREA * home_screen_get_root_width()) {
		home_screen_set_indice_state(INDICE_PREV_PAGE_ON);
		s_info.cp_data.action = PREV_PAGE_CB;
	}

	if (!s_info.change_page_timer) {
		s_info.change_page_timer = ecore_timer_add(CHANGE_PAGE_TIMER, __app_grid_change_page_cb,
				(void *) &s_info.cp_data);
	}
}

static Evas_Object *__app_grid_get_icon_container_ptr(Elm_Object_Item *it)
{
	Evas_Object *container = NULL;

	if (!it) {
		LOGE("[INVALID_PARAM][it='%p']", it);
		return NULL;
	}

	container = elm_object_item_part_content_get(it, GRID_CONTENT);
	return container;
}

static void __app_grid_clear_reposition_structure(void)
{
	s_info.destination_folder = NULL;
	s_info.repositioned_icon_layout = NULL;
	s_info.repositioned_item = NULL;
	s_info.repositioned_icon = NULL;
	s_info.repositioned_node = NULL;
	s_info.opened_folder_node = NULL;
	s_info.src_grid = NULL;
	s_info.src_idx = -1;
	s_info.is_dnd_on = false;

	s_info.f_x = -1;
	s_info.f_y = -1;
	s_info.f_w = -1;
	s_info.f_h = -1;

	if (s_info.outside_folder_timer) {
		ecore_timer_del(s_info.outside_folder_timer);
		s_info.outside_folder_timer = NULL;
	}

	if (s_info.change_page_timer) {
		ecore_timer_del(s_info.change_page_timer);
		s_info.change_page_timer = NULL;
	}
}

static Eina_Bool __app_grid_close_folder_cb(void *data)
{
	data_model_detach_from_folder(s_info.opened_folder_node, s_info.repositioned_node);
	folder_panel_close_folder();
	return EINA_FALSE;
}

static Evas_Object *__app_grid_create_item_container(Evas_Object *parent)
{
	Evas_Object *container = NULL;

	if (!parent) {
		LOGE("[INVALID_PARAM]");
		return NULL;
	}

	container = elm_layout_add(parent);

	if (!elm_layout_file_set(container, util_get_res_file_path(EDJE_DIR"/app_grid.edj"), GROUP_APP_GRID_ITEM_CONTAINER)) {
		LOGE("[FAILED][elm_layout_file_set][%s]", util_get_res_file_path(EDJE_DIR"/app_grid.edj"));
		evas_object_del(container);
		return NULL;
	}

	evas_object_size_hint_weight_set(container, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(container);
	return container;
}

/*=============================== DND FUNCTIONS ==================================================*/

static void __app_grid_return_to_origin_pos(void)
{
	Evas_Object *container = NULL;

	if (!s_info.repositioned_item || !s_info.repositioned_icon_layout) {
		LOGE("[FAILED][repositioned_item='%p'][repositioned_icon='%p']",
				s_info.repositioned_item, s_info.repositioned_icon_layout);
		return;
	}

	container = __app_grid_get_icon_container_ptr(s_info.repositioned_item);
	if (!container) {
		LOGE("[FAILED][__app_grid_get_icon_container_ptr]");
		return;
	}

	elm_object_part_content_set(container, PART_APP_GRID_ITEM, s_info.repositioned_icon_layout);
	elm_object_part_content_set(s_info.repositioned_icon_layout,
				PART_ICON_CONTENT, s_info.repositioned_icon);
	evas_object_show(s_info.repositioned_icon_layout);
}

static void __app_grid_icon_drop(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y,
				Elm_Xdnd_Action action)
{
	static Elm_Object_Item *it = NULL;
	Evas_Object *icon = NULL;
	Evas_Object *old_icon = NULL;
	Elm_Object_Item *old_it = it;
	Tree_node_t *destination_icon_data = NULL;

	it = elm_gengrid_at_xy_item_get(obj, x, y, NULL, NULL);

	app_grid_get_item_content(old_it, &old_icon, NULL);
	app_grid_get_item_content(it, &icon, &destination_icon_data);

	if (!s_info.repositioned_node || !s_info.repositioned_node->data) {
		LOGE("[INVALID_DATA]");
		return;
	}

	if (it != old_it) {
		elm_object_signal_emit(old_icon, SIGNAL_UNHIGHLIGHT_FOLDER, SIGNAL_SOURCE);
		/*function not be invoked when s_info.repositioned_node == NULL;*/
		if (s_info.repositioned_node->data->type == APP_ITEM_FOLDER ||
			folder_panel_get_items_count(destination_icon_data) >= FOLDER_MAX_ITEMS) {
			elm_object_signal_emit(icon, SIGNAL_HIGHLIGHT_IMPOSSIBLE_FOLDER, SIGNAL_SOURCE);
		} else {
			elm_object_signal_emit(icon, SIGNAL_HIGHLIGHT_FOLDER, SIGNAL_SOURCE);
		}
	}

	if (destination_icon_data && destination_icon_data->data->type == APP_ITEM_FOLDER) {
		s_info.destination_folder = destination_icon_data;
	} else {
		s_info.destination_folder = NULL;
	}
}

static void __app_grid_drag_done(void *data, Evas_Object *obj)
{
	Tree_node_t *folder = s_info.destination_folder;
	Tree_node_t *item = s_info.repositioned_node;
	Tree_node_t *src_folder = s_info.opened_folder_node;

	char trbuf[PATH_MAX] = {0, };

	/*Check if folder has empty space*/
	if (item && folder && folder_panel_get_items_count(folder) < FOLDER_MAX_ITEMS
		&& item->data->type != APP_ITEM_FOLDER) {
		home_screen_mvc_drag_item_to_folder_cb(item, folder);
		evas_object_hide(s_info.repositioned_icon);
	} else if (item && folder && folder_panel_get_items_count(folder) >= FOLDER_MAX_ITEMS) {
		popup_data_t *p_data = popup_create_data();
		if (!p_data) {
			LOGE("[FAILED][failed to create popup data]");
			return;
		}

		snprintf(trbuf, sizeof(trbuf), _("IDS_HS_TPOP_MAXIMUM_NUMBER_OF_APPLICATIONS_IN_FOLDER_HPD_REACHED"),
			FOLDER_APPS_PAGE);

		p_data->type = POPUP_INFO;
		p_data->popup_title_text = NULL;
		p_data->popup_content_text = strdup(trbuf);
		p_data->popup_confirm_text = NULL;
		p_data->popup_cancel_text = NULL;
		p_data->orientation = ELM_POPUP_ORIENT_CENTER;
		p_data->visible_time = 1.5;

		popup_show(p_data, NULL, NULL, NULL);

		__app_grid_return_to_origin_pos();
	} else if (item && src_folder && !folder_panel_is_folder_visible()) {
		home_screen_mvc_drag_item_from_folder_cb();
		elm_object_part_content_set(s_info.repositioned_icon_layout, PART_ICON_CONTENT,
				s_info.repositioned_icon);
		app_icon_set_view_mode(s_info.repositioned_icon_layout, home_screen_get_view_type(), false);
	} else {
		__app_grid_return_to_origin_pos();
	}

	if (folder && folder->data->layout)
		elm_object_signal_emit(folder->data->layout, SIGNAL_UNHIGHLIGHT_FOLDER, SIGNAL_SOURCE);

	__app_grid_clear_reposition_structure();
	all_apps_set_scrolling_blocked_state(false);
	folder_panel_enable_entry(true);
}
