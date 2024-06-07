/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.HashMap;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.ExecutorServiceInstance;
import ohos.ace.adapter.capability.bridge.BridgeTaskQueue.TaskTag;

/**
 * Bridge plugin.
 *
 * @since 10
 */
public abstract class BridgePlugin {
    private static final String LOG_TAG = "BridgePlugin";

    private final String bridgeName_;

    private Context context_ = null;

    private boolean isAvailable_ = false;

    private IMessageListener iMessageListener_;

    private IMethodResult iMethodResult_;

    private BridgeManager bridgeManager_;

    private HashMap<String, Method> methodsMap_;

    private Lock methodMapLock_ = new ReentrantLock();

    private BridgeType bridgeType_ = BridgeType.JSON_TYPE;

    private boolean isUseTaskQueue_ = false;

    private BridgeTaskQueueHandler outPutHandler_ = null;

    private BridgeTaskQueueHandler intPutHandler_ = null;

    private ExecutorService executorService_ = ExecutorServiceInstance.getExecutorService();

    /**
     * Constructor of base BridgePlugin.
     *
     * @param context context of the application.
     * @param bridgeName name of bridge.
     * @param instanceId the id of instance.
     * @return BridgePlugin object.
     * @since 10
     * @deprecated since 11
     */
    public BridgePlugin(Context context, String bridgeName, int instanceId) {
        this.bridgeName_ = bridgeName;
        this.context_ = context;
        this.bridgeManager_ = BridgeManager.findBridgeManager(instanceId);
        this.methodsMap_ = new HashMap<String, Method>();
        if (checkBridgeInner()) {
            this.isAvailable_ = this.bridgeManager_.registerBridgePlugin(bridgeName, this);
        }
    }

    /**
     * Constructor of binary BridgePlugin.
     *
     * @param context context of the application.
     * @param bridgeName name of bridge.
     * @param instanceId the id of instance.
     * @param bridgeType the bridge of type.
     * @return BridgePlugin object.
     * @since 10
     * @deprecated since 11
     */
    public BridgePlugin(Context context, String bridgeName, int instanceId, BridgeType bridgeType) {
        this.bridgeName_ = bridgeName;
        this.context_ = context;
        this.bridgeType_ = bridgeType;
        this.bridgeManager_ = BridgeManager.findBridgeManager(instanceId);
        this.methodsMap_ = new HashMap<String, Method>();
        if (checkBridgeInner()) {
            this.isAvailable_ = this.bridgeManager_.registerBridgePlugin(bridgeName, this);
        }
    }

    /**
     * Constructor of base BridgePlugin.
     *
     * @param context context of the application.
     * @param bridgeName name of bridge.
     * @param bridgeManager Object of BridgeManager.
     * @return BridgePlugin object.
     * @since 11
     */
    public BridgePlugin(Context context, String bridgeName, BridgeManager bridgeManager) {
        this.bridgeManager_ = bridgeManager;
        this.bridgeName_ = bridgeName;
        this.context_ = context;
        this.methodsMap_ = new HashMap<String, Method>();
        if (checkBridgeInner()) {
            this.isAvailable_ = this.bridgeManager_.registerBridgePlugin(bridgeName, this);
        }
    }

    /**
     * Constructor of binary BridgePlugin.
     *
     * @param context context of the application.
     * @param bridgeName name of bridge.
     * @param bridgeManager Object of BridgeManager.
     * @param bridgeType the bridge of type.
     * @return BridgePlugin object.
     * @since 11
     */
    public BridgePlugin(Context context, String bridgeName, BridgeManager bridgeManager, BridgeType bridgeType) {
        this.bridgeManager_ = bridgeManager;
        this.bridgeName_ = bridgeName;
        this.context_ = context;
        this.bridgeType_ = bridgeType;
        this.methodsMap_ = new HashMap<String, Method>();
        if (checkBridgeInner()) {
            this.isAvailable_ = this.bridgeManager_.registerBridgePlugin(bridgeName, this);
        }
    }

    /**
     * Constructor of concurrent BridgePlugin.
     *
     * @param context context of the application.
     * @param bridgeName name of bridge.
     * @param bridgeManager Object of BridgeManager.
     * @param bridgeType The bridge of type.
     * @param taskOption Task option of Bridge.
     * @return BridgePlugin object.
     * @since 11
     */
    public BridgePlugin(Context context, String bridgeName, BridgeManager bridgeManager,
        BridgeType bridgeType, TaskOption taskOption) {
        this.bridgeManager_ = bridgeManager;
        this.bridgeName_ = bridgeName;
        this.context_ = context;
        this.bridgeType_ = bridgeType;
        this.methodsMap_ = new HashMap<String, Method>();
        if (taskOption != null && checkBridgeInner()) {
            this.isUseTaskQueue_ = true;
            this.outPutHandler_ = new BridgeTaskQueueHandler(this.executorService_, TaskTag.OUTPUT, taskOption);
            this.intPutHandler_ = new BridgeTaskQueueHandler(this.executorService_, TaskTag.INPUT, taskOption);
            if (this.bridgeManager_.registerTaskQueueHandler(bridgeName, this.intPutHandler_)) {
                this.isAvailable_ = this.bridgeManager_.registerBridgePlugin(bridgeName, this);
            }
        }
    }

    private boolean checkBridgeInner() {
        if (this.bridgeName_ != null && this.bridgeManager_ != null && this.context_ != null) {
            return true;
        } else {
            return false;
        }
    }

    private void registerMethod(String methodName, Method methods) {
        this.methodMapLock_.lock();
        try {
            if (this.methodsMap_ != null) {
                this.methodsMap_.put(methodName, methods);
            } else {
                ALog.e(LOG_TAG, "The methodsMap failed");
                return;
            }
        } finally {
            this.methodMapLock_.unlock();
        }
    }

    private Method findMethod(String methodName) {
        this.methodMapLock_.lock();
        try {
            if (this.methodsMap_ != null) {
                return this.methodsMap_.get(methodName);
            } else {
                return null;
            }
        } finally {
            this.methodMapLock_.unlock();
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
     * Get bridge type.
     *
     * @return The BridgeType.
     */
    public BridgeType getBridgeType() {
        return this.bridgeType_;
    }

    /**
     * Check if BridgePlugin is available.
     *
     * @return The isAvailable of BridgePlugin.
     */
    public boolean isBridgeAvailable() {
        return this.isAvailable_;
    }

    /**
     * Get bridge isUseTaskQueue.
     *
     * @return The Bridge isUseTaskQueue.
     */
    protected boolean isUseTaskQueue() {
        return this.isUseTaskQueue_;
    }

    /**
     * Unregister the created bridge
     *
     * @param bridgeName Name of bridge.
     * @return Success or not.
     */
    public boolean unRegister(String bridgeName) {
        if (this.bridgeManager_ == null) {
            ALog.e(LOG_TAG, "Bridge unRegister failed, bridgeManager is null");
            return false;
        }
        return this.bridgeManager_.unRegisterBridgePlugin(bridgeName);
    }

    /**
     * Set message listening.
     *
     * @param messageListener Interface function for message listening.
     */
    public void setMessageListener(IMessageListener messageListener) {
        this.iMessageListener_ = messageListener;
    }

    /**
     * Set method listening.
     *
     * @param methodResultListener Interface function for method listening.
     */
    public void setMethodResultListener(IMethodResult methodResultListener) {
        this.iMethodResult_ = methodResultListener;
    }

    private void callMethodInner(MethodData methodData) {
        if (this.bridgeManager_ == null) {
            ALog.e(LOG_TAG, "Bridge callMethodInner failed, bridgeManager is null");
            return;
        }
        BridgeErrorCode errorCode = BridgeErrorCode.BRIDGE_ERROR_NO;
        if (this.bridgeType_ == BridgeType.BINARY_TYPE) {
            errorCode = this.bridgeManager_.platformCallMethodBinary(bridgeName_, methodData);
        } else {
            errorCode = this.bridgeManager_.platformCallMethod(bridgeName_, methodData);
        }
        if (this.iMethodResult_ != null && errorCode.getId() != 0) {
            this.iMethodResult_.onError(methodData.getMethodName(), errorCode.getId(), errorCode.getErrorMessage());
        }
    }

    /**
     * Call another platform's registered method.
     *
     * @param methodData Method packaging structure.
     */
    public void callMethod(MethodData methodData) {
        if (!this.isAvailable_) {
            ALog.e(LOG_TAG, "The bridge is not available.");
            return;
        }
        if (this.isUseTaskQueue_) {
            this.outPutHandler_.dispatch(() -> {
                callMethodInner(methodData);
            });
        } else {
            callMethodInner(methodData);
        }
    }

    private void sendMessageInner(Object data) {
        if (this.bridgeManager_ == null) {
            ALog.e(LOG_TAG, "Bridge sendMessageInner failed, bridgeManager is null");
            return;
        }
        if (this.bridgeType_ == BridgeType.BINARY_TYPE) {
            this.bridgeManager_.platformSendMessageBinary(this.bridgeName_, data);
            return;
        }
        this.bridgeManager_.platformSendMessage(this.bridgeName_, data);
    }

    /**
     * Send data to other platforms.
     *
     * @param data Data to be sent.
     */
    public void sendMessage(Object data) {
        if (!this.isAvailable_) {
            ALog.e(LOG_TAG, "The bridge is not available.");
            return;
        }
        if (this.isUseTaskQueue_) {
            this.outPutHandler_.dispatch(() -> {
                sendMessageInner(data);
            });
        } else {
            sendMessageInner(data);
        }
    }

    /**
     * release BridgeManager object.
     *
     */
    public void release() {
        this.isAvailable_ = false;
        this.bridgeManager_ = null;
        this.context_ = null;
    }

    /**
     * Other platforms call methods.
     *
     * @param object Object of bridgePlugin.
     * @param methodData Method packaging structure.
     * @return Return the call result.
     */
    protected Object jsCallMethod(Object object, MethodData methodData) {
        if (object != null && methodData != null) {
            Method callMethod = initMethod(object.getClass(), methodData);
            Object[] parametersObject = methodData.getMethodParameter();
            return bridgeInvokeMethod(object, callMethod, parametersObject);
        } else {
            return BridgeErrorCode.BRIDGE_METHOD_UNIMPL;
        }
    }

    private Object bridgeInvokeMethod(Object object, Method callMethod, Object[] parametersObject) {
        BridgeErrorCode bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_UNIMPL;
        if (object != null && callMethod != null && parametersObject != null) {
            Object resValues = null;
            try {
                if (parametersObject != null && parametersObject.length != 0) {
                    resValues = callMethod.invoke(object, parametersObject);
                } else {
                    resValues = callMethod.invoke(object);
                }
                return resValues;
            } catch (IllegalAccessException e) {
                ALog.e(LOG_TAG, "jsCallMethod failed, IllegalAccessException.");
            } catch (IllegalArgumentException e) {
                ALog.e(LOG_TAG, "jsCallMethod failed, IllegalArgumentException.");
                bridgeErrorCode = BridgeErrorCode.BRIDGE_METHOD_PARAM_ERROR;
            } catch (InvocationTargetException e) {
                ALog.e(LOG_TAG, "jsCallMethod failed, InvocationTargetException.");
            }
        }
        return bridgeErrorCode;
    }

    private Method initMethod(Class<?> clazz, MethodData methodData) {
        if (clazz == null || methodData == null) {
            return null;
        }
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
            callMethod = null;
        }
        return callMethod;
    }

    /**
     * Messages from other platforms.
     *
     * @param data Data to be sent.
     */
    protected void jsSendMessage(Object data) {
        if (this.bridgeManager_ == null) {
            ALog.e(LOG_TAG, "Bridge jsSendMessage failed, bridgeManager is null");
            return;
        }
        if (this.iMessageListener_ != null) {
            Object dataResponse = this.iMessageListener_.onMessage(data);
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
    protected void jsSendMethodResult(Object result, String methodName, int errorCode, String errorMessage) {
        if (this.iMethodResult_ == null) {
            return;
        }
        if (errorCode == 0) {
            this.iMethodResult_.onSuccess(result);
        } else {
            this.iMethodResult_.onError(methodName, errorCode, errorMessage);
        }
    }

    /**
     * MessageResponse from other platforms.
     *
     * @param data Data to be sent.
     */
    protected void jsSendMessageResponse(Object data) {
        if (this.iMessageListener_ != null) {
            this.iMessageListener_.onMessageResponse(data);
        }
    }

    /**
     * Method to unregister the platform.
     *
     * @param bridgeName Object of bridgePlugin.
     * @param methodName Name of method.
     */
    protected void jsCancelMethod(String methodName) {
        if (this.iMethodResult_ != null) {
            this.iMethodResult_.onMethodCancel(methodName);
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