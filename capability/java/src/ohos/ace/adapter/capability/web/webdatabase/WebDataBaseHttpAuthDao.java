/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ohos.ace.adapter.capability.web;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

/**
 * WebDataBaseHttpAuthDao class is used to operate the WebDataBaseHttpAuth table in the database.
 *
 * @since 2024-05-31
 */
public class WebDataBaseHttpAuthDao {
    WebDataBaseHelper dataBaseHelper;
    String[] columns = {
        WebDataBaseColumns.EntryColumns._ID,
        WebDataBaseColumns.EntryColumns.COLUMN_NAME_HOST,
        WebDataBaseColumns.EntryColumns.COLUMN_NAME_REALM
    };

    public WebDataBaseHttpAuthDao(WebDataBaseHelper dataBaseHelper) {
        this.dataBaseHelper = dataBaseHelper;
    }

    /**
     * Insert a new WebDataBaseHttpAuth into the database.
     *
     * @param httpAuth the WebDataBaseHttpAuth to insert
     * @return the row ID of the newly inserted row, or -1 if an error occurred
     */
    public long insert(WebDataBaseHttpAuth httpAuth) {
        ContentValues httpAuthValues = new ContentValues();
        httpAuthValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_HOST, httpAuth.getHost());
        httpAuthValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_REALM, httpAuth.getRealm());

        return dataBaseHelper.getWritableDatabase().insert(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH, null, httpAuthValues);
    };

    /**
     * Delete a WebDataBaseHttpAuth from the database.
     *
     * @param httpAuth the WebDataBaseHttpAuth to delete
     * @return the number of rows affected if a whereClause is passed in, 0 otherwise
     */
    public long delete(WebDataBaseHttpAuth httpAuth) {
        String whereClause = WebDataBaseColumns.EntryColumns._ID + " = ?";
        String[] whereArgs = {httpAuth.getId().toString()};

        return dataBaseHelper.getWritableDatabase().delete(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH, whereClause, whereArgs);
    }

    /**
     * Find a WebDataBaseHttpAuth from the database.
     *
     * @param host the host of the WebDataBaseHttpAuth
     * @param realm the realm of the WebDataBaseHttpAuth
     * @return the WebDataBaseHttpAuth if found, null otherwise
     */
    public WebDataBaseHttpAuth find(String host, String realm) {
        SQLiteDatabase readableDatabase = dataBaseHelper.getReadableDatabase();

        String selection = WebDataBaseColumns.EntryColumns.COLUMN_NAME_HOST + " = ? AND " +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_REALM + " = ?";
        String[] selectionArgs = {host, realm};

        Cursor cursor = readableDatabase.query(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH,
            columns,
            selection,
            selectionArgs,
            null,
            null,
            null
        );

        WebDataBaseHttpAuth httpAuth = null;
        if (cursor.moveToNext()) {
            Long rowId = cursor.getLong(cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns._ID));
            String rowHost = cursor.getString(
                cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_HOST));
            String rowRealm = cursor.getString(
                cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_REALM));
            httpAuth = new WebDataBaseHttpAuth(rowId, rowHost, rowRealm);
        }
        cursor.close();

        return httpAuth;
    }
}