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

#include <sqlite3.h>

#include "homescreen-efl.h"
#include "conf.h"
#include "util.h"
#include "db/db.h"

static struct {
	sqlite3 *db;
	Ecore_Timer *close_timer;
} db_info = {
	.db = NULL,
	.close_timer = NULL,
};

#define CREATE_TABLE_ITEM "CREATE TABLE IF NOT EXISTS item(\
id INTEGER PRIMARY KEY NOT NULL,\
type INTEGER,\
pkg_id TEXT,\
first INTEGER,\
next INTEGER,\
x INTEGER,\
y INTEGER,\
w INTEGER,\
h INTEGER,\
content_info TEXT);"

#define SELECT_ITEM "SELECT * FROM item;"
#define INSERT_ITEM "INSERT OR REPLACE INTO item VALUES(?,?,?,?,?,?,?,?,?,?);"
#define CLEAR_TABLE "DELETE FROM item;"
#define DELETE_ITEM "DELETE FROM item WHERE id = ?;"

#define DATABASE_CLOSE_TIME 5

bool _db_is_success(int return_code);
bool _db_open(void);
void _db_close(void);
bool _db_prepare(const char *query, sqlite3_stmt **stmt);
bool _db_step(sqlite3_stmt *stmt);
bool _db_finalize(sqlite3_stmt *stmt);

bool db_create_tables(void)
{
	if (!_db_open())
		return false;

	sqlite3_exec(db_info.db, CLEAR_TABLE, NULL, NULL, NULL);

	if (sqlite3_exec(db_info.db, CREATE_TABLE_ITEM, NULL, NULL, NULL))
		return false;

	db_item_t root = {0, APP_ITEM_ROOT, "", -1, -1, 0, 0, 0, 0};
	if (!db_update_apps(root, true))
		return false;

	_db_close();

	return true;
}

bool db_get_apps(Eina_List **apps)
{
	const char *content_info = NULL;
	sqlite3_stmt *stmt;
	if (!_db_open())
		return false;

	if (!_db_prepare(SELECT_ITEM, &stmt))
		return false;

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		db_item_t *item = (db_item_t *) malloc(sizeof(db_item_t));
		item->id = sqlite3_column_int(stmt, 0);
		item->type = sqlite3_column_int(stmt, 1);
		item->appid = strdup((const char *) sqlite3_column_text(
			stmt, 2));
		item->first_id = sqlite3_column_int(stmt, 3);
		item->next_id = sqlite3_column_int(stmt, 4);

		item->x = sqlite3_column_int(stmt, 5);
		item->y = sqlite3_column_int(stmt, 6);
		item->w = sqlite3_column_int(stmt, 7);
		item->h = sqlite3_column_int(stmt, 8);


		content_info = (const char *)sqlite3_column_text(stmt, 9);

		if (content_info) {
			item->content_info = strdup(content_info);
		} else {
			item->content_info = NULL;
		}

		*apps = eina_list_append(*apps, item);
		dlog_print(DLOG_DEBUG, LOG_TAG, "App: %s [%s]", item->appid, item->content_info );
	}

	if (!_db_finalize(stmt))
		return false;

	_db_close();

	return true;
}

void db_free_apps(Eina_List *apps)
{
	db_item_t *db_item;
	EINA_LIST_FREE(apps, db_item) {
		free(db_item->appid);
		free(db_item);
	}
}

bool db_update_apps(db_item_t item, bool insert)
{
	sqlite3_stmt *stmt;

	if (!_db_open())
		return false;

	if (insert) {
		if (!_db_prepare(INSERT_ITEM, &stmt)) {
			LOGD("_db_prepare() failed");
			return false;
		}

		sqlite3_bind_int(stmt, 1, item.id);
		sqlite3_bind_int(stmt, 2, item.type);
		sqlite3_bind_text(stmt, 3, item.appid, -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, item.first_id);
		sqlite3_bind_int(stmt, 5, item.next_id);
		sqlite3_bind_int(stmt, 6, item.x);
		sqlite3_bind_int(stmt, 7, item.y);
		sqlite3_bind_int(stmt, 8, item.w);
		sqlite3_bind_int(stmt, 9, item.h);
		sqlite3_bind_text(stmt, 10, item.content_info, -1, SQLITE_STATIC);
	} else {
		if (!_db_prepare(DELETE_ITEM, &stmt)) {
			LOGD("_db_prepare() failed");
			return false;
		}

		sqlite3_bind_int(stmt, 1, item.id);
	}


	if (!_db_step(stmt))
		return false;

	if (!_db_finalize(stmt))
		return false;

	_db_close();

	return true;
}

bool _db_is_success(int return_code)
{
	switch (return_code) {
	case SQLITE_OK:
		LOGD("RETURN CODE: SQLITE_OK");
		break;
	case SQLITE_ERROR:
		LOGD("RETURN CODE: SQLITE_ERROR");
		break;
	case SQLITE_INTERNAL:
		LOGD("RETURN CODE: SQLITE_INTERNAL");
		break;
	case SQLITE_PERM:
		LOGD("RETURN CODE: SQLITE_PERM");
		break;
	case SQLITE_ABORT:
		LOGD("RETURN CODE: SQLITE_ABORT");
		break;
	case SQLITE_BUSY:
		LOGD("RETURN CODE: SQLITE_BUSY");
		break;
	case SQLITE_LOCKED:
		LOGD("RETURN CODE: SQLITE_LOCKED");
		break;
	case SQLITE_NOMEM:
		LOGD("RETURN CODE: SQLITE_NOMEM");
		break;
	case SQLITE_READONLY:
		LOGD("RETURN CODE: SQLITE_READONLY");
		break;
	case SQLITE_INTERRUPT:
		LOGD("RETURN CODE: SQLITE_INTERRUPT");
		break;
	case SQLITE_IOERR:
		LOGD("RETURN CODE: SQLITE_IOERR");
		break;
	case SQLITE_CORRUPT:
		LOGD("RETURN CODE: SQLITE_CORRUPT");
		break;
	case SQLITE_NOTFOUND:
		LOGD("RETURN CODE: SQLITE_NOTFOUND");
		break;
	case SQLITE_FULL:
		LOGD("RETURN CODE: SQLITE_FULL");
		break;
	case SQLITE_CANTOPEN:
		LOGD("RETURN CODE: SQLITE_CANTOPEN");
		break;
	case SQLITE_PROTOCOL:
		LOGD("RETURN CODE: SQLITE_PROTOCOL");
		break;
	case SQLITE_EMPTY:
		LOGD("RETURN CODE: SQLITE_EMPTY");
		break;
	case SQLITE_SCHEMA:
		LOGD("RETURN CODE: SQLITE_SCHEMA");
		break;
	case SQLITE_TOOBIG:
		LOGD("RETURN CODE: SQLITE_TOOBIG");
		break;
	case SQLITE_CONSTRAINT:
		LOGD("RETURN CODE: SQLITE_CONSTRAINT");
		break;
	case SQLITE_MISMATCH:
		LOGD("RETURN CODE: SQLITE_MISMATCH");
		break;
	case SQLITE_MISUSE:
		LOGD("RETURN CODE: SQLITE_MISUSE");
		break;
	case SQLITE_NOLFS:
		LOGD("RETURN CODE: SQLITE_NOLFS");
		break;
	case SQLITE_AUTH:
		LOGD("RETURN CODE: SQLITE_AUTH");
		break;
	case SQLITE_FORMAT:
		LOGD("RETURN CODE: SQLITE_FORMAT");
		break;
	case SQLITE_RANGE:
		LOGD("RETURN CODE: SQLITE_RANGE");
		break;
	case SQLITE_NOTADB:
		LOGD("RETURN CODE: SQLITE_NOTADB");
		break;
	case SQLITE_ROW:
		LOGD("RETURN CODE: SQLITE_ROW");
		break;
	case SQLITE_DONE:
		LOGD("RETURN CODE: SQLITE_DONE");
		break;
	default:
		break;
	}

	return return_code == SQLITE_OK || return_code == SQLITE_DONE;
}

bool _db_open(void)
{
	if (db_info.db)
		return true;

	if (!_db_is_success(sqlite3_open(util_get_data_file_path(DATABASE_FILE), &db_info.db)))
		return false;

	if (sqlite3_exec(db_info.db, "BEGIN IMMEDIATE TRANSACTION", NULL,
		NULL, NULL))
		return false;

	return true;
}

Eina_Bool _db_close_cb(void *data)
{
	LOGD("DB CLOSE");

	if (db_info.db) {
		if (sqlite3_exec(db_info.db, "COMMIT TRANSACTION",
			NULL, NULL, NULL))
			return ECORE_CALLBACK_DONE;

		if (_db_is_success(sqlite3_close(db_info.db)))
			db_info.db = NULL;
	}
	db_info.close_timer = NULL;
	return ECORE_CALLBACK_DONE;
}

void _db_close(void)
{
	if (!db_info.db)
		return;

	if (db_info.close_timer) {
		ecore_timer_reset(db_info.close_timer);
	} else {
		db_info.close_timer = ecore_timer_add(DATABASE_CLOSE_TIME,
					_db_close_cb, NULL);
	}
}

bool _db_prepare(const char *query, sqlite3_stmt **stmt)
{
	return _db_is_success(sqlite3_prepare_v2(db_info.db, query,
		strlen(query), stmt, NULL));
}

bool _db_step(sqlite3_stmt *stmt)
{
	return _db_is_success(sqlite3_step(stmt));
}

bool _db_finalize(sqlite3_stmt *stmt)
{
	return _db_is_success(sqlite3_finalize(stmt));
}
