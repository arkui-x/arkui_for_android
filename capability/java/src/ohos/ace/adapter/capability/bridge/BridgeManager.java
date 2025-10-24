/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
import java.util.Arrays;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.HashMap;
import java.util.Iterator;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.capability.bridge.BridgePlugin.BridgeType;

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

    private static volatile BridgeManager instance = null;

    private static BridgeBinaryCodec bridgeBinaryCodec_ = BridgeBinaryCodec.getInstance();

    private HashMap<String, BridgePlugin> bridgeMap_;

    private Lock bridgeMapLock_ = new ReentrantLock();

    private BridgeManager() {
        this.bridgeMap_ = new HashMap<String, BridgePlugin>();
        nativeInit();
    }

    /**
     * Get singleton instance of BridgeManager.
     *
     * @return BridgeManager singleton instance.
     */
    public static BridgeManager getInstance() {
        if (instance != null) {
            return instance;
        }

        synchronized (INSTANCE_LOCK) {
            if (instance == null) {
                instance = new BridgeManager();
            }
            return instance;
        }
    }

    /**
     * Register Bridge plugin.
     *
     * @param bridgeName Name of bridge.
     * @param bridgePlugin bridgePlugin object.
     * @return Success or fail.
     */
    public boolean registerBridgePlugin(String bridgeName, BridgePlugin bridgePlugin) {
        this.bridgeMapLock_.lock();
        try {
            if (this.bridgeMap_ != null) {
                this.bridgeMap_.put(bridgeName, bridgePlugin);
                ALog.i(LOG_TAG, "The BridgePlugin registered successfully, The bridgeName is " + bridgeName);
                int type = (bridgePlugin.getBridgeType() == BridgeType.JSON_TYPE) ? 0 : 1;
                if (nativeJSBridgeExists(bridgeName, type)) {
                    bridgePlugin.onRegisterResult(true);
                }
                return true;
            } else {
                ALog.e(LOG_TAG, "The BridgePlugin registration failure, The bridgeName is " + bridgeName);
                return false;
            }
        } finally {
            this.bridgeMapLock_.unlock();
        }
    }

    /**
     * Unregister the created bridge
     *
     * @param bridgeName name of bridge.
     * @return Success or fail.
     */
    public boolean unRegisterBridgePlugin(String bridgeName) {
        this.bridgeMapLock_.lock();
        try {
            if (this.bridgeMap_ != null && this.bridgeMap_.remove(bridgeName) != null) {
                return true;
            } else {
                ALog.e(LOG_TAG, "unRegisterBridgePlugin failed");
                return false;
            }
        } finally {
            this.bridgeMapLock_.unlock();
        }
    }

    private BridgePlugin findBridgePlugin(String bridgeName) {
        this.bridgeMapLock_.lock();
        try {
            if (this.bridgeMap_ != null) {
                return this.bridgeMap_.get(bridgeName);
            } else {
                ALog.e(LOG_TAG, "No BridgePlugin found, bridgeName is " + bridgeName);
                return null;
            }
        } finally {
            this.bridgeMapLock_.unlock();
        }
    }

    /**
     * release BridgeManager object.
     *
     */
    public void release() {
        Iterator<HashMap.Entry<String, BridgePlugin>> iterator = this.bridgeMap_.entrySet().iterator();
        while (iterator.hasNext()) {
            HashMap.Entry<String, BridgePlugin> entry = iterator.next();
            entry.getValue().release();
            iterator.remove();
        }
        ALog.i(LOG_TAG, "BridgeManager release.");
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
        ALog.d(LOG_TAG, "jsCallMethod enter, bridgeName is " + bridgeName + ", methodName is " + methodName);
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsCallMethod bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        BridgeThreadExecutor.getInstance().execute(new Runnable() {
            @Override
            public void run() {
                jsCallMethodInner(bridgePlugin, methodName, parameters);
            }
        });
    }

    /**
     * Other platforms call methods.
     *
     * @param bridgeName Name of bridge.
     * @param methodName Name of method.
     * @param parameters Param of the method.
     */
    private void jsCallMethodInner(BridgePlugin bridgePlugin, String methodName, String parameters) {
        JSONObject resultJsonObj = new JSONObject();
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        try {
            Object object = null;
            MethodData methodData = null;
            String splitName = splitMethodName(methodName);
            if (parameters.length() != NO_PARAM) {
                JSONObject paramJsonObj = new JSONObject(parameters);
                methodData = new MethodData(splitName, ParameterHelper.jsonTransformObject(paramJsonObj));
                object = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            } else {
                Object[] objectParameters = {};
                methodData = new MethodData(splitName, objectParameters);
                object = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            }
            ALog.i(LOG_TAG, "End of method call, the methodName is " + splitName);
            if (object != null && !ParameterHelper.isExceedJsSafeInteger(object)) {
                bridgeErrorCode = BridgeErrorCode.BRIDGE_EXCEEDS_SAFE_INTEGER;
            }
            if (object != null && object instanceof BridgeErrorCode) {
                bridgeErrorCode = (BridgeErrorCode) object;
            }
            resultJsonObj = createJsonMethodResult(bridgeErrorCode, object);
            if (resultJsonObj == null) {
                ALog.e(LOG_TAG, "createJsonMethodResult failed");
                resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
            }
            platformSendMethodResult(bridgePlugin.getBridgeName(), methodName, resultJsonObj.toString());
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "jsCallMethod failed");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_UNIMPL;
            resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
            platformSendMethodResult(bridgePlugin.getBridgeName(), methodName, resultJsonObj.toString());
        }
    }

    /**
     * Other platforms call methods.
     *
     * @param bridgeName Name of bridge.
     * @param methodName Name of method.
     * @param parameters Param of the method.
     */
    public String jsCallMethodSync(String bridgeName, String methodName, String parameters) {
        ALog.d(LOG_TAG, "jsCallMethodSync enter, bridgeName is " + bridgeName + ", methodName is " + methodName);
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsCallMethodSync bridgePlugin Not found, bridgeName is " + bridgeName);
            return null;
        }
        return jsCallMethodSyncInner(bridgePlugin, methodName, parameters);
    }

    private String jsCallMethodSyncInner(BridgePlugin bridgePlugin, String methodName, String parameters) {
        JSONObject resultJsonObj = new JSONObject();
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        try {
            Object object = null;
            MethodData methodData = null;
            String splitName = splitMethodName(methodName);
            if (parameters.length() != NO_PARAM) {
                JSONObject paramJsonObj = new JSONObject(parameters);
                methodData = new MethodData(splitName, ParameterHelper.jsonTransformObject(paramJsonObj));
                object = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            } else {
                Object[] objectParameters = {};
                methodData = new MethodData(splitName, objectParameters);
                object = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            }
            ALog.i(LOG_TAG, "End of method call, the methodName is " + splitName);
            if (object != null && !ParameterHelper.isExceedJsSafeInteger(object)) {
                bridgeErrorCode = BridgeErrorCode.BRIDGE_EXCEEDS_SAFE_INTEGER;
            }
            if (object != null && object instanceof BridgeErrorCode) {
                bridgeErrorCode = (BridgeErrorCode) object;
            }
            resultJsonObj = createJsonMethodResult(bridgeErrorCode, object);
            if (resultJsonObj == null) {
                ALog.e(LOG_TAG, "createJsonMethodResult failed");
                resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
            }
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "jsCallMethod failed");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_UNIMPL;
            resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
        }
        return resultJsonObj.toString();
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
            ALog.e(LOG_TAG, "platformSendMethodResult bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        nativePlatformSendMethodResult(bridgeName, methodName, result);
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
            ALog.e(LOG_TAG, "platformCallMethod bridgePlugin Not found, bridgeName is " + bridgeName);
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
        nativePlatformCallMethod(bridgeName, methodName, parameters);
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
            ALog.e(LOG_TAG, "jsSendMethodResult bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        jsSendMethodResultInner(bridgePlugin, methodName, result);
    }

    private void jsSendMethodResultInner(BridgePlugin bridgePlugin, String methodName, String result) {
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
            ALog.e(LOG_TAG, "platformSendMessageResponseErrorInfo bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        try {
            JSONObject dataJson = new JSONObject();
            dataJson.put(MESSAGE_JSON_KEY, JSON_ERROR_CODE);
            dataJson.put(JSON_ERROR_CODE, bridgeErrorCode.getId());
            nativePlatformSendMessageResponse(bridgeName, dataJson.toString());
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
        jsSendMessageInner(bridgePlugin, bridgeName, data);
    }

    private void jsSendMessageInner(BridgePlugin bridgePlugin, String bridgeName, String data) {
        try {
            JSONObject dataJsonObj = new JSONObject(data);
            Object dataObj =
                (dataJsonObj.get(MESSAGE_JSON_KEY) == JSONObject.NULL) ? null : dataJsonObj.get(MESSAGE_JSON_KEY);
            bridgePlugin.jsSendMessage(dataObj);
        } catch (JSONException e) {
            BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_DATA_ERROR;
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
            ALog.e(LOG_TAG, "platformSendMessageResponse bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        try {
            if (data == null) {
                sendMessageResponseErrorCode(bridgeName, BridgeErrorCode.BRIDGE_DATA_ERROR);
                return;
            }
            JSONObject dataJson = new JSONObject();
            if (data instanceof JSONObject) {
                dataJson.put(MESSAGE_JSON_KEY, data.toString());
            } else if (!data.getClass().isArray()) {
                if (!ParameterHelper.isExceedJsSafeInteger(data)) {
                    sendMessageResponseErrorCode(bridgeName, BridgeErrorCode.BRIDGE_EXCEEDS_SAFE_INTEGER);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, data);
            } else if (data.getClass().isArray()) {
                JSONArray array = ParameterHelper.objectTransformJsonArray(data);
                if (array == null) {
                    sendMessageResponseErrorCode(bridgeName, BridgeErrorCode.BRIDGE_DATA_ERROR);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, array);
            } else {
                sendMessageResponseErrorCode(bridgeName, BridgeErrorCode.BRIDGE_DATA_ERROR);
                return;
            }
            dataJson.put(JSON_ERROR_CODE, 0);
            nativePlatformSendMessageResponse(bridgeName, dataJson.toString());
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "platformSendMessageResponse failed, JSONException.");
        }
    }

    private void sendMessageResponseErrorCode(String bridgeName, BridgeErrorCode bridgeErrorCode) {
        JSONObject dataJson = new JSONObject();
        dataJson = createJsonMethodResult(bridgeErrorCode, DATA_ERROR);
        nativePlatformSendMessageResponse(bridgeName, dataJson.toString());
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
            ALog.e(LOG_TAG, "platformSendMessage bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        try {
            JSONObject dataJson = new JSONObject();
            if (data == null) {
                dataJson.put(MESSAGE_JSON_KEY, data);
                dataJson.put(JSON_ERROR_CODE, 0);
                nativePlatformSendMessage(bridgeName, dataJson.toString());
                return;
            }
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
            nativePlatformSendMessage(bridgeName, dataJson.toString());
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
            ALog.e(LOG_TAG, "jsSendMessageResponse bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        jsSendMessageResponseInner(bridgePlugin, data);
    }

    private void jsSendMessageResponseInner(BridgePlugin bridgePlugin, String data) {
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
            ALog.e(LOG_TAG, "jsCancelMethod bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        bridgePlugin.jsCancelMethod(methodName);
    }

    /**
     * Send binaryData to other platforms.
     *
     * @param bridgeName Name of bridge.
     * @param data Data to be sent.
     */
    public void platformSendMessageBinary(String bridgeName, Object data) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformSendMessageBinary bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            return;
        }
        ByteBuffer buffer = bridgeBinaryCodec_.encodeData(data);
        nativePlatformSendMessageBinary(bridgeName, buffer);
    }

    private void platformSendMethodResultBinaryInner(String bridgeName, String methodName,
        ByteBuffer resultBuffer, BridgeErrorCode bridgeErrorCode) {
        nativePlatformSendMethodResultBinary(bridgeName, methodName, resultBuffer,
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
        ALog.d(LOG_TAG, "jsCallMethodBinary enter, bridgeName is " + bridgeName + ", methodName is " + methodName);
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsCallMethodBinary bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        jsCallMethodBinaryInner(bridgePlugin, methodName, bufferData);
    }

    private void jsCallMethodBinaryInner(BridgePlugin bridgePlugin, String methodName, ByteBuffer bufferData) {
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH;
        }
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_CODEC_INVALID;
        }
        Object paramObj = bridgeBinaryCodec_.decodeData(bufferData);
        String splitName = splitMethodName(methodName);
        Object resultObject = null;
        MethodData methodData = null;
        ByteBuffer resultBuffer = bridgeBinaryCodec_.encodeData(null);
        if (paramObj == null) {
            bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        } else {
            Object[] objects = ParameterHelper.binaryTransformObject(paramObj);
            if (objects == null || ParameterHelper.containsNull(Arrays.asList(objects))) {
                ALog.e(LOG_TAG, "Parameter parsing failed.");
                bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_PARAM_ERROR;
            } else {
                methodData = new MethodData(splitName, objects);
                resultObject = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
                ALog.i(LOG_TAG, "End of method call, the methodName is " + splitName);
                if (resultObject != null && resultObject.getClass() == BridgeErrorCode.class) {
                    bridgeErrorCode = (BridgeErrorCode) resultObject;
                } else {
                    resultBuffer = bridgeBinaryCodec_.encodeData(resultObject);
                }
            }
        }
        platformSendMethodResultBinaryInner(bridgePlugin.getBridgeName(), methodName, resultBuffer, bridgeErrorCode);
    }

    /**
     * Other platforms call methods by binary type.
     *
     * @param bridgeName Name of bridge.
     * @param methodName Name of method.
     * @param bufferData Method data.
     */
    public BinaryResultHolder jsCallMethodBinarySync(String bridgeName, String methodName, ByteBuffer bufferData) {
        ALog.d(LOG_TAG, "jsCallMethodBinary enter, bridgeName is " + bridgeName + ", methodName is " + methodName);
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsCallMethodBinary bridgePlugin Not found, bridgeName is " + bridgeName);
            BinaryResultHolder holder = new BinaryResultHolder();
            holder.errorCode = BridgeErrorCode.BRIDGE_INVALID.getId();
            holder.result = ByteBuffer.allocateDirect(0);
            return holder;
        }
        return jsCallMethodBinarySyncInner(bridgePlugin, methodName, bufferData);
    }

    private BinaryResultHolder createResultHolder(int codeId, ByteBuffer result) {
        BinaryResultHolder holder = new BinaryResultHolder();
        holder.errorCode = codeId;
        holder.result = result;
        return holder;
    }

    private BinaryResultHolder jsCallMethodBinarySyncInner(
        BridgePlugin bridgePlugin, String methodName, ByteBuffer bufferData) {
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            return createResultHolder(
                BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH.getId(), ByteBuffer.allocateDirect(0));
        }
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            return createResultHolder(BridgeErrorCode.BRIDGE_CODEC_INVALID.getId(), ByteBuffer.allocateDirect(0));
        }
        Object paramObj = bridgeBinaryCodec_.decodeData(bufferData);
        String splitName = splitMethodName(methodName);
        Object resultObject = null;
        MethodData methodData = null;
        ByteBuffer resultBuffer = bridgeBinaryCodec_.encodeData(null);
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        if (paramObj != null) {
            bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        }
        Object[] objects = ParameterHelper.binaryTransformObject(paramObj);
        if (objects == null || ParameterHelper.containsNull(Arrays.asList(objects))) {
            ALog.e(LOG_TAG, "Parameter parsing failed.");
            bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_PARAM_ERROR;
        } else {
            methodData = new MethodData(splitName, objects);
            resultObject = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
            ALog.i(LOG_TAG, "End of method call, the methodName is " + splitName);
            if (resultObject != null && resultObject instanceof BridgeErrorCode) {
                bridgeErrorCode = (BridgeErrorCode) resultObject;
            } else {
                resultBuffer = bridgeBinaryCodec_.encodeData(resultObject);
            }
        }
        return createResultHolder(bridgeErrorCode.getId(), resultBuffer);
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
            ALog.e(LOG_TAG, "jsSendMessageBinary bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            return;
        }
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            return;
        }
        Object dataObj = bridgeBinaryCodec_.decodeData(bufferData);
        bridgePlugin.jsSendMessage(dataObj);
    }

    /**
     * platforms call methods by binary type.
     *
     * @param bridgeName Name of bridge.
     * @param methodData Method data.
     */
    public BridgeErrorCode platformCallMethodBinary(String bridgeName, MethodData methodData) {
        BridgeErrorCode errorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformCallMethodBinary bridgePlugin Not found, bridgeName is " + bridgeName);
            errorCode = BridgeErrorCode.BRIDGE_NAME_ERROR;
        }
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            errorCode = BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH;
        }
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            errorCode = BridgeErrorCode.BRIDGE_CODEC_INVALID;
        }
        if (errorCode == BridgeErrorCode.BRIDGE_ERROR_NO) {
            String methodName = methodData.getMethodName();
            Object[] params = methodData.getMethodParameter();
            ByteBuffer buffer = bridgeBinaryCodec_.encodeData(params);
            nativePlatformCallMethodBinary(bridgeName, methodName, buffer);
        }
        return errorCode;
    }

    private synchronized ByteBuffer copyByteBuffer(ByteBuffer byteBuffer) {
        if (byteBuffer != null) {
            byte[] byteArray = new byte[byteBuffer.capacity()];
            byteBuffer.get(byteArray);
            ByteBuffer copyBuffer = ByteBuffer.allocateDirect(byteArray.length);
            copyBuffer.put(byteArray);
            copyBuffer.flip();
            return copyBuffer;
        } else {
            return null;
        }
    }

    /**
     * Other platforms send result of the method by binary type.
     *
     * @param bridgeName Name of bridge.
     * @param methodName Name of method.
     * @param result result of the method.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    public void jsSendMethodResultBinary(String bridgeName, String methodName, ByteBuffer result,
    int errorCode, String errorMessage) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMethodResultBinary bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        ByteBuffer copyBufferResult = copyByteBuffer(result);
        jsSendMethodResultBinaryInner(bridgePlugin, methodName, copyBufferResult, errorCode, errorMessage);
    }

    private void jsSendMethodResultBinaryInner(BridgePlugin bridgePlugin, String methodName, ByteBuffer result,
    int errorCode, String errorMessage) {
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            bridgePlugin.jsSendMethodResult(null, methodName,
                BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH.getId(),
                BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH.getErrorMessage());
            return;
        }
        Object resultObj = bridgeBinaryCodec_.decodeData(result);
        bridgePlugin.jsSendMethodResult(resultObj, methodName, errorCode, errorMessage);
    }

    /**
     * Return the call method result.
     *
     * @param bridgeName Name of bridge.
     * @param methodData Method packaging structure.
     * @return Return the execution result of the ArkTS method.
     * @throws IllegalStateException Return error code.
     */
    public Object platformCallMethodSync(String bridgeName, MethodData methodData) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformCallMethod bridgePlugin Not found, bridgeName is " + bridgeName);
            throw new IllegalStateException("Bridge not found: " + bridgeName + ", errorCode = "
                + BridgeErrorCode.BRIDGE_NAME_ERROR.getId());
        }
        String methodName = methodData.getMethodName();
        String parameters;
        if (methodData.getMethodParameter() == null || methodData.getMethodParameter().length == 0) {
            ALog.i(LOG_TAG, "The calling method has no parameters");
            parameters = new JSONObject().toString();
        } else {
            ALog.i(LOG_TAG, "The calling method has parameters");
            JSONObject jsonParameters = ParameterHelper.objectTransformJson(methodData.getMethodParameter());
            if (jsonParameters == null) {
                throw new IllegalStateException("Invalid method parameters, method=" + methodName + ", errorCode = "
                    + BridgeErrorCode.BRIDGE_METHOD_PARAM_ERROR.getId());
            }
            parameters = jsonParameters.toString();
        }
        String result = nativePlatformCallMethodSync(bridgeName, methodName, parameters);
        return parseSyncResultOrThrow(result, methodName);
    }

    private Object parseSyncResultOrThrow(String result, String methodName) {
        try {
            JSONObject resultValue = new JSONObject(result);
            int errorCode = resultValue.getInt(JSON_ERROR_CODE);
            if (errorCode == 0) {
                return resultValue.get(CALL_METHOD_JSON_KEY);
            }
            throw new IllegalStateException(
                "Call method failed, method = " + methodName + ", errorCode = " + errorCode);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "platformCallMethodSync parse result failed, JSONException.");
            throw new RuntimeException("platformCallMethodSync parse result failed.", e);
        }
    }

    /**
     * platforms call methods by binary type.
     *
     * @param bridgeName Name of bridge.
     * @param methodData Method data.
     * @throws IllegalStateException Return error code.
     */
    public Object platformCallMethodSyncBinary(String bridgeName, MethodData methodData) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "platformCallMethodBinary bridgePlugin Not found, bridgeName is " + bridgeName);
            throw new IllegalStateException("Bridge not found: " + bridgeName + ", errorCode = "
                + BridgeErrorCode.BRIDGE_NAME_ERROR.getId());
        }
        if (bridgePlugin.getBridgeType() != BridgeType.BINARY_TYPE) {
            ALog.e(LOG_TAG, "The bridge is not BINARY_TYPE.");
            throw new IllegalStateException("Bridge type mismatch, bridge=" + bridgeName + ", errorCode = "
                + BridgeErrorCode.BRIDGE_CODEC_TYPE_MISMATCH.getId());
        }
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            throw new IllegalStateException("Bridge binary codec invalid, bridge=" + bridgeName + ", errorCode = "
                + BridgeErrorCode.BRIDGE_CODEC_INVALID.getId());
        }
        String methodName = methodData.getMethodName();
        Object[] params = methodData.getMethodParameter();
        ByteBuffer buffer = bridgeBinaryCodec_.encodeData(params);
        BinaryResultHolder holder = nativePlatformCallMethodSyncBinary(bridgeName, methodName, buffer);
        if (holder == null) {
            throw new IllegalStateException("CallMethodSyncBinary failed, method = " + methodName);
        }
        if (holder.errorCode != 0) {
            throw new IllegalStateException("CallMethodSyncBinary failed, method = " + methodName + ", errorCode = "
                + holder.errorCode);
        }
        ByteBuffer copyBufferResult = copyByteBuffer(holder.result);
        return bridgeBinaryCodec_.decodeData(copyBufferResult);
    }

    /**
     * Notify the result of the registration.
     *
     * @param bridgeName Name of bridge.
     * @param bridgeType Type of bridge.
     * @param available Registration result.
     */
    public void jsOnRegisterResult(String bridgeName, int bridgeType, boolean available) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsOnRegisterResult bridgePlugin Not found, bridgeName is " + bridgeName);
            return;
        }
        if (!available) {
            bridgePlugin.onRegisterResult(available);
            return;
        }
        int type = (bridgePlugin.getBridgeType() == BridgeType.JSON_TYPE) ? 0 : 1;
        if (bridgeType == type) {
            bridgePlugin.onRegisterResult(available);
        }
    }

    /**
     * The class that returns a value for binary synchronous calls.
     */
    public static class BinaryResultHolder {
        /**
         * Error codes for binary synchronous calls: 0 indicates success, and non-0 indicates failure.
         */
        public int errorCode;

        /**
         * The data result buffer returned by the binary synchronous call.
         */
        public ByteBuffer result;
    }

    /**
     * Initialize native BridgeManager.
     */
    public native void nativeInit();

    private native void nativePlatformCallMethod(String bridgeName, String methodName, String parameters);
    private native void nativePlatformSendMessageResponse(String bridgeName, String data);
    private native void nativePlatformSendMethodResult(String bridgeName, String methodName, String result);
    private native void nativePlatformSendMessage(String bridgeName, String data);
    private native void nativePlatformSendMessageBinary(String bridgeName, ByteBuffer buffer);
    private native void nativePlatformSendMethodResultBinary(
        String bridgeName, String methodName, ByteBuffer buffer, int errorCode, String errorMessage);
    private native void nativePlatformCallMethodBinary(String bridgeName, String methodName, ByteBuffer parameters);
    private native String nativePlatformCallMethodSync(String bridgeName, String methodName, String parameters);
    private native BinaryResultHolder nativePlatformCallMethodSyncBinary(
        String bridgeName, String methodName, ByteBuffer parameters);
    private native boolean nativeJSBridgeExists(String bridgeName, int bridgeType);
}
