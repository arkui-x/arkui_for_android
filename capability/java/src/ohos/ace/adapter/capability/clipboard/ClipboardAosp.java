/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
import android.content.Intent;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import ohos.ace.adapter.ALog;

/**
 * ClipboardAosp
 *
 * @since 1
 */
public class ClipboardAosp extends ClipboardPluginBase {
    private static final String LOG_TAG = "ClipboardPlugin";
    private static final String MULTI_TYPE_DATA_MIME = "arkuix/multipletypedata";
    private static final String MULTI_TYPE_DATA_RECORDS = "pasteDataRecords";       // CLIPBOARD_RECORD_TYPE_RECORDS
    private static final String MULTI_TYPE_DATA_RECORDS_IS_MULTI = "isMulti";       // CLIPBOARD_RECORD_TYPE_ISMULTI
    private static final String MULTI_TYPE_DATA_RECORDS_TYPE = "type";              // CLIPBOARD_RECORD_TYPE_TYPE
    private static final String MULTI_TYPE_DATA_RECORDS_TEXT = "text";              // CLIPBOARD_RECORD_TYPE_TEXT
    private static final String MULTI_TYPE_DATA_RECORDS_DATA = "data";              // CLIPBOARD_RECORD_TYPE_DATA
    private static final String MULTI_TYPE_DATA_RECORDS_CUSTOM_SPAN_DATA = "customSpanData";
    private static final String MULTI_TYPE_DATA_RECORDS_CUSTOM_URI_DATA = "customUriData";
    private static final String MULTI_TYPE_DATA_RECORDS_TYPE_SPAN_STRING = "3";
    private static final String MULTI_TYPE_DATA_RECORDS_TYPE_URI = "2";
    private static final String MULTI_TYPE_DATA_RECORDS_TYPE_IMAGE = "1";
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

    private String processRecordObject(JSONObject recordObject, Set<String> mimeTypeSet,
        Intent tmpIntent) throws JSONException {
        Iterator<String> it = recordObject.keys();
        String text = null;
        while (it.hasNext()) {
            String key = it.next();
            if (MULTI_TYPE_DATA_RECORDS_TEXT.equals(key)) {
                text = recordObject.getString(MULTI_TYPE_DATA_RECORDS_TEXT);
            } else if (MULTI_TYPE_DATA_RECORDS_TYPE.equals(key)) {
                String type = recordObject.getString(MULTI_TYPE_DATA_RECORDS_TYPE);
                if (MULTI_TYPE_DATA_RECORDS_TYPE_TEXT.equals(type)) {
                    mimeTypeSet.add(ClipDescription.MIMETYPE_TEXT_PLAIN);
                } else if (MULTI_TYPE_DATA_RECORDS_TYPE_URI.equals(type)) {
                    mimeTypeSet.add(ClipDescription.MIMETYPE_TEXT_URILIST);
                } else if (MULTI_TYPE_DATA_RECORDS_TYPE_SPAN_STRING.equals(type)) {
                    mimeTypeSet.add(ClipDescription.MIMETYPE_TEXT_INTENT);
                } else {
                    ALog.w(LOG_TAG, "Unsupported data type.");
                }
            } else if (MULTI_TYPE_DATA_RECORDS_DATA.equals(key)) {
                if (MULTI_TYPE_DATA_RECORDS_TYPE_SPAN_STRING.equals(
                    recordObject.getString(MULTI_TYPE_DATA_RECORDS_TYPE))) {
                    tmpIntent.putExtra(MULTI_TYPE_DATA_RECORDS_CUSTOM_SPAN_DATA,
                        recordObject.getString(MULTI_TYPE_DATA_RECORDS_DATA));
                } else {
                    tmpIntent.putExtra(MULTI_TYPE_DATA_RECORDS_CUSTOM_URI_DATA,
                        recordObject.getString(MULTI_TYPE_DATA_RECORDS_DATA));
                }
            } else {
                ALog.w(LOG_TAG, "Unsupported data type.");
            }
        }
        return text;
    }

    private void setPasteDataFromJson(Set<String> mimeTypeSet, ArrayList<ClipData.Item> itemList,
                                        JSONArray jsonRecordArray) throws JSONException {
        if (jsonRecordArray == null || jsonRecordArray.length() == 0) {
            ALog.w(LOG_TAG, "Invalid JSONArray: null or empty");
            return;
        }
        for (int i = 0; i < jsonRecordArray.length(); i++) {
            JSONObject recordObject = jsonRecordArray.getJSONObject(i);
            Intent tmpIntent = new Intent();
            String text = processRecordObject(recordObject, mimeTypeSet, tmpIntent);
            ClipData.Item item = null;
            if (tmpIntent != null && (tmpIntent.hasExtra(MULTI_TYPE_DATA_RECORDS_CUSTOM_SPAN_DATA) ||
                tmpIntent.hasExtra(MULTI_TYPE_DATA_RECORDS_CUSTOM_URI_DATA)) && text != null) {
                item = new ClipData.Item(text, tmpIntent, null);
            } else if (tmpIntent != null) {
                item = new ClipData.Item(tmpIntent);
            } else if (text != null) {
                item = new ClipData.Item(text);
            } else {
                ALog.w(LOG_TAG, "The current record has no data; skip processing.");
            }
            if (item != null) {
                itemList.add(item);
            }
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
            try {
                JSONObject multiJsonObject = new JSONObject(data);
                JSONArray jsonRecordArray = new JSONArray(multiJsonObject.getString(MULTI_TYPE_DATA_RECORDS));
                Set<String> mimeTypeSet = new HashSet<>();
                ArrayList<ClipData.Item> itemList = new ArrayList<>();
                setPasteDataFromJson(mimeTypeSet, itemList, jsonRecordArray);
                if (itemList.isEmpty()) {
                    ALog.w(LOG_TAG, "Unsupported data type.");
                    return;
                }
                mimeTypeSet.add(MULTI_TYPE_DATA_MIME);
                ClipData clipData = new ClipData("Multiple Items",
                    mimeTypeSet.toArray(new String[0]),
                    itemList.get(0));
                for (int j = 1; j < itemList.size(); j++) {
                    clipData.addItem(itemList.get(j));
                }
                if (clipManager != null) {
                    clipManager.setPrimaryClip(clipData);
                }
            } catch (JSONException e) {
                ALog.e(LOG_TAG, "Failed to parse ArkUI-X clipboard data.");
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
            if (clipData != null && clipData.getItemCount() <= 0) {
                return "{}";
            }
            boolean isMultiTypeData = isMultiTypeData();
            try {
                return convertClipDataToJson(clipData, isMultiTypeData);
            } catch (JSONException e) {
                ALog.e(LOG_TAG, "Failed to convert ClipData to a JSON string.");
            }
        }
        return "{}";
    }

    private String convertClipDataToJson(ClipData clipData, boolean isMultiTypeData) throws JSONException {
        JSONArray jsonRecordArray = new JSONArray();
        for (int i = 0; i < clipData.getItemCount(); i++) {
            ClipData.Item item = clipData.getItemAt(i);
            JSONObject recordObject = new JSONObject();
            int itemCount = 0;
            if (item.getText() != null) {
                recordObject.put(MULTI_TYPE_DATA_RECORDS_TEXT, item.getText().toString());
                recordObject.put(MULTI_TYPE_DATA_RECORDS_TYPE, MULTI_TYPE_DATA_RECORDS_TYPE_TEXT);
                itemCount++;
            }
            if (isMultiTypeData) {
                Intent intent = item.getIntent();
                if (intent != null && intent.hasExtra(MULTI_TYPE_DATA_RECORDS_CUSTOM_SPAN_DATA)) {
                    String customData = intent.getStringExtra(MULTI_TYPE_DATA_RECORDS_CUSTOM_SPAN_DATA);
                    recordObject.put(MULTI_TYPE_DATA_RECORDS_DATA, customData);
                    recordObject.put(MULTI_TYPE_DATA_RECORDS_TYPE, MULTI_TYPE_DATA_RECORDS_TYPE_SPAN_STRING);
                    itemCount++;
                }
                if (intent != null && intent.hasExtra(MULTI_TYPE_DATA_RECORDS_CUSTOM_URI_DATA)) {
                    String customData = intent.getStringExtra(MULTI_TYPE_DATA_RECORDS_CUSTOM_URI_DATA);
                    recordObject.put(MULTI_TYPE_DATA_RECORDS_DATA, customData);
                    recordObject.put(MULTI_TYPE_DATA_RECORDS_TYPE, MULTI_TYPE_DATA_RECORDS_TYPE_URI);
                    itemCount++;
                }
            }
            if (item.getUri() != null) {
                recordObject.put(MULTI_TYPE_DATA_RECORDS_DATA, item.getUri().toString());
                recordObject.put(MULTI_TYPE_DATA_RECORDS_TYPE, MULTI_TYPE_DATA_RECORDS_TYPE_URI);
                itemCount++;
            }
            if (itemCount > 1) {
                recordObject.put(MULTI_TYPE_DATA_RECORDS_IS_MULTI, true);
            } else {
                recordObject.put(MULTI_TYPE_DATA_RECORDS_IS_MULTI, false);
            }
            jsonRecordArray.put(recordObject);
        }
        JSONObject multiJsonObject = new JSONObject();
        multiJsonObject.put(MULTI_TYPE_DATA_RECORDS, jsonRecordArray);
        return multiJsonObject.toString();
    }
}
