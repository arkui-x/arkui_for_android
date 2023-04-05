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

#include "adapter/android/capability/java/jni/bridge/bridge_jni.h"

#include <memory>
#include <unordered_map>

#include "adapter/android/capability/java/jni/bridge/bridge_manager.h"
#include "adapter/android/entrance/java/jni/ace_container.h"
#include "frameworks/core/common/ace_engine.h"

namespace OHOS::Ace::Platform {
namespace {
// Register the native method of jni in java.
const char ACE_BRIDGE_CLASS_NAME[] = "ohos/ace/adapter/capability/bridge/BridgeManager";
static const JNINativeMethod METHODS[] = {
    {
        "nativeInit",
        "()V",
        reinterpret_cast<void *>(&BridgeJni::NativeInit)
    },
    {
        "nativePlatformCallMethod",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V",
        reinterpret_cast<void *>(&BridgeJni::PlatformCallMethod)
    },
    {
        "nativePlatformSendMethodResult",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V",
        reinterpret_cast<void *>(&BridgeJni::PlatformSendMethodResult)
    },
    {
        "nativePlatformSendMessageResponse",
        "(Ljava/lang/String;Ljava/lang/String;I)V",
        reinterpret_cast<void *>(&BridgeJni::PlatformSendMessageResponse)
    },
    {
        "nativePlatformSendMessage",
        "(Ljava/lang/String;Ljava/lang/String;I)V",
        reinterpret_cast<void *>(&BridgeJni::PlatformSendMessage)
    },
};

// Register the native method of java in jni.
static const char JS_CALL_METHOD_JNI[] = "jsCallMethod";
static const char JS_CALL_METHOD_JNI_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V";
static const char JS_CALL_METHOD_RESULT_JNI[] = "jsSendMethodResult";
static const char JS_CALL_METHOD_RESULT_JNI_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V";
static const char JS_SEND_MESSAGE_JNI[] = "jsSendMessage";
static const char JS_SEND_MESSAGE_JNI_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;)V";
static const char JS_SEND_MESSAGE_RESPONSE_JNI[] = "jsSendMessageResponse";
static const char JS_SEND_MESSAGE_RESPONSE_JNI_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;)V";
static const char JS_CANCEL_RESPONSE_JNI[] = "jsCancelMethod";
static const char JS_CANCEL_RESPONSE_JNI_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;)V";

// java methodID and object.
struct {
    jmethodID JSCallMethodJni_;
    jmethodID JSSendMethodResultJni_;
    jmethodID JSSendMessageJni_;
    jmethodID JSSendMessageResponseJni_;
    jmethodID JSCancelMethodJni_;
    jobject object_;
} g_pluginClass;
}  // namespace

RefPtr<TaskExecutor> BridgeJni::GetPlatformTaskExecutor(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        LOGE("JNI GetPlatformTaskExecutor container is null ");
        return nullptr;
    }

    auto pipelineContext = container->GetPipelineContext();
    if (!pipelineContext) {
        LOGE("JNI GetPlatformTaskExecutor pipelineContext is null ");
        return nullptr;
    }
    RefPtr<TaskExecutor> taskExecutor = pipelineContext->GetTaskExecutor();
    if (!taskExecutor) {
        LOGE("JNI GetPlatformTaskExecutor taskExecutor is null ");
        return nullptr;
    }
    return taskExecutor;
}

bool BridgeJni::Register(const std::shared_ptr<JNIEnv> &env)
{
    if (!env) {
        LOGE("Register JNIEnv is nullptr");
        return false;
    }
    jclass myClass = env->FindClass(ACE_BRIDGE_CLASS_NAME);
    if (myClass == nullptr) {
        return false;
    }

    bool ret = env->RegisterNatives(myClass, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(myClass);
    if (!ret) {
        return false;
    }
    return true;
}

void BridgeJni::NativeInit(JNIEnv *env, jobject jobj)
{
    if (!env) {
        LOGE("NativeInit JNIEnv is nullptr");
        return;
    }

    g_pluginClass.object_ = env->NewGlobalRef(jobj);
    if (g_pluginClass.object_ == nullptr) {
        LOGE("jobject object_ not found");
        return;
    }

    jclass cls = env->GetObjectClass(jobj);
    if (cls == nullptr) {
        LOGE("AceBridge JNI Init: class not found");
        return;
    }

    g_pluginClass.JSCallMethodJni_ = env->GetMethodID(cls, JS_CALL_METHOD_JNI, JS_CALL_METHOD_JNI_PARAM);
    g_pluginClass.JSSendMethodResultJni_ = env->GetMethodID(cls,
        JS_CALL_METHOD_RESULT_JNI, JS_CALL_METHOD_RESULT_JNI_PARAM);
    g_pluginClass.JSSendMessageJni_ = env->GetMethodID(cls, JS_SEND_MESSAGE_JNI, JS_SEND_MESSAGE_JNI_PARAM);
    g_pluginClass.JSSendMessageResponseJni_ = env->GetMethodID(cls,
        JS_SEND_MESSAGE_RESPONSE_JNI, JS_SEND_MESSAGE_RESPONSE_JNI_PARAM);
    g_pluginClass.JSCancelMethodJni_ = env->GetMethodID(cls, JS_CANCEL_RESPONSE_JNI, JS_CANCEL_RESPONSE_JNI_PARAM);
    env->DeleteLocalRef(cls);
}

void BridgeJni::JSCallMethodJni(const std::string& bridgeName,
    const std::string& methodName, const std::string& parameters)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.object_);
    CHECK_NULL_VOID(g_pluginClass.JSCallMethodJni_);

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    jstring jParameters = env->NewStringUTF(parameters.c_str());
    if (jBridgeName == nullptr || jMethodName == nullptr || jParameters == nullptr) {
        LOGE("jBridgeName or jMethodName or jParameters is nullptr");
        if (jBridgeName) {
            env->DeleteLocalRef(jBridgeName);
        }
        if (jMethodName) {
            env->DeleteLocalRef(jMethodName);
        }
        if (jParameters) {
            env->DeleteLocalRef(jParameters);
        }
        return;
    }
    env->CallVoidMethod(g_pluginClass.object_, g_pluginClass.JSCallMethodJni_,
        jBridgeName, jMethodName, jParameters);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
    env->DeleteLocalRef(jParameters);
}

void BridgeJni::PlatformSendMethodResult(JNIEnv *env, jobject jobj,
    jstring jBridgeName, jstring jMethodName, jstring jResult, jint instanceId)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName;
    std::string methodName;
    std::string result;
    auto bridgeNameStr = env->GetStringUTFChars(jBridgeName, nullptr);
    if (bridgeNameStr != nullptr) {
        bridgeName = bridgeNameStr;
        env->ReleaseStringUTFChars(jBridgeName, bridgeNameStr);
    }
    auto methodNameStr = env->GetStringUTFChars(jMethodName, nullptr);
    if (methodNameStr != nullptr) {
        methodName = methodNameStr;
        env->ReleaseStringUTFChars(jMethodName, methodNameStr);
    }
    auto resultStr = env->GetStringUTFChars(jResult, nullptr);
    if (resultStr != nullptr) {
        result = resultStr;
        env->ReleaseStringUTFChars(jResult, resultStr);
    }

    auto taskExecutor = GetPlatformTaskExecutor(instanceId);
    ContainerScope scope(instanceId);
    if (!taskExecutor) {
        LOGE("PlatformSendMethodResult taskExecutor is nullptr");
        return;
    }
    auto task = [bridgeName, methodName, result, instanceId] {
        BridgeManager::PlatformSendMethodResult(bridgeName, methodName, result);
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS);
}

void BridgeJni::PlatformCallMethod(JNIEnv *env, jobject jobj,
    jstring jBridgeName, jstring jMethodName, jstring jParam, jint instanceId)
{
    CHECK_NULL_VOID(env);
    std::string callBridgeName;
    std::string callMethodName;
    std::string callParam;
    auto bridgeNameStr = env->GetStringUTFChars(jBridgeName, nullptr);
    if (bridgeNameStr != nullptr) {
        callBridgeName = bridgeNameStr;
        env->ReleaseStringUTFChars(jBridgeName, bridgeNameStr);
    }
    auto methodNameStr = env->GetStringUTFChars(jMethodName, nullptr);
    if (methodNameStr != nullptr) {
        callMethodName = methodNameStr;
        env->ReleaseStringUTFChars(jMethodName, methodNameStr);
    }
    auto paramStr = env->GetStringUTFChars(jParam, nullptr);
    if (paramStr != nullptr) {
        callParam = paramStr;
        env->ReleaseStringUTFChars(jParam, paramStr);
    }
    if (!BridgeManager::JSBridgeExists(callBridgeName)) {
        std::string resultValue = "{\"errorcode\":1, \"errormessage\":\"Bridge name error!\", \"result\":0}";
        JSSendMethodResultJni(callBridgeName, callMethodName, resultValue);
        return;
    }
    auto taskExecutor = GetPlatformTaskExecutor(instanceId);
    ContainerScope scope(instanceId);
    if (!taskExecutor) {
        LOGE("PlatformCallMethod taskExecutor = nullptr");
        return;
    }
    auto task = [callBridgeName, callMethodName, callParam, instanceId] {
        BridgeManager::PlatformCallMethod(callBridgeName, callMethodName, callParam);
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS);
}

void BridgeJni::JSSendMethodResultJni(const std::string& bridgeName,
    const std::string& methodName, const std::string& resultValue)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.object_);
    CHECK_NULL_VOID(g_pluginClass.JSSendMethodResultJni_);

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    jstring jResultValue = env->NewStringUTF(resultValue.c_str());
    if (jBridgeName == nullptr || jMethodName == nullptr || jResultValue == nullptr) {
        LOGE("jBridgeName or jMethodName or jResultValue is nullptr");
        if (jBridgeName) {
            env->DeleteLocalRef(jBridgeName);
        }
        if (jMethodName) {
            env->DeleteLocalRef(jMethodName);
        }
        if (jResultValue) {
            env->DeleteLocalRef(jResultValue);
        }
        return;
    }
    env->CallVoidMethod(g_pluginClass.object_, g_pluginClass.JSSendMethodResultJni_,
        jBridgeName, jMethodName, jResultValue);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
    env->DeleteLocalRef(jResultValue);
}

void BridgeJni::JSSendMessageJni(const std::string& bridgeName, const std::string& data)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.object_);
    CHECK_NULL_VOID(g_pluginClass.JSSendMessageJni_);

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jData = env->NewStringUTF(data.c_str());
    if (jBridgeName == nullptr || jData == nullptr) {
        LOGE("jBridgeName or jData is nullptr");
        if (jBridgeName) {
            env->DeleteLocalRef(jBridgeName);
        }
        if (jData) {
            env->DeleteLocalRef(jData);
        }
        return;
    }
    env->CallVoidMethod(g_pluginClass.object_, g_pluginClass.JSSendMessageJni_, jBridgeName, jData);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jData);
}

void BridgeJni::PlatformSendMessageResponse(JNIEnv *env, jobject jobj,
    jstring jBridgeName, jstring jData, jint instanceId)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName;
    std::string data;
    auto bridgeNameStr = env->GetStringUTFChars(jBridgeName, nullptr);
    if (bridgeNameStr != nullptr) {
        bridgeName = bridgeNameStr;
        env->ReleaseStringUTFChars(jBridgeName, bridgeNameStr);
    }
    auto dataStr = env->GetStringUTFChars(jData, nullptr);
    if (dataStr != nullptr) {
        data = dataStr;
        env->ReleaseStringUTFChars(jData, dataStr);
    }

    auto taskExecutor = GetPlatformTaskExecutor(instanceId);
    ContainerScope scope(instanceId);
    if (!taskExecutor) {
        LOGE("BridgeJni JsSendMessageResponse taskExecutor is nullptr");
        return;
    }
    auto task = [bridgeName, data, instanceId] {
        BridgeManager::PlatformSendMessageResponse(bridgeName, data);
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS);
}

void BridgeJni::PlatformSendMessage(JNIEnv *env, jobject jobj, jstring jBridgeName, jstring jData, jint instanceId)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName;
    std::string data;
    auto bridgeNameStr = env->GetStringUTFChars(jBridgeName, nullptr);
    if (bridgeNameStr != nullptr) {
        bridgeName = bridgeNameStr;
        env->ReleaseStringUTFChars(jBridgeName, bridgeNameStr);
    }
    auto dataStr = env->GetStringUTFChars(jData, nullptr);
    if (dataStr != nullptr) {
        data = dataStr;
        env->ReleaseStringUTFChars(jData, dataStr);
    }

    auto taskExecutor = GetPlatformTaskExecutor(instanceId);
    ContainerScope scope(instanceId);
    if (!taskExecutor) {
        LOGE("BridgeJni PlatformSendMessage taskExecutor is nullptr");
        return;
    }
    auto task = [bridgeName, data, instanceId] {
        BridgeManager::PlatformSendMessage(bridgeName, data);
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS);
}

void BridgeJni::JSSendMessageResponseJni(const std::string& bridgeName, const std::string& data)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.object_);
    CHECK_NULL_VOID(g_pluginClass.JSSendMessageResponseJni_);

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jData = env->NewStringUTF(data.c_str());
    if (jBridgeName == nullptr || jData == nullptr) {
        LOGE("jBridgeName or jData is nullptr");
        if (jBridgeName) {
            env->DeleteLocalRef(jBridgeName);
        }
        if (jData) {
            env->DeleteLocalRef(jData);
        }
        return;
    }
    env->CallVoidMethod(g_pluginClass.object_, g_pluginClass.JSSendMessageResponseJni_, jBridgeName, jData);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jData);
}

void BridgeJni::JSCancelMethodJni(const std::string& bridgeName, const std::string& methodName)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.object_);
    CHECK_NULL_VOID(g_pluginClass.JSCancelMethodJni_);

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    if (jBridgeName == nullptr || jMethodName == nullptr) {
        LOGE("jBridgeName or jMethodName is nullptr");
        if (jBridgeName) {
            env->DeleteLocalRef(jBridgeName);
        }
        if (jMethodName) {
            env->DeleteLocalRef(jMethodName);
        }
        return;
    }
    env->CallVoidMethod(g_pluginClass.object_, g_pluginClass.JSCancelMethodJni_, jBridgeName, jMethodName);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
}
}  // namespace OHOS::Ace::Platform