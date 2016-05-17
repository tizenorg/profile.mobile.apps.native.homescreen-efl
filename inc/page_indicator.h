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

#ifndef __PAGE_INDICATOR_H__
#define __PAGE_INDICATOR_H__

#include <app.h>
#include <Elementary.h>

#define PAGE_INDICATOR_MAX_PAGE_COUNT 7

typedef struct {
    Evas_Object *scroller;
    Evas_Object *box;
    Evas_Object *unit[PAGE_INDICATOR_MAX_PAGE_COUNT];

    int page_count;
    int x;
    int y;
    int w;
    int h;
    int current_page;
} page_indicator_t;

page_indicator_t * page_indictor_create(Evas_Object *scroller);
void page_indicator_set_page_count(page_indicator_t *page_indicator, int count);
void page_indicator_scroller_resize(page_indicator_t *page_indicator, int width, int height);
void page_indicator_set_current_page(page_indicator_t *page_indicator, int page_number);
void page_indicator_show(page_indicator_t *page_indicator);
void page_indicator_hide(page_indicator_t *page_indicator);

#endif /* __PAGE_INDICATOR_H__ */
