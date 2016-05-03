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

static int window_w = 720;
static int window_h = 1280;
static double resolution_scale_h = 1.0;
static double resolution_scale_w = 1.0;

int __conf_get_window_h(void)
{
    return window_h;
}

int __conf_get_window_w(void)
{
    return window_w;
}

double __conf_get_resolution_scale_h()
{
    return resolution_scale_h;
}

double __conf_get_resolution_scale_w()
{
    return resolution_scale_w;
}

void conf_set_resolution_scale(int win_width, int win_height)
{
    window_h = win_height;
    window_w = win_width;
    resolution_scale_w = win_width / 720;
    resolution_scale_h = win_height / 1280;
}
