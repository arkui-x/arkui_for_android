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
import android.database.Cursor;

import ohos.ace.adapter.ALog;

/**
 * Web database helper class for managing database creation and version upgrades.
 *
 * @since 1
 */
public class WebDataBaseHelper extends SQLiteOpenHelper {
    private static final String LOG_TAG = "WebDataBaseHelper";

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

    private static final String SQL_CREATE_GEOLOCATION_TABLE = "CREATE TEMPORARY  TABLE IF NOT EXISTS "
            + WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION + " (" + WebDataBaseColumns.EntryColumns._ID
            + " INTEGER PRIMARY KEY," + WebDataBaseColumns.EntryColumns.COLUMN_NAME_ORIGIN + " TEXT,"
            + WebDataBaseColumns.EntryColumns.COLUMN_NAME_RESULT + " INTEGER," + "UNIQUE("
            + WebDataBaseColumns.EntryColumns.COLUMN_NAME_ORIGIN + ") ON CONFLICT REPLACE" + ");";

    private static final String SQL_SELECT_TABLE_FROM_TABLENAME =
        "SELECT name FROM sqlite_master  WHERE type='table' AND name=? ";

    private static final String SQL_DELETE_HTTP_AUTH_TABLE =
        "DROP TABLE IF EXISTS " + WebDataBaseColumns.EntryColumns.TABLE_NAME_HTTPAUTH;

    private static final String SQL_DELETE_CREDENTIAL_TABLE =
        "DROP TABLE IF EXISTS " + WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL;

    private static final String SQL_DELETE_GEOLOCATION_TABLE =
        "DROP TABLE IF EXISTS " + WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION;

    public WebDataBaseHelper(Context context) {
        super(context, "AceWebDatabase.db", null, 2);
    }

    /**
     * This method is used to create the database tables.
     *
     * @param db SQLite database object
     */
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(SQL_CREATE_HTTPAUTH_TABLE);
        db.execSQL(SQL_CREATE_CREDENTIAL_TABLE);
    }

    /**
     * This method is used to upgrade the database tables.
     *
     * @param db SQLite database object
     * @param oldVersion The old version of the database
     * @param newVersion The new version of the database
     */
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL(SQL_DELETE_HTTP_AUTH_TABLE);
        db.execSQL(SQL_DELETE_CREDENTIAL_TABLE);
        db.execSQL(SQL_DELETE_GEOLOCATION_TABLE);
        onCreate(db);
    }

    /**
     * This method is used to downgrade the database tables.
     *
     * @param db SQLite database object
     * @param oldVersion The old version of the database
     * @param newVersion The new version of the database
     */
    public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        onUpgrade(db, oldVersion, newVersion);
    }

    /**
     * This method is used to clear all the tables in the database.
     *
     * @param db SQLite database object
     */
    public void clearAllTables(SQLiteDatabase db) {
        db.execSQL(SQL_DELETE_HTTP_AUTH_TABLE);
        db.execSQL(SQL_DELETE_CREDENTIAL_TABLE);
        db.execSQL(SQL_DELETE_GEOLOCATION_TABLE);
        onCreate(db);
    }

    /**
     * This method is used to check whether a table with the specified name exists in the database.
     *
     * @param db  SQLite database object
     * @param tableName The name of the table to check if it exists.
     */
    public void isTableExists(SQLiteDatabase db, String tableName) {
        if (WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION.equals(tableName)) {
            Cursor cursor = null;
            try {
                cursor = db.rawQuery(SQL_SELECT_TABLE_FROM_TABLENAME, new String[]{tableName});
                if (!cursor.moveToFirst()) {
                    db.execSQL(SQL_CREATE_GEOLOCATION_TABLE);
                }
            } catch (Exception e) {
                ALog.i(LOG_TAG, "Error occurred during table existence check: " + e.getMessage());
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
    }
}
