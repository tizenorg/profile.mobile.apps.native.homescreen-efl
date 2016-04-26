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

#define SIGNAL_HOME_BTN_SHOW "home_btn_show"
#define SIGNAL_HOME_BTN_HIDE "home_btn_hide"
#define SIGNAL_APPS_BTN_SHOW "apps_btn_show"
#define SIGNAL_APPS_BTN_HIDE "apps_btn_hide"


#define GROUP_HOME_BTN_LY "home_btn_ly"

// cluster
#define GROUP_CLUSTER_LY "cluster_ly"
#define SIZE_SETTER "size_setter"

#define GROUP_PAGE_INDICATOR_UNIT "page_indicator_unit"
#define SIGNAL_PAGE_IDICATOR_ROTATION_CHANGE "rotation,changed"
#define SIGNAL_PAGE_IDICATOR_DEFAULT "default,set"
#define SIGNAL_PAGE_IDICATOR_CURRENT "current,set"


// apps view
#define GROUP_APPS_LY "apps_ly"
#define PART_APPS_VIEW_BG "apps_view_bg"

#define GROUP_APPS_BG_LY "apps_bg_ly"

#define ANIMATION_RESIZE_TIME 0.3
#define SIGNAL_EDIT_MODE_ON "edit_on"
#define SIGNAL_EDIT_MODE_OFF "edit_off"

#define SIGNAL_APPS_VIEW_HIDE "apps_bg_hide"
#define SIGNAL_APPS_VIEW_SHOW "apps_bg_show"
#define SIGNAL_APPS_VIEW_ANIM "apps_bg_anim"
#define APPS_VIEW_COL 4
#define APPS_VIEW_ROW 5

#define APPS_VIEW_BG_OPACITY 70

#define GROUP_APP_ICON_LY "icon_ly"
#define PART_ICON_CONTENT "icon_content"
#define PART_ICON_CONTENT_TOUCH "icon_touch"
#define PART_ICON_NAME "icon_name"

#define SIGNAL_ICON_CLICKED "icon_clicked"

#define GROUP_FOLDER_ICON_LY "folder_ly"
#define SIGNAL_FRAME_POSSIBLE_SHOW "frame_possible_show"
#define SIGNAL_FRAME_IMPOSSIBLE_SHOW "frame_impossible_show"

#endif /* __EDC_CONF_H__ */
