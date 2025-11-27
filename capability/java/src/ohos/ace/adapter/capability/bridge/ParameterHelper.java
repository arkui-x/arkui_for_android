/**
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

package ohos.ace.adapter.capability.bridge;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.HashMap;
import ohos.ace.adapter.ALog;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * Bridge method Parameter helper.
 *
 * @since 10
 */
public class ParameterHelper {
    private static final String LOG_TAG = "ParameterHelper";

    private static final int HASH_CODE = 1;

    private static final long JS_MAX_VALUE = 0x1FFFFFFFFFFFFFL;

    private static final long JS_MIN_VALUE = -0x1FFFFFFFFFFFFFL;

    private static final long[] LONG_ARRAY = {};

    private static final String LONG_ARRAY_CLASS_NAME = LONG_ARRAY.getClass().getName().toString();

    /**
     * Method parameter helper.
     *
     * @return ParameterHelper object.
     */
    public ParameterHelper() {
        ALog.i("ParameterHelper", "ParameterHelper construction");
    }

    /**
     * whether it exceeds the safe integer range.
     *
     * @param number Incoming pending integer.
     * @return Returns whether it exceeds the safe integer range.
     */
    public static boolean isExceedJsSafeInteger(Object number) {
        if (number instanceof Long) {
            if ((long) number > JS_MAX_VALUE || (long) number < JS_MIN_VALUE) {
                ALog.e("ParameterHelper", "Data exceeds JavaScript safe integer.");
                return false;
            }
        }
        return true;
    }

    /**
     * binaryData transform Object.
     *
     * @param paramBinaryObj Parameter in the form of binaryData.
     * @return Parameter in the form of Object.
     */
    public static Object[] binaryTransformObject(Object paramBinaryObj) {
        if (paramBinaryObj instanceof List) {
            List<Object> list = (List) paramBinaryObj;
            return list.toArray();
        }
        return null;
    }

    /**
     * jsonData transform Object.
     *
     * @param paramJsonObj Parameter in the form of jsonData.
     * @return Parameter in the form of Object.
     */
    public static Object[] jsonTransformObject(JSONObject paramJsonObj) {
        if (paramJsonObj == null) {
            return null;
        }
        List<Object> resultObjectList = new ArrayList<>();
        Iterator<String> keyIterator = paramJsonObj.keys();
        while (keyIterator.hasNext()) {
            String currentKey = keyIterator.next();
            Object currentValue = paramJsonObj.opt(currentKey);
            if (currentValue == null) {
                return null;
            }
            if (currentValue instanceof JSONArray) {
                addArraysObject((JSONArray) currentValue, resultObjectList);
            } else if (currentValue instanceof JSONObject) {
                Object convertedMap = convertJsonObjectToTypedMap((JSONObject) currentValue);
                resultObjectList.add(convertedMap != null ? convertedMap : currentValue);
            } else {
                resultObjectList.add(currentValue);
            }
        }
        return resultObjectList.toArray(new Object[0]);
    }

    private static void addArraysObject(JSONArray JsonArray, List<Object> arraysObject) {
        try {
            int next = 0;
            if (JsonArray != null && JsonArray.get(next) instanceof String) {
                addArraysString(JsonArray, arraysObject, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Integer) {
                addArraysInteger(JsonArray, arraysObject, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Boolean) {
                addArraysBoolean(JsonArray, arraysObject, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Character) {
                addArraysCharacter(JsonArray, arraysObject, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Double) {
                addArraysDouble(JsonArray, arraysObject, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Float) {
                addArraysFloat(JsonArray, arraysObject, next);
            } else {
                return;
            }
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "addArraysObject failed, JSONException.");
        }
    }

    private static void addArraysString(JSONArray JsonArray, List<Object> arraysObject, int next) {
        try {
            List<String> stringList = new ArrayList<String>();
            while (JsonArray.hashCode() != HASH_CODE) {
                stringList.add((String) JsonArray.get(next));
                JsonArray.remove(next);
            }
            String[] stringArray = new String[stringList.size()];
            for (int i = 0; i < stringList.size(); i++) {
                stringArray[i] = stringList.get(i);
            }
            arraysObject.add(stringArray);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "addArraysString failed, JSONException.");
        }
    }

    private static void addArraysInteger(JSONArray JsonArray, List<Object> arraysObject, int next) {
        try {
            List<Integer> intList = new ArrayList<Integer>();
            while (JsonArray.hashCode() != HASH_CODE) {
                intList.add((int) JsonArray.get(next));
                JsonArray.remove(next);
            }
            int[] intArray = new int[intList.size()];
            for (int i = 0; i < intList.size(); i++) {
                intArray[i] = intList.get(i);
            }
            arraysObject.add(intArray);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "addArraysInteger failed, JSONException.");
        }
    }

    private static void addArraysBoolean(JSONArray JsonArray, List<Object> arraysObject, int next) {
        try {
            List<Boolean> boolList = new ArrayList<Boolean>();
            while (JsonArray.hashCode() != HASH_CODE) {
                boolList.add((boolean) JsonArray.get(next));
                JsonArray.remove(next);
            }
            boolean[] boolArray = new boolean[boolList.size()];
            for (int i = 0; i < boolList.size(); i++) {
                boolArray[i] = boolList.get(i);
            }
            arraysObject.add(boolArray);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "addArraysBoolean failed, JSONException.");
        }
    }

    private static void addArraysCharacter(JSONArray JsonArray, List<Object> arraysObject, int next) {
        try {
            List<Character> charList = new ArrayList<Character>();
            while (JsonArray.hashCode() != HASH_CODE) {
                charList.add((char) JsonArray.get(next));
                JsonArray.remove(next);
            }
            char[] charArray = new char[charList.size()];
            for (int i = 0; i < charList.size(); i++) {
                charArray[i] = charList.get(i);
            }
            arraysObject.add(charArray);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "addArraysCharacter failed, JSONException.");
        }
    }

    private static void addArraysDouble(JSONArray JsonArray, List<Object> arraysObject, int next) {
        try {
            List<Double> doubleList = new ArrayList<Double>();
            while (JsonArray.hashCode() != HASH_CODE) {
                doubleList.add((double) JsonArray.get(next));
                JsonArray.remove(next);
            }
            double[] doubleArray = new double[doubleList.size()];
            for (int i = 0; i < doubleList.size(); i++) {
                doubleArray[i] = doubleList.get(i);
            }
            arraysObject.add(doubleArray);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "addArraysDouble failed, JSONException.");
        }
    }

    private static void addArraysFloat(JSONArray JsonArray, List<Object> arraysObject, int next) {
        try {
            List<Float> floatList = new ArrayList<Float>();
            while (JsonArray.hashCode() != HASH_CODE) {
                floatList.add((float) JsonArray.get(next));
                JsonArray.remove(next);
            }
            float[] floatArray = new float[floatList.size()];
            for (int i = 0; i < floatList.size(); i++) {
                floatArray[i] = floatList.get(i);
            }
            arraysObject.add(floatArray);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "addArraysFloat failed, JSONException.");
        }
    }

    /**
     * Object transform JSONObject.
     *
     * @param objectParamters Parameter in the form of Object.
     * @return Parameter in the form of JSONObject.
     */
    public static JSONObject objectTransformJson(Object[] objectParamters) {
        try {
            JSONObject jsonParamters = new JSONObject();
            int key = 0;
            for (Object objectParamter : objectParamters) {
                if (objectParamter == null) {
                    return null;
                }
                if (objectParamter.getClass().isArray()) {
                    JSONArray array = objectTransformJsonArray(objectParamter);
                    if (array == null) {
                        return null;
                    }
                    jsonParamters.put(String.valueOf(key), array);
                } else {
                    if (!isExceedJsSafeInteger(objectParamter)) {
                        return null;
                    }
                    jsonParamters.put(String.valueOf(key), objectParamter);
                }
                key++;
            }
            return jsonParamters;
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "objectTransformJson failed, JSONException.");
        }
        return null;
    }

    /**
     * Object transform JSONArray.
     *
     * @param objects Parameter in the form of Object.
     * @return Parameter in the form of JSONArray.
     */
    public static JSONArray objectTransformJsonArray(Object objects) {
        JSONArray JsonArray = new JSONArray();
        try {
            if (objects instanceof String[]) {
                for (String stringIter : (String[]) objects) {
                    JsonArray.put(stringIter);
                }
            } else if (objects instanceof char[]) {
                for (char charIter : (char[]) objects) {
                    JsonArray.put(charIter);
                }
            } else if (objects instanceof int[]) {
                for (int intIter : (int[]) objects) {
                    JsonArray.put(intIter);
                }
            } else if (objects instanceof double[]) {
                for (double doubleIter : (double[]) objects) {
                    JsonArray.put(doubleIter);
                }
            } else if (objects instanceof float[]) {
                for (float floatIter : (float[]) objects) {
                    JsonArray.put(floatIter);
                }
            } else if (objects instanceof boolean[]) {
                for (boolean booleanIter : (boolean[]) objects) {
                    JsonArray.put(booleanIter);
                }
            } else if (LONG_ARRAY_CLASS_NAME.equals(objects.getClass().getName().toString())) {
                for (long longIter : (long[]) objects) {
                    if (!isExceedJsSafeInteger(longIter)) {
                        return null;
                    }
                    JsonArray.put(longIter);
                }
            } else if (objects instanceof Object[]) {
                for (Object objectIter : (Object[]) objects) {
                    if (objectIter == null) {
                        return null;
                    }
                    JsonArray.put(objectIter);
                }
            } else {
                return null;
            }
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "objectTransformJsonArray failed, JSONException.");
            return null;
        }
        return JsonArray;
    }

    private static Object convertJsonObjectToTypedMap(JSONObject jsonObject) {
        ScanResult result = scanJsonObject(jsonObject);
        if (result == null) {
            ALog.e(LOG_TAG, "convertJsonObjectToTypedMap The data to be parsed is empty.");
            return null;
        }
        if (result.allBoolean) {
            return buildBooleanMap(jsonObject, result.keys);
        }
        if (result.allString) {
            return buildStringMap(jsonObject, result.keys);
        }
        if (result.allNumber) {
            if (result.allIntegral) {
                if (result.fitsIntRange) {
                    return buildIntegerMap(jsonObject, result.keys);
                } else if (result.fitsLongRange) {
                    return buildLongMap(jsonObject, result.keys);
                } else {
                    ALog.e(LOG_TAG, "convertJsonObjectToTypedMap Other number types.");
                    return null;
                }
            }
            if (result.maxAbsNumber <= Float.MAX_VALUE) {
                return buildFloatMap(jsonObject, result.keys);
            } else {
                return buildDoubleMap(jsonObject, result.keys);
            }
        }
        ALog.e(LOG_TAG, "convertJsonObjectToTypedMap Beyond the range of the parsing type.");
        return null;
    }

    private static ScanResult scanJsonObject(JSONObject jsonObject) {
        if (jsonObject == null || jsonObject.length() == 0) {
            ALog.e(LOG_TAG, "scanJsonObject jsonObject is null.");
            return null;
        }
        ScanResult scanResult = new ScanResult();
        Iterator<String> keyIterator = jsonObject.keys();
        while (keyIterator.hasNext()) {
            String key = keyIterator.next();
            scanResult.keys.add(key);
            Object value;
            try {
                value = jsonObject.get(key);
            } catch (JSONException e) {
                ALog.e(LOG_TAG, "scanJsonObject get value failed.");
                return null;
            }
            if (!isPrimitiveValue(value)) {
                ALog.e(LOG_TAG, "scanJsonObject value is null.");
                return null;
            }
            updateTypeFlags(scanResult, value);
        }
        return scanResult;
    }

    private static boolean isPrimitiveValue(Object value) {
        return value != null && !(value instanceof JSONObject) && !(value instanceof JSONArray);
    }

    private static void updateTypeFlags(ScanResult scanResult, Object value) {
        scanResult.allBoolean &= value instanceof Boolean;
        scanResult.allString &= value instanceof String;
        scanResult.allNumber &= value instanceof Number;
        if (!(value instanceof Number)) {
            return;
        }
        double numberValue = ((Number) value).doubleValue();
        scanResult.maxAbsNumber = Math.max(scanResult.maxAbsNumber, Math.abs(numberValue));
        if (Math.floor(numberValue) != numberValue) {
            scanResult.allIntegral = false;
            return;
        }
        if (numberValue > Integer.MAX_VALUE || numberValue < Integer.MIN_VALUE) {
            scanResult.fitsIntRange = false;
        }
        if (numberValue > Long.MAX_VALUE || numberValue < Long.MIN_VALUE) {
            scanResult.fitsLongRange = false;
        }
    }

    private static Object buildBooleanMap(JSONObject jsonObjectParam, List<String> keys) {
        HashMap<String, Boolean> map = new HashMap<>();
        for (String key : keys) {
            try {
                map.put(key, jsonObjectParam.getBoolean(key));
            } catch (JSONException jsonException) {
                ALog.e(LOG_TAG, "buildBooleanMap failed, JSONException.");
                return null;
            }
        }
        return map;
    }

    private static Object buildStringMap(JSONObject jsonObjectParam, List<String> keys) {
        HashMap<String, String> map = new HashMap<>();
        for (String key : keys) {
            try {
                map.put(key, jsonObjectParam.getString(key));
            } catch (JSONException jsonException) {
                ALog.e(LOG_TAG, "buildStringMap failed, JSONException.");
                return null;
            }
        }
        return map;
    }

    private static Object buildIntegerMap(JSONObject jsonObjectParam, List<String> keys) {
        HashMap<String, Integer> map = new HashMap<>();
        for (String key : keys) {
            try {
                map.put(key, jsonObjectParam.getInt(key));
            } catch (JSONException jsonException) {
                ALog.e(LOG_TAG, "buildStringMap failed, JSONException.");
                return null;
            }
        }
        return map;
    }

    private static Object buildLongMap(JSONObject jsonObjectParam, List<String> keys) {
        HashMap<String, Long> map = new HashMap<>();
        for (String key : keys) {
            try {
                map.put(key, jsonObjectParam.getLong(key));
            } catch (JSONException jsonException) {
                ALog.e(LOG_TAG, "buildLongMap failed, JSONException.");
                return null;
            }
        }
        return map;
    }

    private static Object buildFloatMap(JSONObject jsonObjectParam, List<String> keys) {
        HashMap<String, Float> map = new HashMap<>();
        for (String key : keys) {
            try {
                map.put(key, (float) jsonObjectParam.getDouble(key));
            } catch (JSONException jsonException) {
                ALog.e(LOG_TAG, "buildFloatMap failed, JSONException.");
                return null;
            }
        }
        return map;
    }

    private static Object buildDoubleMap(JSONObject jsonObjectParam, List<String> keys) {
        HashMap<String, Double> map = new HashMap<>();
        for (String key : keys) {
            try {
                map.put(key, jsonObjectParam.getDouble(key));
            } catch (JSONException jsonException) {
                ALog.e(LOG_TAG, "buildDoubleMap failed, JSONException.");
                return null;
            }
        }
        return map;
    }

    private static class ScanResult {
        List<String> keys = new ArrayList<>();
        boolean allBoolean = true;
        boolean allString = true;
        boolean allNumber = true;
        boolean allIntegral = true;
        boolean fitsIntRange = true;
        boolean fitsLongRange = true;
        double maxAbsNumber = 0.0;
    }

    /**
     * Check if the object array contains null.
     *
     * @param objects Object array to be checked.
     * @return true if contains null, false otherwise.
     */
    public static boolean containsNull(List<Object> objects) {
        for (Object obj : objects) {
            if (obj == null) {
                return true;
            }
        }
        return false;
    }
}
