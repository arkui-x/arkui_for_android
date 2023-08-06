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
import android.preference.Preference;

import ohos.ace.adapter.ALog;

/**
 * PersistentStorage implement of android
 *
 * @since 1
 */
public class PersistentStorageAosp extends PersistentStorageBase {
    private static final String LOG_TAG = "PersistentStorageAosp";

    private String databasePath;

    private Preference preferences;

    private Context context;

    /**
     * constructor of PersistentStorage in AOSP platform
     *
     * @param context the context of application
     */
    public PersistentStorageAosp(Context context) {
        this.context = context;
        if (context == null) {
            ALog.i(LOG_TAG, "PersistentStorageAosp context null");
        }
        preferences = new Preference(context);
        ALog.i(LOG_TAG, "PersistentStorageAosp constructor");
        nativeInit();
    }

    @Override
    public void initializeStorage(String filedb) {
    }

    @Override
    public void set(String key, String value) {
        SharedPreferences.Editor editor = preferences.getEditor();
        if (editor == null) {
            ALog.i(LOG_TAG, "editor null");
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
        SharedPreferences sharePreference = preferences.getSharedPreferences();
        if (sharePreference == null) {
            ALog.i(LOG_TAG, "sharePreference null");
            return "";
        }
        return sharePreference.getString(key, "");
    }

    @Override
    public void clear() {
        SharedPreferences.Editor editor = preferences.getEditor();
        if (editor == null) {
            ALog.i(LOG_TAG, "editor null");
            return;
        }
        editor.clear();
        if (!editor.commit()) {
            ALog.i(LOG_TAG, "fail to commit persistent data after clear");
        }
    }

    @Override
    public void delete(String key) {
        SharedPreferences.Editor editor = preferences.getEditor();
        if (editor == null) {
            ALog.i(LOG_TAG, "editor null");
            return;
        }
        editor.remove(key);
        if (!editor.commit()) {
            ALog.i(LOG_TAG, "fail to commit persistent data after remove key");
        }
    }
}