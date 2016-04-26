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

#include "apps_db.h"
#include "apps_data.h"
#include "util.h"


#define APPS_DB_NAME ".apps-data.db"
#define QUERY_MAXLEN    4096

static sqlite3 *apps_db = NULL;

#define CREATE_APPS_DB_TABLE "create table if not exists apps(\
        appId INTEGER PRIMARY KEY AUTOINCREMENT,\
        parentId    INTEGER, \
        owner      TEXT,\
        isFolder    INTEGER, \
        position    INTEGER, \
        pkgStr      TEXT, \
        label       TEXT,\
        isSystem    INTEGER, \
        isRemovable INTEGER );"

#define SELECT_ITEM "SELECT * FROM apps;"


static bool __apps_db_open(void);

bool apps_db_create(void)
{
    char *errMsg;
    int ret;
    const char *db_path = util_get_data_file_path(APPS_DB_NAME);
    FILE *fp = fopen(db_path, "r");
    if (fp) {
        fclose(fp);
        LOGE("Apps DB[%s] exist", db_path);
        return false;
    }

    ret = sqlite3_open(db_path, &apps_db);
    if (ret != SQLITE_OK) {
        LOGE("sqlite error : [%d] : path [%s]", ret, db_path);
        return false;
    }

    ret = sqlite3_exec(apps_db, "PRAGMA journal_mode = PERSIST",
        NULL, NULL, &errMsg);
    if (ret != SQLITE_OK) {
        LOGE("SQL error(%d) : %s", ret, errMsg);
        sqlite3_free(errMsg);
        return false;
    }

    ret = sqlite3_exec(apps_db, CREATE_APPS_DB_TABLE, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK) {
        LOGE("SQL error(%d) : %s", ret, errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    LOGD("Create DB[%s] : [%s] OK ", db_path, CREATE_APPS_DB_TABLE);
    return true;
}

bool apps_db_close()
{
    if (apps_db) {
        sqlite3_exec(apps_db, "COMMIT TRANSACTION", NULL, NULL, NULL);
        sqlite3_close(apps_db);
        apps_db = NULL;
    }
    return true;
}

bool apps_db_get_list(Eina_List **apps)
{
    sqlite3_stmt *stmt;

    if (!__apps_db_open())
        return false;

    int ret = sqlite3_prepare_v2(apps_db, SELECT_ITEM, strlen(SELECT_ITEM), &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOGE("sqlite error : [%s,%s]", SELECT_ITEM, sqlite3_errmsg(apps_db));
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        app_data_t *item = (app_data_t *)malloc(sizeof(app_data_t));
        memset(item, 0, sizeof(app_data_t));

        item->db_id = sqlite3_column_int(stmt, 0);
        item->parent_db_id = sqlite3_column_int(stmt, 1);
        item->owner =  strdup((const char *) sqlite3_column_text(stmt, 2));
        item->is_folder = sqlite3_column_int(stmt, 3);
        item->position = sqlite3_column_int(stmt, 4);
        item->pkg_str = strdup((const char *) sqlite3_column_text(
            stmt, 5));
        item->label_str = strdup((const char *) sqlite3_column_text(
            stmt, 6));
        item->is_system = sqlite3_column_int(stmt, 7);
        item->is_removable = sqlite3_column_int(stmt, 8);

        item->icon_path_str = NULL;
        *apps = eina_list_append(*apps, item);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool apps_db_update(app_data_t *item)
{
    char query[QUERY_MAXLEN];
    sqlite3_stmt *stmt;
    if (!__apps_db_open())
        return false;
    snprintf(query, QUERY_MAXLEN, "UPDATE apps set parentId=%d, isFolder=%d, position=%d, pkgStr='%s', label='%s', isSystem=%d, isRemovable=%d WHERE appId = %d",
            item->parent_db_id,
            item->is_folder,
            item->position,
            item->pkg_str,
            item->label_str,
            item->is_system,
            item->is_removable,
            item->db_id);
    int ret = sqlite3_prepare(apps_db, query, QUERY_MAXLEN , &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(apps_db));
        return false;
    }
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    apps_db_close();
    return true;
}

bool apps_db_insert(app_data_t *item)
{
    char query[QUERY_MAXLEN];
    sqlite3_stmt *stmt;
    if (!__apps_db_open())
        return false;

    snprintf(query, QUERY_MAXLEN, "INSERT into apps ( parentId, isFolder, owner, position, pkgStr, label, isSystem, isRemovable) VALUES(%d,%d,'%s',%d,'%s','%s',%d,%d)",
            item->parent_db_id,
            item->is_folder,
            item->owner,
            item->position,
            item->pkg_str,
            item->label_str,
            item->is_system,
            item->is_removable);
    int ret = sqlite3_prepare(apps_db, query, QUERY_MAXLEN , &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(apps_db));
        return false;
    }
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    item->db_id = (int)sqlite3_last_insert_rowid(apps_db);

    apps_db_close();
    return true;
}


bool apps_db_delete(app_data_t *item)
{
    char query[QUERY_MAXLEN];
    sqlite3_stmt *stmt;
    if (!__apps_db_open())
        return false;

    snprintf(query, QUERY_MAXLEN, "DELETE FROM apps WHERE appId=%d", item->db_id);
    int ret = sqlite3_prepare(apps_db, query, QUERY_MAXLEN , &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(apps_db));
        return false;
    }

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    apps_db_close();
    return true;
}

bool __apps_db_open()
{
    if (!apps_db) {
        int ret;
        ret = sqlite3_open(util_get_data_file_path(APPS_DB_NAME), &apps_db);
        if (ret != SQLITE_OK) {
            LOGE("sqlite error : [%d] : path [%s]", ret, util_get_data_file_path(APPS_DB_NAME));
            return false;
        }
    }
    return true;
}
