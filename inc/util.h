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

#ifndef UTIL_H_
#define UTIL_H_

#ifndef __MODULE__
#define __MODULE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define LOG_(prio, tag, fmt, arg...) \
    ({ do { \
        dlog_print(prio, tag, "%s: %s(%d) > " fmt, __MODULE__, __func__, __LINE__, ##arg);\
    } while (0); })

#define LOGD(format, arg...) LOG_(DLOG_DEBUG, LOG_TAG, format, ##arg)
#define LOGI(format, arg...) LOG_(DLOG_INFO, LOG_TAG, format, ##arg)
#define LOGW(format, arg...) LOG_(DLOG_WARN, LOG_TAG, format, ##arg)
#define LOGE(format, arg...) LOG_(DLOG_ERROR, LOG_TAG, format, ##arg)
#define LOGF(format, arg...) LOG_(DLOG_FATAL, LOG_TAG, format, ##arg)

#endif /* UTIL_H_ */
