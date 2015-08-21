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

#include <stdbool.h>
#include <Elementary.h>

#include "util.h"
#include "homescreen-efl.h"
#include "page_scroller.h"

#define PAGE_SCROLLER_WIDTH 272
#define PAGE_SCROLLER_HEIGHT 272

#define EDJE_PAGE_SCROLLER_FILENAME "/usr/apps/com.samsung.home-lite/res/edje/page_scroller.edj"
#define EDJE_PAGE_SCROLLER_LIVEBOX_GROUP_NAME "page_scroller_livebox"
#define EDJE_PAGE_SCROLLER_ALL_APPS_GROUP_NAME "page_scroller_all_apps"
#define EDJE_PAGE_SCROLLER_CONTENT_GROUP_NAME "page_scroller_content"


static Evas_Object *__page_scroller_create_scroller(void);
static void __page_scroller_change_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static Eina_Bool __page_scroller_blocker_cb(void *data);

static struct {
	Ecore_Timer *blocker;
} s_info = {
	.blocker = NULL,
};


HAPI Evas_Object *page_scroller_create(int page_width, int page_height)
{
	Evas_Object *scroller = NULL;

	scroller = __page_scroller_create_scroller();
	if (!scroller) {
		LOGE("Failed to create scroller");
		return NULL;
	}

	elm_scroller_page_size_set(scroller, page_width, page_height);
	elm_scroller_loop_set(scroller, EINA_TRUE, EINA_FALSE);
	elm_object_signal_callback_add(scroller, "*", SIGNAL_SOURCE, __page_scroller_change_cb, NULL);

	return scroller;
}


HAPI void page_scroller_destroy(Evas_Object *scroller)
{
	LOGD("Destroy page scroller");

	if (!scroller) {
		LOGE("Invalid param : scroller is NULL");
		return;
	}

	evas_object_del(scroller);
}

HAPI bool page_scroller_add_page(Evas_Object *scroller, Evas_Object *page)
{
	LOGD("Add page");

	Evas_Object *box = NULL;

	if (!page || !scroller) {
		LOGE("Invalid parameters : page or scroller is NULL");
		return false;
	}

	box = elm_object_content_get(scroller);
	if (!box) {
		LOGE("Failed to obtain box from scroller");
		return false;
	}

	elm_box_pack_end(box, page);

	return true;
}

HAPI void page_scroller_remove_page(Evas_Object *scroller, Evas_Object *page)
{
	LOGD("Remove page");
	Evas_Object *box = NULL;

	if (!page || !scroller) {
		LOGE("Page or scroller is NULL");
		return;
	}

	box = elm_object_content_get(scroller);
	if (!box) {
		LOGE("Failed to get scroller");
		return;
	}

	elm_box_unpack(box, page);
	evas_object_del(page);
	return;
}

HAPI Eina_List* page_scroller_get_all_pages(Evas_Object *scroller)
{
	Evas_Object *box = NULL;
	Eina_List *pages = NULL;

	if (!scroller) {
		LOGE("Invalid parameter : scroller is NULL");
		return NULL;
	}

	box = elm_object_content_get(scroller);
	if (!box) {
		LOGE("Failed to get box");
		return NULL;
	}

	pages = elm_box_children_get(box);
	if (!pages) {
		LOGE("Failed to get pages list");
		return NULL;
	}

	return pages;
}


HAPI Evas_Object* page_scroller_get_page(Evas_Object *scroller, int n)
{
	Eina_List *pages = NULL;
	Evas_Object *page = NULL;

	pages = page_scroller_get_all_pages(scroller);
	if (!pages) {
		LOGE("Failed to get page");
		return NULL;
	}

	page = (Evas_Object*) eina_list_nth(pages, n);

	eina_list_free(pages);

	return page;
}


HAPI bool page_scroller_freeze(Evas_Object *scroller)
{
	LOGD("Freeze page scroller");

	if (!scroller) {
		LOGE("Page Scroller is NULL");
		return false;
	}

	elm_scroller_movement_block_set(scroller, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);

	return true;
}

HAPI bool page_scroller_unfreeze(Evas_Object *scroller)
{
	LOGD("Unfreeze page scroller");

	if (!scroller) {
		LOGE("Page Scroller is NULL");
		return false;
	}

	elm_scroller_movement_block_set(scroller, ELM_SCROLLER_MOVEMENT_NO_BLOCK);

	return true;
}

HAPI int page_scroller_get_page_count(Evas_Object *scroller)
{
	Eina_List *page_list = NULL;
	int page_count = 0;

	if (!scroller) {
		LOGE("invalid param : scroller is NULL");
		return -1;
	}

	page_list = page_scroller_get_all_pages(scroller);
	page_count = eina_list_count(page_list);

	eina_list_free(page_list);

	return page_count;
}


HAPI Eina_List* page_scroller_unpack_pages(Evas_Object *scroller)
{
	Evas_Object *box = NULL;
	Eina_List *page_list = NULL;

	if (!scroller) {
		LOGE("Page scroller is NULL");
		return NULL;
	}

	page_list = page_scroller_get_all_pages(scroller);

	if (!page_list) {
		LOGE("Failed to get m_page list");
		return NULL;
	}

	box = elm_object_content_get(scroller);
	elm_box_unpack_all(box); ///@todo better content unset?

	return page_list;
}


HAPI void page_scroller_pack_pages(Evas_Object *scroller, Eina_List *page_list)
{
	LOGD("");
	Eina_List *item = NULL;
	Evas_Object *page = NULL;
	Evas_Object *box = NULL;

	if (!scroller || !page_list) {
		LOGE("Invalid parameters : scroller [%p] page_list [%p]", scroller, page_list);
		return;
	}

	box = elm_object_content_get(scroller);
	if (!box) {
		LOGE("Cannot obtain box, pages will remain unpacked");
		return;
	}

	EINA_LIST_FOREACH(page_list, item, page) {
		if (!page) {
			LOGE("Failed to get page");
			continue;
		}
		elm_box_pack_end(box, page);
		evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(page, EVAS_HINT_FILL, EVAS_HINT_FILL);
	}
	elm_box_recalculate(box);
}


HAPI Evas_Object* page_scroller_get_page_thumbnail(Evas_Object *page, unsigned int w, unsigned int h)
{
	LOGD("");
	Evas_Object *image = NULL;

	if (!page) {
		LOGE("Invalid page number");
		return NULL;
	}

	image = evas_object_image_filled_add(evas_object_evas_get(home_screen_get_win()));
	if (!image) {
		LOGE("Could not create image proxy object");
		return NULL;
	}

	/*set image proxy*/
	if (!evas_object_image_source_set(image, page)) {
		evas_object_del(image);
		LOGE("Could not set proxy image source");
		return NULL;
	}

	evas_object_image_fill_set(image, 0, 0, w, h);
	evas_object_move(image, 0, 0);
	evas_object_show(image);

	return image;
}

HAPI int page_scroller_get_current_page(Evas_Object *page_scroller)
{
	int h_page = -1;

	if (!page_scroller) {
		LOGE("Invalid param : page_scroller is NULL");
		return -1;
	}

	elm_scroller_current_page_get(page_scroller, &h_page, NULL);

	return h_page;
}

HAPI bool page_scroller_set_current_page(Evas_Object *page_scroller, int idx)
{
	if (!page_scroller || idx < 0) {
		LOGE("[INVALID_PARAM][page_scroller='%p'][idx='%d']", page_scroller, idx);
		return false;
	}


	elm_scroller_page_bring_in(page_scroller, idx, 0);
	return true;
}

HAPI void page_scroller_show_page(Evas_Object *page_scroller, int idx)
{
	if (!page_scroller || idx < 0) {
		LOGE("[INVALID_PARAM][page_scroller='%p'][idx='%d']", page_scroller, idx);
		return;
	}

	elm_scroller_page_show(page_scroller, idx, 0);
}

static Evas_Object* __page_scroller_create_scroller(void)
{
	Evas_Object *win = home_screen_get_win();
	Evas_Object *scroller = NULL;
	Evas_Object *box = NULL;

	if (!win) {
		print_log(DLOG_ERROR, LOG_TAG, "Window doesn't exist");
		return NULL;
	}

	scroller = elm_scroller_add(win);
	if (!scroller) {
		LOGE("Cannot create scroller");
		return NULL;
	}

	elm_scroller_content_min_limit(scroller, EINA_FALSE, EINA_FALSE);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 1);

	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	box = elm_box_add(scroller);
	if (!box) {
		LOGE("Cannot create box");
		evas_object_del(scroller);
		return NULL;
	}
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_box_align_set(box, 0.5, 0.5);
	evas_object_show(scroller);
	elm_object_content_set(scroller, box);

	return scroller;
}

static void __page_scroller_change_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	int cnt = -1, curr = -1, dest = -1;

	cnt = page_scroller_get_page_count(obj);
	curr = page_scroller_get_current_page(obj);

	if (s_info.blocker)
		return;

	if (!strncmp(emission, SIGNAL_NEXT_PAGE_CHANGE, strlen(emission))) {
		if (curr == cnt - 1) {
			dest = 0;
		} else {
			dest = ++curr;
		}
		page_scroller_set_current_page(obj, dest);
	} else if (!strncmp(emission, SIGNAL_PREV_PAGE_CHANGE, strlen(emission))) {
		if (curr == 0) {
			dest = cnt - 1;
		} else {
			dest = --curr;
		}
		page_scroller_set_current_page(obj, dest);
	}

	s_info.blocker = ecore_timer_add(0.5, __page_scroller_blocker_cb, NULL);
}

static Eina_Bool __page_scroller_blocker_cb(void *data)
{
	s_info.blocker = NULL;
	return ECORE_CALLBACK_CANCEL;
}
