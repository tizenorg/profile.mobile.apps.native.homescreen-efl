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

#include "folder_panel.h"
#include "homescreen-efl.h"
#include "util.h"
#include "page_scroller.h"
#include "app_icon.h"
#include "app_grid.h"
#include "layout.h"
#include "option_menu.h"
#include "tree.h"
#include "data_model.h"
#include "all_apps.h"

static struct
{
	Evas_Object *opened_folder_window;
	Evas_Object *opened_folder_popup;
	Elm_Theme *folder_entry_theme;
	app_item_t *folder_item;
	app_item_t *folder_item_add;
	Tree_node_t *destination_folder;
	bool click_ignore;
	bool is_opened;
	bool entry_activated;
	bool opened_folder_name_is_empty;
	bool opened_folder_entrychange_cb_ignore;
} s_info = {
	.opened_folder_window = NULL,
	.opened_folder_popup = NULL,
	.folder_entry_theme = NULL,
	.folder_item = NULL,
	.folder_item_add = NULL,
	.destination_folder = NULL,
	.click_ignore = false,
	.is_opened = false,
	.entry_activated = false,
	.opened_folder_name_is_empty = true,
	.opened_folder_entrychange_cb_ignore = true
};

typedef struct {
	bool all_checked;
	bool any_checked;
} check_info_s;

static void __folder_panel_create_folder_window_layout_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __folder_window_destroy_signal_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static bool __folder_panel_detach_grid_items(Tree_node_t *parent, Tree_node_t *item, void *data);
static void __folder_panel_name_set_cb(void *data, Evas_Object *obj, void *event_info);
static bool __folder_panel_get_items_count(Tree_node_t *parent, Tree_node_t *item, void *count);
/*frees folder UI data related to opened window:*/
static void __folder_panel_free_popup_data(folder_data_t *folder_data);

static Evas_Object* __folder_panel_create_icon(Tree_node_t *new_folder);
static Evas_Object *__folder_panel_create_scroller(Evas_Object *folder_window_layout);
static bool __folder_panel_create_layout(Evas_Object *parent, app_item_t *folder, bool keypad_show);
static bool __folder_panel_fill(Evas_Object *folder_window_layout, Evas_Object *folder_page_scroller,app_item_t *folder);
static bool __folder_panel_set_current_view_state(Tree_node_t *parent, Tree_node_t *item, void *data);
static bool __folder_panel_show_checkbox(Tree_node_t *parent, Tree_node_t *leaf, void *data);
static bool __folder_panel_get_checked_icons(Tree_node_t *parent, Tree_node_t *leaf, void *data);
static Evas_Object *__folder_panel_create_add_icon(app_item_t *folder);
static Evas_Object *__folder_panel_create_popup(Evas_Object *parent, app_item_t *item);

static Evas_Object *__folder_panel_create_entry(Evas_Object *parent, const char *default_text);
static void __folder_panel_entry_activated_cb(void *data, Evas_Object *obj, void *ei);
static void __folder_panel_entry_deactivated_cb(void *data, Evas_Object *obj, void *ei);
static void __folder_panel_entry_changed_cb(void *data, Evas_Object *obj, void *ei);
static void __folder_panel_entry_clicked_cb(void *data, Evas_Object *obj, void *ei);
static void __folder_panel_entry_done_cb(void *data, Evas_Object *obj, void *ei);
static void __folder_panel_entry_clear_button_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

static void __folder_panel_item_add_to_folder_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __folder_panel_print_labels(Tree_node_t *folder_tree);
static void __folder_panel_close_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

/*========================================== PUBLIC FUNCTIONS IMPLEMENTATION =============================================*/
HAPI void folder_panel_init_module(void)
{
	if (!s_info.folder_entry_theme) {
		s_info.folder_entry_theme = elm_theme_new();
		if (!s_info.folder_entry_theme) {
			LOGE("Failed to create theme");
			return;
		}

		elm_theme_ref_set(s_info.folder_entry_theme, NULL);
		elm_theme_extension_add(s_info.folder_entry_theme, util_get_res_file_path(EDJE_DIR"/folder_entry.edj"));
	}
}

HAPI void folder_panel_deinit_module(void)
{
	elm_theme_free(s_info.folder_entry_theme);
	s_info.folder_entry_theme = NULL;
}

HAPI app_item_t *folder_panel_create(Tree_node_t *data)
{
	LOGI("");
	Evas_Object *folder_icon_layout = NULL;
	folder_data_t *f_data = NULL;

	f_data = (folder_data_t *) calloc(1, sizeof(folder_data_t));
	if (!f_data) {
		LOGE("Failed to create either data: %d", f_data);
		return NULL;
	}

	folder_icon_layout = __folder_panel_create_icon(data);
	if (!folder_icon_layout) {
		LOGE("Failed to create folder");
		free(f_data);
		f_data = NULL;
	}

	data->data->layout = folder_icon_layout;
	data->data->data = f_data;

	__folder_panel_print_labels(data);

	return data->data;
}

HAPI void folder_panel_set_click_ignore_state(bool ignore)
{
	s_info.click_ignore = ignore;
}

HAPI void folder_panel_enable_entry(bool state)
{
	if (s_info.opened_folder_popup) {
		if (state) {
			elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_ENTRY_BLOCK_OFF, SIGNAL_SOURCE);
		} else {
			elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_ENTRY_BLOCK_ON, SIGNAL_SOURCE);
		}
	}
}

HAPI Elm_Object_Item * folder_panel_attach_to_grid(Evas_Object *icon, folder_data_t *folder_data)
{
	Evas_Object *grid_layout = NULL;
	Evas_Object *grid = NULL;

	if (!icon || !folder_data) {
		LOGE("icon: %d folder_data: %d", icon, folder_data);
		return NULL;
	}

	if (!folder_data->folder_scroller)
		return NULL;

	grid_layout = page_scroller_get_page(folder_data->folder_scroller, 0);
	grid = elm_object_part_content_get(grid_layout, PART_APP_GRID_CONTENT);

	/*the last item is "+:"*/
	return app_grid_insert_item_relative(grid_layout, icon, elm_gengrid_last_item_get(grid));
}

HAPI bool folder_panel_open_folder(app_item_t *folder, bool keypad_show)
{
	LOGI("");
	s_info.is_opened = true;
	s_info.opened_folder_entrychange_cb_ignore = true;

	return __folder_panel_create_layout(home_screen_get_win(), folder, keypad_show);
}

HAPI void folder_panel_close_folder(void)
{
	s_info.is_opened = false;
	elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_FOLDER_PANEL_CLOSE, SIGNAL_SOURCE);
	elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_FOLDER_UNNAMED_TEXT_HIDE, SIGNAL_SOURCE);
	elm_object_signal_callback_add(s_info.opened_folder_popup, SIGNAL_FOLDER_ANIM_DONE, SIGNAL_SOURCE,
		__folder_panel_close_cb, NULL);

}

HAPI void folder_panel_cancel_add_icon(void)
{
	if (s_info.folder_item_add)
		app_icon_set_color(s_info.folder_item_add->layout, 255, 255, 255, 255);
	s_info.folder_item_add = NULL;
	s_info.destination_folder = NULL;
}

HAPI bool folder_panel_is_folder_visible(void)
{
	return s_info.is_opened;
}

HAPI folder_checked_t folder_panel_set_content_checked(Tree_node_t *node)
{
	LOGD("");
	check_info_s check_info = {
		.all_checked = true,
		.any_checked = false
	};

	tree_in_depth_browse(node, __folder_panel_get_checked_icons, &check_info);

	if (check_info.all_checked) {
		LOGD("All items checked");
		return FOLDER_PANEL_CHECKED_ALL;
	}
	else if (check_info.any_checked) {
		LOGD("Any item checked");
		return FOLDER_PANEL_CHECKED_ANY;
	}
	else {
		LOGD("None items checked");
		return FOLDER_PANEL_CHECKED_NONE;
	}
}

HAPI Tree_node_t *folder_panel_get_opened_folder_node(void)
{
	return s_info.destination_folder;
}

HAPI const app_item_t *folder_panel_get_opened_folder_item(void)
{
	return s_info.folder_item;
}

HAPI int folder_panel_get_items_count(Tree_node_t *folder)
{
	int i = 0;
	data_model_iterate_pages(folder, NULL, __folder_panel_get_items_count, &i);

	return i;
}

HAPI void folder_panel_remove_empty_folder(Tree_node_t *folder)
{
	if (!folder) {
		LOGE("[INVALID_PARAM][folder=NULL]");
		return;
	}

	app_icon_remove(folder->data->layout);
	folder->data->layout = NULL;
}

HAPI void folder_panel_get_folder_geometry(int *x, int *y, int *w, int *h)
{
	if (!s_info.opened_folder_popup) {
		if (x)
			*x = -1;

		if (y)
			*y = -1;

		if (w)
			*w = -1;

		if (h)
			*h = -1;
	}

	evas_object_geometry_get(s_info.opened_folder_popup, x, y, w, h);
}

HAPI void folder_panel_hide_input_panel(void)
{
	Evas_Object *entry = NULL;

	if (!s_info.opened_folder_popup)
		return;

	entry = elm_object_part_content_get(s_info.opened_folder_popup, PART_FOLDER_NAME);
	if (!entry) {
		LOGE("[FAILED][entry=NULL]");
		return;
	}

	elm_entry_input_panel_hide(entry);
}
/*====================================== PRIVATE FUNCTIONS IMPLEMENTATION =================================================*/
static bool __folder_panel_get_items_count(Tree_node_t *parent, Tree_node_t *item, void *count)
{
	(*(int *)count)++;

	return true;
}

static void __folder_panel_create_folder_window_layout_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source)
{
	bool keypad_show = false;

	if (s_info.click_ignore && s_info.destination_folder) { /*don't open a folder when adding items to it*/
		LOGI("Click ignore");
		return;
	}

	s_info.folder_item = (app_item_t*) data;
	if (!s_info.folder_item) {
		LOGE("Failed to get folder item");
		return;
	}

	if (!emission && !source)
		keypad_show = true;

	if (!folder_panel_open_folder(s_info.folder_item, keypad_show)) {
		LOGE("Failed to open folder");
		return;
	}

	app_icon_highlight_folder_icon(s_info.folder_item->layout, FOLDER_ICON_FRAME_POSSIBLE);
}

static void __folder_window_destroy_signal_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	folder_panel_close_folder();
}

static void __folder_panel_name_set_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGI("");
	app_item_t *folder = NULL;
	Tree_node_t *node = NULL;
	const char *entry_text;
	char prev_label[NAME_MAX];

	folder = (app_item_t*) data;
	if (!data) {
		LOGE("invalid cb parameter : data is NULL");
		return;
	}

	strncpy(prev_label, folder->label, NAME_MAX - sizeof('\0'));

	if (elm_entry_is_empty(obj) || s_info.opened_folder_name_is_empty) {
		free((void *)folder->label);
		folder->label = strdup("");
		elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_FOLDER_UNNAMED_TEXT_SHOW,
				SIGNAL_SOURCE);
	} else {
		entry_text = elm_object_text_get(obj);
		if (!entry_text) {
			LOGE("Failed to get the text from the entry");
			return;
		}

		free((void *)folder->label);
		folder->label = strdup(entry_text);
		if (!folder->label) {
			LOGE("Failed to copy the text");
			return;
		}

		folder->appid = strdup(entry_text);
		if (!folder->appid) {
			LOGE("Failed to copy the text");
			return;
		}
	}

	if (folder->label &&
		strncasecmp(prev_label, folder->label, NAME_MAX) != 0) {
		app_icon_set_label(folder->layout, folder->label);

		node = (Tree_node_t*)evas_object_data_get(folder->layout, KEY_ICON_DATA);
		if (!node) {
			LOGE("node == NULL");
			return;
		}
		data_model_update_item(node);
		home_screen_mvc_update_view();
	}

	home_screen_print_tree();
}

static Evas_Object* __folder_panel_create_icon(Tree_node_t *new_folder)
{
	Evas_Object *folder_icon_layout = app_icon_create(new_folder, APP_ICON_TYPE_FOLDER);
	if (!folder_icon_layout) {
		LOGE("Failed to create folder");
		return NULL;
	}

	elm_object_signal_callback_add(folder_icon_layout, SIGNAL_ICON_CLICKED, SIGNAL_SOURCE, __folder_panel_create_folder_window_layout_cb, new_folder->data);

	return folder_icon_layout;
}

static bool __folder_panel_create_layout(Evas_Object *parent, app_item_t *folder, bool keypad_show)
{
	Evas_Object *entry = NULL;
	Evas_Object *add_icon = NULL;

	folder_data_t *f_data = NULL;
	Tree_node_t *f_node = NULL;

	int count = -1;

	if (!parent || !folder) {
		LOGE("Invalid parameter");
		return false;
	}

	f_data = (folder_data_t *) folder->data;
	if (!f_data) {
		LOGE("Failed to get folder data");
		return false;
	}

	s_info.opened_folder_window = elm_layout_add(parent);
	if (!s_info.opened_folder_window) {
		LOGE("Failed to create folder icon layout");
		return false;
	}

	if (!elm_layout_file_set(s_info.opened_folder_window, util_get_res_file_path(EDJE_DIR"/folder.edj"), GROUP_FOLDER_WINDOW)) {
		LOGE("Failed to load edje file for layout");
		evas_object_del(s_info.opened_folder_window);
		s_info.opened_folder_window = NULL;
		return false;
	}

	s_info.opened_folder_popup = __folder_panel_create_popup(parent, folder);
	if (!s_info.opened_folder_popup) {
		LOGE("Failed to create folder_popup ");
		evas_object_del(s_info.opened_folder_window);
		s_info.opened_folder_window = NULL;
		return false;
	}
	evas_object_show(s_info.opened_folder_popup);

	entry = __folder_panel_create_entry(s_info.opened_folder_popup, folder->label);
	if (!entry) {
		LOGE("Failed to create entry");
		evas_object_del(s_info.opened_folder_window);
		evas_object_del(s_info.opened_folder_popup);
		s_info.opened_folder_window = NULL;
		s_info.opened_folder_popup = NULL;
		return false;
	}

	f_data->folder_scroller = __folder_panel_create_scroller(home_screen_get_win());
	if (!f_data->folder_scroller) {
		LOGE("Failed to create folder layout");
		evas_object_del(s_info.opened_folder_window);
		evas_object_del(s_info.opened_folder_popup);
		s_info.opened_folder_window = NULL;
		s_info.opened_folder_popup = NULL;
		return false;
	}
	elm_object_part_content_set(s_info.opened_folder_popup, PART_FOLDER_CONTENT, f_data->folder_scroller);
	if (!__folder_panel_fill(s_info.opened_folder_popup, f_data->folder_scroller, folder)) {
		LOGE("Failed to fill folder grid");
		evas_object_del(s_info.opened_folder_window);
		evas_object_del(s_info.opened_folder_popup);
		__folder_panel_free_popup_data(folder->data);
		s_info.opened_folder_window = NULL;
		s_info.opened_folder_popup = NULL;
		return false;
	}

	f_node = evas_object_data_get(folder->layout, KEY_ICON_DATA);
	if (!f_node) {
		LOGE("[FAILED][f_node=NULL]");
		evas_object_del(s_info.opened_folder_window);
		evas_object_del(s_info.opened_folder_popup);
		__folder_panel_free_popup_data(folder->data);
		s_info.opened_folder_window = NULL;
		s_info.opened_folder_popup = NULL;
		return false;
	}

	count = folder_panel_get_items_count(f_node);
	if (count < FOLDER_MAX_ITEMS) {
		add_icon = __folder_panel_create_add_icon(folder);

		if (!app_grid_append_item(page_scroller_get_page(f_data->folder_scroller, page_scroller_get_page_count(f_data->folder_scroller)-1), add_icon)) {
			LOGE("Creating folder window without \"+\"");
			evas_object_del(add_icon);
		}
	}

	evas_object_resize(s_info.opened_folder_window, FOLDER_WINDOW_W, FOLDER_WINDOW_H);
	evas_object_move(s_info.opened_folder_window, 0, 0);
	evas_object_show(s_info.opened_folder_window);

	elm_object_signal_callback_add(s_info.opened_folder_window, SIGNAL_BG_CLICKED, SIGNAL_SOURCE, __folder_window_destroy_signal_cb, folder);
	evas_object_smart_callback_add(entry, "focused", __folder_panel_entry_activated_cb, folder);
	evas_object_smart_callback_add(entry, "unfocused", __folder_panel_entry_deactivated_cb, folder);
	elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_FOLDER_PANEL_OPEN, SIGNAL_SOURCE);

	return true;
}

static Evas_Object* __folder_panel_create_scroller(Evas_Object *folder_window_layout)
{
	Evas_Object *folder_page_scroller = NULL;
	LOGI("");

	if (!folder_window_layout) {
		LOGE("Invalid parameter");
		return NULL;
	}

	folder_page_scroller = page_scroller_create(FOLDER_GRID_W_REL*home_screen_get_root_width(), FOLDER_GRID_H_REL*home_screen_get_root_height());
	if (!folder_page_scroller) {
		LOGE("Failed to create folder page scroller layout");
		return NULL;
	}

	elm_scroller_movement_block_set(folder_page_scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
	elm_scroller_movement_block_set(folder_page_scroller, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
	elm_scroller_loop_set(folder_page_scroller, EINA_FALSE, EINA_FALSE);

	return folder_page_scroller;
}

static bool __folder_panel_detach_grid_items(Tree_node_t *parent, Tree_node_t *item, void *data)
{
	if (item->data && item->data->grid_item) {
		app_grid_unpack_item(item->data->grid_item);
		elm_object_item_del(item->data->grid_item);
		evas_object_hide(item->data->layout);
		item->data->grid_item = NULL;

		return true;
	}

	return false;
}

static bool __folder_panel_set_current_view_state(Tree_node_t *parent, Tree_node_t *item, void *data)
{
	app_icon_set_view_mode(item->data->layout, home_screen_get_view_type(), true);
	return true;
}

static bool __folder_panel_show_checkbox(Tree_node_t *parent, Tree_node_t *leaf, void *data)
{
	if (leaf->data->type == APP_ITEM_ICON) {
		if (leaf->data->is_checked)
			elm_object_signal_emit(leaf->data->layout, SIGNAL_CHECKBOX_SHOW_CHECKED, SIGNAL_SOURCE);
		else
			elm_object_signal_emit(leaf->data->layout, SIGNAL_CHECKBOX_SHOW_UNCHECKED, SIGNAL_SOURCE);
	}
	return true;
}

static bool __folder_panel_get_checked_icons(Tree_node_t *parent, Tree_node_t *leaf, void *data)
{
	check_info_s *check_info = data;

	if (leaf->data->type == APP_ITEM_ICON) {
		check_info->all_checked = check_info->all_checked && leaf->data->is_checked;
		check_info->any_checked = check_info->any_checked || leaf->data->is_checked;
	}

	return true;
}

static bool __folder_panel_fill(Evas_Object *folder_window_layout, Evas_Object *folder_page_scroller, app_item_t *folder)
{
	folder_data_t *folder_data = NULL;
	Tree_node_t *it;
	Tree_node_t *folder_app_tree = NULL;
	Evas_Object *folder_app_grid = NULL;
	LOGI("");

	if (!folder_page_scroller || !folder) {
		LOGE("Invalid parameter");
		return false;
	}

	folder_data = (folder_data_t*)folder->data;
	if (!folder_data) {
		LOGE("Failed to get folder data");
		return false;
	}

	folder_app_tree = evas_object_data_get(folder->layout, KEY_ICON_DATA);
	if (!folder_app_tree) {
		LOGE("Failed to get folder app tree");
		return false;
	}

	TREE_NODE_FOREACH(folder_app_tree, it) {
		folder_app_grid = app_grid_create(folder_page_scroller, it,
				FOLDER_GRID_W_REL*home_screen_get_root_width(), FOLDER_GRID_H_REL*home_screen_get_root_height(), APP_GRID_TYPE_FOLDER);
		if (!folder_app_grid) {
			LOGE("[FAILED][app_grid_create]");
			return false;
		}

		if (!page_scroller_add_page(folder_page_scroller, folder_app_grid)) {
			LOGE("[FAILED][page_scroller_page_add]");
			evas_object_del(folder_app_grid);
			return false;
		}

		data_model_iterate(it, __folder_panel_set_current_view_state, NULL);
	}

	if (home_screen_get_view_type() == HOMESCREEN_VIEW_ALL_APPS_CHOOSE)
		data_model_iterate(folder_app_tree, __folder_panel_show_checkbox, NULL);

	__folder_panel_print_labels(folder_app_tree);

	return true;
}

static Evas_Object* __folder_panel_create_add_icon(app_item_t *folder)
{
	Evas_Object *icon_layout = NULL;
	Evas_Object *add_icon = NULL;
	LOGI("");

	icon_layout = util_create_edje_layout(home_screen_get_win(), util_get_res_file_path(EDJE_DIR"/icon.edj"), GROUP_ICON_LY);
	if (!icon_layout) {
		LOGE("Failed to create icon layout");
		return NULL;
	}

	add_icon = elm_layout_add(home_screen_get_win());
	if (!add_icon) {
		LOGE("Failed to create folder icon layout");
		evas_object_del(icon_layout);
		return NULL;
	}

	if (!elm_layout_file_set(add_icon, util_get_res_file_path(EDJE_DIR"/folder.edj"), GROUP_FOLDER_ADD_ICON)) {
		LOGE("Failed to load edje file for layout");
		evas_object_del(add_icon);
		evas_object_del(icon_layout);
		return NULL;
	}

	elm_object_part_content_set(icon_layout, PART_ICON_CONTENT, add_icon);
	evas_object_size_hint_align_set(icon_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(icon_layout);
	evas_object_show(add_icon);

	elm_object_signal_callback_add(add_icon, "mouse,clicked,1", "add_icon", __folder_panel_item_add_to_folder_cb, s_info.folder_item);
	evas_object_data_set(icon_layout, KEY_IS_REPOSITIONABLE, (void*)false);

	return icon_layout;
}

static Evas_Object *__folder_panel_create_popup(Evas_Object *parent, app_item_t *folder)
{
	Evas_Object *folder_popup = NULL;
	Tree_node_t *folder_app_tree = NULL;

	if (!parent || !folder) {
		LOGE("Invalid parameters: [%p] [%p]", parent, folder);
		return NULL;
	}

	folder_app_tree = evas_object_data_get(folder->layout, KEY_ICON_DATA);
	if (!folder_app_tree) {
		LOGE("Failed to get folder app tree");
		return NULL;
	}

	folder_popup = elm_layout_add(parent);
	if (!folder_popup) {
		LOGE("Failed to create folder_popup ");
		return NULL;
	}

	if (!elm_layout_file_set(folder_popup, util_get_res_file_path(EDJE_DIR"/folder.edj"), GROUP_FOLDER_POPUP)) {
		LOGE("Failed to set folder popup");
		evas_object_del(folder_popup);
		return NULL;
	}

	elm_object_part_text_set(folder_popup, PART_FOLDER_UNNAMED_TEXT, gettext("IDS_COM_HEADER_UNNAMED_FOLDER"));

	evas_object_resize(folder_popup, FOLDER_POPUP_W_REL*home_screen_get_root_width(), FOLDER_3_ROW_H_REL*home_screen_get_root_height());

	evas_object_move(folder_popup, FOLDER_POPUP_X_REL*home_screen_get_root_width(),
		FOLDER_POPUP_Y_REL*home_screen_get_root_height());

	return folder_popup;
}

static Evas_Object *__folder_panel_create_entry(Evas_Object *parent, const char *default_text)
{
	Evas_Object *entry = NULL;

	if (!parent) {
		LOGE("Invalid parameter : parent is NULL");
		return NULL;
	}

	entry = elm_entry_add(parent);
	if (!entry) {
		LOGE("Failed to create entry widget");
		return NULL;
	}

	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	evas_object_show(entry);

	if (s_info.folder_entry_theme) {
		elm_object_theme_set(entry, s_info.folder_entry_theme);
		elm_object_style_set(entry, "default");
	}

	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	evas_object_smart_callback_add(entry, "changed", __folder_panel_entry_changed_cb, parent);
	evas_object_smart_callback_add(entry, "clicked", __folder_panel_entry_clicked_cb, parent);
	evas_object_smart_callback_add(entry, "activated", __folder_panel_entry_done_cb, parent);

	elm_object_signal_callback_add(parent, SIGNAL_CLEAR_BTN_CLICKED, SIGNAL_SOURCE,
				__folder_panel_entry_clear_button_clicked_cb, entry);

	if (strlen(default_text) > 0) {
		elm_entry_entry_set(entry, default_text);
		s_info.opened_folder_name_is_empty = false;
	} else {
		elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_FOLDER_UNNAMED_TEXT_SHOW,
						SIGNAL_SOURCE);
		elm_object_text_set(entry, "");
		s_info.opened_folder_name_is_empty = true;
	}

	elm_object_part_content_set(s_info.opened_folder_popup, PART_FOLDER_NAME, entry);

	return entry;
}

static void __folder_panel_entry_activated_cb(void *data, Evas_Object *obj, void *ei)
{
	app_item_t *folder = (app_item_t*) data;
	LOGI("Entry");

	if (!folder) {
		LOGE("Failed to get folder");
		return;
	}

	if (!strcmp(folder->label, ""))
		s_info.opened_folder_name_is_empty = true;

	elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_FOLDER_UNNAMED_TEXT_HIDE,
			SIGNAL_SOURCE);

	s_info.entry_activated = true;
	s_info.opened_folder_entrychange_cb_ignore = false;
}

static void __folder_panel_entry_deactivated_cb(void *data, Evas_Object *obj, void *ei)
{
	const char *label = NULL;
	label = elm_entry_entry_get(obj);

	if (!label || !strcmp(label, "")) {
		s_info.opened_folder_name_is_empty = true;
		elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_FOLDER_UNNAMED_TEXT_SHOW,
			SIGNAL_SOURCE);
	}

	elm_object_signal_emit(s_info.opened_folder_popup, SIGNAL_CLEAR_BTN_HIDE, SIGNAL_SOURCE);
}

static void __folder_panel_entry_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	Evas_Object *folder = (Evas_Object *)data;
	const char *entry_text = elm_object_text_get(obj);

	if (s_info.opened_folder_entrychange_cb_ignore)
		return;

	if (!entry_text || strlen(entry_text) == 0) {
		s_info.opened_folder_name_is_empty = true;
		elm_object_signal_emit(folder, SIGNAL_CLEAR_BTN_HIDE, SIGNAL_SOURCE);
	} else {
		s_info.opened_folder_name_is_empty = false;
		elm_object_signal_emit(folder, SIGNAL_CLEAR_BTN_SHOW, SIGNAL_SOURCE);
	}
}

static void __folder_panel_entry_clicked_cb(void *data, Evas_Object *obj, void *ei)
{
	Evas_Object *folder_popup = (Evas_Object*) data;
	LOGI("");

	if (!folder_popup) {
		LOGE("Invalid data");
		return;
	}

	if (s_info.entry_activated) {
		elm_entry_cursor_end_set(obj);
		s_info.entry_activated = false;
	}

	if (!s_info.opened_folder_name_is_empty)
		elm_object_signal_emit(folder_popup, SIGNAL_CLEAR_BTN_SHOW, SIGNAL_SOURCE);
	else
		elm_object_signal_emit(folder_popup, SIGNAL_CLEAR_BTN_HIDE, SIGNAL_SOURCE);
	elm_entry_input_panel_show(obj);
}

static void __folder_panel_entry_done_cb(void *data, Evas_Object *obj, void *ei)
{
	LOGI("");
	Evas_Object *entry = obj;
	Evas_Object *folder_popup = data;

	if (!folder_popup || !entry) {
		LOGE("Invalid data");
		return;
	}

	elm_entry_input_panel_hide(entry);
}

static void __folder_panel_entry_clear_button_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	LOGI("");
	Evas_Object *entry = NULL;

	entry = (Evas_Object*) data;
	if (!entry) {
		LOGE("Invalid data pointer");
		return;
	}

	elm_object_text_set(entry, "");
	s_info.opened_folder_name_is_empty = true;
	elm_object_signal_emit(obj, SIGNAL_CLEAR_BTN_HIDE, SIGNAL_SOURCE);
}

static void __folder_panel_item_add_to_folder_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	app_item_t *folder_item = NULL;
	Tree_node_t *node = NULL;

	home_screen_set_view_type(HOMESCREEN_VIEW_ALL_APPS_CHOOSE);
	option_menu_update_on_item_selected();
	folder_panel_close_folder();

	folder_item = (app_item_t*) data;
	if (!folder_item) {
		LOGE("Failed to get folder item");
		return;
	}

	node = evas_object_data_get(folder_item->layout, KEY_ICON_DATA);
	if (!node) {
		LOGE("[FAILED][node=NULL]");
		return;
	}
	s_info.destination_folder = node;

	elm_object_signal_emit(folder_item->layout, SIGNAL_CHECKBOX_CHECKED, SIGNAL_SOURCE);
	elm_object_signal_emit(folder_item->layout, SIGNAL_CHECKBOX_SHOW_CHECKED, SIGNAL_SOURCE);
	elm_object_signal_emit(folder_item->layout, SIGNAL_CHECKBOX_BLOCK, SIGNAL_SOURCE);

	app_icon_set_color(folder_item->layout, 150, 150, 150, 150);
	s_info.folder_item_add = folder_item;
}

static void __folder_panel_print_labels(Tree_node_t *folder_tree)
{
	app_item_t *t = NULL;
	Tree_node_t *page = NULL;
	Tree_node_t *it = NULL;
	int i=0;
	TREE_NODE_FOREACH(folder_tree, page) {
		TREE_NODE_FOREACH(page, it) {
			t = it->data;
			LOGI("item %d: %s", i++, t->label);
		}
	}
}

static void __folder_panel_free_popup_data(folder_data_t *folder_data)
{
	if (!folder_data)
		return;

	evas_object_del(folder_data->folder_scroller);
	folder_data->folder_scroller = NULL;
}

static void __folder_panel_close_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Tree_node_t *folder_app_tree = NULL;
	Evas_Object *entry = NULL;

	entry = elm_object_part_content_get(s_info.opened_folder_popup, PART_FOLDER_NAME);
	elm_object_signal_callback_del(s_info.opened_folder_popup, SIGNAL_FOLDER_ANIM_DONE, SIGNAL_SOURCE, __folder_panel_close_cb);
	__folder_panel_name_set_cb(s_info.folder_item, entry, NULL);

	if (s_info.folder_item) {
		folder_app_tree = evas_object_data_get(s_info.folder_item->layout, KEY_ICON_DATA);
		if (folder_app_tree) {
			__folder_panel_print_labels(folder_app_tree);
			data_model_iterate_pages(folder_app_tree, NULL, __folder_panel_detach_grid_items, NULL);
		}

		app_icon_highlight_folder_icon(s_info.folder_item->layout, FOLDER_ICON_FRAME_NONE);
		//app_icon_set_color(s_info.folder_item->layout, 255, 255, 255, 255);

		__folder_panel_free_popup_data(s_info.folder_item->data);
		s_info.folder_item = NULL;
	}

	if (s_info.opened_folder_popup) {
		evas_object_del(s_info.opened_folder_popup);
		s_info.opened_folder_popup = NULL;
	}
	if (s_info.opened_folder_window) {
		evas_object_del(s_info.opened_folder_window);
		s_info.opened_folder_window = NULL;
	}
}
