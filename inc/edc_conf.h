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

#ifndef __EDC_CONF_H__
#define __EDC_CONF_H__

#define GROUP_HOME_LY "home_ly"
#define HOME_BG "home_bg"
#define HOME_CONTENT "part_content"

#define HOME_BUTTON "home_btn"
#define MENU_BUTTON "menu_btn"

#define SIGNAL_MENU_BTN_CLICKED "menu_btn_clicked"
#define SIGNAL_HOME_BTN_CLICKED "home_btn_clicked"
#define SIGNAL_SOURCE "source"

#define SIGNAL_BTN_SHOW_HOME "btn_show_home"
#define SIGNAL_BTN_SHOW_APPS "btn_show_apps"
#define SIGNAL_BTN_HIDE_HOME "btn_hide_home"
#define SIGNAL_BTN_HIDE_APPS "btn_hide_apps"
#define SIGNAL_HOME_BTN_SHOW "home_btn_show"
#define SIGNAL_HOME_BTN_HIDE "home_btn_hide"
#define SIGNAL_APPS_BTN_SHOW "apps_btn_show"
#define SIGNAL_APPS_BTN_HIDE "apps_btn_hide"

#define GROUP_HOME_BTN_LY "home_btn_ly"

// cluster
#define GROUP_CLUSTER_LY "cluster_ly"
#define SIZE_SETTER "size_setter"
#define CLUSTER_BOX "cluster_box"

#define GROUP_PAGE_INDICATOR_UNIT "page_indicator_unit"
#define SIGNAL_PAGE_IDICATOR_ROTATION_CHANGE "rotation,changed"
#define SIGNAL_PAGE_IDICATOR_DEFAULT "default,set"
#define SIGNAL_PAGE_IDICATOR_CURRENT "current,set"


// apps view
#define GROUP_APPS_LY "apps_ly"
#define APPS_APPS_VIEW_BG "apps_view_bg"

#define GROUP_APPS_BG_LY "apps_bg_ly"
#define GROUP_APPS_CHOOSER_BTN_LY "apps_chooser_btn_ly"

#define APPS_CHOOSER_LEFT_LABEL "apps_chooser_left_label"
#define APPS_CHOOSER_MIDDLE_LABEL "apps_chooser_middle_label"
#define APPS_CHOOSER_RIGHT_LABEL "apps_chooser_right_label"

#define SIGNAL_CHOOSER_BUTTON_SHOW "chooser_button_show"
#define SIGNAL_CHOOSER_BUTTON_HIDE "chooser_button_hide"

#define ANIMATION_RESIZE_TIME 0.3
#define SIGNAL_SET_BLACK_TEXT "set_black_text"
#define SIGNAL_EDIT_MODE_ON_ANI "edit_on_ani"
#define SIGNAL_EDIT_MODE_ON "edit_on"
#define SIGNAL_EDIT_MODE_OFF_ANI "edit_off_ani"
#define SIGNAL_EDIT_MODE_OFF "edit_off"
#define SIGNAL_CHOOSER_MODE_ON "chooser_on"
#define SIGNAL_CHOOSER_MODE_OFF "chooser_off"
#define SIGNAL_CHOOSER_LEFT_BTN_CLICKED "chooser_left_btn_clicked"
#define SIGNAL_CHOOSER_RIGHT_BTN_CLICKED "chooser_right_btn_clicked"
#define SIGNAL_CHECK_CHECK_BOX "check"
#define SIGNAL_UNCHECK_CHECK_BOX "uncheck"
#define SIGNAL_CHANGED_CHECK_BOX "changed_checkbox_state"

#define SIGNAL_APPS_VIEW_HIDE "apps_bg_hide"
#define SIGNAL_APPS_VIEW_SHOW "apps_bg_show"
#define SIGNAL_APPS_VIEW_ANIM "apps_bg_anim"
#define APPS_VIEW_COL 4
#define APPS_VIEW_ROW 5

#define APPS_VIEW_BG_OPACITY 70

#define GROUP_APP_ICON_LY "icon_ly"
#define APPS_ICON_CONTENT "icon_content"
#define APPS_ICON_CONTENT_TOUCH "icon_touch"
#define APPS_ICON_NAME "icon_name"
#define APPS_ICON_BADGE_TEXT "badge_text"
#define SIGNAL_ICON_DISABLE "icon_disable"
#define SIGNAL_ICON_ENABLE "icon_enable"
#define SIGNAL_BADGE_SHOW "badge_show"
#define SIGNAL_BADGE_HIDE "badge_hide"

#define SIGNAL_ICON_CLICKED "icon_clicked"

#define GROUP_FOLDER_ICON_LY "folder_ly"
#define SIGNAL_FRAME_POSSIBLE_SHOW "frame_possible_show"
#define SIGNAL_FRAME_IMPOSSIBLE_SHOW "frame_impossible_show"

#define APPS_ICON_UNINSTALL_BUTTON "uninstall_btn"
#define SIGNAL_UNINSTALL_BUTTON_SHOW_ANI "uninstall_btn_show_ani"
#define SIGNAL_UNINSTALL_BUTTON_SHOW "uninstall_btn_show"
#define SIGNAL_UNINSTALL_BUTTON_HIDE_ANI "uninstall_btn_hide_ani"
#define SIGNAL_UNINSTALL_BUTTON_HIDE "uninstall_btn_hide"
#define SIGNAL_UNINSTALL_BUTTON_CLICKED "uninstall_btn_click"

#define GROUP_APPS_FOLDER_POPUP_LY "apps_folder_popup_ly"
#define APPS_FOLDER_BG "apps_folder_bg"
#define APPS_FOLDER_COL 3
#define APPS_FOLDER_ROW 3
#define APPS_FOLDER_TITLE "apps_folder_title"
#define APPS_FOLDER_SEPARATOR "apps_folder_separator"
#define APPS_FOLDER_CONTAINER "apps_folder_container"
#define APPS_FOLDER_PLUS_ICON "apps_folder_plus"
#define SIGNAL_APPS_FOLDER_HIDE "apps_folder_hide"
#define SIGNAL_APPS_FOLDER_SHOW "apps_folder_show"
#define SIGNAL_APPS_FOLDER_SET_PLUS_ICON "apps_folder_set_plus_icon"
#define SIGNAL_APPS_FOLDER_HIDE_PLUS_ICON "apps_folder_hide_plus_icon"
#define SIGNAL_APPS_FOLDER_CLICKED_PLUS_ICON "apps_folder_clicked_plus_icon"

#endif /* __EDC_CONF_H__ */
