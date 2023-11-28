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

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class WebDataBaseHelper extends SQLiteOpenHelper {
    private static final String SQL_CREATE_HTTPAUTH_TABLE =
        "CREATE TABLE " + WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH + " (" +
            WebDataBaseColumns.EntryColumns._ID + " INTEGER PRIMARY KEY," +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_HOST + " TEXT NOT NULL," +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_REALM + " TEXT," +
            "UNIQUE(" + WebDataBaseColumns.EntryColumns.COLUMN_NAME_HOST + ", " +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_REALM +
            ")" +
        ");";

    private static final String SQL_CREATE_CREDENTIAL_TABLE =
        "CREATE TABLE " + WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL + " (" +
            WebDataBaseColumns.EntryColumns._ID + " INTEGER PRIMARY KEY," +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERNAME + " TEXT NOT NULL," +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERPASS + " TEXT NOT NULL," +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_HTTP_AUTH_ID + " INTEGER NOT NULL," +
            "UNIQUE(" + WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERNAME + ", " +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERPASS + ", " +
            WebDataBaseColumns.EntryColumns.COLUMN_NAME_HTTP_AUTH_ID + ")," +
            "FOREIGN KEY (" + WebDataBaseColumns.EntryColumns.COLUMN_NAME_HTTP_AUTH_ID + ") REFERENCES " +
            WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH + " (" + WebDataBaseColumns.EntryColumns._ID +
            ") ON DELETE CASCADE" +
        ");";

    private static final String SQL_DELETE_HTTP_AUTH_TABLE =
        "DROP TABLE IF EXISTS " + WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH;

    private static final String SQL_DELETE_CREDENTIAL_TABLE =
        "DROP TABLE IF EXISTS " + WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL;

    public WebDataBaseHelper(Context context) {
        super(context, "AceWebDatabase.db", null, 2);
    }

    public void onCreate(SQLiteDatabase db) {
        db.execSQL(SQL_CREATE_HTTPAUTH_TABLE);
        db.execSQL(SQL_CREATE_CREDENTIAL_TABLE);
    }

    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL(SQL_DELETE_HTTP_AUTH_TABLE);
        db.execSQL(SQL_DELETE_CREDENTIAL_TABLE);
        onCreate(db);
    }

    public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        onUpgrade(db, oldVersion, newVersion);
    }

    public void clearAllTables(SQLiteDatabase db) {
        db.execSQL(SQL_DELETE_HTTP_AUTH_TABLE);
        db.execSQL(SQL_DELETE_CREDENTIAL_TABLE);
        onCreate(db);
    }
}
