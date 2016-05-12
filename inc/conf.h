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

#ifndef __CONF_H__
#define __CONF_H__

#define FRAMES_PER_SECOND 1.0/60.0

#define EDJE_DIR "edje"
#define IMAGE_DIR "images"

#define HOME_ANIMATION_TIME 0.3
#define HOME_FOLDR_ANIMATION_TIME 0.1
#define MAX_FILE_PATH_LEN 1024

extern double __conf_get_resolution_scale_h();
extern double __conf_get_resolution_scale_w();
extern int __conf_get_window_h();
extern int __conf_get_window_w();

#define INDICATOR_H ((45) * (__conf_get_resolution_scale_h()))

#define WINDOW_H __conf_get_window_h()
#define WINDOW_W __conf_get_window_w()

#define APPS_VIEW_PADDING_TOP INDICATOR_H + ((28) * (__conf_get_resolution_scale_h()))
#define APPS_VIEW_W ((720) * (__conf_get_resolution_scale_w()))
#define APPS_VIEW_H ((1060) * (__conf_get_resolution_scale_h()))
#define APPS_VIEW_ANIMATION_DELTA ((100) * (__conf_get_resolution_scale_h()))

#define APPS_VIEW_ICON_IMAGE ((118) * (__conf_get_resolution_scale_w()))

#define CLUSTER_VIEW_W ((720) * (__conf_get_resolution_scale_w()))
#define CLUSTER_VIEW_H ((1150) * (__conf_get_resolution_scale_h()))
#define CLUSTER_W CLUSTER_VIEW_W
#define CLUSTER_H CLUSTER_W
#define CLUSTER_ALL_PAGE_W ((320) * (__conf_get_resolution_scale_w()))
#define CLUSTER_ALL_PAGE_H CLUSTER_ALL_PAGE_W

#define CLUSTER_ROW 4
#define CLUSTER_COL 4
#define CLUSTER_HOME_PAGE 0
#define CLUSTER_MAX_PAGE 6

#define PAGE_INDICATOR_PADDING_BOTTON ((16) * (__conf_get_resolution_scale_h()))
#define PAGE_INDICATOR_H ((76) * (__conf_get_resolution_scale_h()))
#define PAGE_INDICATOR_W ((720) * (__conf_get_resolution_scale_w()))
#define PAGE_INDICATOR_GAP 4
#define PAGE_INDICATOR_X 0
#define PAGE_INDICATOR_UNIT ((40) * (__conf_get_resolution_scale_w()))

#define APPS_FOLDER_MAX_ITEM 9

void conf_set_resolution_scale(int win_width, int win_height);

#endif /* __CONF_H__ */
