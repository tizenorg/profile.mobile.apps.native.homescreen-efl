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

#include "conf.h"

double resolution_scale_h = 1.0;
double RESOLUTION_SCALE_W = 1.0;

double __conf_get_resolution_scale_h() {
    return resolution_scale_h;
}

double __conf_get_resolution_scale_w() {
    return RESOLUTION_SCALE_W;
}

void conf_set_resolution_scale(int win_width, int win_height)
{
    RESOLUTION_SCALE_W = win_width / 720;
    resolution_scale_h = win_height / 1280;
}
