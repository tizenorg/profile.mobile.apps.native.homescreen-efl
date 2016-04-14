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

#ifndef UTIL_H_
#define UTIL_H_

#include <dlog.h>

#ifndef __MODULE__
#define __MODULE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "HOMESCREEN_EFL"

#define LOG_(prio, tag, fmt, arg...) \
    ({ do { \
        dlog_print(prio, tag, "%s: %s(%d) > " fmt, __MODULE__, __func__, __LINE__, ##arg);\
    } while (0); })

#define LOGD(format, arg...) LOG_(DLOG_DEBUG, LOG_TAG, format, ##arg)
#define LOGI(format, arg...) LOG_(DLOG_INFO, LOG_TAG, format, ##arg)
#define LOGW(format, arg...) LOG_(DLOG_WARN, LOG_TAG, format, ##arg)
#define LOGE(format, arg...) LOG_(DLOG_ERROR, LOG_TAG, format, ##arg)
#define LOGF(format, arg...) LOG_(DLOG_FATAL, LOG_TAG, format, ##arg)

enum app_subdir {
	APP_DIR_DATA,
	APP_DIR_CACHE,
	APP_DIR_RESOURCE,
	APP_DIR_SHARED_DATA,
	APP_DIR_SHARED_RESOURCE,
	APP_DIR_SHARED_TRUSTED,
	APP_DIR_EXTERNAL_DATA,
	APP_DIR_EXTERNAL_CACHE,
	APP_DIR_EXTERNAL_SHARED_DATA,
};

const char *util_get_file_path(enum app_subdir dir, const char *relative);

#define util_get_data_file_path(x) util_get_file_path(APP_DIR_DATA, (x))
#define util_get_cache_file_path(x) util_get_file_path(APP_DIR_CACHE, (x))
#define util_get_res_file_path(x) util_get_file_path(APP_DIR_RESOURCE, (x))
#define util_get_shared_data_file_path(x) util_get_file_path(APP_DIR_SHARED_DATA, (x))
#define util_get_shared_res_file_path(x) util_get_file_path(APP_DIR_SHARED_RESOURCE, (x))
#define util_get_trusted_file_path(x) util_get_file_path(APP_DIR_SHARED_TRUSTED, (x))
#define util_get_external_data_file_path(x) util_get_file_path(APP_DIR_EXTERNAL_DATA, (x))
#define util_get_external_cache_file_path(x) util_get_file_path(APP_DIR_EXTERNAL_CACHE, (x))
#define util_get_external_shared_data_file_path(x) util_get_file_path(APP_DIR_EXTERNAL_SHARED_DATA, (x))

#endif /* UTIL_H_ */
