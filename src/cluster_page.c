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

#include "cluster_page.h"
#include "conf.h"
#include "edc_conf.h"
#include "util.h"

void __cluster_page_set(cluster_page_t *page, widget_data_t *item);
void __cluster_page_print_space(cluster_page_t *page);
void __cluster_page_get_widget_size(widget_size_type_e type, int *w, int *h);

cluster_page_t *cluster_page_new(Evas_Object* parent)
{
	cluster_page_t *page_t = (cluster_page_t *)malloc(sizeof(cluster_page_t));
	memset(page_t, 0, sizeof(cluster_page_t));

	page_t->page_layout = elm_layout_add(parent);
	elm_layout_file_set(page_t->page_layout, util_get_res_file_path(EDJE_DIR"/cluster_page.edj"), GROUP_CLUSTER_PAGE_LY);
	evas_object_size_hint_align_set(page_t->page_layout, 0, 0);
	evas_object_size_hint_weight_set(page_t->page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(page_t->page_layout, CLUSTER_W, CLUSTER_H);
	evas_object_show(page_t->page_layout);

	Evas_Object *page_bg = evas_object_rectangle_add(evas_object_evas_get(page_t->page_layout));
	evas_object_size_hint_min_set(page_bg, CLUSTER_W, CLUSTER_H);
	evas_object_size_hint_align_set(page_bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(page_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(page_bg, 0, 0, 0, 0);
	evas_object_show(page_bg);
	elm_object_part_content_set(page_t->page_layout, SIZE_SETTER, page_bg);

	Evas_Object *grid = elm_grid_add(page_t->page_layout);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_grid_size_set(grid, CLUSTER_COL, CLUSTER_ROW);
	evas_object_show(grid);
	elm_layout_content_set(page_t->page_layout, CLUSTER_BOX, grid);
	page_t->grid = grid;
	return page_t;
}

void cluster_page_delete(cluster_page_t *page)
{
	evas_object_del(page->page_layout);
	eina_list_free(page->widget_list);
	free(page);
}

bool cluster_page_set_widget(cluster_page_t *page, widget_data_t *item)
{
	if (item == NULL) {
		LOGE("widget data is NULL");
		return false;
	}

	int x, y;
	if (cluster_page_check_empty_space(page, item->type, item->pos_x, item->pos_y, &x, &y)) {
		item->page_idx = page->page_index;
		item->pos_x = x;
		item->pos_y = y;
		__cluster_page_set(page, item);
		return true;
	}
	return false;
}

bool cluster_page_unset(cluster_page_t *page, widget_data_t *item)
{
	int w, h;
	int i, j;
	__cluster_page_get_widget_size(item->type, &w, &h);

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			page->space[item->pos_y + i][item->pos_x + j] = 0;
		}
	}

	elm_grid_unpack(page->grid, item->widget_layout);
	page->widget_list = eina_list_remove(page->widget_list, item);
	return true;
}

bool cluster_page_check_empty_space(cluster_page_t *page, widget_size_type_e type,
		int pos_x, int pos_y, int *empty_x, int *empty_y)
{
	LOGD("%d %d", pos_x, pos_y);
	int w, h;
	int from_x, to_x, from_y, to_y;
	int x, y, i, j;
	__cluster_page_get_widget_size(type, &w, &h);

	if (pos_x == INIT_VALUE || pos_y == INIT_VALUE) {
		from_x = from_y = 0;
		to_x = to_y = 3;
	} else if (pos_x < 0 || pos_y < 0 || pos_x >= CLUSTER_COL || pos_y >= CLUSTER_ROW) {
		return false;
	} else {
		from_x = to_x = pos_x;
		from_y = to_y = pos_y;
	}
	LOGD("from %d %d", from_x, from_y);

	for (y = from_y; y <= to_y; y++) {
		for (x = from_x; x <= to_x; x++) {
			bool find = true;
			for (i = 0; i < h; i++) {
				for (j = 0; j < w; j++) {
					if (y + i >= 4 || x + j >= 4 || page->space[y + i][x + j] == 1) {
						find = false;
						break;
					}
					if (!find)
						break;
				}
			}
			if (find) {
				if (empty_x)
					*empty_x = x;
				if (empty_y)
					*empty_y = y;
				LOGD("empty");
				return true;
			}
		}
	}

	if (empty_x)
		*empty_x = INIT_VALUE;
	if (empty_y)
		*empty_y = INIT_VALUE;
	LOGD("NOT empty");
	return false;
}

void cluster_page_check_empty_space_pos(cluster_page_t *page, widget_data_t *item, int *out_x, int *out_y)
{
	int x, y;
	*out_x = INIT_VALUE;
	*out_y = INIT_VALUE;

	int gx, gy, w, h;
	evas_object_geometry_get(page->grid, &gx, &gy, &w, &h);
	if (cluster_page_check_empty_space(page, item->type, item->pos_x, item->pos_y, &x, &y)) {
		*out_x = gx + x * (w / CLUSTER_COL);
		*out_y = gy + y * (h / CLUSTER_ROW);
	} else if (cluster_page_check_empty_space(page, item->type, INIT_VALUE, INIT_VALUE, &x, &y)) {
		*out_x = gx + x * (w / CLUSTER_COL);
		*out_y = gy + y * (h / CLUSTER_ROW);
	} else {
		*out_x = gx + item->pos_x * (w / CLUSTER_COL);
		*out_y = gy + item->pos_y * (h / CLUSTER_ROW);
	}
}

void cluster_page_get_highlight_xy(cluster_page_t *page, int *x, int *y)
{
	int gx, gy, w, h;
	evas_object_geometry_get(page->grid, &gx, &gy, &w, &h);
	if (page->highlight_pos_x == INIT_VALUE || page->highlight_pos_y == INIT_VALUE) {
		*x = INIT_VALUE;
		*y = INIT_VALUE;
	} else {
		*x = gx + page->highlight_pos_x * (w / CLUSTER_COL);
		*y = gy + page->highlight_pos_y * (h / CLUSTER_ROW);
	}
}

void __cluster_page_set(cluster_page_t *page, widget_data_t *item)
{
	int w, h;
	int i, j;
	__cluster_page_get_widget_size(item->type, &w, &h);

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			page->space[item->pos_y + i][item->pos_x + j] = 1;
		}
	}

	elm_grid_pack(page->grid, item->widget_layout, item->pos_x, item->pos_y, w, h);
	cluster_data_update(item);
	page->widget_list = eina_list_append(page->widget_list, item);
}

Evas_Object *cluster_page_thumbnail(cluster_page_t *page_t)
{
	Evas_Object *image = NULL;
	if (!page_t || !page_t->page_layout) {
		LOGE("Invalid page");
		return NULL;
	}

	image = evas_object_image_filled_add(evas_object_evas_get(page_t->page_layout));
	if (!image) {
		LOGE("Could not create image proxy object");
		return NULL;
	}

	if (!evas_object_image_source_set(image, page_t->page_layout)) {
		evas_object_del(image);
		LOGE("Could not set proxy image source");
		return NULL;
	}

	evas_object_image_fill_set(image, 0, 0, CLUSTER_ALL_PAGE_W, CLUSTER_ALL_PAGE_H);
	evas_object_show(image);

	return image;
}
void cluster_page_drag_cancel(cluster_page_t *page)
{
	if (page->highlight) {
		evas_object_del(page->highlight);
		page->highlight = NULL;
		page->highlight_pos_x = INIT_VALUE;
		page->highlight_pos_y = INIT_VALUE;
	}
}

void cluster_page_drag_widget(cluster_page_t *page, widget_size_type_e type, int pos_x, int pos_y)
{
	if (!page->highlight) {
		int w = 0, h = 0;
		LOGD("Create Highlight");
		page->highlight_pos_x = INIT_VALUE;
		page->highlight_pos_y = INIT_VALUE;
		page->highlight = evas_object_rectangle_add(evas_object_evas_get(page->page_layout));
		edje_object_part_geometry_get(elm_layout_edje_get(page->page_layout), CLUSTER_BG, NULL, NULL, &w, &h);
		evas_object_size_hint_min_set(page->highlight, w /CLUSTER_COL, h / CLUSTER_ROW);
		evas_object_size_hint_align_set(page->highlight, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(page->highlight, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_color_set(page->highlight, 255 , 255, 255, 100);
	}

	if (cluster_page_check_empty_space(page, type, pos_x, pos_y, NULL, NULL)) {
		if (pos_x != page->highlight_pos_x || pos_y != page->highlight_pos_y) {
			int w, h;
			page->highlight_pos_x = pos_x;
			page->highlight_pos_y = pos_y;
			elm_grid_unpack(page->grid, page->highlight);
			__cluster_page_get_widget_size(type, &w, &h);
			elm_grid_pack(page->grid, page->highlight, pos_x, pos_y, w, h);
			evas_object_show(page->highlight);
		}
	} else {
		page->highlight_pos_x = INIT_VALUE;
		page->highlight_pos_y = INIT_VALUE;
		elm_grid_unpack(page->grid, page->highlight);
		evas_object_hide(page->highlight);
	}
}

bool cluster_page_drop_widget(cluster_page_t *page, widget_data_t *widget)
{
	if (!page->highlight || page->highlight_pos_x == INIT_VALUE || page->highlight_pos_y == INIT_VALUE) {
		int x, y;
		if (cluster_page_check_empty_space(page, widget->type, widget->pos_x, widget->pos_y, &x, &y) ||
				cluster_page_check_empty_space(page, widget->type, INIT_VALUE, INIT_VALUE, &x, &y)) {
			widget->page_idx = page->page_index;
			widget->pos_x = x;
			widget->pos_y = y;
			__cluster_page_set(page, widget);
			return true;
		}
		return false;
	}

	elm_grid_unpack(page->grid, page->highlight);
	evas_object_del(page->highlight);
	page->highlight = NULL;

	widget->page_idx = page->page_index;
	widget->pos_x = page->highlight_pos_x;
	widget->pos_y = page->highlight_pos_y;
	__cluster_page_set(page, widget);

	page->highlight_pos_x = INIT_VALUE;
	page->highlight_pos_y = INIT_VALUE;
	return true;
}

void __cluster_page_print_space(cluster_page_t *page)
{
	LOGD("=========================================");
	int i;
	for (i = 0; i < 4; i++) {
		LOGD("%d %d %d %d", page->space[i][0], page->space[i][1], page->space[i][2], page->space[i][3]);
	}
	LOGD("=========================================");
}

void __cluster_page_get_widget_size(widget_size_type_e type, int *w, int *h)
{
	switch (type) {
	case WIDGET_SIZE_TYPE_4x2:
		*w = 4;
		*h = 2;
		break;
	case WIDGET_SIZE_TYPE_4x4:
		*w = 4;
		*h = 4;
		break;
	default:
		*w = *h = 0;
		LOGE("NOT support type : %d", type);
		break;
	}
}
