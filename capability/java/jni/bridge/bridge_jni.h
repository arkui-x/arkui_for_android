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

#ifndef FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_ACE_BRIDGE_JNI_H
#define FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_ACE_BRIDGE_JNI_H

#include <functional>
#include <vector>

#include "adapter/android/capability/java/jni/bridge/buffer_mapping.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/thread/task_executor.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "bridge_receiver.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "jni.h"

namespace OHOS::Ace::Platform {
class BridgeJni {
public:
    BridgeJni() = delete;
    ~BridgeJni() = delete;

    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static void NativeInit(JNIEnv* env, jobject jobj);

    static void JSCallMethodJni(
        const std::string& bridgeName, const std::string& methodName, const std::string& parameters);
    static std::string JSCallMethodSyncJni(
        const std::string& bridgeName, const std::string& methodName, const std::string& parameters);
    static void PlatformSendMethodResult(
        JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jstring jResult);
    static void PlatformCallMethod(
        JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jstring jParam);
    static void JSSendMethodResultJni(
        const std::string& bridgeName, const std::string& methodName, const std::string& resultValue);

    static void JSSendMessageJni(const std::string& bridgeName, const std::string& data);
    static void PlatformSendMessageResponse(
        JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jData);
    static void PlatformSendMessage(JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jData);
    static void JSSendMessageResponseJni(const std::string& bridgeName, const std::string& data);
    static void JSCancelMethodJni(const std::string& bridgeName, const std::string& methodName);

    static void JSSendMessageBinaryJni(const std::string& bridgeName, const std::vector<uint8_t>& data);
    static void PlatformSendMessageBinary(
        JNIEnv* env, jobject jobj, jstring jBridgeName, jobject jBuffer);
    static void JSCallMethodBinaryJni(
        const std::string& bridgeName, const std::string& methodName, const std::vector<uint8_t>& data);
    static BinaryResultHolder JSCallMethodBinarySyncJni(
        const std::string& bridgeName, const std::string& methodName, const std::vector<uint8_t>& data);
    static void PlatformSendMethodResultBinary(JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName,
        jobject jBuffer, jint jErrorCode, jstring jErrorMessage);
    static void PlatformCallMethodBinary(
        JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jobject jBuffer);
    static void JSSendMethodResultBinaryJni(const std::string& bridgeName, const std::string& methodName,
        const int32_t errorCode, const std::string& errorMessage, std::unique_ptr<std::vector<uint8_t>> result);

    static jstring PlatformCallMethodSync(
        JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jstring jParam);
    static void JSOnRegisterResultJni(const std::string& bridgeName, int32_t bridgeType, bool available);
    static jboolean JSBridgeExistsJni(JNIEnv* env, jobject jobj, jstring jBridgeName, jint jBridgeType);
    static jobject PlatformCallMethodSyncBinary(
        JNIEnv* env, jobject jobj, jstring jBridgeName, jstring jMethodName, jobject jBuffer);
};
}  // namespace OHOS::Ace::Platform
#endif  // FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_ACE_BRIDGE_JNI_H