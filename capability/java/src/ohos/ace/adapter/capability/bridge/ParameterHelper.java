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
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Iterator;
import java.util.List;
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

    private static List<Object> arraysObject = new ArrayList<Object>();

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
        Lock transformObjectLock = new ReentrantLock();
        transformObjectLock.lock();
        try {
            arraysObject.clear();
            Iterator<String> keys = paramJsonObj.keys();
            while (keys.hasNext()) {
                String str = keys.next();
                if (paramJsonObj.get(str) instanceof JSONArray && arraysObject != null) {
                    JSONArray JsonArray = (JSONArray) paramJsonObj.get(str);
                    addArraysObject(JsonArray);
                } else if (arraysObject != null) {
                    arraysObject.add(paramJsonObj.get(str));
                } else {
                    return null;
                }
            }
            return arraysObject.toArray();
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "jsonTransformObject failed, JSONException.");
        } finally {
            transformObjectLock.unlock();
        }
        return null;
    }

    private static void addArraysObject(JSONArray JsonArray) {
        try {
            int next = 0;
            if (JsonArray != null && JsonArray.get(next) instanceof String) {
                addArraysString(JsonArray, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Integer) {
                addArraysInteger(JsonArray, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Boolean) {
                addArraysBoolean(JsonArray, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Character) {
                addArraysCharacter(JsonArray, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Double) {
                addArraysDouble(JsonArray, next);
            } else if (JsonArray != null && JsonArray.get(next) instanceof Float) {
                addArraysFloat(JsonArray, next);
            } else {
                return;
            }
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "addArraysObject failed, JSONException.");
        }
    }

    private static void addArraysString(JSONArray JsonArray, int next) {
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

    private static void addArraysInteger(JSONArray JsonArray, int next) {
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

    private static void addArraysBoolean(JSONArray JsonArray, int next) {
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

    private static void addArraysCharacter(JSONArray JsonArray, int next) {
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

    private static void addArraysDouble(JSONArray JsonArray, int next) {
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

    private static void addArraysFloat(JSONArray JsonArray, int next) {
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
            } else if (objects.getClass().getName().toString().equals(LONG_ARRAY_CLASS_NAME)) {
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
}
