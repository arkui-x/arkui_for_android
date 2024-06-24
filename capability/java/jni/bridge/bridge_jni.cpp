/*
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

#include "adapter/android/capability/java/jni/bridge/bridge_jni.h"

#include <memory>
#include <unordered_map>

#include "adapter/android/capability/java/jni/bridge/bridge_manager.h"
#include "core/common/container.h"
#include "frameworks/core/common/ace_engine.h"

namespace OHOS::Ace::Platform {
namespace {
static const std::string CONVER_FAILED = "error";
// Register the native method of jni in java.
const char ACE_BRIDGE_CLASS_NAME[] = "ohos/ace/adapter/capability/bridge/BridgeManager";
static const JNINativeMethod METHODS[] = {
    {
        "nativeInit",
        "(I)V",
        reinterpret_cast<void *>(&BridgeJni::NativeInit)
    },
    {
        "nativeUpdateCurrentInstanceId",
        "(I)V",
        reinterpret_cast<void *>(&BridgeJni::NativeUpdateCurrentInstanceId)
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
    {
        "nativePlatformSendMessageBinary",
        "(Ljava/lang/String;Ljava/nio/ByteBuffer;I)V",
        reinterpret_cast<void *>(&BridgeJni::PlatformSendMessageBinary)
    },
    {
        "nativePlatformSendMethodResultBinary",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;IILjava/lang/String;)V",
        reinterpret_cast<void *>(&BridgeJni::PlatformSendMethodResultBinary)
    },
    {
        "nativePlatformCallMethodBinary",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;I)V",
        reinterpret_cast<void *>(&BridgeJni::PlatformCallMethodBinary)
    }
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
static const char JS_CALL_METHOD_BINARY_JNI[] = "jsCallMethodBinary";
static const char JS_CALL_METHOD_BINARY_JNI_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;)V";
static const char JS_SEND_MESSAGE_BINARY_JNI[] = "jsSendMessageBinary";
static const char JS_SEND_MESSAGE_BINARY_JNI_PARAM[] = "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V";
static const char JS_SEND_METHOD_RESULT_BINARY_JNI[] = "jsSendMethodResultBinary";
static const char JS_SEND_METHOD_RESULT_BINARY_JNI_PARAM[] =
    "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;ILjava/lang/String;)V";

// java methodID and object.
struct {
    jmethodID JSCallMethodJni_;
    jmethodID JSSendMethodResultJni_;
    jmethodID JSSendMessageJni_;
    jmethodID JSSendMessageResponseJni_;
    jmethodID JSCancelMethodJni_;
    jmethodID JSCallMethodBinaryJni_;
    jmethodID JSSendMessageBinaryJni_;
    jmethodID JSSendMethodResultBinaryJni_;
} g_pluginClass;

std::unordered_map<jint, JniEnvironment::JavaGlobalRef> g_jobjects;
std::mutex g_bridgeJniLock;
static int32_t g_currentInstanceId;
}  // namespace

jobject GetJObjectByInstanceId(const int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(g_bridgeJniLock);
    auto finder = g_jobjects.find(instanceId);
    if (finder == g_jobjects.end()) {
        LOGE("BridgeJobject is not exist. the instanceId is %{public}d", instanceId);
        return nullptr;
    }
    return finder->second.get();
}

RefPtr<TaskExecutor> BridgeJni::GetPlatformTaskExecutor(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        LOGE("JNI GetPlatformTaskExecutor container is null. the instanceId is %{public}d", instanceId);
        return nullptr;
    }

    auto pipelineContext = container->GetPipelineContext();
    if (!pipelineContext) {
        LOGE("JNI GetPlatformTaskExecutor pipelineContext is null. the instanceId is %{public}d", instanceId);
        return nullptr;
    }
    RefPtr<TaskExecutor> taskExecutor = pipelineContext->GetTaskExecutor();
    if (!taskExecutor) {
        LOGE("JNI GetPlatformTaskExecutor taskExecutor is null. the instanceId is %{public}d", instanceId);
        return nullptr;
    }
    return taskExecutor;
}

std::string TransformString(JNIEnv *env, jstring jString)
{
    CHECK_NULL_RETURN(env, CONVER_FAILED);
    std::string resultString;
    auto converString = env->GetStringUTFChars(jString, nullptr);
    if (converString != nullptr) {
        resultString = converString;
        env->ReleaseStringUTFChars(jString, converString);
    }
    return resultString;
}

void DeleteLocalRefString(std::shared_ptr<JNIEnv> env, jstring jString)
{
    CHECK_NULL_VOID(env);
    if (jString) {
        env->DeleteLocalRef(jString);
    }
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
    return ret;
}

void BridgeJni::NativeInit(JNIEnv *env, jobject jobj, jint instanceId)
{
    if (!env) {
        LOGE("NativeInit JNIEnv is nullptr");
        return;
    }

    auto id = static_cast<int32_t>(instanceId);
    std::lock_guard<std::mutex> lock(g_bridgeJniLock);
    g_currentInstanceId = id;
    LOGI("BridgeJni NativeInit InstanceId is %{public}d", g_currentInstanceId);
    g_jobjects.emplace(id, JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj));

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
    g_pluginClass.JSCallMethodBinaryJni_ = env->GetMethodID(cls,
        JS_CALL_METHOD_BINARY_JNI, JS_CALL_METHOD_BINARY_JNI_PARAM);
    g_pluginClass.JSSendMessageBinaryJni_ = env->GetMethodID(cls,
        JS_SEND_MESSAGE_BINARY_JNI, JS_SEND_MESSAGE_BINARY_JNI_PARAM);
    g_pluginClass.JSSendMethodResultBinaryJni_ = env->GetMethodID(cls,
        JS_SEND_METHOD_RESULT_BINARY_JNI, JS_SEND_METHOD_RESULT_BINARY_JNI_PARAM);
    env->DeleteLocalRef(cls);
}

void BridgeJni::NativeUpdateCurrentInstanceId(JNIEnv *env, jobject jobj, jint instanceId)
{
    if (!env) {
        LOGE("NativeUpdateCurrentInstanceId JNIEnv is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(g_bridgeJniLock);
    g_currentInstanceId = static_cast<int32_t>(instanceId);
    LOGI("BridgeJni Update Current InstanceId is %{public}d", g_currentInstanceId);
}

void BridgeJni::JSCallMethodJni(const int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const std::string& parameters)
{
    LOGD("JSCallMethodJni enter, instanceId is %{public}d, bridgeName is %{public}s, methodName is %{public}s",
        instanceId, bridgeName.c_str(), methodName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSCallMethodJni_);
    jobject obj = GetJObjectByInstanceId(instanceId);
    if (obj == nullptr) {
        LOGE("JSCallMethodJni failed instanceId is %{public}d, bridgeName is %{public}s, methodName is %{public}s",
            instanceId, bridgeName.c_str(), methodName.c_str());
        return;
    }

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    jstring jParameters = env->NewStringUTF(parameters.c_str());
    if (jBridgeName == nullptr || jMethodName == nullptr || jParameters == nullptr) {
        LOGE("jBridgeName or jMethodName or jParameters is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jMethodName);
        DeleteLocalRefString(env, jParameters);
        return;
    }
    env->CallVoidMethod(obj, g_pluginClass.JSCallMethodJni_, jBridgeName, jMethodName, jParameters);
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
        BridgeManager::PlatformSendMethodResult(instanceId, bridgeName, methodName, result);
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS, "ArkUI-XBridgeJniPlatformSendMethodResult");
}

void BridgeJni::PlatformCallMethod(JNIEnv *env, jobject jobj,
    jstring jBridgeName, jstring jMethodName, jstring jParam, jint instanceId)
{
    CHECK_NULL_VOID(env);
    std::string callBridgeName;
    std::string callMethodName;
    std::string callParam;
    const auto *bridgeNameStr = env->GetStringUTFChars(jBridgeName, nullptr);
    if (bridgeNameStr != nullptr) {
        callBridgeName = bridgeNameStr;
        env->ReleaseStringUTFChars(jBridgeName, bridgeNameStr);
    }
    const auto *methodNameStr = env->GetStringUTFChars(jMethodName, nullptr);
    if (methodNameStr != nullptr) {
        callMethodName = methodNameStr;
        env->ReleaseStringUTFChars(jMethodName, methodNameStr);
    }
    const auto *paramStr = env->GetStringUTFChars(jParam, nullptr);
    if (paramStr != nullptr) {
        callParam = paramStr;
        env->ReleaseStringUTFChars(jParam, paramStr);
    }
    if (!BridgeManager::JSBridgeExists(instanceId, callBridgeName)) {
        std::string resultValue = "{\"errorCode\":1, \"errorMessage\":\"Bridge name error!\", \"result\":0}";
        JSSendMethodResultJni(instanceId, callBridgeName, callMethodName, resultValue);
        return;
    }
    auto taskExecutor = GetPlatformTaskExecutor(instanceId);
    ContainerScope scope(instanceId);
    if (!taskExecutor) {
        LOGE("PlatformCallMethod taskExecutor = nullptr, the instanceId is %{public}d", instanceId);
        return;
    }
    auto task = [callBridgeName, callMethodName, callParam, instanceId] {
        BridgeManager::PlatformCallMethod(instanceId, callBridgeName, callMethodName, callParam);
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS, "ArkUI-XBridgeJniPlatformCallMethod");
}

void BridgeJni::JSSendMethodResultJni(const int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const std::string& resultValue)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMethodResultJni_);
    jobject obj = GetJObjectByInstanceId(instanceId);
    if (obj == nullptr) {
        LOGE("JSSendMethodResultJni failed instanceId is %{public}d, bridgeName is %{public}s, methodName is "
            "%{public}s", instanceId, bridgeName.c_str(), methodName.c_str());
        return;
    }

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    jstring jResultValue = env->NewStringUTF(resultValue.c_str());
    if (jBridgeName == nullptr || jMethodName == nullptr || jResultValue == nullptr) {
        LOGE("jBridgeName or jMethodName or jResultValue is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jMethodName);
        DeleteLocalRefString(env, jResultValue);
        return;
    }
    env->CallVoidMethod(obj, g_pluginClass.JSSendMethodResultJni_, jBridgeName, jMethodName, jResultValue);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
    env->DeleteLocalRef(jResultValue);
}

void BridgeJni::JSSendMessageJni(const int32_t instanceId, const std::string& bridgeName, const std::string& data)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMessageJni_);
    jobject obj = GetJObjectByInstanceId(instanceId);
    if (obj == nullptr) {
        LOGE("JSSendMessageJni failed instanceId is %{public}d, bridgeName is %{public}s",
            instanceId, bridgeName.c_str());
        return;
    }

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jData = env->NewStringUTF(data.c_str());
    if (jBridgeName == nullptr || jData == nullptr) {
        LOGE("jBridgeName or jData is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jData);
        return;
    }
    env->CallVoidMethod(obj, g_pluginClass.JSSendMessageJni_, jBridgeName, jData);
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
        BridgeManager::PlatformSendMessageResponse(instanceId, bridgeName, data);
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS, "ArkUI-XBridgeJniPlatformSendMessageResponse");
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
        BridgeManager::PlatformSendMessage(instanceId, bridgeName, data);
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS, "ArkUI-XBridgeJniPlatformSendMessage");
}

void BridgeJni::JSSendMessageResponseJni(const int32_t instanceId,
    const std::string& bridgeName, const std::string& data)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMessageResponseJni_);
    jobject obj = GetJObjectByInstanceId(instanceId);
    if (obj == nullptr) {
        LOGE("JSSendMessageResponseJni failed instanceId is %{public}d, bridgeName is %{public}s",
            instanceId, bridgeName.c_str());
        return;
    }

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jData = env->NewStringUTF(data.c_str());
    if (jBridgeName == nullptr || jData == nullptr) {
        LOGE("jBridgeName or jData is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jData);
        return;
    }
    env->CallVoidMethod(obj, g_pluginClass.JSSendMessageResponseJni_, jBridgeName, jData);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jData);
}

void BridgeJni::JSCancelMethodJni(const int32_t instanceId,
    const std::string& bridgeName, const std::string& methodName)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSCancelMethodJni_);
    jobject obj = GetJObjectByInstanceId(instanceId);
    if (obj == nullptr) {
        LOGE("JSCancelMethodJni failed instanceId is %{public}d, bridgeName is %{public}s, methodName is %{public}s",
            instanceId, bridgeName.c_str(), methodName.c_str());
        return;
    }

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    if (jBridgeName == nullptr || jMethodName == nullptr) {
        LOGE("jBridgeName or jMethodName is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jMethodName);
        return;
    }
    env->CallVoidMethod(obj, g_pluginClass.JSCancelMethodJni_, jBridgeName, jMethodName);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
}

void BridgeJni::JSCallMethodBinaryJni(const int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const std::vector<uint8_t>& data)
{
    LOGD("JSCallMethodBinaryJni enter, instanceId is %{public}d, bridgeName is %{public}s, methodName is "
        "%{public}s", instanceId, bridgeName.c_str(), methodName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSCallMethodBinaryJni_);
    jobject obj = GetJObjectByInstanceId(instanceId);
    if (obj == nullptr) {
        LOGE("JSCallMethodBinaryJni failed instanceId is %{public}d, bridgeName is %{public}s, methodName is "
             "%{public}s", instanceId, bridgeName.c_str(), methodName.c_str());
        return;
    }
    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    jobject jByteBuffer = env->NewDirectByteBuffer((void *)data.data(), data.size());
    if (jBridgeName == nullptr || jMethodName == nullptr || jByteBuffer == nullptr) {
        LOGE("jBridgeName or jMethodName or jByteBuffer is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jMethodName);
        if (jByteBuffer) {
            env->DeleteLocalRef(jByteBuffer);
        }
        return;
    }

    env->CallVoidMethod(obj, g_pluginClass.JSCallMethodBinaryJni_,
        jBridgeName, jMethodName, jByteBuffer);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
    env->DeleteLocalRef(jByteBuffer);
}

void BridgeJni::PlatformSendMethodResultBinary(JNIEnv *env, jobject jobj, jstring jBridgeName, jstring jMethodName,
    jobject jBuffer, jint instanceId, jint jErrorCode, jstring jErrorMessage)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName = TransformString(env, jBridgeName);
    std::string methodName = TransformString(env, jMethodName);
    std::string errorMessage = TransformString(env, jErrorMessage);
    if (bridgeName == CONVER_FAILED || methodName == CONVER_FAILED || errorMessage == CONVER_FAILED) {
        LOGE("bridgeName or methodName or errorMessage conversion failed");
        return;
    }
    uint8_t* bufferAddress = (unsigned char *)env->GetDirectBufferAddress(jBuffer);
    size_t bufferSize = static_cast<size_t>(env->GetDirectBufferCapacity(jBuffer));
    uint8_t* buffer = BufferMapping::Copy(bufferAddress, bufferSize).Release();

    auto taskExecutor = GetPlatformTaskExecutor(instanceId);
    ContainerScope scope(instanceId);
    if (!taskExecutor) {
        LOGE("BridgeJni PlatformSendMessage taskExecutor is nullptr");
        return;
    }
    
    auto task = [bridgeName, methodName, jErrorCode, errorMessage, buffer, bufferSize, instanceId] {
        auto mapping = std::make_unique<BufferMapping>(buffer, bufferSize);
        BridgeManager::PlatformSendMethodResultBinary(instanceId, bridgeName, methodName,
            jErrorCode, errorMessage, std::move(mapping));
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS, "ArkUI-XBridgeJniPlatformSendMethodResultBinary");
}

void BridgeJni::JSSendMessageBinaryJni(
    const int32_t instanceId, const std::string& bridgeName, const std::vector<uint8_t>& data)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMessageBinaryJni_);
    jobject obj = GetJObjectByInstanceId(instanceId);
    if (obj == nullptr) {
        LOGE("JSSendMessageBinaryJni failed instanceId is %{public}d, bridgeName is %{public}s",
            instanceId, bridgeName.c_str());
        return;
    }
    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jobject jByteBuffer = env->NewDirectByteBuffer((void *)data.data(), data.size());
    if (jByteBuffer == nullptr) {
        LOGE("jByteBuffer is nullptr");
        return;
    }
    env->CallVoidMethod(obj, g_pluginClass.JSSendMessageBinaryJni_, jBridgeName, jByteBuffer);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jByteBuffer);
}

void BridgeJni::PlatformSendMessageBinary(JNIEnv *env, jobject jobj,
    jstring jBridgeName, jobject jBuffer, jint instanceId)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName;
    auto bridgeNameStr = env->GetStringUTFChars(jBridgeName, nullptr);
    if (bridgeNameStr != nullptr) {
        bridgeName = bridgeNameStr;
        env->ReleaseStringUTFChars(jBridgeName, bridgeNameStr);
    }
    uint8_t* bufferAddress = (unsigned char *)env->GetDirectBufferAddress(jBuffer);
    size_t bufferSize = static_cast<size_t>(env->GetDirectBufferCapacity(jBuffer));
    uint8_t* buffer = BufferMapping::Copy(bufferAddress, bufferSize).Release();

    auto taskExecutor = GetPlatformTaskExecutor(instanceId);
    ContainerScope scope(instanceId);
    if (!taskExecutor) {
        LOGE("BridgeJni PlatformSendMessage taskExecutor is nullptr");
        return;
    }

    auto task = [bridgeName, buffer, bufferSize, instanceId] {
        auto mapping = std::make_unique<BufferMapping>(buffer, bufferSize);
        BridgeManager::PlatformSendMessageBinary(instanceId, bridgeName, std::move(mapping));
    };
    taskExecutor->PostTask(task, TaskExecutor::TaskType::JS, "ArkUI-XBridgeJniPlatformSendMessageBinary");
}

void BridgeJni::JSSendMethodResultBinaryJni(const int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const int32_t errorCode, const std::string& errorMessage,
    std::unique_ptr<std::vector<uint8_t>> result)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMethodResultBinaryJni_);
    jobject obj = GetJObjectByInstanceId(instanceId);
    if (obj == nullptr) {
        LOGE("JSSendMethodResultBinaryJni failed instanceId is %{public}d, bridgeName is %{public}s, methodName is "
            "%{public}s", instanceId, bridgeName.c_str(), methodName.c_str());
        return;
    }
    jobject jByteBuffer = nullptr;
    if (result != nullptr) {
        jByteBuffer = env->NewDirectByteBuffer((void *)result->data(), result->size());
        if (jByteBuffer == nullptr) {
            return;
        }
    }
    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    jstring jErrorMessage = env->NewStringUTF(errorMessage.c_str());
    if (jBridgeName == nullptr || jMethodName == nullptr || jErrorMessage == nullptr) {
        LOGE("jBridgeName or jMethodName or jErrorMessage is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jMethodName);
        DeleteLocalRefString(env, jErrorMessage);
        return;
    }
    env->CallVoidMethod(obj, g_pluginClass.JSSendMethodResultBinaryJni_,
        jBridgeName, jMethodName, jByteBuffer, errorCode, jErrorMessage);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
    env->DeleteLocalRef(jErrorMessage);
    env->DeleteLocalRef(jByteBuffer);
}

void BridgeJni::PlatformCallMethodBinary(JNIEnv *env, jobject jobj,
    jstring jBridgeName, jstring jMethodName, jobject jBuffer, jint instanceId)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName = TransformString(env, jBridgeName);
    std::string methodName = TransformString(env, jMethodName);
    if (bridgeName == CONVER_FAILED || methodName == CONVER_FAILED) {
        LOGE("bridgeName or methodName conversion failed");
        return;
    }
    auto taskExecutor = GetPlatformTaskExecutor(instanceId);
    ContainerScope scope(instanceId);
    if (!taskExecutor) {
        LOGE("PlatformCallMethodBinary taskExecutor = nullptr, the instanceId is %{public}d", instanceId);
        return;
    }
    if (jBuffer == nullptr) {
        auto task = [bridgeName, methodName, instanceId] {
            BridgeManager::PlatformCallMethodBinary(instanceId, bridgeName, methodName, nullptr);
        };
        taskExecutor->PostTask(task, TaskExecutor::TaskType::JS, "ArkUI-XBridgeJniPlatformCallMethodBinaryIf");
    } else {
        uint8_t* bufferAddress = (unsigned char *)env->GetDirectBufferAddress(jBuffer);
        size_t bufferSize = static_cast<size_t>(env->GetDirectBufferCapacity(jBuffer));
        uint8_t* buffer = BufferMapping::Copy(bufferAddress, bufferSize).Release();

        auto task = [bridgeName, methodName, buffer, bufferSize, instanceId] {
            auto mapping = std::make_unique<BufferMapping>(buffer, bufferSize);
            BridgeManager::PlatformCallMethodBinary(instanceId, bridgeName, methodName, std::move(mapping));
        };
        taskExecutor->PostTask(task, TaskExecutor::TaskType::JS, "ArkUI-XBridgeJniPlatformCallMethodBinaryElse");
    }
}

void BridgeJni::ReleaseInstance(int32_t instanceId)
{
    g_jobjects.erase(instanceId);
}

int32_t BridgeJni::GetCurrentInstanceId()
{
    return g_currentInstanceId;
}
}  // namespace OHOS::Ace::Platform