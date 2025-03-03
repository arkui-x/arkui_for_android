/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.storage;

import android.content.Context;
import android.content.SharedPreferences;

import ohos.ace.adapter.ALog;

/**
 * PersistentStorage implement of android
 *
 * @since 1
 */
public class PersistentStorageAosp extends PersistentStorageBase {
    private static final String LOG_TAG = "PersistentStorageAosp";

    private static final String SHARE_PREFERENCE_NAME = "storage_data";

    SharedPreferences mSharePreferences;

    private String databasePath;

    private Context context;

    /**
     * constructor of PersistentStorage in AOSP platform
     *
     * @param context the context of application
     */
    public PersistentStorageAosp(Context context) {
        this.context = context;
        if (context == null) {
            ALog.w(LOG_TAG, "PersistentStorageAosp context null");
        }
        mSharePreferences = context.getSharedPreferences(SHARE_PREFERENCE_NAME, context.MODE_PRIVATE);
        ALog.i(LOG_TAG, "PersistentStorageAosp constructor");
        nativeInit();
    }

    @Override
    public void initializeStorage(String filedb) {
    }

    @Override
    public void set(String key, String value) {
        if (mSharePreferences == null) {
            ALog.w(LOG_TAG, "set method sharePreference instantiate is null");
            return;
        }
        SharedPreferences.Editor editor = mSharePreferences.edit();
        if (editor == null) {
            ALog.w(LOG_TAG, "editor null");
            return;
        }
        editor.putString(key, value);
        if (!editor.commit()) {
            ALog.i(LOG_TAG, "fail to commit persistent data after set");
        }
    }

    @Override
    public String get(String key) {
        ALog.i(LOG_TAG, "PersistentStorageAosp get");
        if (mSharePreferences == null) {
            ALog.w(LOG_TAG, "sharePreference null");
            return "";
        }
        return mSharePreferences.getString(key, "");
    }

    @Override
    public void clear() {
        if (mSharePreferences == null) {
            ALog.w(LOG_TAG, "clear method sharePreference instantiate is null");
            return;
        }
        SharedPreferences.Editor editor = mSharePreferences.edit();
        if (editor == null) {
            ALog.w(LOG_TAG, "editor null");
            return;
        }
        editor.clear();
        if (!editor.commit()) {
            ALog.i(LOG_TAG, "fail to commit persistent data after clear");
        }
    }

    @Override
    public void delete(String key) {
        if (mSharePreferences == null) {
            ALog.w(LOG_TAG, "delete method sharePreference instantiate is null");
            return;
        }
        SharedPreferences.Editor editor = mSharePreferences.edit();
        if (editor == null) {
            ALog.w(LOG_TAG, "editor null");
            return;
        }
        editor.remove(key);
        if (!editor.commit()) {
            ALog.i(LOG_TAG, "fail to commit persistent data after remove key");
        }
    }
}