/*
 * Copyright (c) 2000 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <system_settings.h>
#include "homescreen-efl.h"

static struct {
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *conformant;
	Evas_Object *main_layout;
	Evas_Object *all_apps_layout;
	Evas_Object *home_layout;
	int root_width;
	int root_height;
	homescreen_view_t view_type;
} main_info = {
	.win = NULL,
	.bg = NULL,
	.conformant = NULL,
	.main_layout = NULL,
	.all_apps_layout = NULL,
	.home_layout = NULL,
	.root_width = 0,
	.root_height = 0,
	.view_type = HOMESCREEN_VIEW_HOME,
};

static void __homescreen_efl_create_base_gui(void);
static void __homescreen_efl_get_window_size(Evas_Object *win);
static void __homescreen_efl_set_main_layout(void);
static void __homescreen_efl_set_wallpaper(void);
static void __homescreen_efl_set_conformant(void);

static void __homescreen_efl_home_bg_changed_cb(system_settings_key_e key, void *data);

static void __homescreen_efl_lang_changed_cb(app_event_info_h event_info, void *user_data)
{
	LOGD("called");
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

	__homescreen_efl_create_base_gui();
	return true;
}

static void __homescreen_efl_app_control_cb(app_control_h app_control, void *data)
{

}

static void __homescreen_efl_app_pause_cb(void *data)
{

}

static void __homescreen_efl_app_resume_cb(void *data)
{
}

static void __homescreen_efl_app_terminate_cb(void *data)
{

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
		dlog_print(DLOG_ERROR, LOG_TAG, "ui_app_main() is failed. err = %d", ret);

	main_info.view_type = HOMESCREEN_VIEW_HOME;

	return ret;
}

static void __homescreen_efl_create_base_gui(void)
{
	main_info.win = elm_win_util_standard_add(PACKAGE, PACKAGE);

	__homescreen_efl_get_window_size(main_info.win);

	__homescreen_efl_set_main_layout();

	__homescreen_efl_set_wallpaper();

	__homescreen_efl_set_conformant();

	evas_object_show(main_info.win);
}

static void __homescreen_efl_get_window_size(Evas_Object *win)
{
	elm_win_screen_size_get(win, NULL, NULL, &main_info.root_width, &main_info.root_height);
	LOGD("Width: [%d], Height: [%d]", main_info.root_width, main_info.root_height);
}

static void __homescreen_efl_set_main_layout(void)
{
	char edj_path[PATH_MAX] = {0, };

	snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/home.edj"));

	main_info.main_layout = elm_layout_add(main_info.win);
	elm_layout_file_set(main_info.main_layout, edj_path, GROUP_HOME_LY);
	evas_object_size_hint_weight_set(main_info.main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(main_info.win, main_info.main_layout);

	evas_object_show(main_info.main_layout);
}

static void __homescreen_efl_set_wallpaper(void)
{
	const char *bg_path = util_get_res_file_path(IMAGE_DIR"/default_bg.png");
	char *buf = NULL;
	int ret = -1;

	if (main_info.bg == NULL) {
		main_info.bg = evas_object_image_filled_add(evas_object_evas_get(main_info.main_layout));

		system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, __homescreen_efl_home_bg_changed_cb, NULL);

		evas_object_size_hint_min_set(main_info.bg, main_info.root_width, main_info.root_height);
		elm_object_part_content_set(main_info.main_layout, "size_setter", main_info.bg);
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


