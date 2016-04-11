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
#include <package_manager.h>
#include <package_info.h>
#include <app_control.h>

#include "homescreen-efl.h"
#include "app_icon.h"
#include "all_apps.h"
#include "util.h"
#include "folder_panel.h"
#include "option_menu.h"
#include "folder_panel.h"
#include "popup.h"
#include "app_grid.h"
#include "data_model.h"

#define MAX_BADGE_COUNT_STR_LEN 32
#define MAX_BADGE_DISPLAY_COUNT 999
#define ABOVE_MAX_BADGE_APPENDIX "+"

static struct {
	Evas_Object *icon_layout;
	Evas_Object *icon_image;
	char *runned_app_id;
	bool click_ignore;

} s_info = {
	.icon_layout = NULL,
	.icon_image = NULL,
	.runned_app_id = NULL,
	.click_ignore = false,
};

static void __app_icon_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __app_icon_pressed_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __app_icon_launch_request_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data);
static bool __app_icon_check_removeable_state(Evas_Object *icon);
static void __app_icon_check_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __app_icon_uncheck_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static Evas_Object *__app_icon_load(Evas_Object *parent, const char *icon_path);
static void __app_icon_uninstall_button_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __app_icon_highlight_folder_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __app_icon_unhighlight_folder_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __app_icon_higlight_impossible_folder_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

/*=========================================PUBLIC FUNCTIONS IMPLEMENTATION===========================================*/
HAPI Evas_Object *app_icon_create(Tree_node_t *tree_data, app_icon_type_t type)
{
	Evas_Object *icon_image = NULL, *icon_layout = NULL, *folder_icon_layout = NULL;
	Evas_Object *parent = home_screen_get_win();
	app_item_t *item = NULL;

	if (!parent || !tree_data) {
		LOGE("[INVALID_PARAM][parent='%p'][item='%p']", parent, tree_data);
		return NULL;
	}

	item = tree_data->data;
	if (!item) {
		LOGE("[FAILED][item='%p']", item);
		return NULL;
	}

	if (type == APP_ICON_TYPE_APP || type == APP_ICON_TYPE_FOLDER) {
		icon_layout = util_create_edje_layout(parent, util_get_res_file_path(EDJE_DIR"/icon.edj"), GROUP_ICON_LY);
		if (!icon_layout) {
			LOGE("Failed to create icon layout");
			return NULL;
		}

		elm_object_signal_callback_add(icon_layout, SIGNAL_CHECKBOX_CHECKED, SIGNAL_SOURCE,
				__app_icon_check_cb, tree_data);
		elm_object_signal_callback_add(icon_layout, SIGNAL_CHECKBOX_UNCHECKED, SIGNAL_SOURCE,
				__app_icon_uncheck_cb, tree_data);
		elm_object_signal_callback_add(icon_layout, SIGNAL_UNINSTALL_BUTTON_CLICKED, SIGNAL_SOURCE,
				__app_icon_uninstall_button_clicked_cb, tree_data);

		elm_object_signal_callback_add(icon_layout, "mouse,down,1", SIGNAL_PRESS_SOURCE,
				__app_icon_pressed_cb, (void *)item->type);

		home_screen_mvc_set_ly_scale(icon_layout);
	}

	if (type == APP_ICON_TYPE_APP) {
		icon_image = __app_icon_load(icon_layout, item->icon);
		if (!icon_image) {
			LOGE("Failed to create icon layout");
			evas_object_del(icon_layout);
			return NULL;
		}

		elm_object_part_content_set(icon_layout, PART_ICON_CONTENT, icon_image);
		elm_object_signal_callback_add(icon_layout, SIGNAL_ICON_CLICKED, SIGNAL_SOURCE, __app_icon_clicked_cb, (void *)item->appid);

		app_icon_set_label(icon_layout, item->label);

		if (item->badge_count > 0)
			app_icon_show_badge(icon_layout, item->badge_count);

		evas_object_show(icon_image);
	} else if (type == APP_ICON_TYPE_FOLDER) {
		folder_icon_layout = elm_layout_add(parent);
		if (!folder_icon_layout) {
			LOGE("Failed to create icon layout");
			evas_object_del(icon_layout);
			return NULL;
		}

		if (!elm_layout_file_set(folder_icon_layout, util_get_res_file_path(EDJE_DIR"/folder.edj"), GROUP_FOLDER_ICON_LAYOUT)) {
			LOGE("Failed to load edje file for layout");
			evas_object_del(folder_icon_layout);
			evas_object_del(icon_layout);
			return NULL;
		}

		elm_object_part_content_set(icon_layout, PART_ICON_CONTENT, folder_icon_layout);
		elm_object_signal_callback_add(icon_layout, SIGNAL_HIGHLIGHT_FOLDER, SIGNAL_SOURCE, __app_icon_highlight_folder_icon_cb, item);
		elm_object_signal_callback_add(icon_layout, SIGNAL_HIGHLIGHT_IMPOSSIBLE_FOLDER, SIGNAL_SOURCE, __app_icon_higlight_impossible_folder_icon_cb, item);
		elm_object_signal_callback_add(icon_layout, SIGNAL_UNHIGHLIGHT_FOLDER, SIGNAL_SOURCE, __app_icon_unhighlight_folder_icon_cb, item);
		app_icon_set_label(icon_layout, item->label);
		evas_object_show(folder_icon_layout);
	} else {
		/*create miniatures*/
		if (item->icon) {
			icon_layout = __app_icon_load(parent, item->icon);
			if (!icon_layout) {
				LOGE("Failed to create icon layout");
				return NULL;
			}
		}
	}

	evas_object_size_hint_align_set(icon_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(icon_layout);

	evas_object_data_set(icon_layout, KEY_ICON_DATA, tree_data);
	evas_object_data_set(icon_layout, KEY_IS_REPOSITIONABLE, (void *)true);

	return icon_layout;
}

HAPI void app_icon_update_folder_icon(Tree_node_t *folder_node)
{
	LOGI("");
	Evas_Object *miniatures_list_item = NULL;
	Evas_Object *folder_icon = NULL;
	Evas_Object *item = NULL;
	Eina_List *it = NULL;
	Tree_node_t *it2;
	folder_data_t *folder_data = NULL;
	app_item_t *tree_item = NULL;

	char part_buf[BUF_SIZE];
	char signal_buf[BUF_SIZE];

	int miniatures = 0;
	int miniatures_iterator = 0;

	if (!folder_node || !folder_node->data || !folder_node->data->layout || !folder_node->data->data) {
		LOGE("[INVALID_PARAMS]");
		return;
	}

	folder_icon = elm_object_part_content_get(folder_node->data->layout, PART_ICON_CONTENT);
	if (!folder_icon) {
		LOGE("Failed to get folder icon");
		return;
	}

	folder_data = (folder_data_t *)folder_node->data->data;

	if (!folder_data) {
		LOGE("Failed to get folder_data");
		return;
	}

	EINA_LIST_FOREACH(folder_data->miniatures, it, miniatures_list_item) {
		snprintf(part_buf, sizeof(part_buf), "miniature_%d", miniatures_iterator++);
		elm_object_part_content_unset(folder_icon, part_buf);
		evas_object_del(miniatures_list_item);
	}
	folder_data->miniatures = eina_list_free(folder_data->miniatures);
	folder_data->miniatures = NULL;

	if (!folder_node->first || !(miniatures = folder_node->first->count)) {
		elm_object_signal_emit(folder_icon, SIGNAL_EMPTY_BG_SHOW, SIGNAL_SOURCE);
		return;
	}

	if (miniatures >= 3) {
		miniatures = 4;
	}

	miniatures_iterator = 0;
	TREE_NODE_FOREACH(folder_node->first, it2)
	{
		tree_item = it2->data;

		if (tree_item) {
			if (miniatures_iterator >= FOLDER_ICON_MAX_MINIATURES)
				break;
			LOGI("label %s", tree_item->label);
			item  = app_icon_create(it2, APP_ICON_TYPE_MINIATURE);

			if (!item)
				return;

			folder_data->miniatures = eina_list_append(folder_data->miniatures, item);
			if (!folder_data->miniatures) {
				app_icon_destroy(item);
				return;
			}

			snprintf(part_buf, sizeof(part_buf), "miniature_%d", miniatures_iterator++);
			elm_object_part_content_set(folder_icon, part_buf, item);
		}
	}

	snprintf(signal_buf, sizeof(signal_buf), "set,miniatures,%d", miniatures);
	elm_object_signal_emit(folder_icon, signal_buf, SIGNAL_SOURCE);
}

HAPI void app_icon_highlight_folder_icon(Evas_Object *folder_icon_layout, folder_icon_frame_type_t frame_type)
{
	Evas_Object *icon = NULL;
	Tree_node_t *node = NULL;

	if (!folder_icon_layout) {
		LOGE("Invalid parameter");
		return;
	}

	icon = elm_object_part_content_get(folder_icon_layout, PART_ICON_CONTENT);
	if (!icon) {
		LOGE("Failed to get icon");
		return;
	}

	node = evas_object_data_get(folder_icon_layout, KEY_ICON_DATA);
	if (!node) {
		LOGE("[FAILED][node=NULL]");
		return;
	}

	LOGI("count %d ", node->count);

	if (frame_type == FOLDER_ICON_FRAME_POSSIBLE) {
		elm_object_signal_emit(icon, SIGNAL_FRAME_POSSIBLE_SHOW, SIGNAL_SOURCE);
	} else if (frame_type == FOLDER_ICON_FRAME_IMPOSSIBLE) {
		elm_object_signal_emit(icon, SIGNAL_FRAME_IMPOSSIBLE_SHOW, SIGNAL_SOURCE);
	} else {
		elm_object_signal_emit(icon, SIGNAL_FRAME_HIDE, SIGNAL_SOURCE);
	}
}

HAPI void app_icon_set_click_ignore(bool ignore)
{
	s_info.click_ignore = ignore;
	folder_panel_set_click_ignore_state(ignore);
}

HAPI bool app_icon_set_view_mode(Evas_Object *icon, homescreen_view_t view, bool in_folder)
{
	Tree_node_t *node = NULL;

	switch (view) {
	case HOMESCREEN_VIEW_ALL_APPS:
		in_folder ? elm_object_signal_emit(icon, SIGNAL_EDIT_MODE_BLACK_OFF, SIGNAL_SOURCE) :
				elm_object_signal_emit(icon, SIGNAL_EDIT_MODE_OFF, SIGNAL_SOURCE);
		elm_object_signal_emit(icon, SIGNAL_UNINSTALL_BUTTON_HIDE, SIGNAL_SOURCE);
		elm_object_signal_emit(icon, SIGNAL_CHECKBOX_HIDE, SIGNAL_SOURCE);
		app_icon_set_click_ignore(false);
		break;
	case HOMESCREEN_VIEW_ALL_APPS_EDIT:
		in_folder ? elm_object_signal_emit(icon, SIGNAL_EDIT_MODE_BLACK_ON, SIGNAL_SOURCE) :
				elm_object_signal_emit(icon, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);
		if (__app_icon_check_removeable_state(icon))
			elm_object_signal_emit(icon, SIGNAL_UNINSTALL_BUTTON_SHOW, SIGNAL_SOURCE);
		app_icon_set_click_ignore(true);
		break;
	case HOMESCREEN_VIEW_ALL_APPS_CHOOSE:
		in_folder ? elm_object_signal_emit(icon, SIGNAL_EDIT_MODE_BLACK_ON, SIGNAL_SOURCE) :
				elm_object_signal_emit(icon, SIGNAL_EDIT_MODE_ON, SIGNAL_SOURCE);
		elm_object_signal_emit(icon, SIGNAL_UNINSTALL_BUTTON_HIDE, SIGNAL_SOURCE);

		node = evas_object_data_get(icon, KEY_ICON_DATA);
		if (node && node->data->type != APP_ITEM_FOLDER)
			elm_object_signal_emit(icon, SIGNAL_CHECKBOX_SHOW_UNCHECKED, SIGNAL_SOURCE);
		else
			elm_object_signal_emit(icon, SIGNAL_CHECKBOX_HIDE, SIGNAL_SOURCE);

		app_icon_set_click_ignore(true);
		break;
	case HOMESCREEN_VIEW_HOME:
	case HOMESCREEN_VIEW_HOME_EDIT:
	case HOMESCREEN_VIEW_HOME_ALL_PAGES:
	default:
		app_icon_set_click_ignore(true);
		break;
	}

	return true;
}

HAPI void app_icon_set_label(Evas_Object *icon_layout, const char* label)
{
	if (!icon_layout || !label) {
		LOGE("Invalid parameter");
		return;
	}

	elm_object_part_text_set(icon_layout, PART_ICON_NAME, label);
}

HAPI void app_icon_set_label_black(Evas_Object *icon_layout)
{
	if (home_screen_get_view_type() == HOMESCREEN_VIEW_ALL_APPS_EDIT) {
		elm_object_signal_emit(icon_layout, SIGNAL_EDIT_MODE_BLACK_ON, SIGNAL_SOURCE);
	} else {
		elm_object_signal_emit(icon_layout, SIGNAL_BLACK_TEXT, SIGNAL_SOURCE);
	}
}

HAPI void app_icon_set_color(Evas_Object *icon_layout, int r, int g, int b, int a)
{
	if (!icon_layout) {
		LOGE("Failed to get folder icon layout");
		return;
	}

	evas_object_color_set(icon_layout, r, g, b, a);
}

HAPI void app_icon_destroy(Evas_Object *icon_layout)
{
	if (!icon_layout) {
		LOGE("Invalid parameter : icon_layout is NULL");
		return;
	}

	evas_object_del(icon_layout);
}


HAPI void app_icon_show_badge(Evas_Object *app_icon_layout, const unsigned int number)
{
	char number_str[MAX_BADGE_COUNT_STR_LEN];

	if (!app_icon_layout) {
		LOGE("Invalid state of the application - icon_layout is NULL");
		return;
	}

	if (number == 0) {
		app_icon_hide_badge(app_icon_layout);
		return;
	}

	if (number > MAX_BADGE_DISPLAY_COUNT) {
		eina_convert_itoa(MAX_BADGE_DISPLAY_COUNT, number_str);
		strncat(number_str, ABOVE_MAX_BADGE_APPENDIX, MAX_BADGE_COUNT_STR_LEN);
	} else {
		eina_convert_itoa(number, number_str);
	}

	elm_layout_text_set(app_icon_layout, "icon_badge_text", number_str);

	elm_object_signal_emit(app_icon_layout, SIGNAL_BADGE_SHOW, SIGNAL_SOURCE);
}

HAPI void app_icon_hide_badge(Evas_Object *app_icon_layout)
{
	if (!app_icon_layout) {
		LOGE("Invalid state of the application - icon_layout is NULL");
		return;
	}

	elm_object_signal_emit(app_icon_layout, SIGNAL_BADGE_HIDE, SIGNAL_SOURCE);
}

HAPI void app_icon_show_label(Evas_Object *app_icon_layout)
{
	Evas_Object *icon = NULL;

	if (app_icon_layout) {
		icon = elm_object_part_content_get(app_icon_layout, PART_ICON_CONTENT);
		if (icon)
			elm_object_signal_emit(icon, SIGNAL_LABEL_SHOW, SIGNAL_SOURCE);
	}
}

HAPI void app_icon_hide_label(Evas_Object *app_icon_layout)
{
	Evas_Object *icon = NULL;

	if (app_icon_layout) {
		icon = elm_object_part_content_get(app_icon_layout, PART_ICON_CONTENT);
		if (icon)
			elm_object_signal_emit(icon, SIGNAL_LABEL_HIDE, SIGNAL_SOURCE);
	}
}

HAPI void app_icon_remove(Evas_Object *icon_layout)
{
	if (!icon_layout) {
		LOGE("[INVALID_PARAM]");
		return;
	}

	evas_object_data_del(icon_layout, KEY_ICON_DATA);
	evas_object_del(icon_layout);
}


/*=========================================PRIVATE FUNCTIONS IMPLEMENTATION======================================*/
static void __app_icon_launch_request_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data)
{
	LOGD("[__app_icon_launch_request_cb]");
	switch (result) {
	case APP_CONTROL_RESULT_CANCELED:
		LOGD("[APP_CONTROL_RESULT_CANCELED]");
		s_info.runned_app_id = NULL;
		break;
	case APP_CONTROL_RESULT_FAILED:
		LOGD("[APP_CONTROL_RESULT_FAILED]");
		s_info.runned_app_id = NULL;
		break;
	case APP_CONTROL_RESULT_SUCCEEDED:
		LOGD("[APP_CONTROL_RESULT_SUCCEEDED]");
		break;
	default:
		LOGD("UNKNOWN ERROR");
		s_info.runned_app_id = NULL;
		break;
	}
}

static void __app_icon_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	char *appid = (char *) data;
	app_control_h app_control_handle = NULL;

	if (!appid) {
		LOGE("Failed to obtain pgk_id of clicked application");
		return;
	}

	if (s_info.click_ignore == false) {
		if (!s_info.runned_app_id) {
			folder_panel_close_folder();

			if (app_control_create(&app_control_handle) != APP_CONTROL_ERROR_NONE) {
				LOGE("[FAILED][app_control_create]");
				return;
			}

			if (app_control_set_app_id(app_control_handle, appid) != APP_CONTROL_ERROR_NONE) {
				LOGE("[FAILED][app_control_set_app_id]");
				app_control_destroy(app_control_handle);
				return;
			}

			if (app_control_send_launch_request(app_control_handle, __app_icon_launch_request_cb, NULL) != APP_CONTROL_ERROR_NONE) {
				LOGE("[FAILED][app_control_send_launch_request]");
				app_control_destroy(app_control_handle);
				return;
			}
			app_control_destroy(app_control_handle);
		}
	}

}

static void __app_icon_pressed_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	app_item_type_t type = (app_item_type_t)data;

	if (type == APP_ITEM_FOLDER ||
		home_screen_get_view_type() == HOMESCREEN_VIEW_ALL_APPS)
			elm_object_signal_emit(obj, SIGNAL_ICON_PRESS, SIGNAL_PRESS_SOURCE);
}

static bool __app_icon_check_removeable_state(Evas_Object *icon)
{
	Tree_node_t *item = NULL;
	if (!icon) {
		LOGE("[INVALID_PARAM][icon='%p']", icon);
		return false;
	}

	item = evas_object_data_get(icon, KEY_ICON_DATA);
	if (!item) {
		LOGE("[FAILED][evas_object_data_get]");
		return false;
	}

	if (!item->data) {
		LOGE("[FAILED][item->data='%p']", item->data);
		return false;
	}

	return item->data->removable;
}

static void __app_icon_check_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	folder_checked_t check_info = FOLDER_PANEL_CHECKED_NONE;
	Tree_node_t *node = (Tree_node_t *)data;
	app_item_t *item = node->data;
	int count = -1;
	char trbuf[PATH_MAX] = {0, };

	if (item->type == APP_ITEM_ICON) {
		item->is_checked = true;
		if (node->parent->parent->data->type == APP_ITEM_FOLDER) {

			check_info = folder_panel_set_content_checked(node->parent->parent);

			switch (check_info) {
			case FOLDER_PANEL_CHECKED_NONE:
				elm_object_signal_emit(node->parent->parent->data->layout, SIGNAL_CHECKBOX_HIDE, SIGNAL_SOURCE);
				break;
			case FOLDER_PANEL_CHECKED_ANY:
				elm_object_signal_emit(node->parent->parent->data->layout, SIGNAL_CHECKBOX_SHOW_DIMMED, SIGNAL_SOURCE);
				break;
			case FOLDER_PANEL_CHECKED_ALL:
				elm_object_signal_emit(node->parent->parent->data->layout, SIGNAL_CHECKBOX_SHOW_CHECKED, SIGNAL_SOURCE);
				break;
			}
		}
	} else if (item->type == APP_ITEM_FOLDER) {
		data_model_check_all_apps(node, true);
	}
	option_menu_update_on_item_selected();

	count = data_model_get_app_check_state();
	if (count > FOLDER_MAX_ITEMS) {
		__app_icon_uncheck_cb(data, obj, SIGNAL_CHECKBOX_UNCHECKED, SIGNAL_SOURCE);
		elm_object_signal_emit(node->data->layout, SIGNAL_CHECKBOX_SHOW_UNCHECKED, SIGNAL_SOURCE);

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
		p_data->visible_time = 2.0;

		popup_show(p_data, NULL, NULL, NULL);
	}
}

static void __app_icon_uncheck_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	folder_checked_t check_info = FOLDER_PANEL_CHECKED_NONE;

	Tree_node_t *node = (Tree_node_t *)data;
	app_item_t *item = node->data;
	if (item->type == APP_ITEM_ICON) {
		item->is_checked = false;

		if (node->parent->parent->data->type == APP_ITEM_FOLDER) {

			check_info = folder_panel_set_content_checked(node->parent->parent);

			switch (check_info) {
			case FOLDER_PANEL_CHECKED_NONE:
				elm_object_signal_emit(node->parent->parent->data->layout, SIGNAL_CHECKBOX_HIDE, SIGNAL_SOURCE);
				break;
			case FOLDER_PANEL_CHECKED_ANY:
				elm_object_signal_emit(node->parent->parent->data->layout, SIGNAL_CHECKBOX_SHOW_DIMMED, SIGNAL_SOURCE);
				break;
			case FOLDER_PANEL_CHECKED_ALL:
				elm_object_signal_emit(node->parent->parent->data->layout, SIGNAL_CHECKBOX_SHOW_CHECKED, SIGNAL_SOURCE);
				break;
			}
		}
	} else if (item->type == APP_ITEM_FOLDER) {
		data_model_check_all_apps(node, false);
		elm_object_signal_emit(item->layout, SIGNAL_CHECKBOX_HIDE, SIGNAL_SOURCE);
	}
	option_menu_update_on_item_selected();
}

Evas_Object *__app_icon_load(Evas_Object *parent, const char *icon_path)
{
	Evas_Object *icon_image = NULL;

	if (!parent || !icon_path) {
		LOGE("Invalid parameter");
		return NULL;
	}

	icon_image = elm_image_add(parent);
	if (!icon_image) {
		LOGE("Failed to create an icon.");
		return NULL;
	}

	if ((strncmp("", icon_path, strlen(icon_path)) && elm_image_file_set(icon_image, icon_path, NULL)) || elm_image_file_set(icon_image, util_get_res_file_path(DEFAULT_APP_ICON), NULL))
		return icon_image;

	evas_object_del(icon_image);
	return NULL;
}

static void __app_icon_uninstall_button_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	LOGI("__app_icon_uninstall_button_clicked_cb");
	Tree_node_t *app_node = NULL;
	char buf[BUF_SIZE] = {0, };

	app_node = (Tree_node_t *) data;
	if (!app_node) {
		LOGE("[FAILED][evas_object_data_get]");
		return;
	}

	if (!app_node->data || !app_node->data->label) {
		LOGE("[INVALID][invalid node data]");
		return;
	}

	popup_data_t *p_data = popup_create_data();
	if (!p_data) {
		LOGE("[FAILED][failed to create popup data]");
		return;
	}

	if ((app_node->data->type == APP_ITEM_FOLDER) && (folder_panel_get_items_count(app_node) > 0)) {
		p_data->type = POPUP_TWO_BUTTONS;
		p_data->popup_title_text = strdup(_("IDS_HS_HEADER_REMOVE_FOLDER_ABB"));
		p_data->popup_content_text = strdup(_("IDS_HS_BODY_FOLDER_WILL_BE_REMOVED_APPLICATIONS_IN_THIS_FOLDER_WILL_NOT_BE_UNINSTALLED"));
		p_data->popup_confirm_text = strdup(_("IDS_ST_BUTTON_REMOVE_ABB2"));
		p_data->popup_cancel_text = strdup(_("IDS_CAM_SK_CANCEL"));
		p_data->orientation = ELM_POPUP_ORIENT_CENTER;
		popup_show(p_data, app_icon_uninstall, NULL, (void *) obj);
	} else if (app_node->data->type == APP_ITEM_ICON) {
		snprintf(buf, sizeof(buf), _("IDS_HS_POP_PS_WILL_BE_UNINSTALLED"), app_node->data->label);
		p_data->type = POPUP_TWO_BUTTONS;
		p_data->popup_title_text = strdup(_("IDS_AT_BODY_UNINSTALL"));
		p_data->popup_content_text = strdup(buf);
		p_data->popup_confirm_text = strdup(_("IDS_AT_BODY_UNINSTALL"));
		p_data->popup_cancel_text = strdup(_("IDS_CAM_SK_CANCEL"));
		p_data->orientation = ELM_POPUP_ORIENT_CENTER;
		popup_show(p_data, app_icon_uninstall, NULL, (void *) obj);
	} else {
		app_icon_uninstall((void *) obj);
		popup_data_free(p_data);
	}
}

void app_icon_uninstall(void *data)
{
	Tree_node_t *app_node = NULL;
	Evas_Object *icon = NULL;

	icon = (Evas_Object *) data;
	if (!icon) {
		LOGE("[INVALID_PARAM][data='%p']", data);
		return;
	}

	LOGI("[%s]", evas_object_type_get(icon));

	app_node = evas_object_data_get(icon, KEY_ICON_DATA);
	if (!app_node || !app_node->data) {
		LOGE("[FAILED][evas_object_data_get]");
		return;
	}

	all_apps_detach_from_grid(app_node->data->grid_item);
	app_node->data->grid_item = NULL;
	app_icon_remove(icon);
	app_node->data->layout = NULL;

	home_screen_mvc_item_uninstall_cb(app_node);
}

static void __app_icon_highlight_folder_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *icon = elm_object_part_content_get(obj, PART_ICON_CONTENT);
	elm_object_signal_emit(icon, SIGNAL_FRAME_POSSIBLE_SHOW, SIGNAL_SOURCE);
}

static void __app_icon_unhighlight_folder_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *icon = elm_object_part_content_get(obj, PART_ICON_CONTENT);
	elm_object_signal_emit(icon, SIGNAL_FRAME_HIDE, SIGNAL_SOURCE);
}

static void __app_icon_higlight_impossible_folder_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *icon = elm_object_part_content_get(obj, PART_ICON_CONTENT);
	elm_object_signal_emit(icon, SIGNAL_FRAME_IMPOSSIBLE_SHOW, SIGNAL_SOURCE);
}
/*===============================================================================================================*/
