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

#include "adapter/android/capability/java/jni/bridge/bridge_jni.h"

#include <memory>
#include <unordered_map>
#include "ui/base/utils/utils.h"

#include "adapter/android/capability/java/jni/bridge/bridge_manager.h"
#include "core/common/container.h"
#include "frameworks/core/common/ace_engine.h"
#include "plugins/bridge/utils/include/error_code.h"

using namespace OHOS::Plugin::Bridge;
namespace OHOS::Ace::Platform {
namespace {
static const std::string CONVERSION_FAILED = "error";
// Register the native method of jni in java.
const char ACE_BRIDGE_CLASS_NAME[] = "ohos/ace/adapter/capability/bridge/BridgeManager";
static const JNINativeMethod METHODS[] = {
    {
        "nativeInit",
        "()V",
        reinterpret_cast<void*>(&BridgeJni::NativeInit)
    },
    {
        "nativePlatformCallMethod",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
        reinterpret_cast<void*>(&BridgeJni::PlatformCallMethod)
    },
    {
        "nativePlatformSendMethodResult",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
        reinterpret_cast<void*>(&BridgeJni::PlatformSendMethodResult)
    },
    {
        "nativePlatformSendMessageResponse",
        "(Ljava/lang/String;Ljava/lang/String;)V",
        reinterpret_cast<void*>(&BridgeJni::PlatformSendMessageResponse)
    },
    {
        "nativePlatformSendMessage",
        "(Ljava/lang/String;Ljava/lang/String;)V",
        reinterpret_cast<void*>(&BridgeJni::PlatformSendMessage)
    },
    {
        "nativePlatformSendMessageBinary",
        "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V",
        reinterpret_cast<void*>(&BridgeJni::PlatformSendMessageBinary)
    },
    {
        "nativePlatformSendMethodResultBinary",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;ILjava/lang/String;)V",
        reinterpret_cast<void*>(&BridgeJni::PlatformSendMethodResultBinary)
    },
    {
        "nativePlatformCallMethodBinary",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;)V",
        reinterpret_cast<void*>(&BridgeJni::PlatformCallMethodBinary)
    },
    {
        "nativePlatformCallMethodSync",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
        reinterpret_cast<void*>(&BridgeJni::PlatformCallMethodSync),
    },
    {
        "nativePlatformCallMethodSyncBinary",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;)Lohos/ace/adapter/capability/bridge/"
        "BridgeManager$BinaryResultHolder;",
        reinterpret_cast<void*>(&BridgeJni::PlatformCallMethodSyncBinary),
    },
    {
        "nativeJSBridgeExists",
        "(Ljava/lang/String;I)Z",
        reinterpret_cast<void*>(&BridgeJni::JSBridgeExistsJni),
    },
};

// Register the native method of java in jni.
static const char JS_CALL_METHOD_JNI[] = "jsCallMethod";
static const char JS_CALL_METHOD_JNI_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V";
static const char JS_CALL_METHOD_SYNC_JNI[] = "jsCallMethodSync";
static const char JS_CALL_METHOD_SYNC_JNI_PARAM[] =
    "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;";
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
static const char JS_CALL_METHOD_BINARY_SYNC_JNI[] = "jsCallMethodBinarySync";
static const char JS_CALL_METHOD_BINARY_SYNC_JNI_PARAM[] =
    "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;)Lohos/ace/adapter/capability/bridge/"
        "BridgeManager$BinaryResultHolder;";
static const char JS_SEND_MESSAGE_BINARY_JNI[] = "jsSendMessageBinary";
static const char JS_SEND_MESSAGE_BINARY_JNI_PARAM[] = "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V";
static const char JS_SEND_METHOD_RESULT_BINARY_JNI[] = "jsSendMethodResultBinary";
static const char JS_SEND_METHOD_RESULT_BINARY_JNI_PARAM[] =
    "(Ljava/lang/String;Ljava/lang/String;Ljava/nio/ByteBuffer;ILjava/lang/String;)V";
static const char JS_ON_REGISTER_RESULT_JNI[] = "jsOnRegisterResult";
static const char JS_ON_REGISTER_RESULT_JNI_PARAM[] = "(Ljava/lang/String;IZ)V";

// java methodID and object.
struct {
    jmethodID JSCallMethodJni_;
    jmethodID JSCallMethodSyncJni_;
    jmethodID JSSendMethodResultJni_;
    jmethodID JSSendMessageJni_;
    jmethodID JSSendMessageResponseJni_;
    jmethodID JSCancelMethodJni_;
    jmethodID JSCallMethodBinaryJni_;
    jmethodID JSCallMethodBinarySyncJni_;
    jmethodID JSSendMessageBinaryJni_;
    jmethodID JSSendMethodResultBinaryJni_;
    jmethodID JSOnRegisterResultJni_;
} g_pluginClass;

JniEnvironment::JavaGlobalRef g_JObject(nullptr, nullptr);
std::mutex g_bridgeJniLock;

static void DeleteLocalRefString(const std::shared_ptr<JNIEnv>& env, jstring jString)
{
    CHECK_NULL_VOID(env);
    if (jString) {
        env->DeleteLocalRef(jString);
    }
}

static void DeleteLocalRefObject(const std::shared_ptr<JNIEnv>& env, jobject jObject)
{
    CHECK_NULL_VOID(env);
    if (jObject) {
        env->DeleteLocalRef(jObject);
    }
}

static bool PrepareBinarySyncParams(const std::shared_ptr<JNIEnv>& env, const std::string& bridgeName,
    const std::string& methodName, const std::vector<uint8_t>& data, jstring& jBridgeName, jstring& jMethodName,
    jobject& jByteBuffer, int32_t& errorCode)
{
    CHECK_NULL_RETURN(env, false);
    jBridgeName = env->NewStringUTF(bridgeName.c_str());
    if (!jBridgeName) {
        LOGE("PrepareBinarySyncParams bridgeName convert failed.");
        errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_NAME_ERROR);
        return false;
    }
    jMethodName = env->NewStringUTF(methodName.c_str());
    if (!jMethodName) {
        LOGE("PrepareBinarySyncParams methodName convert failed.");
        DeleteLocalRefString(env, jBridgeName);
        errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_METHOD_NAME_ERROR);
        return false;
    }
    jByteBuffer = env->NewDirectByteBuffer((void*)data.data(), data.size());
    if (!jByteBuffer) {
        LOGE("PrepareBinarySyncParams params convert failed.");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jMethodName);
        errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_METHOD_PARAM_ERROR);
        return false;
    }
    return true;
}

static void FillHolderFromJava(const std::shared_ptr<JNIEnv>& env, jobject holderObj, BinaryResultHolder& holder)
{
    if (!env || !holderObj) {
        holder.errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_INVALID);
        return;
    }
    jclass holderCls = env->GetObjectClass(holderObj);
    if (!holderCls) {
        holder.errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_INVALID);
        env->DeleteLocalRef(holderObj);
        return;
    }
    jfieldID errorCodeField = env->GetFieldID(holderCls, "errorCode", "I");
    jfieldID resultField = env->GetFieldID(holderCls, "result", "Ljava/nio/ByteBuffer;");
    if (!errorCodeField || !resultField) {
        holder.errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_METHOD_UNIMPL);
        env->DeleteLocalRef(holderCls);
        env->DeleteLocalRef(holderObj);
        return;
    }
    holder.errorCode = static_cast<int32_t>(env->GetIntField(holderObj, errorCodeField));
    jobject resultBufferObj = env->GetObjectField(holderObj, resultField);
    if (resultBufferObj) {
        auto* addr = (unsigned char*)env->GetDirectBufferAddress(resultBufferObj);
        size_t size = static_cast<size_t>(env->GetDirectBufferCapacity(resultBufferObj));
        if (addr && size > 0) {
            uint8_t* copyBuf = BufferMapping::Copy(addr, size).Release();
            holder.buffer = std::make_unique<BufferMapping>(copyBuf, size);
        } else {
            LOGE("FillHolderFromJava buffer invalid");
        }
        env->DeleteLocalRef(resultBufferObj);
    }
    env->DeleteLocalRef(holderCls);
    env->DeleteLocalRef(holderObj);
}
}  // namespace

std::string TransformString(JNIEnv* env, jstring jString)
{
    CHECK_NULL_RETURN(env, CONVERSION_FAILED);
    std::string resultString;
    auto converString = env->GetStringUTFChars(jString, nullptr);
    if (converString != nullptr) {
        resultString = converString;
        env->ReleaseStringUTFChars(jString, converString);
    }
    return resultString;
}

bool BridgeJni::Register(const std::shared_ptr<JNIEnv>& env)
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

void BridgeJni::NativeInit(JNIEnv* env, jobject jobj)
{
    if (!env) {
        LOGE("NativeInit JNIEnv is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(g_bridgeJniLock);
    LOGI("BridgeJni NativeInit - Singleton initialization");
    g_JObject = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj);

    jclass cls = env->GetObjectClass(jobj);
    if (cls == nullptr) {
        LOGE("Bridge JNI Init: class not found");
        return;
    }

    g_pluginClass.JSCallMethodJni_ = env->GetMethodID(cls, JS_CALL_METHOD_JNI, JS_CALL_METHOD_JNI_PARAM);
    g_pluginClass.JSCallMethodSyncJni_ = env->GetMethodID(cls,
        JS_CALL_METHOD_SYNC_JNI, JS_CALL_METHOD_SYNC_JNI_PARAM);
    g_pluginClass.JSSendMethodResultJni_ = env->GetMethodID(cls,
        JS_CALL_METHOD_RESULT_JNI, JS_CALL_METHOD_RESULT_JNI_PARAM);
    g_pluginClass.JSSendMessageJni_ = env->GetMethodID(cls, JS_SEND_MESSAGE_JNI, JS_SEND_MESSAGE_JNI_PARAM);
    g_pluginClass.JSSendMessageResponseJni_ = env->GetMethodID(cls,
        JS_SEND_MESSAGE_RESPONSE_JNI, JS_SEND_MESSAGE_RESPONSE_JNI_PARAM);
    g_pluginClass.JSCancelMethodJni_ = env->GetMethodID(cls, JS_CANCEL_RESPONSE_JNI, JS_CANCEL_RESPONSE_JNI_PARAM);
    g_pluginClass.JSCallMethodBinaryJni_ = env->GetMethodID(cls,
        JS_CALL_METHOD_BINARY_JNI, JS_CALL_METHOD_BINARY_JNI_PARAM);
    g_pluginClass.JSCallMethodBinarySyncJni_ = env->GetMethodID(cls,
        JS_CALL_METHOD_BINARY_SYNC_JNI, JS_CALL_METHOD_BINARY_SYNC_JNI_PARAM);
    g_pluginClass.JSSendMessageBinaryJni_ = env->GetMethodID(cls,
        JS_SEND_MESSAGE_BINARY_JNI, JS_SEND_MESSAGE_BINARY_JNI_PARAM);
    g_pluginClass.JSSendMethodResultBinaryJni_ = env->GetMethodID(cls,
        JS_SEND_METHOD_RESULT_BINARY_JNI, JS_SEND_METHOD_RESULT_BINARY_JNI_PARAM);
    g_pluginClass.JSOnRegisterResultJni_ = env->GetMethodID(cls,
        JS_ON_REGISTER_RESULT_JNI, JS_ON_REGISTER_RESULT_JNI_PARAM);
    env->DeleteLocalRef(cls);
}

void BridgeJni::JSCallMethodJni(const std::string& bridgeName,
    const std::string& methodName, const std::string& parameters)
{
    LOGD("JSCallMethodJni  enter, bridgeName is %{public}s, methodName is %{public}s",
        bridgeName.c_str(), methodName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSCallMethodJni_);
    if (g_JObject == nullptr) {
        LOGE("JSCallMethodJni failed BridgeManager object is null, bridgeName is %{public}s, methodName is %{public}s",
            bridgeName.c_str(), methodName.c_str());
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
    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSCallMethodJni_, jBridgeName, jMethodName, jParameters);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
    env->DeleteLocalRef(jParameters);
}

std::string BridgeJni::JSCallMethodSyncJni(const std::string& bridgeName,
    const std::string& methodName, const std::string& parameters)
{
    LOGD("JSCallMethodSyncJni  enter, bridgeName is %{public}s, methodName is %{public}s",
        bridgeName.c_str(), methodName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN(env, "");
    CHECK_NULL_RETURN(g_pluginClass.JSCallMethodJni_, "");
    if (g_JObject == nullptr) {
        LOGE("JSCallMethodSyncJni failed BridgeManager object is null,"
            "bridgeName is %{public}s, methodName is %{public}s", bridgeName.c_str(), methodName.c_str());
        return "";
    }

    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    jstring jParameters = env->NewStringUTF(parameters.c_str());
    if (jBridgeName == nullptr || jMethodName == nullptr || jParameters == nullptr) {
        LOGE("jBridgeName or jMethodName or jParameters is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jMethodName);
        DeleteLocalRefString(env, jParameters);
        return "";
    }
    jstring jStr = (jstring) env->CallObjectMethod(g_JObject.get(),
        g_pluginClass.JSCallMethodSyncJni_,
        jBridgeName, jMethodName, jParameters);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    const char* cStr = env->GetStringUTFChars(jStr, nullptr);
    if (cStr) {
        env->ReleaseStringUTFChars(jStr, cStr);
    }
    std::string result(cStr);
    env->DeleteLocalRef(jStr);
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
    env->DeleteLocalRef(jParameters);
    return result;
}

void BridgeJni::PlatformSendMethodResult(JNIEnv* env, jobject jobj,
    jstring jBridgeName, jstring jMethodName, jstring jResult)
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

    BridgeManager::PlatformSendMethodResult(bridgeName, methodName, result);
}

void BridgeJni::PlatformCallMethod(JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jstring jParam)
{
    CHECK_NULL_VOID(env);
    std::string callBridgeName;
    std::string callMethodName;
    std::string callParam;
    const auto* bridgeNameStr = env->GetStringUTFChars(jBridgeName, nullptr);
    if (bridgeNameStr != nullptr) {
        callBridgeName = bridgeNameStr;
        env->ReleaseStringUTFChars(jBridgeName, bridgeNameStr);
    }
    const auto* methodNameStr = env->GetStringUTFChars(jMethodName, nullptr);
    if (methodNameStr != nullptr) {
        callMethodName = methodNameStr;
        env->ReleaseStringUTFChars(jMethodName, methodNameStr);
    }
    const auto* paramStr = env->GetStringUTFChars(jParam, nullptr);
    if (paramStr != nullptr) {
        callParam = paramStr;
        env->ReleaseStringUTFChars(jParam, paramStr);
    }
    if (!BridgeManager::JSBridgeExists(callBridgeName)) {
        std::string resultValue = "{\"errorCode\":1, \"errorMessage\":\"Bridge name error!\", \"result\":0}";
        JSSendMethodResultJni(callBridgeName, callMethodName, resultValue);
        return;
    }

    BridgeManager::PlatformCallMethod(callBridgeName, callMethodName, callParam);
}

void BridgeJni::JSSendMethodResultJni(
    const std::string& bridgeName, const std::string& methodName, const std::string& resultValue)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMethodResultJni_);
    if (g_JObject == nullptr) {
        LOGE("JSSendMethodResultJni failed - BridgeManager object is null, bridgeName is %{public}s, methodName is "
            "%{public}s", bridgeName.c_str(), methodName.c_str());
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
    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSSendMethodResultJni_, jBridgeName, jMethodName, jResultValue);

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
    CHECK_NULL_VOID(g_pluginClass.JSSendMessageJni_);
    if (g_JObject == nullptr) {
        LOGE("JSSendMessageJni failed - BridgeManager object is null, bridgeName is %{public}s",
            bridgeName.c_str());
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
    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSSendMessageJni_, jBridgeName, jData);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jData);
}

void BridgeJni::PlatformSendMessageResponse(JNIEnv* env, jobject jobj,
    jstring jBridgeName, jstring jData)
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

    BridgeManager::PlatformSendMessageResponse(bridgeName, data);
}

void BridgeJni::PlatformSendMessage(JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jData)
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

    BridgeManager::PlatformSendMessage(bridgeName, data);
}

void BridgeJni::JSSendMessageResponseJni(const std::string& bridgeName, const std::string& data)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMessageResponseJni_);
    if (g_JObject == nullptr) {
        LOGE("JSSendMessageResponseJni failed - BridgeManager object is null, bridgeName is %{public}s",
            bridgeName.c_str());
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
    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSSendMessageResponseJni_, jBridgeName, jData);
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
    CHECK_NULL_VOID(g_pluginClass.JSCancelMethodJni_);
    if (g_JObject == nullptr) {
        LOGE("JSCancelMethodJni failed BridgeManager object is null,"
            "bridgeName is %{public}s, methodName is %{public}s", bridgeName.c_str(), methodName.c_str());
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
    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSCancelMethodJni_, jBridgeName, jMethodName);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
}

void BridgeJni::JSCallMethodBinaryJni(
    const std::string& bridgeName, const std::string& methodName, const std::vector<uint8_t>& data)
{
    LOGD("JSCallMethodBinaryJni enter, bridgeName is %{public}s, methodName is %{public}s",
        bridgeName.c_str(), methodName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSCallMethodBinaryJni_);
    if (g_JObject == nullptr) {
        LOGE("JSCallMethodBinaryJni failed - BridgeManager object is null, bridgeName is %{public}s, methodName is "
             "%{public}s", bridgeName.c_str(), methodName.c_str());
        return;
    }
    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jstring jMethodName = env->NewStringUTF(methodName.c_str());
    jobject jByteBuffer = env->NewDirectByteBuffer((void*)data.data(), data.size());
    if (jBridgeName == nullptr || jMethodName == nullptr || jByteBuffer == nullptr) {
        LOGE("jBridgeName or jMethodName or jByteBuffer is nullptr");
        DeleteLocalRefString(env, jBridgeName);
        DeleteLocalRefString(env, jMethodName);
        if (jByteBuffer) {
            env->DeleteLocalRef(jByteBuffer);
        }
        return;
    }

    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSCallMethodBinaryJni_,
        jBridgeName, jMethodName, jByteBuffer);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
    env->DeleteLocalRef(jMethodName);
    env->DeleteLocalRef(jByteBuffer);
}

BinaryResultHolder BridgeJni::JSCallMethodBinarySyncJni(
    const std::string& bridgeName, const std::string& methodName, const std::vector<uint8_t>& data)
{
    LOGD("JSCallMethodBinarySyncJni enter, bridgeName=%{public}s, methodName=%{public}s", bridgeName.c_str(),
        methodName.c_str());
    BinaryResultHolder holder;
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env || !g_pluginClass.JSCallMethodBinarySyncJni_ || !g_JObject) {
        holder.errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_INVALID);
        return holder;
    }

    jstring jBridgeName = nullptr;
    jstring jMethodName = nullptr;
    jobject jByteBuffer = nullptr;
    if (!PrepareBinarySyncParams(env, bridgeName, methodName, data,
        jBridgeName, jMethodName, jByteBuffer, holder.errorCode)) {
        return holder;
    }

    jobject holderObj = env->CallObjectMethod(
        g_JObject.get(), g_pluginClass.JSCallMethodBinarySyncJni_, jBridgeName, jMethodName, jByteBuffer);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    DeleteLocalRefString(env, jBridgeName);
    DeleteLocalRefString(env, jMethodName);
    DeleteLocalRefObject(env, jByteBuffer);

    if (!holderObj) {
        holder.errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_METHOD_UNIMPL);
        return holder;
    }
    FillHolderFromJava(env, holderObj, holder);
    return holder;
}

void BridgeJni::PlatformSendMethodResultBinary(JNIEnv* env, jobject jobj,
    jstring jBridgeName, jstring jMethodName, jobject jBuffer, jint jErrorCode, jstring jErrorMessage)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName = TransformString(env, jBridgeName);
    std::string methodName = TransformString(env, jMethodName);
    std::string errorMessage = TransformString(env, jErrorMessage);
    if (bridgeName == CONVERSION_FAILED || methodName == CONVERSION_FAILED || errorMessage == CONVERSION_FAILED) {
        LOGE("bridgeName or methodName or errorMessage conversion failed");
        return;
    }
    uint8_t* bufferAddress = (unsigned char*)env->GetDirectBufferAddress(jBuffer);
    size_t bufferSize = static_cast<size_t>(env->GetDirectBufferCapacity(jBuffer));
    uint8_t* buffer = BufferMapping::Copy(bufferAddress, bufferSize).Release();
    auto mapping = std::make_unique<BufferMapping>(buffer, bufferSize);

    BridgeManager::PlatformSendMethodResultBinary(
        bridgeName, methodName, jErrorCode, errorMessage, std::move(mapping));
}

void BridgeJni::JSSendMessageBinaryJni(
    const std::string& bridgeName, const std::vector<uint8_t>& data)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMessageBinaryJni_);
    if (g_JObject == nullptr) {
        LOGE("JSSendMessageBinaryJni failed - BridgeManager object is null, bridgeName is %{public}s",
            bridgeName.c_str());
        return;
    }
    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jobject jByteBuffer = env->NewDirectByteBuffer((void*)data.data(), data.size());
    if (jByteBuffer == nullptr) {
        LOGE("jByteBuffer is nullptr");
        return;
    }
    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSSendMessageBinaryJni_, jBridgeName, jByteBuffer);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jByteBuffer);
}

void BridgeJni::PlatformSendMessageBinary(JNIEnv* env, jobject jobj, jstring jBridgeName, jobject jBuffer)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName;
    auto bridgeNameStr = env->GetStringUTFChars(jBridgeName, nullptr);
    if (bridgeNameStr != nullptr) {
        bridgeName = bridgeNameStr;
        env->ReleaseStringUTFChars(jBridgeName, bridgeNameStr);
    }
    uint8_t* bufferAddress = (unsigned char*)env->GetDirectBufferAddress(jBuffer);
    size_t bufferSize = static_cast<size_t>(env->GetDirectBufferCapacity(jBuffer));
    uint8_t* buffer = BufferMapping::Copy(bufferAddress, bufferSize).Release();
    auto mapping = std::make_unique<BufferMapping>(buffer, bufferSize);

    BridgeManager::PlatformSendMessageBinary(bridgeName, std::move(mapping));
}

void BridgeJni::JSSendMethodResultBinaryJni(const std::string& bridgeName, const std::string& methodName,
    const int32_t errorCode, const std::string& errorMessage, std::unique_ptr<std::vector<uint8_t>> result)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSSendMethodResultBinaryJni_);
    if (g_JObject == nullptr) {
        LOGE("JSSendMethodResultBinaryJni failed BridgeManager object is null,"
            "bridgeName is %{public}s, methodName is %{public}s", bridgeName.c_str(), methodName.c_str());
        return;
    }
    jobject jByteBuffer = nullptr;
    if (result != nullptr) {
        jByteBuffer = env->NewDirectByteBuffer((void*)result->data(), result->size());
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
    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSSendMethodResultBinaryJni_,
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

void BridgeJni::PlatformCallMethodBinary(
    JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jobject jBuffer)
{
    CHECK_NULL_VOID(env);
    std::string bridgeName = TransformString(env, jBridgeName);
    std::string methodName = TransformString(env, jMethodName);
    if (bridgeName == CONVERSION_FAILED || methodName == CONVERSION_FAILED) {
        LOGE("bridgeName or methodName conversion failed");
        return;
    }

    if (jBuffer == nullptr) {
        BridgeManager::PlatformCallMethodBinary(bridgeName, methodName, nullptr);
    } else {
        uint8_t* bufferAddress = (unsigned char*)env->GetDirectBufferAddress(jBuffer);
        size_t bufferSize = static_cast<size_t>(env->GetDirectBufferCapacity(jBuffer));
        uint8_t* buffer = BufferMapping::Copy(bufferAddress, bufferSize).Release();
        auto mapping = std::make_unique<BufferMapping>(buffer, bufferSize);
        BridgeManager::PlatformCallMethodBinary(bridgeName, methodName, std::move(mapping));
    }
}

jstring BridgeJni::PlatformCallMethodSync(
    JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jstring jParam)
{
    CHECK_NULL_RETURN(env, nullptr);
    if (!jBridgeName || !jMethodName || !jParam) {
        LOGE("PlatformCallMethodSync: One or more parameters are null");
        std::string errorResult = "{\"errorCode\":1, \"errorMessage\":\"Bridge name error!\", \"result\":null}";
        return env->NewStringUTF(errorResult.c_str());
    }

    std::string callBridgeName = TransformString(env, jBridgeName);
    std::string callMethodName = TransformString(env, jMethodName);
    std::string callParam = TransformString(env, jParam);
    if (callBridgeName == CONVERSION_FAILED || callMethodName == CONVERSION_FAILED || callParam == CONVERSION_FAILED) {
        LOGE("PlatformCallMethodSync: Parameter conversion failed");
        std::string errorResult = "{\"errorCode\":4, \"errorMessage\":\"Method name error!\", \"result\":null}";
        return env->NewStringUTF(errorResult.c_str());
    }

    if (callBridgeName.empty() || callMethodName.empty()) {
        LOGE("PlatformCallMethodSync: Bridge name or method name is empty");
        std::string errorResult = "{\"errorCode\":6, \"errorMessage\":\"Method not implemented!\", \"result\":null}";
        return env->NewStringUTF(errorResult.c_str());
    }

    std::string result = BridgeManager::PlatformCallMethodSync(callBridgeName, callMethodName, callParam);
    if (result.empty()) {
        LOGW("PlatformCallMethodSync: Method returned empty result");
        std::string defaultResult = "{\"errorCode\":0, \"errorMessage\":\"\", \"result\":null}";
        return env->NewStringUTF(defaultResult.c_str());
    }

    return env->NewStringUTF(result.c_str());
}

void BridgeJni::JSOnRegisterResultJni(const std::string& bridgeName, int32_t bridgeType, bool available)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.JSOnRegisterResultJni_);
    if (g_JObject == nullptr) {
        LOGE("JSOnRegisterResultJni failed - BridgeManager object is null, bridgeName is %{public}s",
            bridgeName.c_str());
        return;
    }
    jstring jBridgeName = env->NewStringUTF(bridgeName.c_str());
    jint jBridgeType = static_cast<jint>(bridgeType);
    jboolean jAvailable = available ? JNI_TRUE : JNI_FALSE;
    if (jBridgeName == nullptr) {
        LOGE("JSOnRegisterResultJni jBridgeName is null");
        return;
    }
    env->CallVoidMethod(g_JObject.get(), g_pluginClass.JSOnRegisterResultJni_, jBridgeName, jBridgeType, jAvailable);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jBridgeName);
}

jboolean BridgeJni::JSBridgeExistsJni(JNIEnv* env, jobject, jstring jBridgeName, jint jBridgeType)
{
    CHECK_NULL_RETURN(env, JNI_FALSE);
    std::string bridgeName = TransformString(env, jBridgeName);
    if (bridgeName.empty()) {
        LOGE("JSBridgeExistsJni bridgeName is empty.");
        return JNI_FALSE;
    }
    bool exists = BridgeManager::JSBridgeExists(bridgeName);
    exists = exists && (jBridgeType == BridgeManager::GetBridgeType(bridgeName));
    return (exists ? JNI_TRUE : JNI_FALSE);
}

jobject BridgeJni::PlatformCallMethodSyncBinary(
    JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jobject jBuffer)
{
    CHECK_NULL_RETURN(env, nullptr);
    std::string bridgeName = TransformString(env, jBridgeName);
    std::string methodName = TransformString(env, jMethodName);
    if (bridgeName.empty() || methodName.empty()) {
        LOGE("PlatformCallMethodSyncBinary empty bridge or method");
        return nullptr;
    }
    std::unique_ptr<BufferMapping> paramMapping = nullptr;
    if (jBuffer != nullptr) {
        uint8_t* bufferAddress = (unsigned char*)env->GetDirectBufferAddress(jBuffer);
        size_t bufferSize = static_cast<size_t>(env->GetDirectBufferCapacity(jBuffer));
        if (bufferAddress && bufferSize > 0) {
            uint8_t* buffer = BufferMapping::Copy(bufferAddress, bufferSize).Release();
            paramMapping = std::make_unique<BufferMapping>(buffer, bufferSize);
        }
    }
    int32_t errorCode = 0;
    auto resultMapping =
        BridgeManager::PlatformCallMethodSyncBinary(bridgeName, methodName, std::move(paramMapping), errorCode);
    jclass holderCls = env->FindClass("ohos/ace/adapter/capability/bridge/BridgeManager$BinaryResultHolder");
    CHECK_NULL_RETURN(holderCls, nullptr);
    jmethodID ctor = env->GetMethodID(holderCls, "<init>", "()V");
    CHECK_NULL_RETURN(ctor, nullptr);
    jobject holderObj = env->NewObject(holderCls, ctor);
    CHECK_NULL_RETURN(holderObj, nullptr);
    jfieldID errorField = env->GetFieldID(holderCls, "errorCode", "I");
    CHECK_NULL_RETURN(errorField, nullptr);
    jfieldID resultField = env->GetFieldID(holderCls, "result", "Ljava/nio/ByteBuffer;");
    CHECK_NULL_RETURN(resultField, nullptr);
    env->SetIntField(holderObj, errorField, static_cast<jint>(errorCode));
    jobject retBuffer = nullptr;
    if (resultMapping && resultMapping->GetSize() > 0 && resultMapping->GetMapping()) {
        retBuffer = env->NewDirectByteBuffer((void*)resultMapping->GetMapping(), resultMapping->GetSize());
    }
    if (retBuffer) {
        env->SetObjectField(holderObj, resultField, retBuffer);
    }
    env->DeleteLocalRef(holderCls);
    return holderObj;
}
}  // namespace OHOS::Ace::Platform