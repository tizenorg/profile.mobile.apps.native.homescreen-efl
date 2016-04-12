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

#include <tizen.h>
#include <Elementary.h>
#include <app.h>
#include <app_preference.h>
#include <system_settings.h>
#include <alloca.h>
#include "homescreen-efl.h"
#include "add_viewer/add_viewer.h"
#include "key.h"
#include "option_menu.h"
#include "add_viewer/add_viewer.h"
#include "livebox/livebox_panel.h"
#include "livebox/livebox_utils.h"
#include "mouse.h"
#include "app_mgr.h"
#include "app_icon.h"
#include "all_apps.h"
#include "data_model.h"
#include "layout.h"
#include "folder_panel.h"
#include "livebox_all_pages.h"
#include "mouse.h"
#include "popup.h"

static struct {
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *gradient_bg;
	Evas_Object *conformant;
	Evas_Object *gesture_rect;
	Evas_Object *layout;
	Evas_Object *all_apps;
	Evas_Object *home;
	Elm_Gesture_Layer *gesture_layer;
	int root_width;
	int root_height;
	homescreen_view_t view_type;
	bool is_bg_blurred;
	Ecore_Animator* animator;
} s_info = {
	.win = NULL,
	.bg = NULL,
	.gradient_bg = NULL,
	.conformant = NULL,
	.gesture_rect = NULL,
	.layout = NULL,
	.all_apps = NULL,
	.home = NULL,
	.gesture_layer = NULL,
	.root_width = 0,
	.root_height = 0,
	.view_type = HOMESCREEN_VIEW_HOME,
	.is_bg_blurred = false,
	.animator = NULL
};

static bool mvc_badge_refresh_enable = true;

/*======================================UI HELPER FUNCTIONS========================*/
static void __homescreen_efl_create_base_gui(void);
static void __homescreen_efl_win_delete_request_cb(void *data, Evas_Object *obj, void *event_info);
static void __homescreen_efl_get_window_size(Evas_Object *win);
static void __homescreen_efl_home_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __homescreen_efl_menu_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __homescreen_efl_attach_badge_callback(void);
static void __homescreen_efl_detach_badge_callback(void);
static bool __homescreen_efl_update_icon_label(Tree_node_t *parent, Tree_node_t *node, void *data);
static void __homescreen_efl_make_gradient_bg(void);
static void __homescreen_efl_show_gradient_bg(void);
static void __homescreen_efl_show_normal_bg(void);

/*======================================APP CONTROL CALLBACKS=======================*/
static bool __homescreen_efl_app_create_cb(void *data);
static void __homescreen_efl_app_control_cb(app_control_h app_control, void *data);
static void __homescreen_efl_app_pause_cb(void *data);
static void __homescreen_efl_app_resume_cb(void *data);
static void __homescreen_efl_app_terminate_cb(void *data);


/*============================UI CALLBACKS==========================================*/
static void __homescreen_efl_lang_changed_cb(app_event_info_h event_info, void *user_data);
static void __homescreen_efl_orient_changed_cb(app_event_info_h event_info, void *user_data);
static void __homescreen_efl_region_changed_cb(app_event_info_h event_info, void *user_data);
static void __homescreen_efl_low_battery_cb(app_event_info_h event_info, void *user_data);
static void __homescreen_efl_low_memory_cb(app_event_info_h event_info, void *user_data);

/*====================  MVC Controller functions implementation: ===================*/
static bool __update_all_badges_count(Tree_node_t *parent, Tree_node_t *node, void *data);
static void __badge_change_cb(unsigned int action, const char *app_id, unsigned int count, void *user_data);
static bool __homescreen_efl_update_icon_badge(Tree_node_t *parent, Tree_node_t *node, void *data);
static void __homescreen_efl_badge_count_update_cb(void *app_item);
static void __homescreen_efl_activate_chooser_caption(void);
static void __homescreen_efl_deactivate_chooser_caption(void);
static void __homescreen_efl_chooser_caption_done_button_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __homescreen_efl_chooser_caption_cancel_button_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static bool __homescreen_efl_free_grid_item(Tree_node_t *page, Tree_node_t *item, void *data);
static bool __homescreen_efl_increment_page(Tree_node_t *parent, Tree_node_t *item, void *data);
static bool __homescreen_efl_attach_all_apps_grid_item(Tree_node_t *page, Tree_node_t *item, void *data);
static void __homescreen_efl_apply_data_model_to_view(void);
static void __homescreen_efl_show_all_apps(void);
static void __homescreen_efl_show_home_view(void);
static void __homescreen_efl_home_bg_changed_cb(system_settings_key_e key, void *data);
static void __homescreen_efl_apply_home_bg_effect(bool is_on);
static Eina_Bool __homescreen_efl_show_all_apps_anim(void *data, double pos);
static Eina_Bool __homescreen_efl_show_home_anim(void *data, double pos);
/*====================PUBLIC FUNCTIONS IMPLEMENTATION===============================*/

/*used to iterate over data model:*/
static bool folder_count_pending = false;
static unsigned int folder_badge_counter = 0;

DAPI int main(int argc, char *argv[])
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "Starting homescreen-efl");

	int ret = 0;
	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = __homescreen_efl_app_create_cb;
	event_callback.terminate = __homescreen_efl_app_terminate_cb;
	event_callback.pause = __homescreen_efl_app_pause_cb;
	event_callback.resume = __homescreen_efl_app_resume_cb;
	event_callback.app_control = __homescreen_efl_app_control_cb;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, __homescreen_efl_low_battery_cb, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, __homescreen_efl_low_memory_cb, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, __homescreen_efl_orient_changed_cb, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, __homescreen_efl_lang_changed_cb, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, __homescreen_efl_region_changed_cb, NULL);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, NULL);
	if (ret != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "ui_app_main() is failed. err = %d", ret);

	s_info.view_type = HOMESCREEN_VIEW_HOME;

	return ret;
}

HAPI int home_screen_get_root_width(void)
{
	return s_info.root_width;
}

HAPI int home_screen_get_root_height(void)
{
	return s_info.root_height;
}

HAPI Evas_Object *home_screen_get_win(void)
{
	return s_info.win;
}

HAPI Evas_Object *home_screen_get_layout(void)
{
	return s_info.layout;
}

HAPI homescreen_view_t home_screen_get_view_type(void)
{
	return s_info.view_type;
}

HAPI void home_screen_set_view_type(homescreen_view_t view)
{
	switch (view) {
	case HOMESCREEN_VIEW_HOME:
		__homescreen_efl_apply_home_bg_effect(false);
		__homescreen_efl_show_normal_bg();
		if (s_info.view_type == HOMESCREEN_VIEW_ALL_APPS) {
			all_apps_set_view_mode(view);
			__homescreen_efl_show_home_view();
		}
		else if(s_info.view_type == HOMESCREEN_VIEW_HOME_EDIT)
			elm_object_signal_emit(s_info.layout, SIGNAL_BOTTOM_BUTTONS_HOME_STATE_SET, SIGNAL_SOURCE);
		break;
	case HOMESCREEN_VIEW_HOME_EDIT:
		LOGI("HOMESCREEN_VIEW_HOME_EDIT");
		livebox_panel_set_edit_mode_layout(true);
		livebox_panel_change_edit_mode_state(false);
		elm_object_signal_emit(s_info.layout, SIGNAL_BOTTOM_BUTTONS_HOME_EDIT_STATE_SET, SIGNAL_SOURCE);
		break;
	case HOMESCREEN_VIEW_HOME_ADD_VIEWER:
		LOGI("HOMESCREEN_VIEW_ADD_VIEWER");
		add_viewer_window_create(s_info.root_width, s_info.root_height);
		break;
	case HOMESCREEN_VIEW_HOME_ALL_PAGES:
		LOGI("HOMESCREEN_VIEW_HOME_ALL_PAGES");
		livebox_all_pages_show();
		break;
	case HOMESCREEN_VIEW_ALL_APPS:
		__homescreen_efl_apply_home_bg_effect(true);
		__homescreen_efl_show_gradient_bg();
		if (s_info.view_type == HOMESCREEN_VIEW_HOME)
			__homescreen_efl_show_all_apps();

		if (s_info.view_type == HOMESCREEN_VIEW_ALL_APPS_CHOOSE)
			__homescreen_efl_deactivate_chooser_caption();
		all_apps_set_view_mode(view);
		elm_object_signal_emit(s_info.layout, SIGNAL_BOTTOM_BUTTONS_ALL_APPS_STATE_SET, SIGNAL_SOURCE);
		__badge_change_cb(0, NULL, 0, NULL);
		break;
	case HOMESCREEN_VIEW_ALL_APPS_EDIT:
		LOGI("HOMESCREEN_VIEW_ALL_APPS_EDIT");
		if (s_info.view_type == HOMESCREEN_VIEW_ALL_APPS_CHOOSE)
			__homescreen_efl_deactivate_chooser_caption();
		all_apps_set_view_mode(view);
		elm_object_signal_emit(s_info.layout, SIGNAL_BOTTOM_BUTTONS_ALL_APPS_EDIT_STATE_SET, SIGNAL_SOURCE);
		break;
	case HOMESCREEN_VIEW_ALL_APPS_CHOOSE:
		LOGI("HOMESCREEN_VIEW_ALL_APPS_CHOOSE");
		__homescreen_efl_activate_chooser_caption();
		all_apps_set_view_mode(view);
		break;
	case HOMESCREEN_VIEW_UNKNOWN:
		LOGI("HOMESCREEN_VIEW_UNKNOWN");
		break;
	default:
		LOGE("UKNOWN VIEW TYPE");
		break;
	}

	s_info.view_type = view;
}

HAPI void home_screen_set_indice_state(indice_type_t indice)
{
	switch (indice) {
	case INDICE_NEXT_PAGE_ON:
		elm_object_signal_emit(s_info.layout, SIGNAL_NEXT_PAGE_INDICE_SHOW, SIGNAL_SOURCE);
		break;
	case INDICE_PREV_PAGE_ON:
		elm_object_signal_emit(s_info.layout, SIGNAL_PREV_PAGE_INDICE_SHOW, SIGNAL_SOURCE);
		break;
	case INDICE_OFF:
		elm_object_signal_emit(s_info.layout, SIGNAL_INDICE_HIDE, SIGNAL_SOURCE);
		break;
	default:
		break;
	}
}

HAPI void home_screen_mvc_set_ly_scale(Evas_Object *layout)
{
	Evas_Object *edje = NULL;
	int width = -1;
	int height = -1;

	if (!layout) {
		LOGE("[FAILED][invalid parameter]");
		return;
	}

	edje = elm_layout_edje_get(layout);

	width = s_info.root_width;
	height = s_info.root_height;

	if (width == HD_WIDTH && height == HD_HEIGHT) {
		LOGD("HD_RES");
		edje_object_scale_set(edje, HD_SCALE_FACTOR);
	} else if(width == QHD_WIDTH && height == QHD_HEIGHT) {
		LOGD("QHD_RES");
		edje_object_scale_set(edje, QHD_SCALE_FACTOR);
	} else if(width == WVGA_WIDTH && height == WVGA_HEIGHT) {
		LOGD("WVGA_RES");
		edje_object_scale_set(edje, WVGA_SCALE_FACTOR);
	} else if(width == UHD_WIDTH && height == UHD_HEIGHT) {
		edje_object_scale_set(edje, UHD_SCALE_FACTOR);
	}
}

HAPI void home_screen_close_all_apps_choose_view(void)
{
	folder_panel_cancel_add_icon();
	folder_panel_set_click_ignore_state(false);
	data_model_check_all_apps(data_model_get_all_apps(), false);
	home_screen_set_view_type(HOMESCREEN_VIEW_ALL_APPS);
}

HAPI int home_screen_gesture_cb_set(Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
	Elm_Gesture_Layer *gesture_layer = s_info.gesture_layer;

	if (!gesture_layer) {
		LOGE("Can not set callback");
		return -1;
	}

	elm_gesture_layer_cb_set(gesture_layer, idx, cb_type, cb, NULL);

	return 0;
}

HAPI int home_screen_gesture_cb_unset(Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
	Elm_Gesture_Layer *gesture_layer = s_info.gesture_layer;

	if (!gesture_layer) {
		LOGE("Can not unset callback");
		return -1;
	}

	elm_gesture_layer_cb_del(gesture_layer, idx, cb_type, cb, NULL);

	return 0;
}

/*====================END OF PUBLIC FUNCTIONS IMPLEMENTATION=========================*/

static void __homescreen_efl_win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static Evas_Object *_create_conformant(void)
{
	Evas_Object *conformant = NULL;

	if (!s_info.win) {
		LOGE("Window is not created");
		return NULL;
	}

	conformant = elm_conformant_add(s_info.win);
	if (!conformant) {
		LOGE("Failed to create conformant");
		return NULL;
	}
	elm_win_conformant_set(s_info.win, EINA_TRUE);

	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(s_info.win, conformant);

	elm_win_indicator_mode_set(s_info.win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(s_info.win, ELM_WIN_INDICATOR_TRANSLUCENT);
	elm_object_signal_emit(conformant, "elm,state,indicator,overlap", "elm");

	evas_object_show(conformant);

	return conformant;
}

static Evas_Object *_create_gesture_rect(Evas *e, int w, int h)
{
	LOGD("");
	Evas_Object *gesture_rect = NULL;

	if (!s_info.win) {
		LOGE("Window is not created");
		return NULL;
	}

	gesture_rect = evas_object_rectangle_add(e);
	if(!gesture_rect) { LOGE("Failed to create gesture rectangle");
		return NULL;
	}

	evas_object_layer_set(gesture_rect, EVAS_LAYER_MAX);
	evas_object_color_set(gesture_rect, 0, 0, 0, 0);
	evas_object_geometry_set(gesture_rect, 0, 0, w, h);
	evas_object_repeat_events_set(gesture_rect, EINA_TRUE);

	evas_object_show(gesture_rect);

	return gesture_rect;
}

static Elm_Gesture_Layer *_create_gesture_layer(void)
{
	Elm_Gesture_Layer *gesture_layer = NULL;
	Eina_Bool ret = EINA_FALSE;

	if(!s_info.gesture_rect) {
		LOGE("Gesture rectangle is not created");
		return NULL;
	}

	gesture_layer = elm_gesture_layer_add(s_info.win);
	if(!gesture_layer) {
		LOGE("Failed to create gesture layer");
		return NULL;
	}

	ret = elm_gesture_layer_attach(gesture_layer, s_info.gesture_rect);
	if(ret == EINA_FALSE) {
		LOGE("Failed to attach gesture layer to gesture rectangle");
		free(gesture_layer);
		return NULL;
	}


	return gesture_layer;
}

static void __homescreen_efl_create_base_gui(void)
{
	char edj_path[PATH_MAX] = {0, };
	const char *bg_path = util_get_res_file_path(IMAGE_DIR"/default_bg.png");
	char trbuf[PATH_MAX] = {0, };
	char *buf = NULL;
	int ret = -1;
	Evas *e_conf = NULL;

	/* Window */
	s_info.win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	// evas_object_resize(s_info.win, 1440, 2560);
	elm_win_autodel_set(s_info.win, EINA_TRUE);

	evas_object_smart_callback_add(s_info.win, "delete,request", __homescreen_efl_win_delete_request_cb, NULL);

	/*Background*/
	s_info.bg = evas_object_image_filled_add(evas_object_evas_get(s_info.win));

	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, &buf);

	if (!buf || ret != SYSTEM_SETTINGS_ERROR_NONE || !ecore_file_exists(buf)
			|| !ecore_file_can_read(buf))
		evas_object_image_file_set(s_info.bg, bg_path, "bg");
	else
		evas_object_image_file_set(s_info.bg, buf, "bg");

	free(buf);

	system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, __homescreen_efl_home_bg_changed_cb, NULL);

	elm_win_resize_object_add(s_info.win, s_info.bg);
	evas_object_show(s_info.bg);

	__homescreen_efl_get_window_size(s_info.win);

	/* Conformant */
	s_info.conformant = _create_conformant();

	/* Base Layout */
	snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/home.edj"));
	s_info.layout = elm_layout_add(s_info.win);
	elm_layout_file_set(s_info.layout, edj_path, GROUP_HOME_LY);
	evas_object_size_hint_weight_set(s_info.layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(s_info.win, s_info.layout);
	evas_object_show(s_info.layout);

	home_screen_mvc_set_ly_scale(s_info.layout);

	s_info.home = livebox_panel_create(s_info.layout);
	if (!s_info.home) {
		evas_object_del(s_info.win);
		evas_object_del(s_info.bg);
		evas_object_del(s_info.conformant);
		evas_object_del(s_info.layout);

		LOGE("s_info.home == NULL");
		return;
	}

	e_conf = evas_object_evas_get(s_info.conformant);
	s_info.gesture_rect = _create_gesture_rect(e_conf, s_info.root_width, s_info.root_height);
	s_info.gesture_layer = _create_gesture_layer();

	elm_object_part_content_set(s_info.layout, PART_CONTENT, s_info.home);

	/* Show window after base gui is set up */
	dlog_print(DLOG_INFO, LOG_TAG ,"WINDOW SIZE IS : %d %d", s_info.root_width, s_info.root_height);
	evas_object_show(s_info.win);
	elm_object_signal_callback_add(s_info.layout, SIGNAL_HOME_BTN_CLICKED, SIGNAL_SOURCE, __homescreen_efl_home_btn_clicked, NULL);
	elm_object_signal_callback_add(s_info.layout, SIGNAL_MENU_BTN_CLICKED, SIGNAL_SOURCE, __homescreen_efl_menu_btn_clicked, NULL);

	/*Chooser caption setup:*/
	elm_layout_signal_callback_add(s_info.layout, "mouse,clicked,*",
		"chooser_caption_left_clickable_space",
		__homescreen_efl_chooser_caption_cancel_button_cb, NULL);
	elm_layout_signal_callback_add(s_info.layout, "mouse,clicked,*",
		"chooser_caption_right_clickable_space",
		__homescreen_efl_chooser_caption_done_button_cb, NULL);

	snprintf(trbuf, sizeof(trbuf), _("IDS_MEMO_HEADER_PD_SELECTED_ABB2"), 0);
	layout_main_caption_set_menu_texts(trbuf,
		_("IDS_TPLATFORM_ACBUTTON_CANCEL_ABB"),
		_("IDS_TPLATFORM_ACBUTTON_DONE_ABB"));
}

/*TODO: coding convention!*/
bool tree_print_cb(Tree_node_t *parent_node, Tree_node_t *tn, void *data)
{
	app_item_t *item = tn->data;
	app_item_t *parent = NULL;

	if (tn->parent)
		parent = tn->parent->data;

	LOGD("TYPE: %2d; PARENT name: %12.12s; id: %2d | ITEM appid: %28.28s; id: %2d; [%d, %d; %d, %d]; %s",
			item->type,
			parent ? parent->appid  : "NULL",
			parent ? parent->unique_id : -1,
			item->appid,
			item->unique_id,
			item->col,
			item->row,
			item->col_span,
			item->row_span,
			item->content_info);

	return true;
}

/*TODO: coding convention!*/
void home_screen_print_tree(void)
{
	LOGD("TREE TEST START");
	data_model_iterate(data_model_get_data(), tree_print_cb, NULL);
}

static void __homescreen_efl_attach_badge_callback(void)
{
	app_mgr_register_badge_callback(__badge_change_cb);
}

static void __homescreen_efl_detach_badge_callback(void)
{
	app_mgr_unregister_badge_callback(__badge_change_cb);
}

static bool __homescreen_efl_app_create_cb(void *data)
{
	elm_config_accel_preference_set("3d");
	ecore_animator_frametime_set(FRAMES_PER_SECOND);

	key_register();
	mouse_register();

	__homescreen_efl_create_base_gui();
	folder_panel_init_module();
	app_mgr_init();

	if(!data_model_load_db()) {
		data_model_load_app_mgr();
	}

	livebox_panel_fill();
	livebox_panel_set_content(s_info.layout);
	__homescreen_efl_attach_badge_callback();

	home_screen_mvc_update_view();
	return true;
}

static void __homescreen_efl_app_control_cb(app_control_h app_control, void *data)
{
	elm_win_activate(s_info.win);
}

static void __homescreen_efl_app_pause_cb(void *data)
{
	/* Take necessary actions when application becomes invisible. */
	livebox_panel_paused();
}

static void __homescreen_efl_app_resume_cb(void *data)
{
	/* Take necessary actions when application becomes visible. */
	livebox_panel_resumed();
}

static void __homescreen_efl_app_terminate_cb(void *data)
{
	/* Release all resources. */
	__homescreen_efl_detach_badge_callback();
	mouse_unregister();
	key_unregister();
	app_mgr_deinit();
	all_apps_del();
	popup_destroy();
	folder_panel_deinit_module();
	data_model_free();
	livebox_panel_destroy();
}

static bool __homescreen_efl_update_icon_label(Tree_node_t *parent, Tree_node_t *node, void *data)
{
	if(!node || !node->data)
		return false;

	/*update model*/
	if (node->data->label && node->data->type == APP_ITEM_ICON) {
		free(node->data->label);
		node->data->label = app_mgr_get_app_label(node->data->appid);
	}

	/*update view*/
	elm_object_part_text_set(node->data->layout, PART_ICON_NAME, node->data->label);

	return true;
}

static void __homescreen_efl_make_gradient_bg(void)
{
#ifdef HOME_HOME_SCREEN_GRADIENT_ENABLED
	Evas_Map *m = NULL;
	Evas_Colorspace ec = EVAS_COLORSPACE_ARGB8888;
	char *image_raw_data = NULL;
	int image_width = 0;
	int image_height = 0;
	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;

	if(!s_info.bg)
		return;

	ec = evas_object_image_colorspace_get(s_info.bg);

	if(ec != EVAS_COLORSPACE_ARGB8888)
		return;

	image_raw_data = evas_object_image_data_get(s_info.bg, EINA_FALSE);
	if(!image_raw_data)
		return;

	evas_object_image_size_get(s_info.bg, &image_width, &image_height);
	if(image_width == 0 || image_height == 0)
		return;

	if(!s_info.gradient_bg)
		s_info.gradient_bg = evas_object_rectangle_add(evas_object_evas_get(s_info.win));

	if(!s_info.gradient_bg)
		return;

	evas_object_resize(s_info.gradient_bg, s_info.root_width, s_info.root_height);

	m = evas_map_new(4);
	if(!m)
		return;

	evas_map_util_points_populate_from_object(m, s_info.gradient_bg);

	r = image_raw_data[2] & 0xff;
	g = image_raw_data[1] & 0xff;
	b = image_raw_data[0] & 0xff;
	a = image_raw_data[3] & 0xff;
	evas_map_point_color_set(m, 0, r, g, b, a);

	r = image_raw_data[4*image_width -4 + 2] & 0xff;
	g = image_raw_data[4*image_width -4 + 1] & 0xff;
	b = image_raw_data[4*image_width -4 + 0] & 0xff;
	a = image_raw_data[4*image_width -4 + 3] & 0xff;
	evas_map_point_color_set(m, 1, r, g, b, a);

	r = image_raw_data[4*image_width -4 + (image_height-1)*4*image_width + 2] & 0xff;
	g = image_raw_data[4*image_width -4 + (image_height-1)*4*image_width + 1] & 0xff;
	b = image_raw_data[4*image_width -4 + (image_height-1)*4*image_width + 0] & 0xff;
	a = image_raw_data[4*image_width -4 + (image_height-1)*4*image_width + 3] & 0xff;
	evas_map_point_color_set(m, 2, r, g, b, a);

	r = image_raw_data[(image_height-1)*4*image_width + 2] & 0xff;
	g = image_raw_data[(image_height-1)*4*image_width + 1] & 0xff;
	b = image_raw_data[(image_height-1)*4*image_width + 0] & 0xff;
	a = image_raw_data[(image_height-1)*4*image_width + 3] & 0xff;
	evas_map_point_color_set(m, 3, r, g, b, a);

	evas_object_map_set(s_info.gradient_bg, m);
	evas_object_map_enable_set(s_info.gradient_bg, EINA_TRUE);
	evas_map_free(m);
#endif
}

static void __homescreen_efl_show_gradient_bg(void)
{
#ifdef HOME_HOME_SCREEN_GRADIENT_ENABLED
	if(s_info.gradient_bg) {
		evas_object_hide(s_info.bg);
		elm_win_resize_object_del(s_info.win, s_info.bg);
		elm_win_resize_object_add(s_info.win, s_info.gradient_bg);
		evas_object_show(s_info.gradient_bg);
		evas_object_raise(s_info.layout);
	}
#endif
}

static void __homescreen_efl_show_normal_bg(void)
{
#ifdef HOME_HOME_SCREEN_GRADIENT_ENABLED
	evas_object_hide(s_info.gradient_bg);
	elm_win_resize_object_del(s_info.win, s_info.gradient_bg);
	elm_win_resize_object_add(s_info.win, s_info.bg);
	evas_object_show(s_info.bg);
	evas_object_raise(s_info.layout);
#endif
}

static void __homescreen_efl_lang_changed_cb(app_event_info_h event_info, void *user_data)
{
	data_model_iterate(NULL, __homescreen_efl_update_icon_label, NULL);
}

static void __homescreen_efl_orient_changed_cb(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
}

static void __homescreen_efl_region_changed_cb(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void __homescreen_efl_low_battery_cb(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void __homescreen_efl_low_memory_cb(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

static void __homescreen_efl_get_window_size(Evas_Object *win)
{
	elm_win_screen_size_get(win, NULL, NULL, &s_info.root_width, &s_info.root_height);
	dlog_print(DLOG_DEBUG, LOG_TAG, "Width: [%d], Height: [%d]", s_info.root_width, s_info.root_height);
}

static void __homescreen_efl_home_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	if (s_info.animator == NULL) {
		homescreen_view_t view_t = home_screen_get_view_type();
		if (view_t == HOMESCREEN_VIEW_HOME) {
			home_screen_set_view_type(HOMESCREEN_VIEW_ALL_APPS);
		} else if (view_t == HOMESCREEN_VIEW_ALL_APPS) {
			home_screen_set_view_type(HOMESCREEN_VIEW_HOME);
		}
	}
}

static void __homescreen_efl_menu_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	homescreen_view_t view = home_screen_get_view_type();

	switch (view) {
	case HOMESCREEN_VIEW_HOME:
	case HOMESCREEN_VIEW_ALL_APPS:
		option_menu_show();
		break;
	default:
		break;
	}
}

/*====================  MVC Controller functions implementation: ===================*/

/*TODO: code convention!*/
void home_screen_mvc_item_uninstall_cb(Tree_node_t *item)
{
	if (!item || !item->data)
		return;

	if (item->data->type == APP_ITEM_ICON) {
		app_mgr_uninstall_app(item->data->appid);
		data_model_uninstall_application(item);
	} else {/*folder item type:*/
		data_model_delete_folder(item);
	}

	/*by default model is being sorted:*/
	data_model_sort(NULL);
	data_model_free_empty_pages(NULL);
	__homescreen_efl_apply_data_model_to_view();
	all_apps_remove_empty_pages();
}

void home_screen_mvc_update_view(void)
{
	data_model_sort(NULL);
	__homescreen_efl_apply_data_model_to_view();
}

void home_screen_mvc_drag_item_to_folder_cb(Tree_node_t *dragged_item, Tree_node_t *folder)
{
	if (dragged_item && folder) {
		if (!folder->last)
			data_model_append_node_to_folder(folder, dragged_item); /*there were no pages in folder*/
		else
			data_model_reposition_item(dragged_item, folder->last, folder->last->last, REPOSITION_SIDE_TO_RIGHT, FOLDER_APPS_PAGE);

		/*for now model is always sorted:*/
		data_model_sort(NULL);
		data_model_free_empty_pages(NULL);
		__homescreen_efl_apply_data_model_to_view();
		all_apps_remove_empty_pages();
	}
}

/*TODO:All mvc funcitons will be moved to separeted file.*/
void home_screen_mvc_drag_item_from_folder_cb(void)
{
	__homescreen_efl_apply_data_model_to_view();
}

static void __homescreen_efl_activate_chooser_caption(void)
{
	data_model_check_all_apps(data_model_get_all_apps(), false);
	elm_layout_signal_emit(s_info.layout, SIGNAL_CHOOSER_CAPTION_ACTIVATE, SIGNAL_SOURCE);
	elm_win_indicator_mode_set(s_info.win, ELM_WIN_INDICATOR_HIDE);
}

static void __homescreen_efl_deactivate_chooser_caption(void)
{
	elm_layout_signal_emit(s_info.layout, SIGNAL_CHOOSER_CAPTION_DEACTIVATE, SIGNAL_SOURCE);
	elm_win_indicator_mode_set(s_info.win, ELM_WIN_INDICATOR_SHOW);
}

static bool __homescreen_efl_free_grid_item(Tree_node_t *page, Tree_node_t *item, void *data)
{
	if (!item->data || !item->data->grid_item) {
		LOGE("__homescreen_efl_free_grid_item, skipping: %d %d", item->data, item->data ? item->data->type : -1);
		return true;
	}

	if (!item->data->layout)
		LOGD("__homescreen_efl_free_grid_item, %s item->data->layout == NULL", item->data->label);

	all_apps_detach_from_grid(item->data->grid_item);
	item->data->grid_item = NULL;
	evas_object_hide(item->data->layout);

	return true;
}

static bool __homescreen_efl_increment_page(Tree_node_t *parent, Tree_node_t *item, void *data)
{
	(*((int *)data))++;

	return true;
}

static bool __homescreen_efl_attach_all_apps_grid_item(Tree_node_t *page, Tree_node_t *item, void *data)
{
	if (!item->data) {
		LOGE("__homescreen_efl_attach_all_apps_grid_item, skipping: %d", item->data);
		return true;
	}

	item->data->grid_item = all_apps_attach_to_grid(item->data->layout, *(int *)data);

	if (!item->data->grid_item) {
	/*failed to attach item - there is no room in page scroller
	so we have to add another page. This problem does not concern folders.*/

		if (!all_apps_append_page(page))
			LOGE("Could not create page for item: %s", item->data->label);
	}

	app_icon_set_view_mode(item->data->layout, home_screen_get_view_type(), false);

	return true;
}

static bool _folder_grid_item_att(Tree_node_t *page, Tree_node_t *item, void *data)
{
	if (!item->data || !page->parent || !page->parent->data ||
		page->parent->data->type != APP_ITEM_FOLDER ||
		folder_panel_get_opened_folder_item() != page->parent->data) {
		LOGE("_folder_grid_item_att, skipping: %s dest_folder: %d", item->data ? item->data->appid : NULL);
		return true;
	}

	item->data->grid_item = folder_panel_attach_to_grid(item->data->layout, ((app_item_t *)page->parent->data)->data);

	return true;
}

static bool _folder_data_model_to_view_apply_cb(Tree_node_t *page, Tree_node_t *item, void *data)
{
	if (!item->data || item->data->type != APP_ITEM_FOLDER)
		return true;

	data_model_iterate_pages(item, NULL, __homescreen_efl_free_grid_item, NULL);
	data_model_iterate_pages(item, NULL, _folder_grid_item_att, NULL);
	app_icon_update_folder_icon(item);
	return true;
}

static void __homescreen_efl_apply_data_model_to_view(void)
{
	int page_counter = -1;
	Tree_node_t *apps_root = data_model_get_all_apps();

	if (!apps_root || !apps_root->data || apps_root->data->type != APP_ITEM_ALL_APPS_ROOT) {
		LOGE("Can't apply view [apps root]: %d %d", apps_root, apps_root ? apps_root->data : NULL);
		return;
	}

	mvc_badge_refresh_enable = false;

	data_model_iterate_pages(NULL, NULL, __homescreen_efl_free_grid_item, NULL);
	data_model_iterate_pages(NULL, NULL, _folder_data_model_to_view_apply_cb, NULL);
	data_model_iterate_pages(NULL, __homescreen_efl_increment_page, __homescreen_efl_attach_all_apps_grid_item, &page_counter);
	all_apps_update_dynamic_index_count();

	mvc_badge_refresh_enable = true;

	__badge_change_cb(0, NULL, 0, NULL);
}

static bool _checked_items_get(Tree_node_t *root, Tree_node_t *item, void *list)
{
	Eina_List **checked_items_list = (Eina_List **)list;

	if (item->data && item->data->is_checked) {
		*checked_items_list = eina_list_append(*checked_items_list, item);
		item->data->is_checked = false;
	}

	return true;
}

static void __homescreen_efl_chooser_caption_done_button_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	/*Folder creation/modification:*/
	app_item_t *destination_folder_data = NULL;
	Eina_List *iterator = NULL, *checked_items_list = NULL;
	Tree_node_t *item = NULL;
	Tree_node_t *destination_folder = folder_panel_get_opened_folder_node();

	if (!destination_folder) {
		destination_folder_data = app_item_create(APP_ITEM_FOLDER, -1, "", NULL, NULL, NULL, NULL, true, 0, 0, 0, 0, NULL);

		if (!destination_folder_data) {
			LOGE("caption_done_button_cb: failed to create destination_folder_data");
			return;
		}

		destination_folder = data_model_create_folder(destination_folder_data);

		if (!destination_folder) {
			LOGE("caption_done_button_cb: failed to create destination_folder");
			folder_panel_cancel_add_icon();
			folder_panel_set_click_ignore_state(false);
			app_item_free(destination_folder_data);
			return;
		}
		folder_panel_create(destination_folder);
	}

	/*all checked items should be added to the folder:*/
	data_model_iterate(NULL, _checked_items_get, &checked_items_list);

	EINA_LIST_FOREACH(checked_items_list, iterator, item)
		data_model_append_node_to_folder(destination_folder, item);

	eina_list_free(checked_items_list);
	/*for now they are always sorted:*/
	data_model_sort(NULL);

	folder_panel_cancel_add_icon();
	folder_panel_set_click_ignore_state(false);

	data_model_free_empty_pages(NULL);
	home_screen_set_view_type(HOMESCREEN_VIEW_ALL_APPS);
	__homescreen_efl_apply_data_model_to_view();
	all_apps_remove_empty_pages();
}

static void __homescreen_efl_chooser_caption_cancel_button_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	home_screen_close_all_apps_choose_view();
}

static void __homescreen_efl_show_all_apps(void)
{
	livebox_panel_unset_content(s_info.layout);
	all_apps_show();

	if (!s_info.all_apps)
		s_info.all_apps = all_apps_create(s_info.layout, NULL);

	elm_object_part_content_unset(s_info.layout, PART_CONTENT);

	evas_object_hide(s_info.all_apps);
	evas_object_move(s_info.all_apps, 0, s_info.root_height);

	elm_object_signal_emit(s_info.layout, SIGNAL_BLOCK_EVENTS, SIGNAL_SOURCE);
	app_icon_set_click_ignore(true);

	s_info.animator = ecore_animator_timeline_add(APPS_SHOW_HIDE_ANIMATION_TIME, __homescreen_efl_show_all_apps_anim, NULL);
}

static void __homescreen_efl_show_home_view(void)
{
	elm_object_part_content_unset(s_info.layout, PART_CONTENT);
	all_apps_hide();
	elm_object_signal_emit(s_info.layout, SIGNAL_BOTTOM_BUTTONS_HOME_STATE_SET, SIGNAL_SOURCE);

	elm_object_signal_emit(s_info.layout, SIGNAL_BLOCK_EVENTS, SIGNAL_SOURCE);
	app_icon_set_click_ignore(true);

	s_info.animator = ecore_animator_timeline_add(APPS_SHOW_HIDE_ANIMATION_TIME, __homescreen_efl_show_home_anim, NULL);
}

static bool __update_all_badges_count(Tree_node_t *parent, Tree_node_t *node, void *data)
{
	app_item_t *item = NULL;

	if (node) {
		item = node->data;
		__homescreen_efl_badge_count_update_cb(item);
	}

	return true;
}

static void __badge_change_cb(unsigned int action, const char *app_id, unsigned int count, void *user_data)
{
	data_model_iterate(NULL, __update_all_badges_count, NULL);
}

static bool __homescreen_efl_update_icon_badge(Tree_node_t *parent, Tree_node_t *node, void *data)
{
	app_item_t *app = (app_item_t *)data;

	if (!folder_count_pending && app != node->data)
		return true;

	if (!folder_count_pending && app == node->data) {
		folder_count_pending = true;
		data_model_iterate(node, __homescreen_efl_update_icon_badge, app);
		folder_count_pending = false;
	}

	if (folder_count_pending && node->data->type == APP_ITEM_ICON) {
		app_mgr_app_get_badge_count(node->data->appid, &node->data->badge_count);
		folder_badge_counter += node->data->badge_count;
	}

	if (!folder_count_pending && app == node->data)
		return false;

	return true;
}

static void __homescreen_efl_badge_count_update_cb(void *app_item)
{
	app_item_t *app = (app_item_t *)app_item;

	if (!mvc_badge_refresh_enable)
		return;

	if (!app) {
		LOGE("badge_count_update_cb: app_item is NULL");
		return;
	}

	if (app->type == APP_ITEM_ICON && !app_mgr_app_get_badge_count(app->appid, &app->badge_count)) {
		LOGE("badge_count_update_cb: get_badge_count returned error");
		return;
	}

	if (app->type == APP_ITEM_FOLDER) {
		folder_count_pending = false;
		folder_badge_counter = 0;
		data_model_iterate(NULL, __homescreen_efl_update_icon_badge, app);
		app->badge_count = folder_badge_counter;
	}

	app_icon_show_badge(app->layout, app->badge_count);
}

static void __homescreen_efl_home_bg_changed_cb(system_settings_key_e key, void *data)
{
	char *buf = NULL;
	int ret = -1;

	if (!s_info.bg) {
		LOGE("[FAILED][bg=NULL]");
		return;
	}

	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, &buf);
	if (!buf || ret != SYSTEM_SETTINGS_ERROR_NONE || !ecore_file_exists(buf)
			|| !ecore_file_can_read(buf)) {
		LOGE("[FAILED][failed to get bg path]");
		return;
	}

	evas_object_image_file_set(s_info.bg, buf, "bg");
	free(buf);

	__homescreen_efl_make_gradient_bg();
}

static void __homescreen_efl_apply_home_bg_effect(bool is_on)
{
	#ifndef HOME_HOME_SCREEN_BLUR_ENABLED
		return;
	#endif
	char *buf = NULL;

	/*source data and destination data (pixel values)*/
	uint32_t *src = NULL, *dst = NULL;

	/*values of channels in each pixel*/
	uint32_t A = 0, R = 0, G = 0, B = 0;

	/*helper variables*/
	uint32_t *s = NULL, *d = NULL, w = 0, p = 0;

	int width = -1, height = -1;

	/*iterators*/
	int i = 0, j = 0, k = 0;

	/*kernel used for transform the image*/
	uint8_t kernel[25];
	const int size = COUNT_OF(kernel);
	const int half = size / 2;

	int ret = -1;

	if (!s_info.bg) {
		LOGE("[FAILED][bg=NULL]");
		return;
	}

	/*if is_on == false, then restore original image data*/
	if (!is_on) {
		/*restore original data and return;*/
		ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, &buf);
		if (!buf || ret != SYSTEM_SETTINGS_ERROR_NONE || !ecore_file_exists(buf)
				|| !ecore_file_can_read(buf)) {

			LOGE("[FAILED][failed to get bg path]");
			return;
		}

		evas_object_image_file_set(s_info.bg, buf, "bg");
		free(buf);
		s_info.is_bg_blurred = false;
		return;
	}

	evas_object_image_size_get(s_info.bg, &width, &height);
	src = evas_object_image_data_get(s_info.bg, EINA_TRUE);

	/*if image is already blurred then we should not perform once again*/
	if (s_info.is_bg_blurred)
		return;

	dst = (uint32_t *) alloca(sizeof(uint32_t) * width * height);
	if (!dst) {
		LOGE("[FAILED][dst=NULL]");
		return;
	}

	/*Fill the kernel*/
	w = 0;
	for (i = 0; i < size; i++) {
		double f = i - half;
		w += kernel[i] = exp(-f * f / 60.0) * 40;
	}

	/*Apply modifications from src - > dst*/
	for (i = 0; i < height; i++) {
		s = (uint32_t *) (src + i * width);
		d = (uint32_t *) (dst + i * width);

		for (j = 0; j < width; j++) {
			A = R = G = B = 0;
			for (k = 0; k < size; k++) {
				if (j - half + k < 0 || j - half + k >= width) {
					d[j] = s[j];
					continue;
				}

				p = s[j - half + k];

				A = (p & 0xff000000);
				R += ((p >> 16) & 0xff) * kernel[k];
				G += ((p >>  8) & 0xff) * kernel[k];
				B += ((p >>  0) & 0xff) * kernel[k];
			}
			d[j] = A | (((uint32_t)(R / w)) << 16) | (((uint32_t)(G / w)) << 8) | ((uint32_t)(B / w));
		}
	}

	/*Apply modification from dst -> srcs*/
	for (i = 0; i < height; i++) {
		s = (uint32_t *) (dst + i * width);
		d = (uint32_t *) (src + i * width);
		for (j = 0; j < width; j++) {
			A = R = G = B = 0;
			for (k = 0; k < size; k++) {
				if (i - half + k < 0 || i - half + k >= height) {
					d[j] = s[j];
					continue;
				}

				s = (uint32_t *) (dst + (i - half + k) * width);
				p = s[j];

				A = (p & 0xff000000);
				R += ((p >> 16) & 0xff) * kernel[k];
				G += ((p >>  8) & 0xff) * kernel[k];
				B += ((p >>  0) & 0xff) * kernel[k];
			}
			d[j] = A | (((uint32_t)(R / w)) << 16) | (((uint32_t)(G / w)) << 8) | ((uint32_t)(B / w));
		}
	}

	s_info.is_bg_blurred = true;
	evas_object_image_data_update_add(s_info.bg, 0, 0, width, height);
}

static Eina_Bool __homescreen_efl_show_all_apps_anim(void *data, double pos)
{
	Evas_Object *box = elm_object_content_get(s_info.all_apps);

	evas_object_show(s_info.all_apps);
	evas_object_move(s_info.all_apps, 0, (1.0 - pos) * ALL_APPS_ANIMATION_DELTA_Y);
	evas_object_move(s_info.home, 0, -pos * ALL_APPS_ANIMATION_HOME_DELTA_Y);

	evas_object_color_set(box, 255, 255, 255, pos*(double)255);

	if (pos >= 0.99) {
		elm_object_part_content_set(s_info.layout, PART_CONTENT, s_info.all_apps);
		all_apps_show();
		elm_object_signal_emit(s_info.layout, SIGNAL_UNBLOCK_EVENTS, SIGNAL_SOURCE);
		s_info.animator = NULL;
		return ECORE_CALLBACK_DONE;
	}

	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool __homescreen_efl_show_home_anim(void *data, double pos)
{
	Evas_Object *box = elm_object_content_get(s_info.all_apps);

	evas_object_move(s_info.all_apps, 0, pos * ALL_APPS_ANIMATION_DELTA_Y);
	evas_object_move(s_info.home, 0, -(1.0 - pos ) * ALL_APPS_ANIMATION_HOME_DELTA_Y);

	evas_object_color_set(box, 255, 255, 255, (1.0 - pos * 2.0)*(double)255);

	if (pos >= 0.99) {
		if (s_info.layout && elm_object_part_content_get(s_info.layout, PART_CONTENT) == NULL && s_info.home) {
			elm_object_part_content_set(s_info.layout, PART_CONTENT, s_info.home);
			livebox_panel_set_content(s_info.layout);
		}

		elm_object_signal_emit(s_info.layout, SIGNAL_UNBLOCK_EVENTS, SIGNAL_SOURCE);
		s_info.animator = NULL;
		return ECORE_CALLBACK_DONE;
	}

	return ECORE_CALLBACK_RENEW;
}
