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
#include "edc_conf.h"
#include "util.h"


void __cluster_page_set(cluster_page_t *page, widget_data_t *item);

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
//    elm_box_unpack_all(elm_layout_content_get(page->page_layout, CLUSTER_BOX));
    evas_object_del(page->page_layout);
    free(page);
}

bool cluster_page_set(cluster_page_t *page, widget_data_t *item)
{
    if(cluster_page_check_empty_space(page, item->type, item->pos_x, item->pos_y))
    {
        __cluster_page_set(page, item);
        return true;
    }
    return false;
}

bool cluster_page_unset(cluster_page_t *page, widget_data_t *item)
{
    int w, h;
    int i, j;
    switch(item->type) {
    case WIDGET_SIZE_TYPE_4x2:
        w = 4;
        h = 2;
        break;
    case WIDGET_SIZE_TYPE_4x4:
        w = 4;
        h = 4;
        break;
    default:
        LOGE("NOT support type : %d", item->type);
        return false;
    }

    for(i = 0; i < h; i++) {
        for(j = 0; j < w; j++) {
            page->space[item->pos_y + i][item->pos_x + j] = 0;
        }
    }
    elm_grid_unpack(page->grid, item->widget_layout);
    return true;
}

bool cluster_page_check_empty_space(cluster_page_t *page, widget_size_type_e type, int pos_x, int pos_y)
{
    int w, h;
    int from_x, to_x, from_y, to_y;
    int x, y, i, j;
    switch(type) {
    case WIDGET_SIZE_TYPE_4x2:
        w = 4;
        h = 2;
        break;
    case WIDGET_SIZE_TYPE_4x4:
        w = 4;
        h = 4;
        break;
    default:
        LOGE("NOT support type : %d", type);
        return false;
    }

    if (pos_x < 0 || pos_y < 0) {
        from_x = from_y = 0;
        to_x = to_y = 3;
    } else {
        from_x = to_x = pos_x;
        from_y = to_y = pos_y;
    }

    for(y = from_y; y <= to_y; y++) {
        for(x = from_x; x <= to_x; x++) {
            bool find = true;
            for(i = 0; i < h; i++) {
                for(j = 0; j < w; j++) {
                    if (y + i < 4 && x + j < 4 && page->space[y + i][x + j] == 1) {
                        find = false;
                        break;
                    }
                }
            }
            if(find)
                return true;
        }
    }

    return false;
}

void __cluster_page_set(cluster_page_t *page, widget_data_t *item)
{
    int w, h;
    int i, j;
    switch(item->type) {
    case WIDGET_SIZE_TYPE_4x2:
        w = 4;
        h = 2;
        break;
    case WIDGET_SIZE_TYPE_4x4:
        w = 4;
        h = 4;
        break;
    default:
        LOGE("NOT support type : %d", item->type);
        return ;
    }

    for(i = 0; i < h; i++) {
        for(j = 0; j < w; j++) {
            page->space[item->pos_y + i][item->pos_x + j] = 1;
        }
    }
    elm_grid_pack(page->grid, item->widget_layout, item->pos_x, item->pos_y, w, h);
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
