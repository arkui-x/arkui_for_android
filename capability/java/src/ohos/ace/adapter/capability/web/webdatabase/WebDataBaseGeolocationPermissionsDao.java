/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import java.util.ArrayList;
import java.util.List;

import android.content.ContentValues;
import android.database.Cursor;

/**
 * The WebDataBaseGeolocationPermissionsDao class is used to handle database
 * operations on Web component geolocation permission management objects.
 *
 * @since 1
 */
public class WebDataBaseGeolocationPermissionsDao {
    private static final String LOG_TAG = "WebDataBaseGeolocationPermissionsDao";

    String[] columns = {
        WebDataBaseColumns.EntryColumns._ID,
        WebDataBaseColumns.EntryColumns.COLUMN_NAME_ORIGIN,
        WebDataBaseColumns.EntryColumns.COLUMN_NAME_RESULT
    };

    private WebDataBaseHelper dataBaseHelper;

    /**
     * constructor of WebDataBaseGeolocationPermissionsDao
     *
     * @param dataBaseHelper An instance of the database helper class.
     */
    public WebDataBaseGeolocationPermissionsDao(WebDataBaseHelper dataBaseHelper) {
        this.dataBaseHelper = dataBaseHelper;
    }

    /**
     * Insert geolocation permission information into the database.
     *
     * @param geolocationPermissions WebDataBaseGeolocationPermissions object, including origin and result information.
     * @return The number of rows affected by the insertion operation
     */
    public long insertPermissionByOrigin(WebDataBaseGeolocationPermissions geolocationPermissions) {
        ContentValues credentialValues = new ContentValues();
        credentialValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_ORIGIN, geolocationPermissions.getOrigin());
        credentialValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_RESULT, geolocationPermissions.getResult());
        dataBaseHelper.isTableExists(dataBaseHelper.getReadableDatabase(),
                WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION);
        return dataBaseHelper.getWritableDatabase().insert(WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION, null,
                credentialValues);
    }

    /**
     * Clear geolocation permission information from the database based on the origin.
     *
     * @param origin Url source.
     * @return The number of rows affected by the clearing operation
     */
    public int clearPermissionByOrigin(String origin) {
        String whereClause = WebDataBaseColumns.EntryColumns.COLUMN_NAME_ORIGIN + " = ?";
        String[] whereArgs = {origin};
        dataBaseHelper.isTableExists(dataBaseHelper.getReadableDatabase(),
                WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION);
        return dataBaseHelper.getWritableDatabase().delete(WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION,
                whereClause, whereArgs);
    }

    /**
     * Clear all geolocation permission information from the database
     *
     * @return The number of rows affected by the clearing operation
     */
    public int clearAllPermission() {
        dataBaseHelper.isTableExists(dataBaseHelper.getReadableDatabase(),
                WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION);
        return dataBaseHelper.getWritableDatabase().delete(WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION, null,
                null);
    }

    /**
     * Retrieve geolocation permission results from the database based on the URL.
     *
     * @param origin Url source.
     * @return WebDataBaseGeolocationPermissions object containing permission information
     */
    public WebDataBaseGeolocationPermissions getPermissionResultByOrigin(String origin) {
        String selection = WebDataBaseColumns.EntryColumns.COLUMN_NAME_ORIGIN + " = ?";
        String[] selectionArgs = {origin};
        dataBaseHelper.isTableExists(dataBaseHelper.getReadableDatabase(),
                WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION);
        Cursor cursor = dataBaseHelper.getReadableDatabase().query(
                WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION, columns, selection, selectionArgs, null, null,
                null);
        try {
            if (!cursor.moveToFirst()) {
                return null;
            }
            Long id = cursor.getLong(cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns._ID));
            String returnOrigin = cursor
                    .getString(cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_ORIGIN));
            Integer result = cursor
                    .getInt(cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_RESULT));
            return new WebDataBaseGeolocationPermissions(id, returnOrigin, result);
        } finally {
            cursor.close();
        }
    }

    /**
     * Retrieve all authorized url source information from the database.
     *
     * @return List of WebDataBaseGeolocationPermissions objects containing geolocation permission information.
     */
    public List<WebDataBaseGeolocationPermissions> getOriginsByPermission() {
        dataBaseHelper.isTableExists(dataBaseHelper.getReadableDatabase(),
                WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION);
        Cursor cursor = dataBaseHelper.getReadableDatabase()
                .query(WebDataBaseColumns.EntryColumns.TABLE_NAME_GEOLOCATION, columns, null, null, null, null, null);

        try {
            if (!cursor.moveToFirst()) {
                return new ArrayList<>();
            }
            List<WebDataBaseGeolocationPermissions> credentials = new ArrayList<>(cursor.getCount());
            do {
                Long id = cursor.getLong(cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns._ID));
                String origin = cursor
                        .getString(cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_ORIGIN));
                Integer result = cursor
                        .getInt(cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_RESULT));
                credentials.add(new WebDataBaseGeolocationPermissions(id, origin, result));
            } while (cursor.moveToNext());
            return credentials;
        } finally {
            cursor.close();
        }
    }
}