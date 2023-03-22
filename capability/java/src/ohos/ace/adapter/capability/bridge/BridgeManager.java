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

package ohos.ace.adapter.capability.bridge;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.HashMap;
import ohos.ace.adapter.ALog;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * Bridge plugin manager.
 *
 * @since 10
 */
public class BridgeManager {
    private static final String LOG_TAG = "BridgeManager";

    private static final String JSON_ERROR_CODE = "errorcode";

    private static final String JSON_ERROR_MESSAGE = "errormessage";

    private static final String CALL_METHOD_JSON_KEY = "result";

    private static final String MESSAGE_JSON_KEY = "result";

    private static final String DATA_ERROR = "data_error";

    private static final int NO_PARAM = 4;

    private static final Object INSTANCE_LOCK = new Object();

    private static volatile BridgeManager INSTANCE = null;

    private static HashMap<String, BridgePlugin> bridgeMap_;

    /**
     * Bridge plugin manager.
     *
     * @return BridgePlugin object.
     */
    private BridgeManager() {
        ALog.i(LOG_TAG, "BridgeManager start");
        bridgeMap_ = new HashMap<String, BridgePlugin>();
    }

    /**
     * Get BridgeManager object.
     *
     * @return The BridgeManager object.
     */
    public static BridgeManager getInstance() {
        if (INSTANCE != null) {
            return INSTANCE;
        }

        synchronized (INSTANCE_LOCK) {
            if (INSTANCE != null) {
                return INSTANCE;
            }

            INSTANCE = new BridgeManager();
            return INSTANCE;
        }
    }

    /**
     * Register Bridge plugin
     *
     * @param bridgeName name of bridge.
     * @param bridgePlugin bridgePlugin object.
     */
    public void registerBridgePlugin(String bridgeName, BridgePlugin bridgePlugin) {
        Lock registerLock = new ReentrantLock();
        registerLock.lock();
        try {
            if (bridgeMap_.containsKey(bridgeName)) {
                ALog.e(LOG_TAG, "The bridgeName Already exists");
                return;
            } else {
                ALog.i(LOG_TAG, "bridgeMap_.put success");
                bridgeMap_.put(bridgeName, bridgePlugin);
            }
        } finally {
            registerLock.unlock();
        }
    }

    /**
     * Unregister the created bridge
     *
     * @param bridgeName name of bridge.
     * @return Success or not.
     */
    public boolean unRegisterBridgePlugin(String bridgeName) {
        Lock unregisterLock = new ReentrantLock();
        unregisterLock.lock();
        try {
            if (bridgeMap_.containsKey(bridgeName)) {
                bridgeMap_.remove(bridgeName);
                return true;
            } else {
                return false;
            }
        } finally {
            unregisterLock.unlock();
        }
    }

    private BridgePlugin findBridgePlugin(String bridgeName) {
        Lock findBridgeLock = new ReentrantLock();
        findBridgeLock.lock();
        try {
            if (bridgeMap_.containsKey(bridgeName)) {
                return bridgeMap_.get(bridgeName);
            } else {
                ALog.e(LOG_TAG, "The bridgeName is not found.");
                return null;
            }
        } finally {
            findBridgeLock.unlock();
        }
    }

    private JSONObject createJsonMethodResult(BridgeErrorCode bridgeErrorCode, Object result) {
        try {
            JSONObject resultJsonObj = new JSONObject();
            resultJsonObj.put(JSON_ERROR_CODE, bridgeErrorCode.getId());
            resultJsonObj.put(JSON_ERROR_MESSAGE, bridgeErrorCode.getErrorMessage());
            if (result == null) {
                resultJsonObj.put(CALL_METHOD_JSON_KEY, result);
            } else if (result.getClass().isArray()) {
                JSONArray array = ParameterHelper.objectTransformJsonArray(result);
                if (array != null) {
                    resultJsonObj.put(CALL_METHOD_JSON_KEY, array);
                }
            } else {
                resultJsonObj.put(CALL_METHOD_JSON_KEY, result.toString());
            }
            return resultJsonObj;
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Other platforms call methods.
     *
     * @param bridgeName Name of bridgePlugin.
     * @param methodName Name of method.
     * @param parameters Param of the method.
     */
    public void jsCallMethod(String bridgeName, String methodName, String parameters) {
        JSONObject resultJsonObj = new JSONObject();
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsCallMethod bridgeName is not found");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_INVALID;
            resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
            nativePlatformSendMethodResult(bridgeName, methodName,
                resultJsonObj.toString(), bridgePlugin.getInstanceId());
            return;
        }
        try {
            Object object = null;
            MethodData methodData = null;
            if (parameters.length() != NO_PARAM) {
                ALog.i(LOG_TAG, "The calling method has parameters");
                JSONObject paramJsonObj = new JSONObject(parameters);
                methodData = new MethodData(methodName, ParameterHelper.jsonTransformObject(paramJsonObj));
                object = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            } else {
                ALog.i(LOG_TAG, "The calling method has no parameters");
                Object[] objectParamters = {};
                methodData = new MethodData(methodName, objectParamters);
                object = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            }
            if (object == null) {
                bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_UNIMPL;
            }
            resultJsonObj = createJsonMethodResult(bridgeErrorCode, object);
            if (resultJsonObj == null) {
                resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
            }
            platformSendMethodResult(bridgeName, methodName, resultJsonObj.toString());
        } catch (JSONException e) {
            bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_UNIMPL;
            resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
            platformSendMethodResult(bridgeName, methodName, resultJsonObj.toString());
        }
    }

    /**
     * Return the call result.
     *
     * @param bridgeName Name of bridgePlugin.
     * @param methodName Name of method.
     * @param result result of the method.
     */
    public void platformSendMethodResult(String bridgeName, String methodName, String result) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformSendMethodResult bridgeName is not found.");
            return;
        }
        nativePlatformSendMethodResult(bridgeName, methodName, result, bridgePlugin.getInstanceId());
    }

    /**
     * Return the call method result.
     *
     * @param bridgeName Name of bridgePlugin.
     * @param methodData Method packaging structure.
     * @return Return the call method result.
     */
    public BridgeErrorCode platformCallMethod(String bridgeName, MethodData methodData) {
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformCallMethod bridgeName is not found.");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_NAME_ERROR;
            return bridgeErrorCode;
        }
        String methodName = methodData.getMethodName();
        String parameters = null;
        if (methodData.getMethodParameter() == null || methodData.getMethodParameter().length == 0) {
            ALog.i(LOG_TAG, "The calling method has no parameters");
            JSONObject paramJsonObj = new JSONObject();
            parameters = paramJsonObj.toString();
        } else {
            ALog.i(LOG_TAG, "The calling method has parameters");
            JSONObject JsonParameters = ParameterHelper.objectTransformJson(methodData.getMethodParameter());
            if (JsonParameters == null) {
                bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_PARAM_ERROR;
                return bridgeErrorCode;
            }
            parameters = JsonParameters.toString();
        }
        nativePlatformCallMethod(bridgeName, methodName, parameters, bridgePlugin.getInstanceId());
        return bridgeErrorCode;
    }

    /**
     * Other platforms send result of the method.
     *
     * @param bridgeName Object of bridgePlugin.
     * @param methodName Name of method.
     * @param result result of the method.
     */
    public void jsSendMethodResult(String bridgeName, String methodName, String result) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMethodResult bridgeName is not found.");
            return;
        }
        try {
            JSONObject resultValue = new JSONObject(result);
            Object resultObject = resultValue.get(CALL_METHOD_JSON_KEY);
            int errorCode = resultValue.getInt(JSON_ERROR_CODE);
            String errorMessage = resultValue.getString(JSON_ERROR_MESSAGE);
            bridgePlugin.jsSendMethodResult(resultObject, methodName, errorCode, errorMessage);
        } catch (JSONException e) {
            BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_DATA_ERROR;
            bridgePlugin.jsSendMethodResult(null, methodName,
                bridgeErrorCode.getId(), bridgeErrorCode.getErrorMessage());
            e.printStackTrace();
        }
    }

    private void platformSendMessageResponseErrorInfo(String bridgeName, BridgeErrorCode bridgeErrorCode) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMethodResult bridgeName is not found.");
            return;
        }
        try {
            JSONObject dataJson = new JSONObject();
            dataJson.put(MESSAGE_JSON_KEY, JSON_ERROR_CODE);
            dataJson.put(JSON_ERROR_CODE, bridgeErrorCode.getId());
            nativePlatformSendMessageResponse(bridgeName, dataJson.toString(), bridgePlugin.getInstanceId());
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    /**
     * Other platforms send Message data.
     *
     * @param bridgeName Object of bridgePlugin.
     * @param data Message data.
     */
    public void jsSendMessage(String bridgeName, String data) {
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            bridgeErrorCode = BridgeErrorCode.BRIDGE_NAME_ERROR;
            platformSendMessageResponseErrorInfo(bridgeName, bridgeErrorCode);
            return;
        }
        try {
            JSONObject dataJsonObj = new JSONObject(data);
            Object dataObj = dataJsonObj.get(MESSAGE_JSON_KEY);
            bridgePlugin.jsSendMessage(dataObj);
        } catch (JSONException e) {
            bridgeErrorCode = BridgeErrorCode.BRIDGE_DATA_ERROR;
            platformSendMessageResponseErrorInfo(bridgeName, bridgeErrorCode);
            e.printStackTrace();
        }
    }

    /**
     * Send data response to other platforms.
     *
     * @param bridgeName Object of bridgePlugin.
     * @param data Data to be sent.
     */
    public void platformSendMessageResponse(String bridgeName, Object data) {
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformSendMessageResponse bridgeName is not found.");
            return;
        }
        try {
            JSONObject dataJson = new JSONObject();
            if (data == null) {
                sendMessageResponseErrorCode(bridgeName, bridgePlugin.getInstanceId());
                return;
            } else if (data instanceof JSONObject) {
                dataJson.put(MESSAGE_JSON_KEY, data.toString());
            } else if (!data.getClass().isArray()) {
                dataJson.put(MESSAGE_JSON_KEY, data);
            } else if (data.getClass().isArray()) {
                JSONArray array = ParameterHelper.objectTransformJsonArray(data);
                if (array == null) {
                    sendMessageResponseErrorCode(bridgeName, bridgePlugin.getInstanceId());
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, array);
            } else {
                sendMessageResponseErrorCode(bridgeName, bridgePlugin.getInstanceId());
                return;
            }
            dataJson.put(JSON_ERROR_CODE, 0);
            nativePlatformSendMessageResponse(bridgeName, dataJson.toString(), bridgePlugin.getInstanceId());
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void sendMessageResponseErrorCode(String bridgeName, int instanceId) {
        JSONObject dataJson = new JSONObject();
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_DATA_ERROR;
        dataJson = createJsonMethodResult(bridgeErrorCode, DATA_ERROR);
        nativePlatformSendMessageResponse(bridgeName, dataJson.toString(), instanceId);
    }

    /**
     * Send data to other platforms.
     *
     * @param bridgeName Object of bridgePlugin.
     * @param data Data to be sent.
     */
    public void platformSendMessage(String bridgeName, Object data) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformSendMessage bridgeName is not found.");
            return;
        }
        try {
            JSONObject dataJson = new JSONObject();
            if (data == null) {
                jsSendMessageResponseErrorCode(bridgePlugin);
                return;
            } else if (data instanceof JSONObject) {
                dataJson.put(MESSAGE_JSON_KEY, data.toString());
            } else if (data.getClass().isArray()) {
                JSONArray array = ParameterHelper.objectTransformJsonArray(data);
                if (array == null) {
                    jsSendMessageResponseErrorCode(bridgePlugin);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, array);
            } else {
                dataJson.put(MESSAGE_JSON_KEY, data);
            }
            dataJson.put(JSON_ERROR_CODE, 0);
            nativePlatformSendMessage(bridgeName, dataJson.toString(), bridgePlugin.getInstanceId());
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void jsSendMessageResponseErrorCode(BridgePlugin bridgePlugin) {
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_DATA_ERROR;
        JSONObject dataJson = new JSONObject();
        dataJson = createJsonMethodResult(bridgeErrorCode, null);
        bridgePlugin.jsSendMessageResponse(dataJson);
    }

    /**
     * Other platforms send Message response data.
     *
     * @param bridgeName Object of bridgePlugin.
     * @param data Message data.
     */
    public void jsSendMessageResponse(String bridgeName, String data) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMessageResponse bridgeName is not found.");
            return;
        }
        try {
            JSONObject dataJsonObj = new JSONObject(data);
            Object dataObj = dataJsonObj.get(MESSAGE_JSON_KEY);
            bridgePlugin.jsSendMessageResponse(dataObj);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    /**
     * Method to unregister the platform.
     *
     * @param bridgeName Object of bridgePlugin.
     * @param methodName Name of method.
     */
    public void jsCancelMethod(String bridgeName, String methodName) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMessageResponse bridgeName is not found.");
            return;
        }
        bridgePlugin.jsCancelMethod(methodName);
    }

    /**
     * Init BridgeManager jni.
     *
     */
    public native void nativeInit();
    private native void nativePlatformCallMethod(String bridgeName,
        String methodName, String parameters, int instanceId);
    private native void nativePlatformSendMessageResponse(String bridgeName, String data, int instanceId);
    private native void nativePlatformSendMethodResult(String bridgeName,
        String methodName, String result, int instanceId);
    private native void nativePlatformSendMessage(String bridgeName, String data, int instanceId);
}
