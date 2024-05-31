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

#ifndef FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_ACE_BRIDGE_JNI_H
#define FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_ACE_BRIDGE_JNI_H

#include <functional>
#include <vector>

#include "adapter/android/capability/java/jni/bridge/buffer_mapping.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/thread/task_executor.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "jni.h"

namespace OHOS::Ace::Platform {
using CallBackJsMethod = std::function<void(std::string)>;
class BridgeJni {
public:
    BridgeJni() = delete;
    ~BridgeJni() = delete;

    static RefPtr<TaskExecutor> GetPlatformTaskExecutor(const int32_t instanceId);
    static bool Register(const std::shared_ptr<JNIEnv> &env);
    static void NativeInit(JNIEnv *env, jobject jobj, jint instanceId);
    static void JSCallMethodJni(const int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const std::string& parameters);
    static void PlatformSendMethodResult(JNIEnv *env, jobject jobj,
        jstring jBridgeName, jstring jMethodName, jstring jResult, jint instanceId);
    static void PlatformCallMethod(JNIEnv *env, jobject jobj,
        jstring jBridgeName, jstring jMethodName, jstring jParam, jint instanceId);
    static void JSSendMethodResultJni(const int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const std::string& resultValue);
    static void JSSendMessageJni(const int32_t instanceId, const std::string& bridgeName, const std::string& data);
    static void PlatformSendMessageResponse(JNIEnv *env, jobject jobj,
        jstring jBridgeName, jstring jData, jint instanceId);
    static void PlatformSendMessage(JNIEnv *env, jobject jobj, jstring jBridgeName, jstring jData, jint instanceId);
    static void JSSendMessageResponseJni(const int32_t instanceId,
        const std::string& bridgeName, const std::string& data);
    static void JSCancelMethodJni(const int32_t instanceId,
        const std::string& bridgeName, const std::string& methodName);

    static void JSSendMessageBinaryJni(const int32_t instanceId,
        const std::string& bridgeName, const std::vector<uint8_t>& data);
    static void PlatformSendMessageBinary(JNIEnv *env, jobject jobj,
        jstring jBridgeName, jobject jBuffer, jint instanceId);
    static void JSCallMethodBinaryJni(const int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const std::vector<uint8_t>& data);
    static void PlatformSendMethodResultBinary(JNIEnv *env, jobject jobj, jstring jBridgeName, jstring jMethodName,
        jobject jBuffer, jint instanceId, jint jErrorCode, jstring jErrorMessage);
    static void PlatformCallMethodBinary(JNIEnv *env, jobject jobj,
        jstring jBridgeName, jstring jMethodName, jobject jBuffer, jint instanceId);
    static void JSSendMethodResultBinaryJni(const int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const int32_t errorCode, const std::string& errorMessage,
        std::unique_ptr<std::vector<uint8_t>> result);
};
}  // namespace OHOS::Ace::Platform
#endif  // FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_ACE_BRIDGE_JNI_H