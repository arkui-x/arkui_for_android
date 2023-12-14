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

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.capability.bridge.BridgePlugin.BridgeType;

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

    private static final String JSON_ERROR_CODE = "errorCode";

    private static final String JSON_ERROR_MESSAGE = "errorMessage";

    private static final String CALL_METHOD_JSON_KEY = "result";

    private static final String MESSAGE_JSON_KEY = "result";

    private static final String DATA_ERROR = "data_error";

    private static final String SEPARATOR = "$";

    private static final int NO_PARAM = 4;

    private static final Object INSTANCE_LOCK = new Object();

    private static volatile BridgeManager INSTANCE = null;

    private static HashMap<String, BridgePlugin> bridgeMap_;

    private static BridgeBinaryCodec bridgeBinaryCodec = BridgeBinaryCodec.getInstance();

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
     * @param bridgeName Name of bridge.
     * @param bridgePlugin bridgePlugin object.
     * @return Success or not.
     */
    public boolean registerBridgePlugin(String bridgeName, BridgePlugin bridgePlugin) {
        Lock registerLock = new ReentrantLock();
        registerLock.lock();
        try {
            if (bridgeMap_.containsKey(bridgeName)) {
                ALog.e(LOG_TAG, "The bridgeName Already exists");
                return false;
            } else {
                ALog.i(LOG_TAG, "registerBridgePlugin success");
                bridgeMap_.put(bridgeName, bridgePlugin);
                return true;
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

    /**
     * Unregister Bridge plugin by instanceId.
     *
     * @param instanceId the id of instance.
     */
    public static void deleteBridgeByInstanceId(int instanceId) {
        if (bridgeMap_.isEmpty()) {
            return;
        }
        List<String> keyList = new ArrayList<>();
        Lock deleteBridgeLock = new ReentrantLock();
        deleteBridgeLock.lock();
        try {
            for (Map.Entry<String, BridgePlugin> entry : bridgeMap_.entrySet()) {
                if (entry.getValue().getInstanceId() == instanceId) {
                    keyList.add(entry.getKey());
                }
            }
            for (String deleteKey : keyList) {
                bridgeMap_.remove(deleteKey);
                ALog.i("Successfully deleted bridge through instanceId, name is ", deleteKey);
            }
        } finally {
            deleteBridgeLock.unlock();
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
            if (result != null && result.getClass().isArray()) {
                JSONArray array = ParameterHelper.objectTransformJsonArray(result);
                if (array != null) {
                    resultJsonObj.put(CALL_METHOD_JSON_KEY, array);
                } else {
                    resultJsonObj = null;
                }
            } else {
                resultJsonObj.put(CALL_METHOD_JSON_KEY, result);
            }
            return resultJsonObj;
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "createJsonMethodResult failed, JSONException.");
            return null;
        }
    }

    private String splitMethodName(String methodName) {
        if (methodName != null && methodName.contains(SEPARATOR)) {
            return methodName.substring(0, methodName.indexOf(SEPARATOR));
        } else {
            return methodName;
        }
    }

    /**
     * Other platforms call methods.
     *
     * @param bridgeName Name of bridge.
     * @param methodName Name of method.
     * @param parameters Param of the method.
     */
    public void jsCallMethod(String bridgeName, String methodName, String parameters) {
        JSONObject resultJsonObj = new JSONObject();
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsCallMethod bridgeName is not found");
            return;
        }
        try {
            Object object = null;
            MethodData methodData = null;
            String splitName = splitMethodName(methodName);
            if (parameters.length() != NO_PARAM) {
                ALog.i(LOG_TAG, "The calling method has parameters");
                JSONObject paramJsonObj = new JSONObject(parameters);
                methodData = new MethodData(splitName, ParameterHelper.jsonTransformObject(paramJsonObj));
                object = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            } else {
                ALog.i(LOG_TAG, "The calling method has no parameters");
                Object[] objectParamters = {};
                methodData = new MethodData(splitName, objectParamters);
                object = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            }
            if (object != null && !ParameterHelper.isExceedJsSafeInteger(object)) {
                bridgeErrorCode = BridgeErrorCode.BRIDGE_EXCEEDS_SAFE_INTEGER;
            }
            if (object != null && object.getClass() == BridgeErrorCode.class) {
                bridgeErrorCode = (BridgeErrorCode) object;
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
     * @param bridgeName Name of bridge.
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
     * @param bridgeName Name of bridge.
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
     * @param bridgeName Name of bridge.
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
            int errorCode = resultValue.getInt(JSON_ERROR_CODE);
            Object resultObject = resultValue.get(CALL_METHOD_JSON_KEY);
            String errorMessage = BridgeErrorCode.BRIDGE_ERROR_NO.getErrorMessage();
            if (errorCode == 0) {
                bridgePlugin.jsSendMethodResult(resultObject, methodName, errorCode, errorMessage);
            } else {
                errorMessage = resultValue.getString(JSON_ERROR_MESSAGE);
                bridgePlugin.jsSendMethodResult(resultObject, methodName, errorCode, errorMessage);
            }
        } catch (JSONException e) {
            BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_DATA_ERROR;
            bridgePlugin.jsSendMethodResult(null, methodName,
                bridgeErrorCode.getId(), bridgeErrorCode.getErrorMessage());
            ALog.e(LOG_TAG, "jsSendMethodResult failed, JSONException.");
        }
    }

    private void platformSendMessageResponseErrorInfo(String bridgeName, BridgeErrorCode bridgeErrorCode) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformSendMessageResponseErrorInfo bridgeName is not found.");
            return;
        }
        try {
            JSONObject dataJson = new JSONObject();
            dataJson.put(MESSAGE_JSON_KEY, JSON_ERROR_CODE);
            dataJson.put(JSON_ERROR_CODE, bridgeErrorCode.getId());
            nativePlatformSendMessageResponse(bridgeName, dataJson.toString(), bridgePlugin.getInstanceId());
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "platformSendMessageResponseErrorInfo failed, JSONException.");
        }
    }

    /**
     * Other platforms send Message data.
     *
     * @param bridgeName Name of bridge.
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
            ALog.e(LOG_TAG, "jsSendMessage failed, JSONException.");
        }
    }

    /**
     * Send data response to other platforms.
     *
     * @param bridgeName Name of bridge.
     * @param data Data to be sent.
     */
    public void platformSendMessageResponse(String bridgeName, Object data) {
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformSendMessageResponse bridgeName is not found.");
            return;
        }
        int instanceId = bridgePlugin.getInstanceId();
        try {
            if (data == null) {
                sendMessageResponseErrorCode(bridgeName, instanceId, BridgeErrorCode.BRIDGE_DATA_ERROR);
                return;
            }
            JSONObject dataJson = new JSONObject();
            if (data instanceof JSONObject) {
                dataJson.put(MESSAGE_JSON_KEY, data.toString());
            } else if (!data.getClass().isArray()) {
                if (!ParameterHelper.isExceedJsSafeInteger(data)) {
                    sendMessageResponseErrorCode(bridgeName, instanceId, BridgeErrorCode.BRIDGE_EXCEEDS_SAFE_INTEGER);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, data);
            } else if (data.getClass().isArray()) {
                JSONArray array = ParameterHelper.objectTransformJsonArray(data);
                if (array == null) {
                    sendMessageResponseErrorCode(bridgeName, instanceId, BridgeErrorCode.BRIDGE_DATA_ERROR);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, array);
            } else {
                sendMessageResponseErrorCode(bridgeName, instanceId, BridgeErrorCode.BRIDGE_DATA_ERROR);
                return;
            }
            dataJson.put(JSON_ERROR_CODE, 0);
            nativePlatformSendMessageResponse(bridgeName, dataJson.toString(), instanceId);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "platformSendMessageResponse failed, JSONException.");
        }
    }

    private void sendMessageResponseErrorCode(String bridgeName, int instanceId, BridgeErrorCode bridgeErrorCode) {
        JSONObject dataJson = new JSONObject();
        dataJson = createJsonMethodResult(bridgeErrorCode, DATA_ERROR);
        nativePlatformSendMessageResponse(bridgeName, dataJson.toString(), instanceId);
    }

    /**
     * Send data to other platforms.
     *
     * @param bridgeName Name of bridge.
     * @param data Data to be sent.
     */
    public void platformSendMessage(String bridgeName, Object data) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformSendMessage bridgeName is not found.");
            return;
        }
        try {
            if (data == null) {
                jsSendMessageResponseErrorCode(bridgePlugin, BridgeErrorCode.BRIDGE_DATA_ERROR);
                return;
            }
            JSONObject dataJson = new JSONObject();
            if (data instanceof JSONObject) {
                dataJson.put(MESSAGE_JSON_KEY, data.toString());
            } else if (data.getClass().isArray()) {
                JSONArray array = ParameterHelper.objectTransformJsonArray(data);
                if (array == null) {
                    jsSendMessageResponseErrorCode(bridgePlugin, BridgeErrorCode.BRIDGE_DATA_ERROR);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, array);
            } else {
                if (!ParameterHelper.isExceedJsSafeInteger(data)) {
                    jsSendMessageResponseErrorCode(bridgePlugin, BridgeErrorCode.BRIDGE_EXCEEDS_SAFE_INTEGER);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, data);
            }
            dataJson.put(JSON_ERROR_CODE, 0);
            nativePlatformSendMessage(bridgeName, dataJson.toString(), bridgePlugin.getInstanceId());
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "platformSendMessage failed, JSONException.");
        }
    }

    private void jsSendMessageResponseErrorCode(BridgePlugin bridgePlugin, BridgeErrorCode bridgeErrorCode) {
        JSONObject dataJson = new JSONObject();
        dataJson = createJsonMethodResult(bridgeErrorCode, null);
        bridgePlugin.jsSendMessageResponse(dataJson);
    }

    /**
     * Other platforms send Message response data.
     *
     * @param bridgeName Name of bridge.
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
            ALog.e(LOG_TAG, "jsSendMessageResponse failed, JSONException.");
        }
    }

    /**
     * Method to unregister the platform.
     *
     * @param bridgeName Name of bridge.
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
     * Send binarydata to other platforms.
     *
     * @param bridgeName Name of bridge.
     * @param data Data to be sent.
     */
    public void platformSendMessageBinary(String bridgeName, Object data) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformSendMessageBinary bridgeName is not found.");
            return;
        }
        if (bridgeBinaryCodec == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            return;
        }
        ByteBuffer buffer = bridgeBinaryCodec.encodeData(data);
        nativePlatformSendMessageBinary(bridgeName, buffer, bridgePlugin.getInstanceId());
    }

    private void PlatformSendMethodResultBinaryInner(String bridgeName, String methodName,
        ByteBuffer resultBuffer, int instanceId, BridgeErrorCode bridgeErrorCode) {
        nativePlatformSendMethodResultBinary(bridgeName, methodName, resultBuffer, instanceId,
            bridgeErrorCode.getId(), bridgeErrorCode.getErrorMessage());
    }

    /**
     * Other platforms call methods by binary type.
     *
     * @param bridgeName Name of bridge.
     * @param methodName Name of method.
     * @param bufferData Method data.
     */
    public void jsCallMethodBinary(String bridgeName, String methodName, ByteBuffer bufferData) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMessageBinary bridgeName is not found.");
            return;
        }
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH;
        }
        if (bridgeBinaryCodec == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_CODEC_INVALID;
        }
        Object paramObj = bridgeBinaryCodec.decodeData(bufferData);
        String splitName = splitMethodName(methodName);
        Object resultObject = null;
        MethodData methodData = null;
        ByteBuffer resultBuffer = bridgeBinaryCodec.encodeData(null);
        int instanceId = bridgePlugin.getInstanceId();
        if (paramObj == null) {
            bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        } else {
            Object[] objects = ParameterHelper.binaryTransformObject(paramObj);
            if (objects == null) {
                ALog.e(LOG_TAG, "Parameter parsing failed.");
                bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_PARAM_ERROR;
            } else {
                methodData = new MethodData(splitName, objects);
                resultObject = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
                if (resultObject != null && resultObject.getClass() == BridgeErrorCode.class) {
                    bridgeErrorCode = (BridgeErrorCode) resultObject;
                } else {
                    resultBuffer = bridgeBinaryCodec.encodeData(resultObject);
                }
            }
        }
        PlatformSendMethodResultBinaryInner(bridgeName, methodName, resultBuffer, instanceId, bridgeErrorCode);
    }

    /**
     * Other platforms send Message data by binary type.
     *
     * @param bridgeName Name of bridge.
     * @param bufferData Send data.
     */
    public void jsSendMessageBinary(String bridgeName, ByteBuffer bufferData) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMessageBinary bridgeName is not found.");
            return;
        }
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            return;
        }
        if (bridgeBinaryCodec == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            return;
        }
        Object dataObj = bridgeBinaryCodec.decodeData(bufferData);
        bridgePlugin.jsSendMessage(dataObj);
    }

    /**
     * platforms call methods by binary type.
     *
     * @param bridgeName Name of bridge.
     * @param bufferData Method data.
     */
    public BridgeErrorCode platformCallMethodBinary(String bridgeName, MethodData methodData) {
        BridgeErrorCode errorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformCallMethodBinary bridgeName is not found.");
            errorCode = BridgeErrorCode.BRIDGE_NAME_ERROR;
        }
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            errorCode = BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH;
        }
        if (bridgeBinaryCodec == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            errorCode = BridgeErrorCode.BRIDGE_CODEC_INVALID;
        }
        if (errorCode == BridgeErrorCode.BRIDGE_ERROR_NO) {
            String methodName = methodData.getMethodName();
            Object[] params = methodData.getMethodParameter();
            ByteBuffer buffer = bridgeBinaryCodec.encodeData(params);
            nativeplatformCallMethodBinary(bridgeName, methodName, buffer, bridgePlugin.getInstanceId());
        }
        return errorCode;
    }

    /**
     * Other platforms send result of the method by binary type.
     *
     * @param bridgeName Name of bridge.
     * @param methodName Name of method.
     * @param result result of the method.
     */
    public void jsSendMethodResultBinary(String bridgeName, String methodName, ByteBuffer result,
    int errorCode, String errorMessage) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMethodResultBinary bridgeName is not found.");
            return;
        }
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            bridgePlugin.jsSendMethodResult(null, methodName,
                BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH.getId(),
                BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH.getErrorMessage());
            return;
        }
        Object resultObj = bridgeBinaryCodec.decodeData(result);
        bridgePlugin.jsSendMethodResult(resultObj, methodName, errorCode, errorMessage);
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
    private native void nativePlatformSendMessageBinary(String bridgeName, ByteBuffer buffer, int instanceId);
    private native void nativePlatformSendMethodResultBinary(String bridgeName, String methodName, ByteBuffer buffer,
        int instanceId, int errorCode, String ErrorMessage);
    private native void nativeplatformCallMethodBinary(String bridgeName,
        String methodName, ByteBuffer parameters, int instanceId);
}
