/**
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

package ohos.ace.adapter;

import android.util.Log;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * WantParams is a utility class for managing JSON parameters
 * that can be used across different platforms.
 *
 * @since 2025-01-15
 */
public class WantParams {
    private static final String LOG_TAG = "WantParams";
    private static final String WANT_PARAMS_PARAMS = "params";
    private static final String WANT_PARAMS_KEY = "key";
    private static final String WANT_PARAMS_TYPE = "type";
    private static final String WANT_PARAMS_VALUE = "value";
    private static final int VALUE_TYPE_BOOLEAN = 1;
    private static final int VALUE_TYPE_INT = 5;
    private static final int VALUE_TYPE_DOUBLE = 9;
    private static final int VALUE_TYPE_STRING = 10;
    private static final int VALUE_TYPE_WANT_PARAMS_ARRAY = 24;
    private static final int VALUE_TYPE_WANT_PARAMS = 101;
    private static final int VALUE_TYPE_ARRAY = 102;

    private final Map<String, WantValue> wantParamsMap;

    /**
     * Default constructor initializes the WantParams.
     */
    public WantParams() {
        wantParamsMap = new HashMap<>();
    }

    /**
     * Constructor initializes the WantParams from a JSON string.
     *
     * @param wantParamsStr the JSON string to initialize from
     * @throws JSONException if the JSON string is malformed
     */
    public WantParams(String wantParamsStr) {
        wantParamsMap = new HashMap<>();
        try {
            JSONObject wantObject = new JSONObject(wantParamsStr);
            parseWantParams(wantObject, WANT_PARAMS_PARAMS);
        } catch (JSONException jsonException) {
            Log.e(LOG_TAG, "WantParams parse error.");
            jsonException.printStackTrace();
        }
    }

    /**
     * Inner class to hold the value and type of a parameter.
     */
    private static class WantValue {
        /**
         * The type of the value, represented as an integer constant.
         */
        public int type;

        /**
         * The actual value, which can be of any object type.
         */
        public Object value;

        /**
         * Constructs a new WantValue object with the specified type and value.
         *
         * @param type  the type of the value, represented as an integer constant
         * @param value the actual value, which can be of any object type
         */
        public WantValue(int type, Object value) {
            this.type = type;
            this.value = value;
        }
    }

    /**
     * Parses a JSONArray and populates the WantParams map.
     *
     * @param wantArray the JSONArray to parse
     * @return true if parsing was successful, false otherwise
     * @throws JSONException if the JSONArray is malformed
     */
    private boolean parse(JSONArray wantArray) throws JSONException {
        for (int i = 0; i < wantArray.length(); i++) {
            JSONObject jsonElement = wantArray.getJSONObject(i);
            if (!jsonElement.has(WANT_PARAMS_KEY) || !jsonElement.has(WANT_PARAMS_TYPE)
                    || !jsonElement.has(WANT_PARAMS_VALUE)) {
                Log.e(LOG_TAG, "WantParams data format error.");
                return false;
            }
            int typeId = jsonElement.getInt(WANT_PARAMS_TYPE);
            String wantKey = jsonElement.getString(WANT_PARAMS_KEY);
            WantValue wantValue = null;
            switch (typeId) {
                case VALUE_TYPE_BOOLEAN:
                    wantValue = new WantValue(typeId, jsonElement.getBoolean(WANT_PARAMS_VALUE));
                    this.wantParamsMap.put(wantKey, wantValue);
                    break;
                case VALUE_TYPE_INT:
                    wantValue = new WantValue(typeId, jsonElement.getInt(WANT_PARAMS_VALUE));
                    this.wantParamsMap.put(wantKey, wantValue);
                    break;
                case VALUE_TYPE_DOUBLE:
                    wantValue = new WantValue(typeId, jsonElement.getDouble(WANT_PARAMS_VALUE));
                    this.wantParamsMap.put(wantKey, wantValue);
                    break;
                case VALUE_TYPE_STRING:
                    wantValue = new WantValue(typeId, jsonElement.getString(WANT_PARAMS_VALUE));
                    this.wantParamsMap.put(wantKey, wantValue);
                    break;
                case VALUE_TYPE_WANT_PARAMS:
                    WantParams localWantParams = new WantParams();
                    localWantParams.parse(jsonElement.getJSONArray(WANT_PARAMS_VALUE));
                    wantValue = new WantValue(typeId, localWantParams);
                    this.wantParamsMap.put(wantKey, wantValue);
                    break;
                case VALUE_TYPE_ARRAY:
                    parseWantArray(jsonElement);
                    break;
                default:
                    Log.e(LOG_TAG, "Not supported data type");
                    break;
            }
        }
        return true;
    }

    /**
     * Parses a JSONObject representing an array and populates the WantParams map.
     *
     * @param jsonElement the JSONObject to parse
     * @throws JSONException if the JSONObject is malformed
     */
    private void parseWantArray(JSONObject jsonElement) throws JSONException {
        JSONArray localJsonArray = jsonElement.getJSONArray(WANT_PARAMS_VALUE);
        String wantKey = jsonElement.getString(WANT_PARAMS_KEY);
        WantValue wantValue = null;
        if (localJsonArray.length() < 1) {
            wantValue = new WantValue(VALUE_TYPE_ARRAY, new Object());
            this.wantParamsMap.put(wantKey, wantValue);
            return;
        }
        if (localJsonArray.get(0) instanceof Integer) {
            int[] convertArray = new int[localJsonArray.length()];
            for (int j = 0; j < localJsonArray.length(); j++) {
                convertArray[j] = localJsonArray.getInt(j);
            }
            wantValue = new WantValue(VALUE_TYPE_ARRAY, convertArray);
        } else if (localJsonArray.get(0) instanceof Boolean) {
            boolean[] convertArray = new boolean[localJsonArray.length()];
            for (int j = 0; j < localJsonArray.length(); j++) {
                convertArray[j] = localJsonArray.getBoolean(j);
            }
            wantValue = new WantValue(VALUE_TYPE_ARRAY, convertArray);
        } else if (localJsonArray.get(0) instanceof Double) {
            double[] convertArray = new double[localJsonArray.length()];
            for (int j = 0; j < localJsonArray.length(); j++) {
                convertArray[j] = localJsonArray.getDouble(j);
            }
            wantValue = new WantValue(VALUE_TYPE_ARRAY, convertArray);
        } else if (localJsonArray.get(0) instanceof String) {
            String[] convertArray = new String[localJsonArray.length()];
            for (int j = 0; j < localJsonArray.length(); j++) {
                convertArray[j] = localJsonArray.getString(j);
            }
            wantValue = new WantValue(VALUE_TYPE_ARRAY, convertArray);
        } else if (localJsonArray.get(0) instanceof JSONObject) {
            WantParams[] convertArray = new WantParams[localJsonArray.length()];
            WantParams wantParams;
            for (int j = 0; j < localJsonArray.length(); j++) {
                wantParams = new WantParams();
                wantParams.parseWantParams(localJsonArray.getJSONObject(j), WANT_PARAMS_VALUE);
                convertArray[j] = wantParams;
            }
            wantValue = new WantValue(VALUE_TYPE_WANT_PARAMS_ARRAY, convertArray);
        } else {
            Log.e(LOG_TAG, "Not supported data type");
            return;
        }
        this.wantParamsMap.put(wantKey, wantValue);
    }

    /**
     * Parses a JSONObject and populates the WantParams map.
     *
     * @param wantObject the JSONObject to parse
     * @param wantKay    the key to look for in the JSONObject
     * @throws JSONException if the JSONObject is malformed
     */
    private void parseWantParams(JSONObject wantObject, String wantKay) throws JSONException {
        if (!wantObject.has(wantKay)) {
            return;
        }
        if (wantObject.get(wantKay) instanceof JSONArray) {
            JSONArray wantArray = (JSONArray) wantObject.get(wantKay);
            if (!parse(wantArray)) {
                Log.e(LOG_TAG, "WantParams data format error.");
            }
        }
    }

    /**
     * Adds a boolean value to the WantParams.
     *
     * @param key   the key for the value
     * @param value the boolean value to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, boolean value) {
        innerAddValue(key, value, VALUE_TYPE_BOOLEAN);
        return this;
    }

    /**
     * Adds a Object value to the WantParams.
     *
     * @param key   the key for the value
     * @param value the Object value to add
     * @param type  the type
     */
    private void innerAddValue(String key, Object value, int type) {
        if (key != null && !key.isEmpty()) {
            wantParamsMap.put(key, new WantValue(type, value));
        }
    }

    /**
     * Adds an int value to the WantParams.
     *
     * @param key   the key for the value
     * @param value the int value to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, int value) {
        innerAddValue(key, value, VALUE_TYPE_INT);
        return this;
    }

    /**
     * Adds a float value to the WantParams.
     *
     * @param key   the key for the value
     * @param value the float value to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, float value) {
        innerAddValue(key, value, VALUE_TYPE_DOUBLE);
        return this;
    }

    /**
     * Adds a double value to the WantParams.
     *
     * @param key   the key for the value
     * @param value the double value to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, double value) {
        innerAddValue(key, value, VALUE_TYPE_DOUBLE);
        return this;
    }

    /**
     * Adds a String value to the WantParams.
     *
     * @param key   the key for the value
     * @param value the String value to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, String value) {
        innerAddValue(key, value, VALUE_TYPE_STRING);
        return this;
    }

    /**
     * Adds an array of boolean values to the WantParams.
     *
     * @param key   the key for the value
     * @param value the array of boolean values to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, boolean[] value) {
        innerAddArrayValue(key, value);
        return this;
    }

    /**
     * Adds an array of int values to the WantParams.
     *
     * @param key   the key for the value
     * @param value the array of int values to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, int[] value) {
        innerAddArrayValue(key, value);
        return this;
    }

    /**
     * Adds an array of float values to the WantParams.
     *
     * @param key   the key for the value
     * @param value the array of float values to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, float[] value) {
        innerAddArrayValue(key, value);
        return this;
    }

    /**
     * Adds an array of double values to the WantParams.
     *
     * @param key   the key for the value
     * @param value the array of double values to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, double[] value) {
        innerAddArrayValue(key, value);
        return this;
    }

    /**
     * Adds an array of String values to the WantParams.
     *
     * @param key   the key for the value
     * @param value the array of String values to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, String[] value) {
        innerAddArrayValue(key, value);
        return this;
    }

    /**
     * Adds an array of Object values to the WantParams.
     *
     * @param key   the key for the value
     * @param value the array of Object values to add
     */
    private void innerAddArrayValue(String key, Object value) {
        if (key != null && !key.isEmpty() && value != null) {
            this.wantParamsMap.put(key, new WantValue(VALUE_TYPE_ARRAY, value));
        }
    }

    /**
     * Adds a WantParams object to the WantParams.
     *
     * @param key   the key for the value
     * @param value the WantParams object to add
     * @return this instance for method chaining
     */
    public WantParams addValue(String key, WantParams value) {
        innerAddValue(key, value, VALUE_TYPE_WANT_PARAMS);
        return this;
    }

    /**
     * Retrieves a value from the WantParams.
     *
     * @param key the key for the value
     * @return the value associated with the key, or null if not found
     */
    public Object getValue(String key) {
        if (key != null && !key.isEmpty() && this.wantParamsMap.containsKey(key)) {
            return Objects.requireNonNull(this.wantParamsMap.get(key)).value;
        }
        return null;
    }

    private String replaceSpecialChars(String value) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < value.length(); i++) {
            char c = value.charAt(i);
            switch (c) {
                case '\\':
                    sb.append("\\\\");
                    break;
                case '\n':
                    sb.append("\\n");
                    break;
                case '\t':
                    sb.append("\\t");
                    break;
                case '\r':
                    sb.append("\\r");
                    break;
                case '\b':
                    sb.append("\\b");
                    break;
                case '\f':
                    sb.append("\\f");
                    break;
                case '\"':
                    sb.append("\\\"");
                    break;
                default:
                    sb.append(c);
                    break;
            }
        }
        return sb.toString();
    }

    /**
     * Converts the WantParams to a JSON string.
     *
     * @return the JSON string representation of the WantParams
     */
    private String innerToWantParamsString() {
        StringBuilder wantParamsString = new StringBuilder();
        wantParamsString.append("[");
        String valueStr;
        for (Map.Entry<String, WantValue> entry : wantParamsMap.entrySet()) {
            int typeId = entry.getValue().type;
            if (typeId == VALUE_TYPE_ARRAY) {
                try {
                    valueStr = new JSONArray(entry.getValue().value).toString();
                } catch (JSONException jsonException) {
                    Log.e(LOG_TAG, "WantParams array toString failed.");
                    continue;
                }
            } else if (typeId == VALUE_TYPE_WANT_PARAMS) {
                if (entry.getValue().value instanceof WantParams) {
                    valueStr = ((WantParams) entry.getValue().value).innerToWantParamsString();
                } else {
                    continue;
                }
            } else if (typeId == VALUE_TYPE_WANT_PARAMS_ARRAY) {
                valueStr = wantArrayToString(entry);
            } else if (typeId == VALUE_TYPE_STRING) {
                valueStr = "\"" + replaceSpecialChars(entry.getValue().value.toString()) + "\"";
            } else {
                valueStr = entry.getValue().value.toString();
            }
            wantParamsString.append("{\"" + WANT_PARAMS_KEY + "\":\"");
            wantParamsString.append(entry.getKey());
            wantParamsString.append("\",\"" + WANT_PARAMS_TYPE + "\":");
            wantParamsString.append(
                    typeId == VALUE_TYPE_WANT_PARAMS_ARRAY ? VALUE_TYPE_ARRAY : typeId);
            wantParamsString.append(",\"" + WANT_PARAMS_VALUE + "\":");
            wantParamsString.append(valueStr);
            wantParamsString.append("},");
        }
        wantParamsString.deleteCharAt(wantParamsString.length() - 1);
        wantParamsString.append("]");
        return wantParamsString.toString();
    }

    /**
     * Converts an array of WantParams to a JSON string.
     *
     * @param entry the map entry containing the array
     * @return the JSON string representation of the array
     */
    private static String wantArrayToString(Map.Entry<String, WantValue> entry) {
        String valueStr;
        StringBuilder wantArrayString = new StringBuilder();
        wantArrayString.append("[");
        WantParams[] wantArray = ((WantParams[]) entry.getValue().value);
        for (WantParams wantElement : wantArray) {
            wantArrayString.append(
                    "{\"" + WANT_PARAMS_KEY + "\":\"" + WANT_PARAMS_PARAMS + "\",\"" + WANT_PARAMS_TYPE + "\":");
            wantArrayString.append(VALUE_TYPE_WANT_PARAMS);
            wantArrayString.append(",\"" + WANT_PARAMS_VALUE + "\":");
            wantArrayString.append(wantElement.innerToWantParamsString());
            wantArrayString.append("},");
        }
        wantArrayString.deleteCharAt(wantArrayString.length() - 1);
        wantArrayString.append("]");
        valueStr = wantArrayString.toString();
        return valueStr;
    }

    /**
     * Converts the WantParams to a JSON string with a version prefix.
     *
     * @return the JSON string representation of the WantParams with a version
     *         prefix
     */
    public String toWantParamsString() {
        return "{\"" + WANT_PARAMS_PARAMS + "\":" + this.innerToWantParamsString() + "}";
    }
}
