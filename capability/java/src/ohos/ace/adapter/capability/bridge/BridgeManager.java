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
import java.util.HashMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
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

    private HashMap<String, BridgePlugin> bridgeMap_;

    private HashMap<String, BridgeTaskQueueHandler> handlerMap_;

    private static HashMap<Integer, BridgeManager> managerMap_ = new HashMap<Integer, BridgeManager>();

    private Lock bridgeMapLock_ = new ReentrantLock();

    private static Lock managerMapLock_ = new ReentrantLock();

    private Lock handlerMapLock_ = new ReentrantLock();

    private int instanceId_ = 0;

    private static BridgeBinaryCodec bridgeBinaryCodec_ = BridgeBinaryCodec.getInstance();

    /**
     * Constructor of BridgeManager.
     * 
     * @param instanceId the id of instance.
     * @return BridgeManager object.
     */
    public BridgeManager(int instanceId) {
        this.bridgeMap_ = new HashMap<String, BridgePlugin>();
        this.handlerMap_ = new HashMap<String, BridgeTaskQueueHandler>();
        this.instanceId_ = instanceId;
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
                ALog.i(LOG_TAG, "The BridgePlugin registered successfully");
                return true;
            } else {
                ALog.e(LOG_TAG, "BridgeMap is null, the BridgePlugin registration failed");
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
                return null;
            }
        } finally {
            this.bridgeMapLock_.unlock();
        }
    }

    /**
     * Register Bridge plugin.
     *
     * @param instanceId the id of instance.
     * @param bridgeManager BridgeManager object.
     * @return Success or fail.
     */
    public static boolean registerBridgeManager(int instanceId, BridgeManager bridgeManager) {
        managerMapLock_.lock();
        try {
            if (managerMap_ != null && managerMap_.containsKey(instanceId)) {
                ALog.e(LOG_TAG, "The BridgeManager Already exists");
                return false;
            } else {
                managerMap_.put(instanceId, bridgeManager);
                return true;
            }
        } finally {
            managerMapLock_.unlock();
        }
    }

    /**
     * unRegister BridgeManager object.
     *
     * @param instanceId the id of instance.
     * @return Success or fail.
     */
    public static boolean unRegisterBridgeManager(int instanceId) {
        managerMapLock_.lock();
        try {
            if (managerMap_ != null && managerMap_.remove(instanceId) != null) {
                return true;
            } else {
                ALog.e(LOG_TAG, "unRegisterBridgeManager failed");
                return false;
            }
        } finally {
            managerMapLock_.unlock();
        }
    }

    /**
     * Find BridgeManager object.
     *
     * @param instanceId the id of instance.
     * @return BridgeManager object.
     */
    public static BridgeManager findBridgeManager(int instanceId) {
        managerMapLock_.lock();
        try {
            if (managerMap_ != null) {
                return managerMap_.get(instanceId);
            } else {
                return null;
            }
        } finally {
            managerMapLock_.unlock();
        }
    }

    /**
     * Register Bridge TaskQueueHandler.
     *
     * @param bridgeName Name of bridge.
     * @param handler BridgeTaskQueueHandler object.
     * @return Success or fail.
     */
    public boolean registerTaskQueueHandler(String bridgeName, BridgeTaskQueueHandler handler) {
        this.handlerMapLock_.lock();
        try {
            if (this.handlerMap_ != null) {
                this.handlerMap_.put(bridgeName, handler);
                ALog.i(LOG_TAG, "The TaskQueueHandler registered successfully");
                return true;
            } else {
                ALog.e(LOG_TAG, "HandlerMap is null, the TaskQueueHandler registration failed");
                return false;
            }
        } finally {
            this.handlerMapLock_.unlock();
        }
    }

    /**
     * Unregister Bridge TaskQueueHandler.
     *
     * @param bridgeName Name of bridge.
     * @return Success or fail.
     */
    public boolean unRegisterTaskQueueHandler(String bridgeName) {
        this.handlerMapLock_.lock();
        try {
            if (this.handlerMap_ != null && this.handlerMap_.remove(bridgeName) != null) {
                return true;
            } else {
                ALog.e(LOG_TAG, "unRegisterTaskQueueHandler failed");
                return false;
            }
        } finally {
            this.handlerMapLock_.unlock();
        }
    }

    private BridgeTaskQueueHandler findTaskQueueHandler(String bridgeName) {
        this.handlerMapLock_.lock();
        try {
            if (this.handlerMap_ != null) {
                return this.handlerMap_.get(bridgeName);
            } else {
                return null;
            }
        } finally {
            this.handlerMapLock_.unlock();
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
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsCallMethod bridgeName is not found");
            return;
        }
        if (bridgePlugin.isUseTaskQueue()) {
            BridgeTaskQueueHandler handler = findTaskQueueHandler(bridgeName);
            if (handler == null) {
                ALog.e(LOG_TAG, "jsCallMethod Unable to find handler corresponding to bridgePlugin.");
                return;
            }
            handler.dispatch(() -> {
                jsCallMethodInner(bridgePlugin, methodName, parameters);
            });
        } else {
            jsCallMethodInner(bridgePlugin, methodName, parameters);
        }
    }

    private void jsCallMethodInner(BridgePlugin bridgePlugin, String methodName, String parameters) {
        JSONObject resultJsonObj = new JSONObject();
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
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
                ALog.e(LOG_TAG, "createJsonMethodResult failed");
                resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
            }
            platformSendMethodResult(bridgePlugin.getBridgeName(), methodName, resultJsonObj.toString());
        } catch (JSONException e) {
            bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_UNIMPL;
            resultJsonObj = createJsonMethodResult(bridgeErrorCode, null);
            platformSendMethodResult(bridgePlugin.getBridgeName(), methodName, resultJsonObj.toString());
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
        nativePlatformSendMethodResult(bridgeName, methodName, result, this.instanceId_);
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
        nativePlatformCallMethod(bridgeName, methodName, parameters, this.instanceId_);
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
        if (bridgePlugin.isUseTaskQueue()) {
            BridgeTaskQueueHandler handler = findTaskQueueHandler(bridgeName);
            if (handler == null) {
                ALog.e(LOG_TAG, "jsSendMethodResult Unable to find handler corresponding to bridgePlugin.");
                return;
            }
            handler.dispatch(() -> {
                jsSendMethodResultInner(bridgePlugin, methodName, result);
            });
        } else {
            jsSendMethodResultInner(bridgePlugin, methodName, result);
        }
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
            ALog.e(LOG_TAG, "platformSendMessageResponseErrorInfo bridgeName is not found.");
            return;
        }
        try {
            JSONObject dataJson = new JSONObject();
            dataJson.put(MESSAGE_JSON_KEY, JSON_ERROR_CODE);
            dataJson.put(JSON_ERROR_CODE, bridgeErrorCode.getId());
            nativePlatformSendMessageResponse(bridgeName, dataJson.toString(), this.instanceId_);
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
        if (bridgePlugin.isUseTaskQueue()) {
            BridgeTaskQueueHandler handler = findTaskQueueHandler(bridgeName);
            if (handler == null) {
                ALog.e(LOG_TAG, "jsSendMessage Unable to find handler corresponding to bridgePlugin.");
                return;
            }
            handler.dispatch(() -> {
                jsSendMessageInner(bridgePlugin, bridgeName, data);
            });
        } else {
            jsSendMessageInner(bridgePlugin, bridgeName, data);
        }
    }

    private void jsSendMessageInner(BridgePlugin bridgePlugin, String bridgeName, String data) {
        try {
            JSONObject dataJsonObj = new JSONObject(data);
            Object dataObj = dataJsonObj.get(MESSAGE_JSON_KEY);
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
            ALog.e(LOG_TAG, "platformSendMessageResponse bridgeName is not found.");
            return;
        }
        try {
            if (data == null) {
                sendMessageResponseErrorCode(bridgeName, this.instanceId_, BridgeErrorCode.BRIDGE_DATA_ERROR);
                return;
            }
            JSONObject dataJson = new JSONObject();
            if (data instanceof JSONObject) {
                dataJson.put(MESSAGE_JSON_KEY, data.toString());
            } else if (!data.getClass().isArray()) {
                if (!ParameterHelper.isExceedJsSafeInteger(data)) {
                    sendMessageResponseErrorCode(
                        bridgeName, this.instanceId_,BridgeErrorCode.BRIDGE_EXCEEDS_SAFE_INTEGER);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, data);
            } else if (data.getClass().isArray()) {
                JSONArray array = ParameterHelper.objectTransformJsonArray(data);
                if (array == null) {
                    sendMessageResponseErrorCode(bridgeName, this.instanceId_, BridgeErrorCode.BRIDGE_DATA_ERROR);
                    return;
                }
                dataJson.put(MESSAGE_JSON_KEY, array);
            } else {
                sendMessageResponseErrorCode(bridgeName, this.instanceId_, BridgeErrorCode.BRIDGE_DATA_ERROR);
                return;
            }
            dataJson.put(JSON_ERROR_CODE, 0);
            nativePlatformSendMessageResponse(bridgeName, dataJson.toString(), this.instanceId_);
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
            nativePlatformSendMessage(bridgeName, dataJson.toString(), this.instanceId_);
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
        if (bridgePlugin.isUseTaskQueue()) {
            BridgeTaskQueueHandler handler = findTaskQueueHandler(bridgeName);
            if (handler == null) {
                ALog.e(LOG_TAG, "jsSendMessageResponse Unable to find handler corresponding to bridgePlugin.");
                return;
            }
            handler.dispatch(() -> {
                jsSendMessageResponseInner(bridgePlugin, data);
            });
        } else {
            jsSendMessageResponseInner(bridgePlugin, data);
        }
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
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            return;
        }
        ByteBuffer buffer = bridgeBinaryCodec_.encodeData(data);
        nativePlatformSendMessageBinary(bridgeName, buffer, this.instanceId_);
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
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsCallMethodBinary bridgeName is not found.");
            return;
        }
        if (bridgePlugin.isUseTaskQueue()) {
            BridgeTaskQueueHandler handler = findTaskQueueHandler(bridgeName);
            if (handler == null) {
                ALog.e(LOG_TAG, "jsCallMethodBinary Unable to find handler corresponding to bridgePlugin.");
                return;
            }
            handler.dispatch(() -> {
                jsCallMethodBinaryInnter(bridgePlugin, methodName, bufferData);
            });
        } else {
            jsCallMethodBinaryInnter(bridgePlugin, methodName, bufferData);
        }
    }

    private void jsCallMethodBinaryInnter(BridgePlugin bridgePlugin, String methodName, ByteBuffer bufferData) {
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
            if (objects == null) {
                ALog.e(LOG_TAG, "Parameter parsing failed.");
                bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_PARAM_ERROR;
            } else {
                methodData = new MethodData(splitName, objects);
                resultObject = bridgePlugin.jsCallMethod(bridgePlugin, methodData);
                if (resultObject != null && resultObject.getClass() == BridgeErrorCode.class) {
                    bridgeErrorCode = (BridgeErrorCode) resultObject;
                } else {
                    resultBuffer = bridgeBinaryCodec_.encodeData(resultObject);
                }
            }
        }
        PlatformSendMethodResultBinaryInner(bridgePlugin.getBridgeName(), methodName, resultBuffer,
            this.instanceId_, bridgeErrorCode);
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
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            return;
        }
        if (bridgePlugin.isUseTaskQueue()) {
            BridgeTaskQueueHandler handler = findTaskQueueHandler(bridgeName);
            if (handler == null) {
                ALog.e(LOG_TAG, "jsSendMessageBinary Unable to find handler corresponding to bridgePlugin.");
                return;
            }
            handler.dispatch(() -> {
                Object dataObj = bridgeBinaryCodec_.decodeData(bufferData);
                bridgePlugin.jsSendMessage(dataObj);
            });
        } else {
            Object dataObj = bridgeBinaryCodec_.decodeData(bufferData);
            bridgePlugin.jsSendMessage(dataObj);
        }
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
        if (bridgeBinaryCodec_ == null) {
            ALog.e(LOG_TAG, "The bridgeBinaryCodec is null.");
            errorCode = BridgeErrorCode.BRIDGE_CODEC_INVALID;
        }
        if (errorCode == BridgeErrorCode.BRIDGE_ERROR_NO) {
            String methodName = methodData.getMethodName();
            Object[] params = methodData.getMethodParameter();
            ByteBuffer buffer = bridgeBinaryCodec_.encodeData(params);
            nativeplatformCallMethodBinary(bridgeName, methodName, buffer, this.instanceId_);
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
     */
    public void jsSendMethodResultBinary(String bridgeName, String methodName, ByteBuffer result,
    int errorCode, String errorMessage) {
        BridgePlugin bridgePlugin = findBridgePlugin(bridgeName);
        if (bridgePlugin == null) {
            ALog.e(LOG_TAG, "jsSendMethodResultBinary bridgeName is not found.");
            return;
        }
        if (bridgePlugin.isUseTaskQueue()) {
            BridgeTaskQueueHandler handler = findTaskQueueHandler(bridgeName);
            if (handler == null) {
                ALog.e(LOG_TAG, "jsSendMethodResultBinary Unable to find handler corresponding to bridgePlugin.");
                return;
            }
            ByteBuffer copyBufferResult = copyByteBuffer(result);
            handler.dispatch(() -> {
                jsSendMethodResultBinaryInnter(bridgePlugin, methodName, copyBufferResult, errorCode, errorMessage);
            });
        } else {
            jsSendMethodResultBinaryInnter(bridgePlugin, methodName, result, errorCode, errorMessage);
        }
    }

    private void jsSendMethodResultBinaryInnter(BridgePlugin bridgePlugin, String methodName, ByteBuffer result,
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
     * Init BridgeManager jni.
     *
     */
    public native void nativeInit(int instanceId);
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
