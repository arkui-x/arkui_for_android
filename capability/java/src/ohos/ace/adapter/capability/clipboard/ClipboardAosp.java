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

package ohos.ace.adapter.capability.clipboard;

import android.content.ClipData;
import android.content.ClipData.Item;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.Context;

import ohos.ace.adapter.ALog;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * ClipboardAosp
 *
 * @since 1
 */
public class ClipboardAosp extends ClipboardPluginBase {
    private static final String LOG_TAG = "ClipboardPlugin";
    private static final String MULTI_TYPE_DATA_MIME = "arkuix/multipletypedata";
    private static final String MULTI_TYPE_DATA_RECORDS = "pasteDataRecords";
    private static final String MULTI_TYPE_DATA_RECORDS_IS_MULTI = "isMulti";
    private static final String MULTI_TYPE_DATA_RECORDS_TYPE = "type";
    private static final String MULTI_TYPE_DATA_RECORDS_TEXT = "text";
    private static final String MULTI_TYPE_DATA_RECORDS_TYPE_SPAN_STRING = "3";
    private static final String MULTI_TYPE_DATA_RECORDS_TYPE_TEXT = "0";

    private ClipboardManager clipManager;

    private Context context;

    /**
     * ClipboardPlugin on AOSP platform
     *
     * @param context context of the application
     */
    public ClipboardAosp(Context context) {
        this.context = context;
        nativeInit();
    }

    private void initClipManager() {
        if (context != null) {
            Object service = context.getSystemService(Context.CLIPBOARD_SERVICE);
            if (service instanceof ClipboardManager) {
                this.clipManager = (ClipboardManager) service;
            } else {
                ALog.e(LOG_TAG, "unable to get clipboard service");
                this.clipManager = null;
            }
        } else {
            ALog.e(LOG_TAG, "context is null");
            this.clipManager = null;
        }
    }

    private String getPasteDataFromJson(String pasteData) {
        try {
            JSONObject multiJsonObject = new JSONObject(pasteData);
            JSONArray jsonRecordArray = new JSONArray(multiJsonObject.getString(MULTI_TYPE_DATA_RECORDS));
            StringBuilder itemText = new StringBuilder();
            for (int i = jsonRecordArray.length() - 1; i >= 0; i--) {
                JSONObject recordObject = jsonRecordArray.getJSONObject(i);
                if (recordObject.getBoolean(MULTI_TYPE_DATA_RECORDS_IS_MULTI) &&
                    MULTI_TYPE_DATA_RECORDS_TYPE_SPAN_STRING.equals(
                        recordObject.getString(MULTI_TYPE_DATA_RECORDS_TYPE))) {
                    itemText.append(recordObject.getString(MULTI_TYPE_DATA_RECORDS_TEXT));
                } else if (!recordObject.getBoolean(MULTI_TYPE_DATA_RECORDS_IS_MULTI) &&
                    MULTI_TYPE_DATA_RECORDS_TYPE_TEXT.equals(
                        recordObject.getString(MULTI_TYPE_DATA_RECORDS_TYPE))) {
                    itemText.append(recordObject.getString(MULTI_TYPE_DATA_RECORDS_TEXT));
                } else {
                    ALog.w(LOG_TAG, "Unsupported data type.");
                }
            }
            return itemText.toString();
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "getPasteDataFromJson JSONException: " + e.getMessage());
            return "";
        }
    }
    @Override
    public String getData() {
        if (clipManager == null) {
            initClipManager();
        }
        if (clipManager != null) {
            ClipData clipData = clipManager.getPrimaryClip();
            CharSequence charSequence = null;
            if (clipData != null && clipData.getItemCount() > 0) {
                charSequence = clipData.getItemAt(0).getText();
            }
            if (charSequence != null) {
                return charSequence.toString();
            }
        }
        return "";
    }

    @Override
    public void setData(String data) {
        if (clipManager == null) {
            initClipManager();
        }
        if (clipManager != null) {
            ClipData clipData = ClipData.newPlainText(null, data);
            clipManager.setPrimaryClip(clipData);
        }
    }

    @Override
    public boolean hasData() {
        if (clipManager == null) {
            initClipManager();
        }
        if (clipManager != null) {
            return clipManager.hasPrimaryClip();
        }
        return false;
    }

    @Override
    public void clear() {
        if (clipManager == null) {
            initClipManager();
        }
        if (clipManager != null) {
            clipManager.setPrimaryClip(ClipData.newPlainText(null, null));
        }
    }

    @Override
    public boolean isMultiTypeData() {
        if (clipManager == null) {
            initClipManager();
        }
        if (clipManager != null) {
            ClipData clipData = clipManager.getPrimaryClip();
            if (clipData != null) {
                ClipDescription clipDescription = clipData.getDescription();
                if (clipDescription == null) {
                    return false;
                }
                if (clipDescription.hasMimeType(MULTI_TYPE_DATA_MIME)) {
                    return true;
                }
            }
        }
        return false;
    }

    @Override
    public void setMultiTypeData(String data) {
        if (clipManager == null) {
            initClipManager();
        }
        if (clipManager != null) {
            String itemText = getPasteDataFromJson(data);
            ClipData clipData = new ClipData("Multiple Items", new String[]{MULTI_TYPE_DATA_MIME},
                new ClipData.Item(itemText));
            clipData.addItem(new ClipData.Item(data));
            if (clipManager != null) {
                clipManager.setPrimaryClip(clipData);
            }
        }
    }

    @Override
    public String getMultiTypeData() {
        if (clipManager == null) {
            initClipManager();
        }
        if (clipManager != null) {
            ClipData clipData = clipManager.getPrimaryClip();
            CharSequence charSequence = null;
            // To be compatible with the existing clipboard functionality,
            // the clipboard data of multiple data types is stored in the second record.
            if (clipData != null && clipData.getItemCount() > 1) {
                charSequence = clipData.getItemAt(1).getText();
            }
            if (charSequence != null) {
                return charSequence.toString();
            }
        }
        return "{}";
    }
}
