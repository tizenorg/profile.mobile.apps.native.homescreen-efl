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

#include <system_settings.h>
#include <dlog.h>
#include <feedback.h>

#include "conf.h"
#include "edc_conf.h"
#include "util.h"
#include "homescreen-efl.h"
#include "apps_view.h"
#include "cluster_view.h"
#include "hw_key.h"
#include "view.h"
#include "menu.h"
#include "popup.h"

static struct {
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *conformant;
	Evas_Object *main_layout;
	Evas_Object *apps_layout;
	Evas_Object *cluster_layout;
	Evas_Object *btn_layout;
	int root_width;
	int root_height;
	homescreen_view_t view_type;
	Ecore_Animator *animator;
} main_info = {
	.win = NULL,
	.bg = NULL,
	.conformant = NULL,
	.main_layout = NULL,
	.apps_layout = NULL,
	.cluster_layout = NULL,
	.btn_layout = NULL,
	.root_width = 0,
	.root_height = 0,
	.view_type = HOMESCREEN_VIEW_HOME,
	.animator = NULL,
};

static void __homescreen_efl_get_window_size(Evas_Object *win);
static void __homescreen_efl_set_wallpaper(void);
static void __homescreen_efl_set_conformant(void);
static void __homescreen_efl_create_home_btn(void);

static void __homescreen_efl_home_bg_changed_cb(system_settings_key_e key, void *data);
static void __homescreen_efl_menu_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __homescreen_efl_home_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);

static void __homescreen_efl_change_view(void);
static Eina_Bool __homescreen_efl_show_apps_anim(void *data, double pos);
static Eina_Bool __homescreen_efl_show_cluster_anim(void *data, double pos);

static bool __homescreen_efl_create_view(void);
static Eina_Bool __homescreen_efl_init_view(void *data);

static void __homescreen_efl_lang_changed_cb(app_event_info_h event_info, void *user_data)
{
	LOGD("called");
	menu_hide();
	apps_view_lang_changed(main_info.view_type);
}

static void __homescreen_efl_orient_changed_cb(app_event_info_h event_info, void *user_data)
{
	LOGD("called");
}

static void __homescreen_efl_region_changed_cb(app_event_info_h event_info, void *user_data)
{
	LOGD("called");
}

static void __homescreen_efl_low_battery_cb(app_event_info_h event_info, void *user_data)
{
	LOGD("called");
}

static void __homescreen_efl_low_memory_cb(app_event_info_h event_info, void *user_data)
{
	LOGD("called");
}

static bool __homescreen_efl_app_create_cb(void *data)
{
	elm_config_accel_preference_set("3d");
	ecore_animator_frametime_set(FRAMES_PER_SECOND);

	feedback_initialize();

	hw_key_register();

	main_info.win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	__homescreen_efl_get_window_size(main_info.win);

	__homescreen_efl_set_wallpaper();
	evas_object_show(main_info.win);

	__homescreen_efl_create_view();

	__homescreen_efl_set_conformant();
	__homescreen_efl_create_home_btn();

	ecore_timer_add(HOME_LOADING_TIME, __homescreen_efl_init_view, NULL);

	return true;
}

#define APP_CONTROL_HOME_OP_KEY "__HOME_OP__"
#define APP_CONTROL_HOME_OP_VAL_LAUNCH_BY_HOME_KEY "__LAUNCH_BY_HOME_KEY__"
static void __homescreen_efl_app_control_cb(app_control_h app_control, void *data)
{
	char *operation = NULL;
	int ret = 0;

	if (!app_control) {
		LOGE("appcontrol is NULL");
		return;
	}

	ret = app_control_get_operation(app_control, &operation);
	if (ret != APP_CONTROL_ERROR_NONE) {
		LOGE("Failed to get operation");
		return;
	}

	if (!strncmp(operation, APP_CONTROL_OPERATION_DEFAULT, strlen(operation))) {
		char *value = NULL;
		ret = app_control_get_extra_data(app_control, APP_CONTROL_HOME_OP_KEY, &value);
		if (ret != APP_CONTROL_ERROR_NONE) {
			LOGE("Failed to get extra data");
			free(operation);
			return;
		}

		if (value && !strncmp(value, APP_CONTROL_HOME_OP_VAL_LAUNCH_BY_HOME_KEY, strlen(value))) {
			/*
			 * If the homescreen is launched by home key,
			 * show the first view of the homescreen.
			 */
			LOGD("launched by home key");
			if (menu_is_show()) {
				menu_hide();
			} else if (popup_is_show()) {
				popup_hide();
			}
			homescreen_efl_hw_home_key_release();
		}

		free(value);
	}

	free(operation);
}

static void __homescreen_efl_app_pause_cb(void *data)
{

}

static void __homescreen_efl_app_resume_cb(void *data)
{
}

static void __homescreen_efl_app_terminate_cb(void *data)
{
	apps_view_app_terminate();
	cluster_view_app_terminate();
	hw_key_unregister();
	feedback_deinitialize();
}

int main(int argc, char *argv[])
{
	LOGD("Starting homescreen-efl");

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

	ret = ui_app_main(argc, argv, &event_callback, NULL);
	if (ret != APP_ERROR_NONE)
		LOGE("ui_app_main() is failed. err = %d", ret);

	main_info.view_type = HOMESCREEN_VIEW_HOME;

	return ret;
}

static void __homescreen_efl_get_window_size(Evas_Object *win)
{
	elm_win_screen_size_get(win, NULL, NULL, &main_info.root_width, &main_info.root_height);

	conf_set_resolution_scale(main_info.root_width, main_info.root_height);

	LOGD("Width: [%d], Height: [%d]", main_info.root_width, main_info.root_height);
}

static void __homescreen_efl_set_wallpaper(void)
{
	const char *bg_path = util_get_res_file_path(IMAGE_DIR"/default_bg.png");
	char *buf = NULL;
	int ret = -1;

	if (main_info.main_layout == NULL) {
		char edj_path[PATH_MAX] = {0, };
		snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/home.edj"));

		main_info.main_layout = elm_layout_add(main_info.win);
		elm_layout_file_set(main_info.main_layout, edj_path, GROUP_HOME_LY);
		evas_object_size_hint_weight_set(main_info.main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(main_info.win, main_info.main_layout);

		evas_object_show(main_info.main_layout);
	}

	if (main_info.bg == NULL) {
		main_info.bg = evas_object_image_filled_add(evas_object_evas_get(main_info.main_layout));

		system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, __homescreen_efl_home_bg_changed_cb, NULL);

		evas_object_size_hint_min_set(main_info.bg, main_info.root_width, main_info.root_height);
		elm_object_part_content_set(main_info.main_layout, "home_bg", main_info.bg);
		evas_object_show(main_info.bg);
	}

	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, &buf);
	if (!buf || !ecore_file_can_read(buf) || ret != SYSTEM_SETTINGS_ERROR_NONE) {
		evas_object_image_file_set(main_info.bg, bg_path, "bg");
	} else {
		evas_object_image_file_set(main_info.bg, buf, "bg");
	}
	if (buf) free(buf);
}

static void __homescreen_efl_home_bg_changed_cb(system_settings_key_e key, void *data)
{
	__homescreen_efl_set_wallpaper();
}

static void __homescreen_efl_set_conformant(void)
{
	elm_win_conformant_set(main_info.win, EINA_TRUE);

	main_info.conformant = elm_conformant_add(main_info.win);
	evas_object_size_hint_weight_set(main_info.conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(main_info.win, main_info.conformant);

	elm_win_indicator_mode_set(main_info.win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(main_info.win, ELM_WIN_INDICATOR_TRANSLUCENT);
	elm_object_signal_emit(main_info.conformant, "elm,state,indicator,overlap", "elm");

	evas_object_show(main_info.conformant);
}

static void __homescreen_efl_create_home_btn(void)
{
	char edj_path[PATH_MAX] = {0, };

	snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/home_btn.edj"));

	main_info.btn_layout = elm_layout_add(main_info.win);
	elm_layout_file_set(main_info.btn_layout, edj_path, GROUP_HOME_BTN_LY);
	evas_object_size_hint_weight_set(main_info.btn_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(main_info.win, main_info.btn_layout);

	evas_object_show(main_info.btn_layout);

	elm_object_signal_callback_add(main_info.btn_layout, SIGNAL_MENU_BTN_CLICKED, SIGNAL_SOURCE, __homescreen_efl_menu_btn_clicked, NULL);
	elm_object_signal_callback_add(main_info.btn_layout, SIGNAL_HOME_BTN_CLICKED, SIGNAL_SOURCE, __homescreen_efl_home_btn_clicked, NULL);
}

static void __homescreen_efl_menu_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
	homescreen_efl_hw_menu_key_release();
}

static void __homescreen_efl_home_btn_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
	__homescreen_efl_change_view();
}

static void __homescreen_efl_change_view(void)
{
	if (main_info.animator != NULL) {
		LOGE("main_info.animator != NULL");
		return ;
	}

	if (main_info.view_type == HOMESCREEN_VIEW_APPS) {
		apps_view_hide();
		main_info.animator = ecore_animator_timeline_add(HOME_ANIMATION_TIME, __homescreen_efl_show_cluster_anim, NULL);

		elm_object_signal_emit(main_info.btn_layout, SIGNAL_HOME_BTN_ICON_HIDE, SIGNAL_SOURCE);
		elm_object_signal_emit(main_info.btn_layout, SIGNAL_APPS_BTN_ICON_SHOW, SIGNAL_SOURCE);
		main_info.view_type = HOMESCREEN_VIEW_HOME;
	} else if (main_info.view_type == HOMESCREEN_VIEW_HOME) {
		cluster_view_hide();
		main_info.animator = ecore_animator_timeline_add(HOME_ANIMATION_TIME, __homescreen_efl_show_apps_anim, NULL);

		elm_object_signal_emit(main_info.btn_layout, SIGNAL_APPS_BTN_ICON_HIDE, SIGNAL_SOURCE);
		elm_object_signal_emit(main_info.btn_layout, SIGNAL_HOME_BTN_ICON_SHOW, SIGNAL_SOURCE);
		main_info.view_type = HOMESCREEN_VIEW_APPS;
	}
}

static Eina_Bool __homescreen_efl_show_apps_anim(void *data, double pos)
{
	cluster_view_hide_anim(pos);
	apps_view_show_anim(pos);

	if (pos >= (1.0 - (1e-10))) {
		apps_view_show();
		main_info.animator = NULL;
		return ECORE_CALLBACK_DONE;
	}

	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool __homescreen_efl_show_cluster_anim(void *data, double pos)
{
	apps_view_hide_anim(pos);
	cluster_view_show_anim(pos);

	if (pos >= (1.0 - (1e-10))) {
		cluster_view_show();
		main_info.animator = NULL;
		return ECORE_CALLBACK_DONE;
	}

	return ECORE_CALLBACK_RENEW;
}

Evas_Object *homescreen_efl_get_win(void)
{
	return main_info.win;
}

void homescreen_efl_hw_menu_key_release(void)
{
	if (main_info.view_type == HOMESCREEN_VIEW_HOME) {
		cluster_view_hw_menu_key();
	} else if (main_info.view_type == HOMESCREEN_VIEW_APPS) {
		apps_view_hw_menu_key();
	}
}

void homescreen_efl_hw_home_key_release(void)
{
	if (main_info.view_type == HOMESCREEN_VIEW_HOME) {
		cluster_view_hw_home_key();
	} else if (main_info.view_type == HOMESCREEN_VIEW_APPS) {
		if (apps_view_hw_home_key() == false)
			__homescreen_efl_change_view();
	}
}

void homescreen_efl_hw_back_key_release(void)
{
	if (main_info.view_type == HOMESCREEN_VIEW_HOME) {
		if (cluster_view_hw_back_key() == false) {
			__homescreen_efl_change_view();
		}
	} else if (main_info.view_type == HOMESCREEN_VIEW_APPS) {
		if (apps_view_hw_back_key() == false) {
			__homescreen_efl_change_view();
		}
	}
}

void homescreen_efl_btn_show(homescreen_view_t view_t)
{
	LOGD("state %d", view_t);
	switch (view_t) {
	case HOMESCREEN_VIEW_HOME:
		elm_object_signal_emit(main_info.btn_layout, SIGNAL_BTN_SHOW_HOME_STATE, SIGNAL_SOURCE);
		break;
	case HOMESCREEN_VIEW_APPS:
		elm_object_signal_emit(main_info.btn_layout, SIGNAL_BTN_SHOW_APPS_STATE, SIGNAL_SOURCE);
		break;
	default:
		break;
	}
}

void homescreen_efl_btn_hide(homescreen_view_t view_t)
{
	LOGD("state %d", view_t);
	switch (view_t) {
	case HOMESCREEN_VIEW_HOME:
		elm_object_signal_emit(main_info.btn_layout, SIGNAL_BTN_HIDE_HOME_STATE, SIGNAL_SOURCE);
		break;
	case HOMESCREEN_VIEW_APPS:
		elm_object_signal_emit(main_info.btn_layout, SIGNAL_BTN_HIDE_APPS_STATE, SIGNAL_SOURCE);
		break;
	default:
		break;
	}
}

static bool __homescreen_efl_create_view(void)
{
	main_info.cluster_layout = cluster_view_create(main_info.win);
	if (main_info.cluster_layout == NULL) {
		LOGE("main_info.cluster_layout  == NULL");
		return false;
	}
	evas_object_move(main_info.cluster_layout, 0, INDICATOR_H);
	evas_object_show(main_info.cluster_layout);

	main_info.apps_layout = apps_view_create(main_info.win);
	if (main_info.apps_layout == NULL) {
		LOGE("main_info.apps_layout  == NULL");
		return false;
	}
	evas_object_move(main_info.apps_layout, 0, main_info.root_height);
	evas_object_show(main_info.apps_layout);

	return true;
}


static Eina_Bool __homescreen_efl_init_view(void *data)
{
	cluster_view_init();
	apps_view_init();

	return ECORE_CALLBACK_CANCEL;
}
