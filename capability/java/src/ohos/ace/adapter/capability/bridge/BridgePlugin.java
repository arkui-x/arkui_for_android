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

import android.content.Context;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import ohos.ace.adapter.ALog;

/**
 * Bridge plugin.
 *
 * @since 10
 */
public abstract class BridgePlugin {
    private static final String LOG_TAG = "BridgePlugin";

    private final String bridgeName_;

    private final int instanceId_;

    private final Object object_ = null;

    private final Context context_;

    private boolean isAvailable = false;

    private IMessageListener iMessageListener;

    private IMethodResult iMethodResult;

    private BridgeManager bridgeManager_;

    private HashMap<String, Method> methodsMap_;

    private BridgeType bridgeType_ = BridgeType.JSON_TYPE;

    /**
     * Bridge dependent plugin.
     *
     * @param context context of the application.
     * @param bridgeName name of bridge.
     * @param instanceId the id of instance.
     * @return BridgePlugin object.
     */
    public BridgePlugin(Context context, String bridgeName, int instanceId) {
        this.bridgeName_ = bridgeName;
        this.instanceId_ = instanceId;
        this.context_ = context;
        this.bridgeManager_ = BridgeManager.getInstance();
        this.isAvailable = this.bridgeManager_.registerBridgePlugin(bridgeName, this);
        this.methodsMap_ = new HashMap<String, Method>();
    }

    /**
     * Bridge dependent plugin by BINARY_TYPE.
     *
     * @param context context of the application.
     * @param bridgeName name of bridge.
     * @param instanceId the id of instance.
     * @param bridgeType the bridge of type.
     * @return BridgePlugin object.
     */
    public BridgePlugin(Context context, String bridgeName, int instanceId, BridgeType bridgeType) {
        this.bridgeName_ = bridgeName;
        this.instanceId_ = instanceId;
        this.context_ = context;
        this.bridgeType_ = bridgeType;
        this.bridgeManager_ = BridgeManager.getInstance();
        this.isAvailable = this.bridgeManager_.registerBridgePlugin(bridgeName, this);
        this.methodsMap_ = new HashMap<String, Method>();
    }

    private void registerMethod(String methodName, Method methods) {
        Lock registerLock = new ReentrantLock();
        registerLock.lock();
        try {
            if (methodsMap_.containsKey(methodName)) {
                ALog.e(LOG_TAG, "The methodName Already exists");
                return;
            } else {
                ALog.i(LOG_TAG, "registerMethod success");
                methodsMap_.put(methodName, methods);
            }
        } finally {
            registerLock.unlock();
        }
    }

    private Method findMethod(String methodName) {
        Lock findBridgeLock = new ReentrantLock();
        findBridgeLock.lock();
        try {
            if (methodsMap_.containsKey(methodName)) {
                return methodsMap_.get(methodName);
            } else {
                ALog.i(LOG_TAG, "The first call of methodName.");
                return null;
            }
        } finally {
            findBridgeLock.unlock();
        }
    }

    /**
     * Get the name of creation bridge.
     *
     * @return The BridgeName.
     */
    protected String getBridgeName() {
        return this.bridgeName_;
    }

    /**
     * Get the Id of platform.
     *
     * @return The InstanceId.
     */
    protected int getInstanceId() {
        return this.instanceId_;
    }

    /**
     * Get bridge type.
     *
     * @return The BridgeType.
     */
    public BridgeType getBridgeType() {
        return this.bridgeType_;
    }

    /**
     * Unregister the created bridge
     *
     * @param bridgeName Name of bridge.
     * @return Success or not.
     */
    public boolean unRegister(String bridgeName) {
        return this.bridgeManager_.unRegisterBridgePlugin(bridgeName);
    }

    /**
     * Set message listening.
     *
     * @param messageListener Interface function for message listening.
     */
    public void setMessageListener(IMessageListener messageListener) {
        this.iMessageListener = messageListener;
    }

    /**
     * Set method listening.
     *
     * @param methodResultListener Interface function for method listening.
     */
    public void setMethodResultListener(IMethodResult methodResultListener) {
        this.iMethodResult = methodResultListener;
    }

    /**
     * Call another platform's registered method.
     *
     * @param methodData Method packaging structure.
     */
    public void callMethod(MethodData methodData) {
        if (!this.isAvailable) {
            ALog.e(LOG_TAG, "callMethod is fail, The bridge is not available" + this.bridgeName_);
            return;
        }
        BridgeErrorCode errorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        if (this.bridgeType_ == BridgeType.BINARY_TYPE) {
            errorCode = this.bridgeManager_.platformCallMethodBinary(bridgeName_, methodData);
        } else {
            errorCode = this.bridgeManager_.platformCallMethod(bridgeName_, methodData);
        }
        if (this.iMethodResult != null && errorCode.getId() != 0) {
            this.iMethodResult.onError(methodData.getMethodName(), errorCode.getId(), errorCode.getErrorMessage());
        }
    }

    /**
     * Send data to other platforms.
     *
     * @param data Data to be sent.
     */
    public void sendMessage(Object data) {
        if (!this.isAvailable) {
            ALog.e(LOG_TAG, "sendMessage is fail, The bridge is not available" + this.bridgeName_);
            return;
        }
        if (this.bridgeType_ == BridgeType.BINARY_TYPE) {
            this.bridgeManager_.platformSendMessageBinary(this.bridgeName_, data);
            return;
        }
        this.bridgeManager_.platformSendMessage(this.bridgeName_, data);
    }

    /**
     * Other platforms call methods.
     *
     * @param object Object of bridgePlugin.
     * @param methodData Method packaging structure.
     * @return Return the call result.
     */
    protected Object jsCallMethod(Object object, MethodData methodData) {
        Object[] parametersObject = methodData.getMethodParameter();
        Class clazz = object.getClass();
        Object resValues = null;
        Method callMethod = findMethod(methodData.getMethodName());
        try {
            if (callMethod == null) {
                Method[] Methods = clazz.getMethods();
                    Class<?>[] parametersClass = null;
                    for (Method method : Methods) {
                        if (method.getName().equals(methodData.getMethodName())) {
                            parametersClass = method.getParameterTypes();
                        }
                    }
                callMethod = clazz.getMethod(methodData.getMethodName(), parametersClass);
                registerMethod(methodData.getMethodName(), callMethod);
            }
        } catch (NoSuchMethodException e) {
            ALog.e(LOG_TAG, "jsCallMethod failed, NoSuchMethodException.");
            return BridgeErrorCode.BRIDGE_METHOD_NAME_ERROR;
        }
        try {
            if (callMethod != null && parametersObject != null && parametersObject.length != 0) {
                resValues = callMethod.invoke(object, parametersObject);
            } else {
                resValues = callMethod.invoke(object);
            }
            return resValues;
        } catch (IllegalAccessException e) {
            ALog.e(LOG_TAG, "jsCallMethod failed, IllegalAccessException.");
        } catch (IllegalArgumentException e) {
            ALog.e(LOG_TAG, "jsCallMethod failed, IllegalArgumentException.");
        } catch (InvocationTargetException e) {
            ALog.e(LOG_TAG, "jsCallMethod failed, InvocationTargetException.");
        }
        return BridgeErrorCode.BRIDGE_METHOD_UNIMPL;
    }

    /**
     * Messages from other platforms.
     *
     * @param data Data to be sent.
     */
    protected void jsSendMessage(Object data) {
        if (this.iMessageListener != null) {
            Object dataResponse = this.iMessageListener.onMessage(data);
            this.bridgeManager_.platformSendMessageResponse(this.bridgeName_, dataResponse);
        }
    }

    /**
     * Method result from other platforms.
     *
     * @param result Return value of method.
     * @param methodName Name of method.
     * @param errorCode Code of error.
     * @param errorMessage Message of error.
     */
    protected void jsSendMethodResult(Object result, String methodName, int errorCode,
        String errorMessage) {
        if (this.iMethodResult == null) {
            return;
        }
        if (errorCode == 0) {
            this.iMethodResult.onSuccess(result);
        } else {
            this.iMethodResult.onError(methodName, errorCode, errorMessage);
        }
    }

    /**
     * MessageResponse from other platforms.
     *
     * @param data Data to be sent.
     */
    protected void jsSendMessageResponse(Object data) {
        if (this.iMessageListener != null) {
            this.iMessageListener.onMessageResponse(data);
        }
    }

    /**
     * Method to unregister the platform.
     *
     * @param bridgeName Object of bridgePlugin.
     * @param methodName Name of method.
     */
    protected void jsCancelMethod(String methodName) {
        if (this.iMethodResult != null) {
            this.iMethodResult.onMethodCancel(methodName);
        }
    }

    /**
     * Encoding type of Bridge.
     *
     * @since 10
     */
    public enum BridgeType {
        JSON_TYPE,
        BINARY_TYPE;
    }
}