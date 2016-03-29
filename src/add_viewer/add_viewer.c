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

#include "util.h"

#include "add_viewer/add_viewer.h"
#include "add_viewer/list_view.h"
#include "add_viewer/widget.h"

static struct {
	Evas_Object *win;
	Evas_Object *conformant;
	Evas_Object *layout;
	Evas_Object *naviframe;
	Evas_Object *genlist;

	Elm_Theme *theme;
	Elm_Object_Item *navi_item;

} s_info = {
	.win = NULL,
	.conformant = NULL,
	.layout = NULL,
	.naviframe = NULL,
	.genlist = NULL,

	.theme = NULL,
	.navi_item = NULL,
};

static void __add_viewer_window_del_cb(void *data, Evas_Object* obj, void* event_info);
static Elm_Theme *__add_viewer_create_theme(void);
static Evas_Object *__add_viewer_create_conformant(void);
static Evas_Object *__add_viewer_create_layout();
static Evas_Object *__add_viewer_create_naviframe();

Elm_Theme *add_viewer_get_main_theme(void)
{
	return s_info.theme;
}

Evas_Object *add_viewer_get_layout(void)
{
	return s_info.layout;
}

void add_viewer_window_create(Evas_Coord w, Evas_Coord h)
{
	int ret = 0;

	ret = widget_init();
	if (ret != 0) {
		LOGE("Can not init widget list");
		return;
	}

	s_info.win = elm_win_add(NULL, "add_viewer", ELM_WIN_BASIC);
	if (!s_info.win) {
		LOGE("Failed to create a new window");
		return;
	}

	elm_win_alpha_set(s_info.win, EINA_TRUE);
	elm_win_autodel_set(s_info.win, EINA_TRUE);
	evas_object_smart_callback_add(s_info.win, "delete,request", __add_viewer_window_del_cb, NULL);

	s_info.theme = __add_viewer_create_theme();
	s_info.conformant = __add_viewer_create_conformant();
	s_info.layout = __add_viewer_create_layout();
	s_info.naviframe = __add_viewer_create_naviframe();

	s_info.navi_item = list_view_create_content(s_info.naviframe, s_info.genlist);

	elm_object_signal_emit(s_info.layout, "display,normal", "container");

	elm_win_indicator_mode_set(s_info.win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(s_info.win, ELM_WIN_INDICATOR_OPAQUE);
	elm_win_resize_object_add(s_info.win, s_info.conformant);
	elm_win_conformant_set(s_info.win, EINA_TRUE);
	evas_object_resize(s_info.win, w, h);

	evas_object_show(s_info.naviframe);
	evas_object_show(s_info.layout);
	evas_object_show(s_info.conformant);


	evas_object_show(s_info.win);
}

void add_viewer_window_delete(void)
{
	home_screen_set_view_type(HOMESCREEN_VIEW_HOME);
	evas_object_del(s_info.win);
	widget_fini();
}

static void __add_viewer_window_del_cb(void *data, Evas_Object* obj, void* event_info)
{
	evas_object_del(s_info.win);
}

static Elm_Theme *__add_viewer_create_theme(void)
{
	Elm_Theme *theme = elm_theme_new();
	if (!theme) {
		LOGE("Failed to create theme\n");
		return NULL; }

	elm_theme_ref_set(theme, NULL);
	elm_theme_extension_add(theme, util_get_res_file_path(EDJE_DIR"/preview.edj"));

	return theme;
}

static Evas_Object *__add_viewer_create_conformant(void)
{
	Evas_Object *conformant;
	Evas_Object *bg;

	conformant = elm_conformant_add(s_info.win);
	if (!conformant) {
		LOGE("Failed to create a conformant\n");
		return NULL;
	}

	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	bg = elm_bg_add(conformant);
	if (bg) {
		elm_object_style_set(bg, "indicator/headerbg");
		elm_object_part_content_set(conformant, "elm.swallow.indicator_bg", bg);
		evas_object_show(bg);
	} else {
		LOGE("Failed to create a BG object\n");
	}

	return conformant;
}

static Evas_Object *__add_viewer_create_layout()
{
	Evas_Object *layout;
	char edj_path[PATH_MAX] = {0, };

	if (!s_info.conformant) {
		return NULL;
	}

	layout = elm_layout_add(s_info.conformant);
	if (!layout) {
		return NULL;
	}

	snprintf(edj_path, sizeof(edj_path), "%s", util_get_res_file_path(EDJE_DIR"/preview.edj"));
	if (elm_layout_file_set(layout, edj_path, "conformant,frame") != EINA_TRUE) {
		evas_object_del(layout);
		return NULL;
	}
	
	elm_object_part_content_set(s_info.conformant, "elm.swallow.content", layout);

	return layout;
}

static Evas_Object *__add_viewer_create_naviframe()
{
	Evas_Object *naviframe;

	naviframe = elm_naviframe_add(s_info.conformant);
	if (!naviframe) {
		return NULL;
	}

	elm_naviframe_content_preserve_on_pop_set(naviframe, EINA_TRUE);
	elm_object_part_content_set(s_info.layout, "content", naviframe);

	return naviframe;
}

