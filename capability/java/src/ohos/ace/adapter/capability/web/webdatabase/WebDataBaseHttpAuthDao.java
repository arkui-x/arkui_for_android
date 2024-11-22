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

import java.util.ArrayList;
import java.util.List;

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

    public long insert(WebDataBaseHttpAuth httpAuth) {
        ContentValues httpAuthValues = new ContentValues();
        httpAuthValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_HOST, httpAuth.getHost());
        httpAuthValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_REALM, httpAuth.getRealm());

        return dataBaseHelper.getWritableDatabase().insert(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH, null, httpAuthValues);
    };

    public long delete(WebDataBaseHttpAuth httpAuth) {
        String whereClause = WebDataBaseColumns.EntryColumns._ID + " = ?";
        String[] whereArgs = {httpAuth.getId().toString()};

        return dataBaseHelper.getWritableDatabase().delete(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH, whereClause, whereArgs);
    }

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