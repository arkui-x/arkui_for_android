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

import java.util.ArrayList;
import java.util.List;

public class WebDataBaseCredentialDao {
    WebDataBaseHelper dataBaseHelper;
    String[] columns = {
        WebDataBaseColumns.EntryColumns._ID,
        WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERNAME,
        WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERPASS,
        WebDataBaseColumns.EntryColumns.COLUMN_NAME_HTTP_AUTH_ID
    };

    public WebDataBaseCredentialDao(WebDataBaseHelper dataBaseHelper) {
        this.dataBaseHelper = dataBaseHelper;
    }

    public List<WebDataBaseCredential> getAllByHttpAuthId(Long httpAuthId) {
        String selection = WebDataBaseColumns.EntryColumns.COLUMN_NAME_HTTP_AUTH_ID + " = ?";
        String[] selectionArgs = {httpAuthId.toString()};

        Cursor cursor = dataBaseHelper.getReadableDatabase().query(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL,
            columns,
            selection,
            selectionArgs,
            null,
            null,
            null
        );

        List<WebDataBaseCredential> credentials = new ArrayList<>();
        while (cursor.moveToNext()) {
            Long id = cursor.getLong(
                cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns._ID));
            String username = cursor.getString(
                cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERNAME));
            String password = cursor.getString(
                cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERPASS));
            credentials.add(new WebDataBaseCredential(id, username, password, httpAuthId));
        }
        cursor.close();

        return credentials;
    }

    public WebDataBaseCredential find(String username, String password, Long httpAuthId) {
      String selection = WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERNAME + " = ? AND " +
          WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERPASS + " = ? AND " +
          WebDataBaseColumns.EntryColumns.COLUMN_NAME_HTTP_AUTH_ID + " = ?";
      String[] selectionArgs = {username, password, httpAuthId.toString()};

      Cursor cursor = dataBaseHelper.getReadableDatabase().query(
          WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL,
          columns,
          selection,
          selectionArgs,
          null,
          null,
          null
      );

      WebDataBaseCredential credential = null;
      if (cursor.moveToNext()) {
          Long rowId = cursor.getLong(cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns._ID));
          String rowUsername = cursor.getString(
              cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERNAME));
          String rowPassword = cursor.getString(
              cursor.getColumnIndexOrThrow(WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERPASS));
          credential = new WebDataBaseCredential(rowId, rowUsername, rowPassword, httpAuthId);
      }
      cursor.close();

      return credential;
    }

    public long insert(WebDataBaseCredential credential) {
        ContentValues credentialValues = new ContentValues();
        credentialValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERNAME, credential.getUsername());
        credentialValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERPASS, credential.getPassword());
        credentialValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_HTTP_AUTH_ID, credential.getHttpAuthId());

        return dataBaseHelper.getWritableDatabase().insert(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL, null, credentialValues);
    }

    public long delete(WebDataBaseCredential credential) {
        String whereClause = WebDataBaseColumns.EntryColumns._ID + " = ?";
        String[] whereArgs = {credential.getId().toString()};

        return dataBaseHelper.getWritableDatabase().delete(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL, whereClause, whereArgs);
    }

    public long update(WebDataBaseCredential credential) {
        ContentValues credentialValues = new ContentValues();
        credentialValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERNAME, credential.getUsername());
        credentialValues.put(WebDataBaseColumns.EntryColumns.COLUMN_NAME_USERPASS, credential.getPassword());

        String whereClause = WebDataBaseColumns.EntryColumns.COLUMN_NAME_HTTP_AUTH_ID + " = ?";
        String[] whereArgs = {credential.getHttpAuthId().toString()};

        return dataBaseHelper.getWritableDatabase().update(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL, credentialValues, whereClause, whereArgs);
  }

    public boolean exist() {
        Cursor cursor = dataBaseHelper.getReadableDatabase().query(
            WebDataBaseColumns.EntryColumns.TABLE_NAME_CREDENTIAL,
            columns,
            null,
            null,
            null,
            null,
            null
        );
        int count = cursor.getCount();
        cursor.close();

        return count != 0;
    }
}
