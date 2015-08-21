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

#include "homescreen-efl.h"
#include "app_item.h"
#include "app_mgr.h"

static struct
{
	int unique_id;
} s_app_item_info = {
	.unique_id = -1
};

HAPI app_item_t *app_item_create(const app_item_type_t type, const int unique_id,
				const char *label, const char *icon, const char *exec,
				const char *appid, void *data, bool is_removeable,
				int col, int row, int col_span, int row_span,
				const char *content_info)
{
	app_item_t *item = (app_item_t *)calloc(1, sizeof(app_item_t));

	if (!item) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[ALLOC_FAILED]");
		return NULL;
	}

	item->type = type;
	if (unique_id < 0) {
		item->unique_id = ++s_app_item_info.unique_id;
	} else {
		item->unique_id = unique_id;
		s_app_item_info.unique_id = s_app_item_info.unique_id > unique_id ? s_app_item_info.unique_id : unique_id;
	}

	if (label)
		item->label = strdup(label);
	if (icon)
		item->icon = strdup(icon);
	if (exec)
		item->exec = strdup(exec);
	if (appid)
		item->appid = strdup(appid);
	if (content_info)
		item->content_info = strdup(content_info);

	item->badge_count = 0;
	item->data = data;
	item->removable = is_removeable;
	item->is_checked = false;
	item->layout = NULL;
	item->grid_item = NULL;

	item->col = col;
	item->row = row;
	item->col_span = col_span;
	item->row_span = row_span;


	return item;
}

HAPI void app_item_free(app_item_t *item)
{
	if (!item)
		return;

	free((void *)item->label);
	free((void *)item->icon);
	free((void *)item->exec);
	free((void *)item->appid);
	free((void *)item->content_info);

	free(item);
}

HAPI void app_item_geometry_update(app_item_t *app_item, int x, int y, int w, int h)
{
	if (!app_item) {
		LOGE("app_item == NULL");
		return;
	}

	app_item->col = x;
	app_item->row = y;
	app_item->col_span = w;
	app_item->row_span = h;
}

HAPI void app_item_update_content_info(app_item_t *app_item, const char *content_info)
{
	if (!app_item) {
		LOGE("app_item == NULL");
		return;
	}

	if (app_item->content_info)
		free(app_item->content_info);

	if (content_info) {
		app_item->content_info = strdup(content_info);
	} else {
		app_item->content_info = NULL;
	}
}


