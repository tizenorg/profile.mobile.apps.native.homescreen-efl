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
#include <widget_viewer_evas.h>
#include <widget_errno.h>
#include <shortcut.h>

#include "homescreen-efl.h"
#include "livebox/livebox_widget.h"
#include "livebox/livebox_panel.h"
#include "livebox/livebox_utils.h"
#include "util.h"
#include "data_model.h"
#include "page_scroller.h"
#include "popup.h"


static int __livebox_widget_get_pkg_list_cb(const char *pkg_id, const char *widget_id, int is_prime, void *data);

static int request_cb(const char *appid,
		const char *name,
		int type,
		const char *content_info,
		const char *icon,
		int pid,
		double period,
		int allow_duplicate,
		void *data);

static void update_content_info(Evas_Object *widget, const char *content_info);

static struct {
	Eina_List *widget_list;
} s_info = {
	.widget_list = NULL,
};

void livebox_widget_init(void)
{
	int ret = -1;

	ret = widget_service_get_widget_list(__livebox_widget_get_pkg_list_cb, NULL);

	if (ret < 0) {
		LOGE("FAILED TO GET PKGLIST. ERROR CODE: %d", ret);
		return;
	}

	LOGI("SUCCESS TO GET PKGLIST. COUNT: %d", ret);
	widget_viewer_evas_init(home_screen_get_win());

	ret = shortcut_set_request_cb(request_cb, NULL);
	if (ret < 0)
		LOGE("Failed to add shortcut event listener: 0x%X\n", ret);
}

Eina_List *livebox_widget_get_list(void)
{
	if (!s_info.widget_list) {
		LOGE("s_info.widget_list == NULL");
		return NULL;
	}

	return s_info.widget_list;
}

static void widget_content_info_updated_cb(void *data, Evas_Object *obj, void *event_info)
{
	widget_evas_event_info_s *info = event_info;
	const char *content_info;

	if (info->error != WIDGET_ERROR_NONE || !info->widget_app_id)
		return;

	content_info = widget_viewer_evas_get_content_info(obj);
	LOGD("Content inf is updated [%s] - [%s]\n", info->widget_app_id, content_info);

	update_content_info(obj, content_info);

}

Evas_Object *livebox_widget_add(const char *pkg_name, Evas_Object *parent, char *content_info)
{
	Evas_Object *livebox = widget_viewer_evas_add_widget(parent, pkg_name, content_info, WIDGET_VIEWER_EVAS_DEFAULT_PERIOD);
	if (!livebox) {
		LOGE("livebox == NULL");
		return NULL;
	}

	evas_object_smart_callback_add(livebox, WIDGET_SMART_SIGNAL_EXTRA_INFO_UPDATED, widget_content_info_updated_cb, NULL);

	return livebox;
}

void livebox_widget_del(Evas_Object *widget)
{
	if (!widget) {
		LOGE("widget == NULL");
		return;
	}

	LOGI("DELETING WIDGET: %s", widget_viewer_evas_get_widget_id(widget));
	widget_viewer_evas_set_permanent_delete(widget, 1);
}

void livebox_widget_fini(void)
{
	Eina_List *l;
	widget_info_t *widget = NULL;

	if (!s_info.widget_list) {
		LOGE("s_info.widget_list == NULL");
		return;
	}

	EINA_LIST_FOREACH(s_info.widget_list, l, widget) {
		if (!widget) {
			LOGE("widget == NULL");
			continue;
		}

		free(widget->pkg_id);
		free(widget->widget_id);
	}

	eina_list_free(s_info.widget_list);
}

void livebox_widget_viewer_paused(void)
{
	int ret;
	ret = widget_viewer_evas_notify_paused_status_of_viewer();
	LOGD("Viewer is paused: %d\n", ret);
}

void livebox_widget_viewer_resumed(void)
{
	Evas_Object *page = NULL;
	Eina_List *lb_layouts = NULL;
	Eina_List *l = NULL;
	Eina_List *l2 = NULL;
	Eina_List *pages = NULL;
	Evas_Object *widget = NULL;
	Evas_Object *lb_layout = NULL;
	const char *content_info = NULL;
	int ret;
	static Eina_Bool app_created = EINA_TRUE;

	ret = widget_viewer_evas_notify_resumed_status_of_viewer();
	LOGD("Viewer is resumed: %d\n", ret);

	if (app_created) {
		app_created = EINA_FALSE;
		return;
	}

	pages = page_scroller_get_all_pages(livebox_panel_get());

	if (!pages) {
		LOGE("pages == NULL");
		return;
	}

	EINA_LIST_FOREACH(pages, l, page) {
		if (!page) {
			LOGE("page == NULL");
			return;
		}

		lb_layouts = livebox_utils_get_liveboxes_on_page_list(page);
		if (!lb_layouts) {
			LOGE("lb_layouts == NULL");
			continue;
		}

		EINA_LIST_FOREACH(lb_layouts, l2, lb_layout) {
			if (!lb_layout) {
				LOGE("lb_layout == NULL");
				continue;
			}

			widget = elm_layout_content_get(lb_layout, PART_LIVEBOX);
			if (!widget) {
				LOGE("widget == NULL");
				return;
			}

			content_info = widget_viewer_evas_get_content_info(widget);
			update_content_info(widget, content_info);

			LOGD("Content inf is updated [%s] - [%s]\n",
					widget_viewer_evas_get_widget_id(widget),
					content_info);
		}
	}
}

void livebox_widget_get_dimensions(int size, int *x, int *y)
{
	if (!x || !y) {
		LOGE("Wrong arguments provided: %p %p", x, y);
		return;
	}

	switch (size) {
	case WIDGET_SIZE_TYPE_1x1:
		*x = 1;
		*y = 1;
		break;
	case WIDGET_SIZE_TYPE_2x1:
		*x = 2;
		*y = 1;
		break;
	case WIDGET_SIZE_TYPE_2x2:
		*x = 2;
		*y = 2;
		break;
	case WIDGET_SIZE_TYPE_4x1:
		*x = 4;
		*y = 1;
		break;
	case WIDGET_SIZE_TYPE_4x2:
		*x = 4;
		*y = 2;
		break;
	case WIDGET_SIZE_TYPE_4x3:
		*x = 4;
		*y = 3;
		break;
	case WIDGET_SIZE_TYPE_4x4:
		*x = 4;
		*y = 4;
		break;
	case WIDGET_SIZE_TYPE_4x5:
		*x = 4;
		*y = 5;
		break;
	case WIDGET_SIZE_TYPE_4x6:
		*x = 4;
		*y = 6;
		break;
	case WIDGET_SIZE_TYPE_EASY_1x1:
		*x = 1;
		*y = 1;
		break;
	case WIDGET_SIZE_TYPE_EASY_3x1:
		*x = 3;
		*y = 1;
		break;
	case WIDGET_SIZE_TYPE_EASY_3x3:
		*x = 3;
		*y = 3;
		break;
	case WIDGET_SIZE_TYPE_FULL:
		*x = 4;
		*y = 4;
		break;
	case WIDGET_SIZE_TYPE_UNKNOWN:
		*x = -1;
		*y = -1;
		break;
	default:
		*x = -1;
		*y = -1;
	}
}

void livebox_widget_get_size_available(Evas_Object *widget, int *cnt, int **size_list)
{
	int ret = -1;
	const char *widget_id = NULL;
	*cnt = 0;

	if (!widget) {
		LOGE("widget == NULL");
		return;
	}

	if (!size_list) {
		LOGE("size_list == NULL");
		return;
	}

	if (*size_list) {
		free(*size_list);
		*size_list = NULL;
	}

	widget_id = widget_viewer_evas_get_widget_id(widget);
	if (!widget_id) {
		LOGE("widget_id == NULL");
		free(*size_list);
		*size_list = NULL;
		return;
	}

	ret = widget_service_get_supported_size_types(widget_id, cnt, size_list);

	if (ret != 0 || *cnt == 0) {
		LOGE("RESIZE COULD NOT GET SUPPORTED SIZES");
		*size_list = calloc(WIDGET_NR_OF_SIZE_LIST, sizeof(widget_size_type_e));
		*cnt = 2;
		(*size_list)[0] = WIDGET_SIZE_TYPE_4x2;
		(*size_list)[1] = WIDGET_SIZE_TYPE_4x4;
	}

	LOGD("RESIZE AVAILABLE SIZE COUNT: %d", *cnt);
}

void livebox_widget_set_event_blocking(Evas_Object *widget)
{
	if (!widget) {
		LOGE("widget == NULL");
		return;
	}

	widget_viewer_evas_cancel_click_event(widget);
	LOGI("BLOCKING: %s", widget_viewer_evas_get_widget_id(widget));
}

static int __livebox_widget_get_pkg_list_cb(const char *pkg_id, const char *widget_id, int is_prime, void *data)
{
	widget_info_t *widget = NULL;
	int cnt = WIDGET_NR_OF_SIZE_LIST;;
	int *size_list = NULL;
	int ret = -1;

	LOGI("WIDGET: %s; %s; %d", pkg_id, widget_id, is_prime);

	widget = calloc(1, sizeof(widget_info_t));
	if (!widget) {
		LOGE("widget == NULL");
		return 0;
	}

	widget->widget_id = strdup(widget_id);
	widget->pkg_id    = strdup(pkg_id);
	widget->is_prime  = is_prime;

	ret = widget_service_get_supported_size_types(widget_id, &cnt, &size_list);

	if(ret == 0 && cnt > 0) {
		livebox_widget_get_dimensions(size_list[0], &widget->width, &widget->height);
		LOGI("SIZE SET [%d, %d] ret == %d; cnt == %d", widget->width, widget->height, ret, cnt);
	} else {
		LOGE("COULD NOT GET AVAILABLE SiZES ret == %d; cnt == %d; size_list[0] == %d", ret, cnt, size_list[0]);
		widget->width  = LIVEBOX_ON_CREATE_SIZE;
		widget->height = LIVEBOX_ON_CREATE_SIZE;
	}

	s_info.widget_list = eina_list_append(s_info.widget_list, widget);
	return 0;
}

static void livebox_widget_new(int widget_width, int widget_height, const char *content_info)
{
	int page_index = -1;
	int pos_x = 0;
	int pos_y = 0;
	Tree_node_t *page_node = NULL;
	Tree_node_t *item_node = NULL;
	Evas_Object *page = NULL;

	page_index = livebox_panel_find_empty_space(widget_width, widget_height, &pos_x, &pos_y);

	if (page_index > -1)
		page = page_scroller_get_page(livebox_panel_get(), page_index);

	if (!page) {
		pos_x = 0;
		pos_y = 0;

		if (!livebox_panel_can_add_page()) {
			LOGI("Cannot add new page");
			return;
		}

		page_node = data_model_add_widget_page();
		if (!page_node) {
			livebox_panel_destroy();
			LOGE("page_node == NULL");
			return;
		}

		page = livebox_panel_add_page(page_node, livebox_panel_get());
		if (!page) {
			LOGE("page == NULL");
			livebox_panel_destroy();
			data_model_del_item(page_node);
			return;
		}

		page_index = page_scroller_get_page_count(livebox_panel_get()) - 1;
	} else {
		page_node = evas_object_data_get(page, KEY_ICON_DATA);
		if (!page_node) {
			LOGE("obj_node == NULL");
			livebox_panel_destroy();
			return;
		}
	}

	item_node = data_model_add_widget(page_node, content_info, pos_x, pos_y, widget_width, widget_height, NULL);
	if (!item_node) {
		LOGE("item == NULL");
		return;
	}

	elm_scroller_page_bring_in(livebox_panel_get(), page_index, 0);
	livebox_panel_add_livebox(item_node, page, content_info, pos_x, pos_y, widget_width, widget_height, NULL);
	livebox_panel_update_dynamic_index();
}

static void _cannot_add_widget_popup_show(void)
{
	popup_data_t *data = popup_create_data();
	if (!data) {
		LOGE("data == NULL");
		return;
	}

	data->orientation        = ELM_POPUP_ORIENT_CENTER;
	data->popup_cancel_text  = NULL;
	data->popup_confirm_text = NULL;
	data->popup_content_text = strdup(_("IDS_HS_HEADER_UNABLE_TO_ADD_WIDGET_ABB"));
	data->popup_title_text   = NULL;
	data->type               = POPUP_INFO;
	data->visible_time       = 5.0;

	popup_show(data, NULL, NULL, NULL);
}

static int request_cb(const char *appid, const char *name, int type,
	const char *content_info, const char *icon, int pid, double period,
	int allow_duplicate, void *data)
{
	int widget_width;
	int widget_height;

	LOGI("====== WIDGET REQUEST ======");
	LOGI("Package name: %s\n",    appid);
	LOGI("Name: %s\n",            name);
	LOGI("Type: %d\n",            type);
	LOGI("Content: %s\n",         content_info);
	LOGI("Icon: %s\n",            icon);
	LOGI("Requested from: %d\n",  pid);
	LOGI("Period: %lf\n",         period);
	LOGI("Allow duplicate: %d\n", allow_duplicate);
	LOGI("CBDATA: %p\n",          data);
	LOGI("============================");

	switch (type) {
	case WIDGET_SIZE_4x2:
		widget_width =  4;
		widget_height = 2;
		break;
	case WIDGET_SIZE_4x4:
		widget_width =  4;
		widget_height = 4;
		break;
	default:
		/* Unsupported */
		LOGE("Unsupported type: %x\n", type);
		_cannot_add_widget_popup_show();
		return 0;
	}

	LOGI("Widget will be added: %dx%d\n", widget_width, widget_height);

	livebox_widget_new(widget_width, widget_height, content_info);
	home_screen_print_tree();

	return 0; /*returns success. */
}

static void update_content_info(Evas_Object *widget, const char *content_info)
{
	Evas_Object *parent = NULL;
	Tree_node_t *node = NULL;

	if (!widget) {
		LOGE("widget == NULL");
		return;
	}

	parent = elm_object_parent_widget_get(widget);
	if (!parent) {
		LOGE("parent == NULL");
		return;
	}

	node = evas_object_data_get(parent, KEY_ICON_DATA);
	if (!node) {
		LOGE("node == NULL");
		return;
	}

	data_model_update_content_info(node, content_info);
}
