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
	Evas_Object *label;
	main_info.win = elm_win_util_standard_add(PACKAGE, PACKAGE);

	main_info.conformant = elm_conformant_add(main_info.win);

	elm_win_conformant_set(main_info.win, EINA_TRUE);
	evas_object_size_hint_weight_set(main_info.conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_win_resize_object_add(main_info.win, main_info.conformant);

	elm_win_indicator_mode_set(main_info.win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(main_info.win, ELM_WIN_INDICATOR_OPAQUE);
	elm_object_signal_emit(main_info.conformant, "elm,state,indicator,overlap", "elm");

	evas_object_show(main_info.conformant);

	label = elm_label_add(main_info.conformant);
	elm_object_text_set(label, "<align=center>EMPTY PROJECT</align>");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(main_info.conformant, label);

	evas_object_show(main_info.win);
}
