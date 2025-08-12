/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
import android.os.Handler;
import android.os.Looper;
import android.webkit.JavascriptInterface;
import android.webkit.WebView;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import ohos.ace.adapter.ALog;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * The `AceWebJavascriptProxyCallback` class is responsible for handling JavaScript callback methods
 * in the Ace framework. It provides methods to handle asynchronous and synchronous JavaScript execution results.
 *
 * @since 20
 */
public class AceWebJavascriptProxyCallback {
    private static final String LOG_TAG = "AceWebJavascriptProxyCallback";

    private static final long THREAD_TIME_OUT = 1000L;

    private Context mContext;

    private WebView mWebView;

    AceWebJavascriptProxyCallback(Context context, WebView webView) {
        mContext = context;
        mWebView = webView;
    }

    /**
     * This method is called when an asynchronous JavaScript execution result is returned.
     * It parses the parameters and invokes the corresponding callback method on the UI thread.
     *
     * @param className The name of the class that contains the callback method.
     * @param methodName The name of the callback method.
     * @param param The JSON string representing the parameters for the callback method.
     */
    @JavascriptInterface
    public void callAsyncFunction(String className, String methodName, String param) {
        ALog.i(LOG_TAG, "callAsyncFunction className: " + className + ", methodName: " + methodName);
        try {
            JSONArray args = new JSONArray(param);
            List<Object> params = new ArrayList<>();
            for (int i = 0; i < args.length(); i++) {
                Object element = args.get(i);
                params.add(parseJsonElement(element));
            }
            mWebView.post(() -> {
                AceWebPluginBase.onReceiveJavascriptExecuteCall(className, methodName, params.toArray(new Object[0]));
            });
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "callAsyncFunction JSONException error");
        }
    }

    /**
     * Returns the synchronous result of a JavaScript callback.
     *
     * @param className The name of the class.
     * @param methodName The name of the method.
     * @param param The parameter in JSON format.
     * @return The result of the JavaScript callback in JSON format.
     */
    @JavascriptInterface
    public String callSyncFunction(String className, String methodName, String param) {
        ALog.i(LOG_TAG, "callSyncFunction className: " + className + ", methodName: " + methodName);
        final AtomicReference<String> resultRef = new AtomicReference<>("{\"value\": null}");
        try {
            JSONArray args = new JSONArray(param);
            List<Object> params = new ArrayList<>();
            for (int i = 0; i < args.length(); i++) {
                Object element = args.get(i);
                params.add(parseJsonElement(element));
            }
            CountDownLatch latch = new CountDownLatch(1);
            new Handler(Looper.getMainLooper()).post(() -> {
                Object ret = AceWebPluginBase.onReceiveJavascriptExecuteCall(
                    className, methodName, params.toArray(new Object[0]));
                resultRef.set(splitJsonString(ret));
                latch.countDown();
            });

            boolean isCompleted = latch.await(THREAD_TIME_OUT, TimeUnit.MILLISECONDS);
            if (!isCompleted) {
                ALog.e(LOG_TAG, "callSyncFunction timed out waiting for result.");
                resultRef.set("{\"value\": null}");
            }
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "callSyncFunction JSONException error");
            resultRef.set("{\"value\": null}");
        } catch (InterruptedException e) {
            ALog.e(LOG_TAG, "callSyncFunction InterruptedException error");
            resultRef.set("{\"value\": null}");
        }
        return resultRef.get();
    }

    private String splitJsonString(Object result) {
        if (result == null) {
            return String.format("{\"value\": null}");
        }
        String ret = null;
        if (result instanceof String) {
            ret = String.format("{\"value\": \"%s\"}", (String) result);
        } else if (result instanceof Integer || result instanceof Double || result instanceof Boolean) {
            ret = String.format("{\"value\": %s}", result.toString());
        } else if (result instanceof List) {
            ret = String.format("{\"value\": %s}", convertListToJson((List<?>) result));
        } else if (result instanceof Map) {
            ret = String.format("{\"value\": %s}", convertMapToJson((Map<?, ?>) result));
        } else {
            ret = String.format("{\"value\": null}");
        }
        return ret;
    }

    private Object parseJsonElement(Object element) {
        if (element == JSONObject.NULL || element == null) {
            return null;
        } else if (element instanceof JSONArray) {
            return parseJsonArray((JSONArray) element);
        } else if (element instanceof JSONObject) {
            return parseJsonObject((JSONObject) element);
        } else {
            return element;
        }
    }

    private List<Object> parseJsonArray(JSONArray jsonArray) {
        List<Object> list = new ArrayList<>();
        for (int i = 0; i < jsonArray.length(); i++) {
            try {
                Object rawItem = jsonArray.get(i);
                Object item = parseJsonElement(rawItem);
                list.add(item);
            } catch (JSONException e) {
                ALog.e(LOG_TAG, "parseJsonArray JSONException error");
            }
        }
        return list;
    }

    private Map<String, Object> parseJsonObject(JSONObject jsonObject) {
        Map<String, Object> map = new HashMap<>();
        Iterator<String> keys = jsonObject.keys();
        while (keys.hasNext()) {
            String key = keys.next();
            try {
                Object rawValue = jsonObject.get(key);
                Object value = parseJsonElement(rawValue);
                map.put(key, value);
            } catch (JSONException e) {
                ALog.e(LOG_TAG, "parseJsonObject JSONException error");
            }
        }
        return map;
    }

    private String convertListToJson(List<?> list) {
        try {
            JSONArray jsonArray = new JSONArray();
            for (Object element : list) {
                if (element == null) {
                    jsonArray.put(JSONObject.NULL);
                } else if (element instanceof List) {
                    jsonArray.put(new JSONArray(convertListToJson((List<?>) element)));
                } else if (element instanceof Map) {
                    jsonArray.put(new JSONObject(convertMapToJson((Map<?, ?>) element)));
                } else {
                    jsonArray.put(element);
                }
            }
            return jsonArray.toString();
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "convertListToJson error");
            return "[]";
        }
    }

    private String convertMapToJson(Map<?, ?> map) {
        try {
            JSONObject jsonObject = new JSONObject();
            for (Map.Entry<?, ?> entry : map.entrySet()) {
                String key = entry.getKey().toString();
                Object value = entry.getValue();
                if (value == null) {
                    jsonObject.put(key, JSONObject.NULL);
                } else if (value instanceof List) {
                    jsonObject.put(key, new JSONArray(convertListToJson((List<?>) value)));
                } else if (value instanceof Map) {
                    jsonObject.put(key, new JSONObject(convertMapToJson((Map<?, ?>) value)));
                } else {
                    jsonObject.put(key, value);
                }
            }
            return jsonObject.toString();
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "convertMapToJson error");
            return "{}";
        }
    }
}