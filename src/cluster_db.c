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

#include <app.h>
#include <sqlite3.h>

#include "cluster_db.h"
#include "cluster_data.h"
#include "util.h"

#define CLUSTER_DB_NAME ".cluster-data.db"
#define QUERY_MAXLEN	4096

static sqlite3 *cluster_db = NULL;

enum {
	COL_WIDGET_ID = 0,
	COL_PAGE_INDEX,
	COL_POS_Y,
	COL_POS_X,
	COL_PKG_NAME,
	COL_CONTENT_INFO,
	COL_TYPE,
	COL_PERIOD,
	COL_ALLOW_DUPLICATE
};

#define CREATE_CLUSTER_DB_TABLE "CREATE TABLE IF NOT EXISTS clusters (\
		widgetId INTEGER PRIMARY KEY AUTOINCREMENT,\
		pageIndex INTEGER default 0,\
		y INTEGER default 0,\
		x INTEGER default 0,\
		pkgName TEXT,\
		content TEXT,\
		type INTEGER default 0,\
		period REAL default 0.0,\
		allow INTEGER default 1);"

#define UPDATE_CLUSTER_DB_TABLE "UPDATE clusters set \
		pageIndex=%d,\
		y=%d,\
		x=%d,\
		pkgName='%s',\
		content='%s',\
		type=%d,\
		period=%lf,\
		allow=%d WHERE widgetId=%d"

#define INSERT_CLUSTER_DB_TABLE "INSERT into clusters (\
		pageIndex,\
		y,\
		x,\
		pkgName,\
		content,\
		type,\
		period,\
		allow) VALUES(%d,%d, %d,'%s','%s',%d, '%lf',%d)"

#define SELECT_ITEM "SELECT * FROM clusters;"

static bool __cluster_db_open(void);

bool cluster_db_create(void)
{
	char *errMsg;
	int ret;
	const char *db_path = util_get_data_file_path(CLUSTER_DB_NAME);
	FILE *fp = fopen(db_path, "r");
	if (fp) {
		fclose(fp);
		LOGE("cluster DB[%s] exist", db_path);
		return false;
	}

	ret = sqlite3_open(db_path, &cluster_db);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%d] : path [%s]", ret, db_path);
		return false;
	}

	ret = sqlite3_exec(cluster_db, "PRAGMA journal_mode = PERSIST", NULL, NULL, &errMsg);
	if (ret != SQLITE_OK) {
		LOGE("SQL error(%d) : %s", ret, errMsg);
		sqlite3_free(errMsg);
		return false;
	}

	ret = sqlite3_exec(cluster_db, CREATE_CLUSTER_DB_TABLE, NULL, NULL, &errMsg);
	if (ret != SQLITE_OK) {
		LOGE("SQL error(%d) : %s", ret, errMsg);
		sqlite3_free(errMsg);
		return false;
	}
	LOGD("Create DB[%s] : [%s] OK ", db_path, CREATE_CLUSTER_DB_TABLE);
	return true;
}

bool cluster_db_close()
{
	if (cluster_db) {
		sqlite3_exec(cluster_db, "COMMIT TRANSACTION", NULL, NULL, NULL);
		sqlite3_close(cluster_db);
		cluster_db = NULL;
	}
	return true;
}

bool cluster_db_get_list(Eina_List **cluster_list)
{
	sqlite3_stmt *stmt;
	if (!__cluster_db_open())
		return false;
	int ret = sqlite3_prepare_v2(cluster_db, SELECT_ITEM, strlen(SELECT_ITEM), &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%s,%s]", SELECT_ITEM, sqlite3_errmsg(cluster_db));
		return false;
	}

	const char *str;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		widget_data_t *item = (widget_data_t *)malloc(sizeof(widget_data_t));
		memset(item, 0, sizeof(widget_data_t));

		item->widget_id = sqlite3_column_int(stmt, COL_WIDGET_ID);
		item->page_idx = sqlite3_column_int(stmt, COL_PAGE_INDEX);
		item->pos_y = sqlite3_column_int(stmt, COL_POS_Y);
		item->pos_x = sqlite3_column_int(stmt, COL_POS_X);
		str = (const char *) sqlite3_column_text(stmt, COL_PKG_NAME);
		item->pkg_name = (!str || !strlen(str)) ? NULL : strdup(str);
		str = (const char *) sqlite3_column_text(stmt, COL_CONTENT_INFO);
		item->content_info = (!str || !strlen(str)) ? NULL : strdup(str);
		item->type = sqlite3_column_int(stmt, COL_TYPE);
		item->period = sqlite3_column_double(stmt, COL_PERIOD);
		item->allow_duplicate = sqlite3_column_int(stmt, COL_ALLOW_DUPLICATE);
		*cluster_list = eina_list_append(*cluster_list, item);
	}
	sqlite3_finalize(stmt);
	return true;
}

bool cluster_db_update(widget_data_t *item)
{
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt;
	if (!__cluster_db_open())
		return false;

	snprintf(query, QUERY_MAXLEN, UPDATE_CLUSTER_DB_TABLE,
			item->page_idx,
			item->pos_y,
			item->pos_x,
			item->pkg_name,
			item->content_info,
			item->type,
			item->period,
			item->allow_duplicate,
			item->widget_id);
	int ret = sqlite3_prepare(cluster_db, query, QUERY_MAXLEN , &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(cluster_db));
		return false;
	}
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	cluster_db_close();
	return true;
}

bool cluster_db_insert(widget_data_t *item)
{
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt;
	if (!__cluster_db_open())
		return false;
	snprintf(query, QUERY_MAXLEN, INSERT_CLUSTER_DB_TABLE,
			item->page_idx,
			item->pos_y,
			item->pos_x,
			item->pkg_name,
			item->content_info,
			item->type,
			item->period,
			item->allow_duplicate);
	int ret = sqlite3_prepare(cluster_db, query, QUERY_MAXLEN , &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(cluster_db));
		return false;
	}
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	item->widget_id = (int)sqlite3_last_insert_rowid(cluster_db);

	cluster_db_close();
	return true;
}


bool cluster_db_delete(widget_data_t *item)
{
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt;
	if (!__cluster_db_open())
		return false;

	snprintf(query, QUERY_MAXLEN, "DELETE FROM clusters WHERE widgetId=%d", item->widget_id);
	int ret = sqlite3_prepare(cluster_db, query, QUERY_MAXLEN , &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(cluster_db));
		return false;
	}

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	cluster_db_close();
	return true;
}

bool __cluster_db_open()
{
	if (!cluster_db) {
		int ret;
		ret = sqlite3_open(util_get_data_file_path(CLUSTER_DB_NAME), &cluster_db);
		if (ret != SQLITE_OK) {
			LOGE("sqlite error : [%d] : path [%s]", ret, util_get_data_file_path(CLUSTER_DB_NAME));
			return false;
		}
	}
	return true;
}
