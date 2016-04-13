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

#ifndef CONF_H_
#define CONF_H_


/* #define HOME_SCREEN_EFL_TEST_RUN 1 */
/* #define HOME_SCREEN_DND_ENABLED 1 */
/* #define HOME_SCREEN_BLUR_ENABLED 1 */

#define FRAMES_PER_SECOND 1.0/60.0

/*=================================SCREEN CONF================================*/
#define INDICATOR_HEIGHT 52
#define ROOT_WIN_H 1280
#define ROOT_WIN_W 720

#define HD_WIDTH 720
#define HD_HEIGHT 1280
#define QHD_WIDTH 540
#define QHD_HEIGHT 960
#define WVGA_WIDTH 480
#define WVGA_HEIGHT 800
#define UHD_WIDTH 1440
#define UHD_HEIGHT 2560

#define HD_SCALE_FACTOR 1.2
#define QHD_SCALE_FACTOR 0.85
#define WVGA_SCALE_FACTOR 0.70
#define UHD_SCALE_FACTOR 1.3

/*=================================DATA KEYS CONF=============================*/
#define KEY_ICON_DATA "item_data"
#define KEY_REPOSITION_DATA "reposition_data"
#define KEY_APP_GRID_DATA "app_grid_data"
#define KEY_THUMB_DATA "thumb_data"

#define KEY_IS_REPOSITIONABLE "is_repositionable"
#define KEY_INDEX "index_data"

/*=================================DATABASE===================================*/
#define DATABASE_FILE \
	"homescreen-efl.db"

/*=================================RESOURCES==================================*/
#define EDJE_DIR "edje"
#define IMAGE_DIR "images"

/*******************************************************************************
*
*
*				SOURCE CONFIGS
*
*
*******************************************************************************/

/*================================ OPTION_MENU.C==============================*/
#define OPTION_MENU_SETTINGS "org.tizen.wallpaper-ui-service"
#define CAPTION_MENU_CENTER_LABEL_TEXT_LEN_MAX 16

#define INDEX_HEIGHT 45
#define BUTTONS_AREA 0.055
/*================================ ALL_APPS.C=================================*/
#define APP_ROWS 5
#define APP_COLS 4

#define APPS_SHOW_HIDE_ANIMATION_TIME 0.35
#define ALL_APPS_ANIMATION_DELTA_Y (0.2*(double)ROOT_WIN_H)
#define ALL_APPS_ANIMATION_HOME_DELTA_Y (0.8 * (double) ROOT_WIN_H)

/*================================ APP_ICON.C=================================*/
#define DEFAULT_APP_ICON  \
	IMAGE_DIR "/default.png"

#define BADGE_REL_X 0.85
#define BADGE_REL_Y 0.00

#define SIGNAL_HIGHLIGHT_FOLDER "highlight_folder"
#define SIGNAL_UNHIGHLIGHT_FOLDER "unhighlight_folder"
#define SIGNAL_HIGHLIGHT_IMPOSSIBLE_FOLDER "highlight_impossible_folder"

/*================================ APP_GRID.C=================================*/
#define GRID_CONTENT "elm.swallow.icon"
#define EDIT_MODE_SCALE 0.9
#define CHOOSE_MODE_SCALE 0.9
#define CHANGE_PAGE_TIMER 0.3
#define CHANGE_PAGE_LEFT_AREA 0.05
#define CHANGE_PAGE_RIGHT_AREA 0.95

#define SIGNAL_APP_GRID_FULL "app_grid_full"
#define SIGNAL_APP_GRID_BG_HIDE "signal_app_grid_bg_hide"

/*================================ DATA_MODEL.C===============================*/
#define APPS_PAGE APP_ROWS*APP_COLS
#define FOLDER_APPS_PAGE 9

/*================================ LIVEBOX CONFIG ============================*/
#define WIDGET_NR_OF_SIZE_LIST 13
#define LIVEBOX_ON_CREATE_SIZE 4
#define LIVEBOX_MIN_MOUSE_MOVE 100

#define TEST_PKG "/usr/apps/org.tizen.homescreen-efl/res/images/livebox/"
#define TEST_PKG_ALARM        TEST_PKG"preview_alarm.png"
#define TEST_PKG_DUAL_CLOCK   TEST_PKG"preview_dual_clock.png"
#define TEST_PKG_MUSIC        TEST_PKG"preview_music.png"
#define TEST_PKG_CALENDAR     TEST_PKG"preview_calendar.png"
#define TEST_PKG_FMRADIO      TEST_PKG"preview_FMradio.png"
#define TEST_PKG_SINGLE_CLOCK TEST_PKG"preview_singel_clock.png"
#define TEST_PKG_CONTACT_4_2  TEST_PKG"preview_contact_4x2.png"
#define TEST_PKG_GALERY       TEST_PKG"preview_gallery.png"
#define TEST_PKG_WEATHER_4_2  TEST_PKG"preview_weather_4x2.png"
#define TEST_PKG_CONTACT_4_4  TEST_PKG"preview_contact_4x4.png"
#define TEST_PKG_INTERNET     TEST_PKG"preview_internet_4x4.png"
#define TEST_PKG_WEATHER_4_4  TEST_PKG"preview_weather_4x4.png"

#define EDJE_LIVEBOX_CONTAINER_FILENAME EDJE_DIR"/livebox_container.edj"
#define GROUP_LIVEBOX_CONTAINER "livebox_container"

#define EDJE_LIVEBOX_LAYOUT_FILENAME EDJE_DIR"/livebox_layout.edj"
#define GROUP_LIVEBOX_LAYOUT "livebox"

#define LIVEBOX_SCROLLER_MAX_PAGES 6
#define LIVEBOX_GRID_ROWS 4
#define LIVEBOX_GRID_COLUMNS 4
#define LIVEBOX_GRID_ROWS_CELLS_MULTI 1
#define LIVEBOX_TOTAL_ROWS (LIVEBOX_GRID_ROWS * LIVEBOX_GRID_ROWS_CELLS_MULTI)
#define LIVEBOX_TOTAL_COLUMNS \
	(LIVEBOX_GRID_COLUMNS * LIVEBOX_GRID_ROWS_CELLS_MULTI)

#define LIVEBOX_RESIZE_POS_MULTI_ROWS (1.0 / LIVEBOX_TOTAL_ROWS)
#define LIVEBOX_RESIZE_POS_MULTI_COLS (1.0 / LIVEBOX_TOTAL_COLUMNS)
#define LIVEBOX_GRID_BG_PADDING 3
#define LIVEBOX_MAX_BUTTON_NAME_LENGTH 15
#define LIVEBOX_PAGE_HEIGHT_REL (1.0*ROOT_WIN_W/ROOT_WIN_H)
/*this should be smaller than scroller page*/
#define LIVEBOX_SIZE_REL (1.0*ROOT_WIN_W/ROOT_WIN_H)
/*size of the area which causes page change during drag'n drop*/
#define LIVEBOX_PAGE_CHANGE_RECT_SIZE 60
#define LIVEBOX_PAGE_NO_CHANGE -1
#define LIVEBOX_DRAGGED_ITEM_ALPHA 96
#define LIVEBOX_LONGPRESS_TIME 1
#define LIVEBOX_PAGE_CHANGE_PERIOD 0.75
#define LIVEBOX_MAX_STATE_LEN 10
#define LIVEBOX_HORIZONTAL_PADDING 10
#define LIVEBOX_MAKE_SPACE_ANIM_TIME 0.33
#define LIVEBOX_GRID_BG_ALPHA 40 + 51
#define LIVEBOX_REPOSITION_ANIM_TIME 0.33

#define PART_LIVEBOX "livebox"
#define PART_RESIZE_BUTTON_DOWN  "button.down"
#define PART_RESIZE_BUTTON_LEFT  "button.left"
#define PART_RESIZE_BUTTON_UP    "button.up"
#define PART_RESIZE_BUTTON_RIGHT "button.right"
#define PART_ADD_PAGE_BUTTON "add_page_button"
#define PART_PAGE_BG "bg"
#define PART_GRID_BG "grid_bg"
#define PART_GRID "grid"

#define SIGNAL_CONTENT_SOURCE "content"
#define SIGNAL_LIVEBOX_SOURCE "livebox"
#define SIGNAL_LAYOUT_SOURCE "layout"
#define SIGNAL_REMOVE_SOURCE "remove_button"
#define SIGNAL_RESIZING    "drag"
#define SIGNAL_RESIZE_END  "drag,stop"
#define SIGNAL_RESIZE_SLIDER_DRAG_OK "drag,ok"
#define SIGNAL_RESIZE_SLIDER_DRAG_ERR "drag,outside"
#define SIGNAL_RESIZE_SLIDERS_SHOW "sliders,show"
#define SIGNAL_RESIZE_SLIDERS_HIDE "sliders,hide"
/*#define SIGNAL_REMOVE_BUTTON_CLICKED "remove_button,clicked" */
#define SIGNAL_CLICKED "mouse,clicked,*"
#define SIGNAL_LIVEBOX_REMOVE_BUTTON_SHOW "remove_button,show"
#define SIGNAL_LIVEBOX_REMOVE_BUTTON_HIDE "remove_button,hide"
#define SIGNAL_REMOVE_BUTTON_INSTANT_HIDE "remove_button,hide,instant"
#define SIGNAL_LIVEBOX_REPOSITION_BG_SHOW "reposition_bg,show"
#define SIGNAL_LIVEBOX_REPOSITION_BG_HIDE "reposition_bg,hide"
#define SIGNAL_EMPTY_PAGE_SHOW "empty_page,show"
#define SIGNAL_EMPTY_PAGE_HIDE "empty_page,hide"
#define SIGNAL_EMPTY_PAGE_EDIT_START "empty_page,edit,start"
#define SIGNAL_EMPTY_PAGE_EDIT_STOP "empty_page,edit,stop"
#define SIGNAL_EDIT_START "edit,start"
#define SIGNAL_EDIT_STOP "edit,stop"
#define SIGNAL_ADD_SHOW "add,show"
#define SIGNAL_ADD_HIDE "add,hide"
#define SIGNAL_ADD_PAGE_CLICKED "add_page_button_clicked"
#define SIGNAL_SCROLLER_PAGE_COUNT_CHANGED "scroller,count,changed"
#define SIGNAL_MOUSE_DOWN "mouse,down,*"
#define SIGNAL_MOUSE_UP "mouse,up,*"
#define SIGNAL_SCROLLER_DRAG_START "scroll,drag,start"
#define SIGNAL_SCROLLER_SCROLL "scroll"
#define SIGNAL_GRID_SHADOW_SHOW "grid,show"
#define SIGNAL_GRID_SHADOW_HIDE "grid,hide"
#define CANNOT_ADD_PAGE_INFO "There is not enough space on the home screen."
#define CANNOT_ADD_WIDGET_INFO "Unsupported widget size."

/*========================== ALL PAGES CONFIG ================================*/
#define GENGRID_ITEM_CONTENT "elm.swallow.icon"

#define IMAGE_ADD_PAGE_FILENAME \
	"/usr/apps/com.samsung.home-lite/res/images/edit_all_page_create.png"

#define ALL_PAGES_X 0
#define ALL_PAGES_Y 0

#define THUMB_W_REL (320.0/ROOT_WIN_W)
#define THUMB_H_REL (320.0/ROOT_WIN_H)

#define ITEM_W_REL (336.0/ROOT_WIN_W)
#define ITEM_H_REL (397.0/ROOT_WIN_H)

#define MAX_ITEMS 6
#define ITEM_CREATE_TIME 0.5
#define OFFSET 100
/*========================== KEY CONFIG ======================================*/

#define KEY_HOME "XF86Home"
#define KEY_MENU "XF86Menu"
#define KEY_BACK "XF86Back"
#define KEY_RETURN "Return"


/*======================== FOLDER CONFIG =====================================*/
#define FOLDER_MAX_ITEMS 9
#define FOLDER_POPUP_Y_REL 0.1677

/*======================== POPUP CONFIG ================================*/
#define POPUP_MAX_FOLDER_ITEMS_TEXT "There is not enough space in folder"

/*******************************************************************************
*
*
*			EDC FILE CONFIGURATIONS
*
*
*******************************************************************************/
#define SIGNAL_SOURCE "layout"

/*========================== HOME VIEW CONFIG ================================*/
#define _HOME_WGT_MIN_Y 80
#define _HOME_WGT_MAX_Y 1140
#define _HOME_WGT_EDIT_MAX_Y 1034

#define _HOME_BTNS_Y_MIN 1188
#define _HOME_BTNS_Y_MAX 1263
#define _HOME_MENU_BTN_X_MIN 32
#define _HOME_MENU_BTN_X_MAX 161

#define _HOME_HOME_BTN_X_MIN 559
#define _HOME_HOME_BTN_X_MAX 688
#define _HOME_CHOOSER_CAPTION_Y_MIN 0
#define _HOME_CHOOSER_CAPTION_Y_MAX 80

#define _HOME_PG_INDEX_X_MIN 161
#define _HOME_PG_INDEX_X_MAX 559

#define INDICE_NEXT_PAGE_X_MIN 0.99
#define INDICE_NEXT_PAGE_X_MAX 1.00
#define INDICE_PREV_PAGE_X_MIN 0.00
#define INDICE_PREV_PAGE_X_MAX 0.01

#define _MAIN_CONTENT_Y_MIN 72 /*TODO: This value should be 42*/
#define _MAIN_CONTENT_Y_MAX 862
#define _APP_CONTENT_Y_MAX 1172

#define CONTENT_Y_MIN _MAIN_CONTENT_Y_MIN/ROOT_WIN_H;
#define CONTENT_Y_MAX _MAIN_CONTENT_Y_MAX/ROOT_WIN_H;
#define CONTENT_APP_Y_MAX _APP_CONTENT_Y_MAX/ROOT_WIN_H;

#define HOME_WGT_MIN_Y _HOME_WGT_MIN_Y/ROOT_WIN_H
#define HOME_WGT_MAX_Y _HOME_WGT_MAX_Y/ROOT_WIN_H
#define HOME_WGT_EDIT_MAX_Y _HOME_WGT_EDIT_MAX_Y/ROOT_WIN_H

#define HOME_BTNS_Y_MIN _HOME_BTNS_Y_MIN/ROOT_WIN_H
#define HOME_BTNS_Y_MAX _HOME_BTNS_Y_MAX/ROOT_WIN_H
#define HOME_MENU_BTN_X_MIN _HOME_MENU_BTN_X_MIN/ROOT_WIN_W
#define HOME_MENU_BTN_X_MAX _HOME_MENU_BTN_X_MAX/ROOT_WIN_W
#define HOME_HOME_BTN_X_MIN _HOME_HOME_BTN_X_MIN/ROOT_WIN_W
#define HOME_HOME_BTN_X_MAX _HOME_HOME_BTN_X_MAX/ROOT_WIN_W
#define HOME_CHOOSER_CAPTION_Y_MIN _HOME_CHOOSER_CAPTION_Y_MIN/ROOT_WIN_H
#define HOME_CHOOSER_CAPTION_Y_MAX _HOME_CHOOSER_CAPTION_Y_MAX/ROOT_WIN_H

#define HOME_PG_INDEX_X_MIN _HOME_PG_INDEX_X_MIN/ROOT_WIN_W
#define HOME_PG_INDEX_X_MAX _HOME_PG_INDEX_X_MAX/ROOT_WIN_W

#define SIGNAL_MENU_BTN_CLICKED "menu_clicked"
#define SIGNAL_HOME_BTN_CLICKED "home_clicked"
#define SIGNAL_CHOOSER_CAPTION_ACTIVATE "chooser,caption,activate"
#define SIGNAL_CHOOSER_CAPTION_DEACTIVATE "chooser,caption,deactivate"

#define GROUP_HOME_LY "home_ly"
#define PART_CONTENT "content"
#define PART_INDEX "index"
#define PART_EDIT_BG "edit_bg"
#define PART_NEXT_PAGE_INDICE "next_page_indice"
#define PART_PREV_PAGE_INDICE "prev_page_indice"
#define PART_EVENT_BLOCKER "part_event_blocker"
#define PART_CHOOSER_CAPTION_RIGHT_LABEL "chooser_caption_right_label"
#define PART_CHOOSER_CAPTION_LEFT_LABEL "chooser_caption_left_label"
#define PART_CHOOSER_CAPTION_MIDDLE_LABEL "chooser_caption_middle_label"

#define SIGNAL_EDIT_BG_ON "edit_bg_on"
#define SIGNAL_EDIT_BG_OFF "edit_bg_off"
#define SIGNAL_NEXT_PAGE_INDICE_SHOW "next_page_show"
#define SIGNAL_PREV_PAGE_INDICE_SHOW "prev_page_show"
#define SIGNAL_INDICE_HIDE "page_indice_hide"

#define SIGNAL_BLOCK_EVENTS "block_home_events"
#define SIGNAL_UNBLOCK_EVENTS "unblock_home_events"

/*this signals is used for change page on reposition action */
#define SIGNAL_NEXT_PAGE_CHANGE "next_page_change"
#define SIGNAL_PREV_PAGE_CHANGE "prev_page_change"

/*these signals change states of bottom buttons (menu and home/all apps) */
#define SIGNAL_BOTTOM_BUTTONS_HOME_STATE_SET "menu_icon_home_state_set"
#define SIGNAL_BOTTOM_BUTTONS_ALL_APPS_STATE_SET "menu_icon_all_apps_state_set"
#define SIGNAL_BOTTOM_BUTTONS_HOME_EDIT_STATE_SET "menu_icon_home_edit_state_set"
#define SIGNAL_BOTTOM_BUTTONS_ALL_APPS_EDIT_STATE_SET "menu_icon_edit_state_set"

/*========================== LIVEBOX EDC CONFIG ==============================*/
#define _LIVEBOX_X_MARGIN 6
#define _LIVEBOX_X_MIN _LIVEBOX_X_MARGIN
#define _LIVEBOX_X_MAX (ROOT_WIN_W - _LIVEBOX_X_MARGIN)

#define _LIVEBOX_EDIT_X_MARGIN 30
#define _LIVEBOX_EDIT_Y_MARGIN 30
#define _LIVEBOX_EDIT_X_MIN _LIVEBOX_EDIT_X_MARGIN
#define _LIVEBOX_EDIT_X_MAX (ROOT_WIN_W - _LIVEBOX_EDIT_X_MARGIN)
#define _LIVEBOX_EDIT_Y_MIN _LIVEBOX_EDIT_X_MARGIN
/*square area of screen width size is required */
#define _LIVEBOX_EDIT_Y_MAX (ROOT_WIN_W - _LIVEBOX_EDIT_X_MARGIN)

#define _LIVEBOX_ADD_PAGE_BUTTON_SIZE_HALF 60
#define _LIVEBOX_ADD_PAGE_BUTTON_X_MIN \
	(ROOT_WIN_W / 2 - _LIVEBOX_ADD_PAGE_BUTTON_SIZE_HALF)
#define _LIVEBOX_ADD_PAGE_BUTTON_X_MAX \
	(ROOT_WIN_W / 2 + _LIVEBOX_ADD_PAGE_BUTTON_SIZE_HALF)
#define _LIVEBOX_ADD_PAGE_BUTTON_Y_MIN \
	(ROOT_WIN_W / 2 - _LIVEBOX_ADD_PAGE_BUTTON_SIZE_HALF)
#define _LIVEBOX_ADD_PAGE_BUTTON_Y_MAX \
	(ROOT_WIN_W / 2 + _LIVEBOX_ADD_PAGE_BUTTON_SIZE_HALF)

#define LIVEBOX_REMOVE_BUTTON_SIZE 48
#define LIVEBOX_SLIDER_BUTTON_SIZE 12
#define LIVEBOX_SLIDER_FRAME_OFFSET_1 5
#define LIVEBOX_SLIDER_FRAME_OFFSET_2 8

#define LIVEBOX_REMOVE_LB_BUTTON_SIZE 24
#define LIVEBOX_REMOVE_LB_OFFSET_X 9
#define LIVEBOX_REMOVE_LB_OFFSET_Y 8

#define LIVEBOX_REPOSITION_BG_BORDER 10
#define LIVEBOX_REPOSITION_BG_COLOR 162
#define LIVEBOX_REPOSITION_SHADOW_COLOR 96

#define LIVEBOX_X_MIN _LIVEBOX_X_MIN/ROOT_WIN_W
#define LIVEBOX_X_MAX _LIVEBOX_X_MAX/ROOT_WIN_W

#define LIVEBOX_EDIT_X_MIN _LIVEBOX_EDIT_X_MIN/ROOT_WIN_W
#define LIVEBOX_EDIT_X_MAX _LIVEBOX_EDIT_X_MAX/ROOT_WIN_W
#define LIVEBOX_EDIT_Y_MIN _LIVEBOX_EDIT_Y_MIN/ROOT_WIN_W
#define LIVEBOX_EDIT_Y_MAX _LIVEBOX_EDIT_Y_MAX/ROOT_WIN_W

#define LIVEBOX_ADD_PAGE_BUTTON_X_MIN _LIVEBOX_ADD_PAGE_BUTTON_X_MIN/ROOT_WIN_W
#define LIVEBOX_ADD_PAGE_BUTTON_X_MAX _LIVEBOX_ADD_PAGE_BUTTON_X_MAX/ROOT_WIN_W
#define LIVEBOX_ADD_PAGE_BUTTON_Y_MIN _LIVEBOX_ADD_PAGE_BUTTON_Y_MIN/ROOT_WIN_W
#define LIVEBOX_ADD_PAGE_BUTTON_Y_MAX _LIVEBOX_ADD_PAGE_BUTTON_Y_MAX/ROOT_WIN_W

#define LIVEBOX_RESIZE_FRAME_REL_1 \
	(1.0 * LIVEBOX_SLIDER_FRAME_OFFSET_1 / LIVEBOX_SLIDER_BUTTON_SIZE)
#define LIVEBOX_RESIZE_FRAME_REL_2 \
	(1.0 * LIVEBOX_SLIDER_FRAME_OFFSET_2 / LIVEBOX_SLIDER_BUTTON_SIZE)

/*========================== APP ICON CONFIG =================================*/
#define FOLDER_ICON_MAX_MINIATURES 4

#define ICON_CONTAINER_W 132
#define ICON_CONTAINER_H 159

#define ICON_BADGE_W 30
#define ICON_BADGE_H 30

#define _ICON_Y_MIN 8
#define _ICON_Y_MAX 96
#define _ICON_X_MIN 22
#define _ICON_X_MAX 110

#define _ICON_NAME_X_MIN 6
#define _ICON_NAME_X_MAX 126
#define _ICON_NAME_Y_MIN 98
#define _ICON_NAME_Y_MAX 156
#define _ICON_NAME_EDIT_Y_MAX 144

#define _ICON_UNINSTALL_BTN_X_MAX 60
#define _ICON_UNINSTALL_BTN_Y_MAX 60

#define ICON_Y_MIN _ICON_Y_MIN/ICON_CONTAINER_H
#define ICON_Y_MAX _ICON_Y_MAX/ICON_CONTAINER_H
#define ICON_X_MIN _ICON_X_MIN/ICON_CONTAINER_W
#define ICON_X_MAX _ICON_X_MAX/ICON_CONTAINER_W

#define ICON_NAME_X_MIN _ICON_NAME_X_MIN/ICON_CONTAINER_W
#define ICON_NAME_X_MAX _ICON_NAME_X_MAX/ICON_CONTAINER_W
#define ICON_NAME_Y_MIN _ICON_NAME_Y_MIN/ICON_CONTAINER_H
#define ICON_NAME_Y_MAX _ICON_NAME_Y_MAX/ICON_CONTAINER_H
#define ICON_NAME_EDIT_Y_MAX _ICON_NAME_EDIT_Y_MAX/ICON_CONTAINER_H

#define ICON_UNINSTALL_BTN_X_MAX _ICON_UNINSTALL_BTN_X_MAX/ICON_CONTAINER_W
#define ICON_UNINSTALL_BTN_Y_MAX _ICON_UNINSTALL_BTN_Y_MAX/ICON_CONTAINER_H

#define ICON_BADGE_CHECKBOX_REL1_X 0.6588
#define ICON_BADGE_CHECKBOX_REL1_Y 0.0498
#define ICON_BADGE_CHECKBOX_REL2_X 0.9412
#define ICON_BADGE_CHECKBOX_REL2_Y 0.2886

#define GROUP_ICON_LY "app_icon_layout"
#define PART_ICON_CONTENT "icon_content"
#define PART_ICON_NAME "icon_name"

#define SIGNAL_PRESS_SOURCE "icon_content_touch"

#define SIGNAL_ICON_PRESS "icon,pressed"
#define SIGNAL_UNINSTALL_BUTTON_SHOW "uninstall_button,show"
#define SIGNAL_UNINSTALL_BUTTON_HIDE "uninstall_button,hide"
#define SIGNAL_UNINSTALL_BUTTON_CLICKED "uninstall_button,clicked"
#define SIGNAL_CHECKBOX_SHOW_UNCHECKED "checkbox,show,unchecked"
#define SIGNAL_CHECKBOX_SHOW_CHECKED "checkbox,show,checked"
#define SIGNAL_CHECKBOX_HIDE "checkbox,hide"
#define SIGNAL_CHECKBOX_CHECKED "checkbox,checked"
#define SIGNAL_CHECKBOX_UNCHECKED "checkbox,unchecked"
#define SIGNAL_CHECKBOX_BLOCK "checkbox,block"
#define SIGNAL_CHECKBOX_UNBLOCK "checkbox,unblock"
#define SIGNAL_LABEL_SHOW "label,show"
#define SIGNAL_LABEL_HIDE "label,hide"
#define SIGNAL_BADGE_SHOW "badge,show"
#define SIGNAL_BADGE_HIDE "badge,hide"
#define SIGNAL_BADGE_FOLDER_SHOW "folder,badge,show"
#define SIGNAL_BADGE_FOLDER_HIDE "folder,badge,hide"
#define SIGNAL_ICON_CLICKED "icon_clicked"
#define SIGNAL_EDIT_MODE_ON "edit_mode_on"
#define SIGNAL_EDIT_MODE_OFF "edit_mode_off"
#define SIGNAL_EDIT_MODE_BLACK_ON "edit_mode_black_on"
#define SIGNAL_EDIT_MODE_BLACK_OFF "edit_mode_black_off"
#define SIGNAL_CHOOSE_MODE_ON "choose_mode_on"
#define SIGNAL_CHOOSE_MODE_OFF "choose_mode_off"
#define SIGNAL_FRAME_POSSIBLE_SHOW "show_frame_possible"
#define SIGNAL_FRAME_IMPOSSIBLE_SHOW "show_frame_impossible"
#define SIGNAL_FRAME_HIDE "hide_frame"
#define SIGNAL_EMPTY_BG_SHOW "empty_bg"
#define SIGNAL_BLACK_TEXT "label,text,black"
#define PART_CAPTION_MENU_CENTER_LABEL_NAME "chooser_caption_middle_label"
#define PART_CAPTION_MENU_LEFT_LABEL "chooser_caption_left_label"
#define PART_CAPTION_MENU_RIGHT_LABEL "chooser_caption_right_label"

/*========================== INDEX ELEMENT CONFIG ============================*/
#define INDEX_MAX_PAGE_COUNT 7
#define SIGNAL_SET_CURRENT "set_current"
#define SIGNAL_SET_DEFAULT "set_default"
#define INDICE_SIZE_SETTER "indice_size_setter"

#define INDEX_IMAGE IMAGE_DIR"/core_page_indicator_horizontal.png"
#define INDEX_EDJE EDJE_DIR"/index_element.edj"
#define GROUP_INDEX_ELEMENT "index_element"
#define PART_INDEX_ELEMENT "element"

#define INDEX_ELEMENT_CONTAINER_WIDTH 38.0
#define INDEX_ELEMENT_CONTAINER_HEIGHT 38.0

#define INDEX_CURRENT_SIZE 0.03
#define INDEX_NORMAL_SIZE 0.015
#define INDEX_CURRENT_ALPHA 255
#define INDEX_NORMAL_ALPHA 100

/*========================== APP_GRID.EDC CONFIG =============================*/
#define GROUP_APP_GRID "app_grid_ly"
#define GROUP_APP_GRID_ITEM_CONTAINER "app_grid_item_container"

#define PART_APP_GRID_BG "app_grid_bg"
#define PART_APP_GRID_EDIT_BG "app_grid_edit_bg"
#define PART_APP_GRID_CONTENT "app_grid_content"
#define PART_APP_GRID_ITEM "app_grid_item"

#define PART_APP_GRID_SIZE_SETTER "size_setter"

#define ANIMATION_RESIZE_TIME 0.2

/*========================== FOLDER.EDC CONFIG ===============================*/
#define MINIATURE_COL_1_MIN 0.18
#define MINIATURE_COL_1_MAX 0.48
#define MINIATURE_COL_2_MIN 0.52
#define MINIATURE_COL_2_MAX 0.82
#define MINIATURE_ROW_1_MIN 0.18
#define MINIATURE_ROW_1_MAX 0.48
#define MINIATURE_ROW_2_MIN 0.52
#define MINIATURE_ROW_2_MAX 0.82
#define MINIATURE_CENTER_MIN 0.35
#define MINIATURE_CENTER_MAX 0.65

#define FOLDER_HEIGHT 569

#define _NAME_Y_MIN 0
#define _NAME_Y_MAX 62

#define _SEPARATOR_Y_MIN 62
#define _SEPARATOR_Y_MAX 64

#define _CONTENT_Y_MIN 53
#define _CONTENT_Y_MAX 635

#define _INDEX_Y_MIN 355
#define _INDEX_Y_MAX 421

#define FOLDER_NAME_Y_MIN _NAME_Y_MIN/FOLDER_HEIGHT
#define FOLDER_NAME_Y_MAX _NAME_Y_MAX/FOLDER_HEIGHT

#define FOLDER_SEPARATOR_Y_MIN _SEPARATOR_Y_MIN/FOLDER_HEIGHT
#define FOLDER_SEPARATOR_Y_MAX _SEPARATOR_Y_MAX/FOLDER_HEIGHT

#define FOLDER_CONTENT_Y_MIN _CONTENT_Y_MIN/FOLDER_HEIGHT
#define FOLDER_CONTENT_Y_MAX _CONTENT_Y_MAX/FOLDER_HEIGHT

#define INDEX_Y_MIN _INDEX_Y_MIN/FOLDER_3_R_IND_HEIGHT
#define INDEX_Y_MAX _INDEX_Y_MAX/FOLDER_3_R_IND_HEIGHT

#define GROUP_FOLDER_ICON_LAYOUT "folder_icon_layout"
#define GROUP_FOLDER_ADD_ICON "folder_add_icon"
#define GROUP_FOLDER_WINDOW "folder_window"
#define GROUP_FOLDER_POPUP "folder_popup"
#define PART_FOLDER_NAME "folder_name_entry"
#define PART_FOLDER_CONTENT "folder_content"
#define PART_FOLDER_UNNAMED_TEXT "Unnamed folder"

#define SIGNAL_CLEAR_BTN_SHOW "clear_btn_show"
#define SIGNAL_CLEAR_BTN_HIDE "clear_btn_hide"
#define SIGNAL_CLEAR_BTN_CLICKED "clear_btn_clicked"
#define SIGNAL_ENTRY_BLOCK_ON "folder_entry,block,on"
#define SIGNAL_ENTRY_BLOCK_OFF "folder_entry,block,off"

#define SIGNAL_BG_CLICKED "bg_clicked"

#define SIGNAL_FOLDER_PANEL_OPEN "folder_open"
#define SIGNAL_FOLDER_PANEL_CLOSE "folder_close"
#define SIGNAL_FOLDER_ANIM_DONE "anim_done"
#define SIGNAL_FOLDER_UNNAMED_TEXT_SHOW "unnamed_text_show"
#define SIGNAL_FOLDER_UNNAMED_TEXT_HIDE "unnamed_text_hide"

#define FOLDER_1_ROW_H_REL 0.25875
#define FOLDER_2_ROW_H_REL 0.42375
#define FOLDER_3_ROW_H_REL 0.59125

#define FOLDER_ROW_W_REL 0.7
#define FOLDER_POPUP_W_REL 0.9118
#define FOLDER_POPUP_X_REL 0.0441

#define FOLDER_GRID_W_REL 0.9
#define FOLDER_GRID_H_REL 0.6
#define FOLDER_ITEM_MARGIN 0.04

#define FOLDER_LEFT_MARGIN 0.07
#define FOLDER_RIGHT_MARGIN 0.93

#define FOLDER_WINDOW_W home_screen_get_root_width()
#define FOLDER_WINDOW_H home_screen_get_root_height()

#define FOLDER_ROW_APPS 3
#define FOLDER_COL_APPS 3

#define POPUP_SHIFT 15
#define KEYPAD_SPACE 5
#define DEFAULT_FOLDER_NAME "Unnamed folder"

/*========================== ALL PAGES EDC ===================================*/
#define GROUP_ALL_PAGES "page_edit_view"
#define GROUP_GRID_ITEM "grid_item"
#define GROUP_THUMB "thumb"

#define PART_LAYOUT_CONTENT "content"
#define PART_GRID_ITEM_CONTENT "grid_item_content"
#define PART_THUMB_CONTENT "page_thumbnail"

#define SIGNAL_BG_HIDE "bg_hide"
#define SIGNAL_BG_NORMAL "bg_normal"
#define SIGNAL_BG_REPOSITON "bg_reposition"
#define SIGNAL_REMOVE_BUTTON_CLICKED "remove_button_clicked"
#define SIGNAL_THUMBNAIL_CLICKED "thumbnail_clicked"
#define SIGNAL_ADD_ICON_SHOW "add_icon_show"
#define SIGNAL_REMOVE_BUTTON_SHOW "remove_button_show"
#define SIGNAL_REMOVE_BUTTON_HIDE "remove_button_hide"
#define SIGNAL_REMOVE_BUTTON_HIDE_INSTANT "remove_button_hide_instant"

#define BADGE_REL1_X 0.060
#define BADGE_REL1_Y 0.118
#define BADGE_REL2_X 0.250
#define BADGE_REL2_Y 0.284

#define ROW_SHIFT 0.45
#define COL_SHIFT 0.32
#endif /* CONF_H_ */
