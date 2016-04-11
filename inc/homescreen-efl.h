/*
 * Copyright (c) 2000 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __homescreen-efl_H__
#define __homescreen-efl_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <dlog.h>

#include "conf.h"
#include "util.h"

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "HOMESCREEN_EFL"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.homescreen-efl"
#endif

typedef enum {
	HOMESCREEN_VIEW_HOME = 0,
	HOMESCREEN_VIEW_HOME_EDIT,
	HOMESCREEN_VIEW_HOME_ALL_PAGES,
	HOMESCREEN_VIEW_HOME_ADD_VIEWER,
	HOMESCREEN_VIEW_ALL_APPS,
	HOMESCREEN_VIEW_ALL_APPS_EDIT,
	HOMESCREEN_VIEW_ALL_APPS_CHOOSE,
	HOMESCREEN_VIEW_UNKNOWN,
} homescreen_view_t;


#endif /* __homescreen-efl_H__ */
