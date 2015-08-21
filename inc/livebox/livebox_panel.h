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

#ifndef __LIVEBOX_PANEL_H__
#define __LIVEBOX_PANEL_H__

#include <Elementary.h>
#include "tree.h"

/**
 * @brief creates main livebox view
 * @details Creates the livebox scroller and adds pages with liveboxes to it
 * @param parent Layout where the livebox panel will be added
 * @return Created livebox view or NULL on error
 */
extern Evas_Object *livebox_panel_create(Evas_Object *parent);

/**
 * @brief [brief description]
 * @details [long description]
 */
extern void livebox_panel_fill(void);

/**
 *
 */
void livebox_panel_update_dynamic_index(void);


/**
 * @brief Creates dynamic index for livebox panel
 */
extern void livebox_panel_create_dynamic_index(void);

/**
 * @brief Sets livebox panel and its dynamic index to layout
 * @note @p layout has to have "content" and "index" parts
 * @param layout layout to add widgets to
 */
extern void livebox_panel_set_content(Evas_Object *layout);

/**
 * @brief Unsets livebox panel and its dynamic index from layout
 * @note @p layout has to have "content" and "index" parts
 * @param layout layout to remove widgets from
 */
extern void livebox_panel_unset_content(Evas_Object *layout);

/**
 * @brief Create empty livebox scroller
 * @return The created scroller or null on error
 */
extern Evas_Object *livebox_panel_create_scroller(void);

/**
 * @brief destroys the livebox scroller and all of it pages
 */
extern void livebox_panel_destroy(void);

/**
 * Adds new livebox to @p page
 * @param node widget representation in data structure
 * @param page Page where livebox will be added
 * @param livebox_pkgname livebo pkgname
 * @param pos_x position in grid
 * @param pos_y position in grid
 * @param width width of livebox in grid
 * @param height height of livebox in grid
 */
void livebox_panel_add_livebox(Tree_node_t *node, Evas_Object *page,
	const char *livebox_pkgname, int pos_x, int pos_y,
	int width, int height,
	char *content_info);

/**
 * @brief Creates new livebox page
 * @param node page representation in data structure
 * @param livebox_scroller
 * @return new empty page
 */
extern Evas_Object *livebox_panel_add_page(Tree_node_t *node,
	Evas_Object *livebox_scroller);

/**
 * @brief adds an empty page (page layout is described in @p
 * livebox_container.edc to the @p livebox_scroller
 * @param livebox_scroller Scroller to which thepage is added
 * @return TRUE on success FALSE otherwise
 */
extern bool livebox_panel_add_empty_page(Evas_Object *livebox_scroller);

/**
 * @brief Creates an empty page but doesn't add it to the scoller
 * @param node page node in the data model
 * @param livebox_scroller Scroller to which the page can be appended
 * @return
 */
extern Evas_Object *livebox_panel_create_empty_page(Tree_node_t *node,
	Evas_Object *livebox_scroller);

/**
 * @brief Changes the view mode of the livebox page
 * @param turn_on True - edit mode on False - edit mode off
 */
extern void livebox_panel_set_edit_mode_layout(bool turn_on);

/**
 * @brief Changes the view mode of the livebox scroller
 * @param close True - edit mode off False - edit mode on
 */
extern void livebox_panel_change_edit_mode_state(bool close);

/**
 * @brief Destroys given livebox page and all its content
 * @param livebox_scroller_page page to destroy
 */
extern void livebox_panel_destroy_page(Evas_Object *livebox_scroller_page);

/**
 * @brief Function sets the reposition data
 * @param x mouse position x
 * @param y mouse position y
 */
extern void livebox_panel_move_mouse_cb(int x, int y);

/**
 *  @brief sets the pages geometry based on scroller geometry that consits them
 */
extern void livebox_panel_refresh_content_size(void);

/**
 * @brief Returns list of all pages in livebox panel
 * @return Eina_List pointer of all livebox pages
 */
extern Eina_List *livebox_panel_get_pages(void);

/**
 * @brief Gets the livebox dynamic index
 * @return Pointer to dynamic index box
 */
extern Evas_Object *livebox_panel_get_index(void);

/**
 * @brief Gets the livebox panel elm_scroller widget
 * @return Pointer to elm_scroller with livebox pages
 */
extern Evas_Object *livebox_panel_get(void);

/**
 * @brief Function return true if add page is visible.
 */
extern bool livebox_panel_check_if_add_page_is_visible(void);

/**
 * @brief hide and shows add page button
 * @param visible true - show false -hide
 */
void livebox_panel_add_page_visible_state_set(bool visible);

/**
 * @brief Returns true if 'add_page' is the curently selected livebox page
 */
extern bool livebox_panel_is_add_page_selected(void);

/**
 * Gets the current livebox page
 * @return Current scroller page
 */
extern Evas_Object *livebox_scroller_get_page(void);

/**
 *  Returns the last page of the livebox_scroller
 * @return last page or NULL on error
 */
Evas_Object *livebox_scroller_get_last_page(void);

/**
 * Check if a new widget page can be added. Shows a popup if not
 * @return true if page can be added, false if not
 */
extern bool livebox_panel_can_add_page(void);

/**
 * Looks for empty space to put new widget to
 *
 * @param span_w widget size
 * @param span_h widget height
 * @param pos_x position to put new widget
 * @param pos_y position to put new widget
 * @return Index of page where new widget can be placed
 */
extern int livebox_panel_find_empty_space(int span_w, int span_h,
	int *pos_x, int *pos_y);

/**
 */
extern void livebox_panel_resumed(void);

/**
 */
extern void livebox_panel_paused(void);

#endif
