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

#ifndef PAGE_SCROLLER_H_
#define PAGE_SCROLLER_H_


#include <stdbool.h>
#include <Evas.h>
#include <Eina.h>

/**
 * @brief Creates new elm_scroller widget and sets standart parameters used in
 * homescreen application.
 * @param page_width page width.
 * @param page_height page height.
 *
 * @return Pointer to created elm_scroller widget/
 */
extern Evas_Object *page_scroller_create(int page_width, int page_height);

/**
 * @brief Removes all data connected with the page scroller object.
 *
 * @param page_scroller Page scroller pointer.
 */
extern void page_scroller_destroy(Evas_Object *page_scroller);

/**
 * @brief Add new page to the page scroller
 *
 * @param page_scroller Page scroller Evas_Object pointer.
 * @param page Page Evas_Object pointer.
 *
 * @return true if no errors.
 */
extern bool page_scroller_add_page(Evas_Object *page_scroller,
	Evas_Object *page);

/**
 * @brief Removes the page from the page scroller.
 * @param page_scroller Page scroller object pointer.
 * @param page Page which will be removed.
 */
extern void page_scroller_remove_page(Evas_Object *page_scroller,
	Evas_Object *page);

/**
 * @brief Gets the page from the given page scroller.
 * @param page_scroller Pointer to elm_scroller widget.
 * @param n page number.
 *
 * @return Page Evas_Object pointer.
 */
extern Evas_Object *page_scroller_get_page(Evas_Object *page_scroller, int n);

/**
 * @brief Gets all pages inserted in the page scroller list.
 * @param page_scroller Pointer to page scroller Evas_Object
 * @return List of all pages.
 */
extern Eina_List *page_scroller_get_all_pages(Evas_Object *page_scroller);

/**
 * @brief Blocks page scroller movement.
 *
 * @param page_scroller Pointer to page scroller widget.
 * @return true if no errors.
 */
extern bool page_scroller_freeze(Evas_Object *page_scroller);

/**
 * @brief Unblocks page scroller movement.
 *
 * @param page_scroller Pointer to page scroller Evas_Object.
 * @return true if no errors.
 */
extern bool page_scroller_unfreeze(Evas_Object *page_scroller);

/**
 * @brief Returns elm_scroller pointer from page scroller layout.
 *
 * @param page_scroller Page scroller Evas_Object pointer.
 * @return elm_scroller pointer or NULL if errors occurred.
 */
extern Evas_Object *page_scroller_get_scroller(Evas_Object *page_scroller);

/**
 * @brief Unpack all pages from the page scroller widget.
 * @param page_scroller Page scroller Evas_Object.
 * @return List of all pages (Evas_Objects)
 */
extern Eina_List *page_scroller_unpack_pages(Evas_Object *page_scroller);

/**
 * @brief Pack all pages into the page scroller widget.
 *
 * @param page_scroller Pointer to page scroller widget.
 * @param page_list List of pages to pack.
 */
extern void page_scroller_pack_pages(Evas_Object *page_scroller,
	Eina_List *page_list);

/**
 * @brief Gets page thumbnail (proxy object)
 *
 * @param page Pointer to page scroller page.
 * @param int width of miniature
 * @param int height of miniature
 * @return Page copy (miniature) or NULL if errors occurred.
 */
extern Evas_Object *page_scroller_get_page_thumbnail(Evas_Object *page,
	unsigned int w, unsigned int h);

/**
 * @brief Gets the elm_box pointer from the Page_Scroller Evas_Object.
 *
 * @param page_scroller Pointer to Page Scroller Evas_Object
 * @return elm_box pointer placed in  the Page scroller widget.
 */
extern Evas_Object *page_scroller_obtain_box(Evas_Object *page_scroller);

/**
 * @brief Get total number of pages
 * @param page_scroller Elm_scroller pointer
 * @return pages count if ok, -1 if error
 */

extern int page_scroller_get_page_count(Evas_Object *page_scroller);
/**
 * @brief Get current page
 * @return current page number or -1 if something went wrong
 */
extern int page_scroller_get_current_page(Evas_Object *page_scroller);

/**
 * @brief Set current page of the scroller
 * @param page_scroller Pointer to elm_scroller
 * @param idx index of page
 * @return true if success
 */
extern bool page_scroller_set_current_page(Evas_Object *page_scroller, int idx);

/**
 * @brief Shows page without animation
 *
 * @param page_scroller elm_scroller_pointer
 * @param idx page index
 */
void page_scroller_show_page(Evas_Object *page_scroller, int idx);
#endif /* PAGE_SCROLLER_H_ */
