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

#define FRAMES_PER_SECOND 1.0/60.0

#define EDJE_DIR "edje"
#define IMAGE_DIR "images"

#define HOME_ANIMATION_TIME 0.3

extern double __conf_get_resolution_scale_h();
extern double __conf_get_resolution_scale_w();

#define INDICATOR_H ((45) * (__conf_get_resolution_scale_h()))

#define APPS_VIEW_PADDING_TOP INDICATOR_H + ((28) * (__conf_get_resolution_scale_h()))
#define APPS_VIEW_W ((720) * (__conf_get_resolution_scale_w()))
#define APPS_VIEW_H ((1060) * (__conf_get_resolution_scale_h()))
#define APPS_VIEW_ANIMATION_DELTA ((100) * (__conf_get_resolution_scale_h()))

void conf_set_resolution_scale(int win_width, int win_height);

#endif /* CONF_H_ */
