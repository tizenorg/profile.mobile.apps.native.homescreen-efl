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

#include "option_menu.h"
#include "homescreen-efl.h"
#include "app_grid.h"
#include "all_apps.h"


#include "livebox/livebox_panel.h"
#include "util.h"
#include "app_icon.h"
#include "folder_panel.h"
#include "popup.h"
#include "data_model.h"
#include "layout.h"
#include "livebox_all_pages.h"

static struct {
	Evas_Object *option_menu;
	Evas_Object *option_menu_background;
	bool is_visible;
} s_info;

/*Create the background with the clicked callback under the menu*/
static void __option_menu_create_background(void);
static void __option_menu_background_clicked_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __option_menu_dismissed_cb(void *data, Evas_Object *obj, void *event_info);

/*All Apps callbacks*/
static void __option_menu_entry_all_apps_edit_apps_cb(void *data, Evas_Object *obj, void *event_info);
static void __option_menu_entry_all_apps_create_folder_cb(void *data, Evas_Object *obj, void *event_info);

/*Home callback*/
static void __option_menu_entry_all_apps_edit_home_cb(void *data, Evas_Object *obj, void *event_info);

/*Wallpapers callbacks*/
static void __option_menu_entry_all_apps_wallpaper_cb(void *data, Evas_Object *obj, void *event_info);

/*Add widget*/
static void __option_menu_entry_all_apps_add_widget_cb(void *data, Evas_Object *obj, void *event_info);

/*View all pages*/
static void __option_menu_entry_all_apps_view_all_pages_cb(void *data, Evas_Object *obj, void *event_info);

static void __option_menu_update_caption_menu_state_update(void);

HAPI bool option_menu_show(void)
{
	Evas_Object *option_menu = NULL;
	homescreen_view_t view = home_screen_get_view_type();

	__option_menu_create_background();

	option_menu = elm_ctxpopup_add(home_screen_get_win());
	if (!option_menu) {
		LOGE("Could not create all apps option menu");
		evas_object_del(s_info.option_menu_background);
		return false;
	}

	elm_ctxpopup_horizontal_set(option_menu, EINA_FALSE);
	elm_ctxpopup_direction_priority_set(option_menu, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN);

	evas_object_smart_callback_add(option_menu, "dismissed", __option_menu_dismissed_cb, NULL);

	evas_object_layer_set(option_menu, EVAS_LAYER_MAX);
	evas_object_move(option_menu, 0, home_screen_get_root_height()-(BUTTONS_AREA*home_screen_get_root_height()));
	evas_object_size_hint_min_set(option_menu, home_screen_get_root_width(), -1);

	if (view == HOMESCREEN_VIEW_HOME) {
		elm_ctxpopup_item_append(option_menu, _("IDS_HS_OPT_EDIT"), NULL, __option_menu_entry_all_apps_edit_home_cb, NULL);
		elm_ctxpopup_item_append(option_menu, _("IDS_HS_HEADER_ADD_WIDGET"), NULL, __option_menu_entry_all_apps_add_widget_cb, NULL);
		elm_ctxpopup_item_append(option_menu, _("IDS_HS_OPT_CHANGE_WALLPAPER_ABB"), NULL, __option_menu_entry_all_apps_wallpaper_cb, NULL);
		elm_ctxpopup_item_append(option_menu, _("IDS_HS_OPT_ALL_PAGES"), NULL, __option_menu_entry_all_apps_view_all_pages_cb, NULL);
	} else if (view == HOMESCREEN_VIEW_HOME_ALL_PAGES) {
		LOGD("All pages view type");
	} else if (view == HOMESCREEN_VIEW_ALL_APPS) {
		elm_ctxpopup_item_append(option_menu, _("IDS_HS_OPT_EDIT"), NULL, __option_menu_entry_all_apps_edit_apps_cb, NULL);
		elm_ctxpopup_item_append(option_menu, _("IDS_HS_OPT_CREATE_FOLDER_OPT_TTS"), NULL, __option_menu_entry_all_apps_create_folder_cb, NULL);

	} else if (view == HOMESCREEN_VIEW_UNKNOWN) {
		LOGD("Unknown view type");
	} else {
		LOGE("Failed to get view type");
		return false;
	}

	evas_object_show(option_menu);

	s_info.is_visible = true;
	s_info.option_menu = option_menu;

	return true;
}

HAPI void option_menu_hide(void)
{
	if (s_info.option_menu) {
		evas_object_hide(s_info.option_menu);
		evas_object_del(s_info.option_menu);
		s_info.option_menu = NULL;
	}

	if (s_info.option_menu_background) {
		evas_object_hide(s_info.option_menu_background);
		evas_object_del(s_info.option_menu_background);
		s_info.option_menu_background = NULL;
	}
	s_info.is_visible = false;
}

HAPI void option_menu_update_on_item_selected(void)
{
	__option_menu_update_caption_menu_state_update();
}

HAPI void option_menu_change_state_on_hw_menu_key(void)
{
	if (s_info.is_visible) {
		option_menu_hide();
	} else {
		option_menu_show();
	}
}

static void __option_menu_create_background(void)
{
	Evas_Object *background = NULL;

	if (s_info.option_menu_background) {
		evas_object_show(s_info.option_menu_background);

		return;
	}

	background = evas_object_rectangle_add(evas_object_evas_get(home_screen_get_win()));
	evas_object_color_set(background, 255, 255, 255, 0);
	evas_object_size_hint_min_set(background, INDEX_HEIGHT, home_screen_get_root_height());
	evas_object_size_hint_max_set(background, INDEX_HEIGHT, home_screen_get_root_height());
	evas_object_resize(background, INDEX_HEIGHT, home_screen_get_root_height());
	evas_object_show(background);

	evas_object_event_callback_add(background, EVAS_CALLBACK_MOUSE_UP, __option_menu_background_clicked_cb, NULL);

	s_info.option_menu_background = background;
}

static void __option_menu_background_clicked_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	option_menu_hide();
}

static void __option_menu_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	option_menu_hide();
}

static void __option_menu_entry_all_apps_edit_apps_cb(void *data, Evas_Object *obj, void *event_info)
{
	option_menu_hide();
	home_screen_set_view_type(HOMESCREEN_VIEW_ALL_APPS_EDIT);
}

static void __option_menu_entry_all_apps_edit_home_cb(void *data, Evas_Object *obj, void *event_info)
{
	home_screen_set_view_type(HOMESCREEN_VIEW_HOME_EDIT);
	option_menu_hide();
}


static void __option_menu_entry_all_apps_create_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	option_menu_hide();
	home_screen_set_view_type(HOMESCREEN_VIEW_ALL_APPS_CHOOSE);
	__option_menu_update_caption_menu_state_update();
}

static void _launch_request_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data)
{
	switch (result) {
	case APP_CONTROL_RESULT_CANCELED:
		LOGE("[APP_CONTROL_RESULT_CANCELED]");
		break;
	case APP_CONTROL_RESULT_FAILED:
		LOGE("[APP_CONTROL_RESULT_FAILED]");
		break;
	case APP_CONTROL_RESULT_SUCCEEDED:
		LOGD("[APP_CONTROL_RESULT_SUCCEEDED]");
		break;
	default:
		LOGE("UNKNOWN ERROR");
		break;
	}
}

static void __option_menu_entry_all_apps_add_widget_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("__option_menu_entry_all_apps_add_widget_cb");

	home_screen_set_view_type(HOMESCREEN_VIEW_HOME_ADD_VIEWER);

	option_menu_hide();
}

static void __option_menu_entry_all_apps_wallpaper_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("__option_menu_entry_all_apps_wallpaper_cb");

	const char *appid = OPTION_MENU_SETTINGS;
	app_control_h app_control_handle = NULL;

	if (app_control_create(&app_control_handle) != APP_CONTROL_ERROR_NONE) {
		LOGE("[FAILED][app_control_create]");
		return;
	}

	app_control_add_extra_data(app_control_handle, "from", "Homescreen-efl");
	app_control_add_extra_data(app_control_handle, "popup_type", "selection_popup");
	app_control_add_extra_data(app_control_handle, "setas-type", "Homescreen");

	if (app_control_set_app_id(app_control_handle, appid) != APP_CONTROL_ERROR_NONE) {
		LOGE("[FAILED][app_control_set_app_id]");
		app_control_destroy(app_control_handle);
		return;
	}

	if (app_control_send_launch_request(app_control_handle, _launch_request_cb, NULL) != APP_CONTROL_ERROR_NONE) {
		LOGE("[FAILED][app_control_send_launch_request]");
		app_control_destroy(app_control_handle);
		return;
	}

	app_control_destroy(app_control_handle);
	option_menu_hide();
}

static void __option_menu_entry_all_apps_view_all_pages_cb(void *data, Evas_Object *obj, void *event_info)
{
	home_screen_set_view_type(HOMESCREEN_VIEW_HOME_ALL_PAGES);
	option_menu_hide();
}

static void __option_menu_update_caption_menu_state_update(void)
{
	int selected_count = -1;
	char trbuf[CAPTION_MENU_CENTER_LABEL_TEXT_LEN_MAX];
	homescreen_view_t view = home_screen_get_view_type();

	switch (view) {
	case HOMESCREEN_VIEW_HOME:
		break;
	case HOMESCREEN_VIEW_ALL_APPS:
		break;
	case HOMESCREEN_VIEW_ALL_APPS_CHOOSE:
		selected_count = data_model_get_app_check_state();
		snprintf(trbuf, sizeof(trbuf), _("IDS_MEMO_HEADER_PD_SELECTED_ABB2"), selected_count);
		layout_main_caption_set_menu_texts(trbuf,
		_("IDS_TPLATFORM_ACBUTTON_CANCEL_ABB"),
		_("IDS_TPLATFORM_ACBUTTON_DONE_ABB"));
		break;
	case HOMESCREEN_VIEW_ALL_APPS_EDIT:
		layout_main_caption_set_menu_texts(_("IDS_HS_HEADER_EDIT"), "", "");
		break;
	default:
		break;
	}
}

