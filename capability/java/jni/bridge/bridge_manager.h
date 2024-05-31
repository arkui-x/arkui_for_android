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

#ifndef FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_MANAGER_H
#define FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "adapter/android/capability/java/jni/bridge/bridge_jni.h"
#include "base/utils/macros.h"
#include "bridge_receiver.h"

namespace OHOS::Ace::Platform {
class ACE_EXPORT BridgeManager final {
public:
    BridgeManager() = default;
    ~BridgeManager() = default;

    static bool JSRegisterBridge(int32_t instanceId, std::shared_ptr<BridgeReceiver> callback);
    static void JSUnRegisterBridge(int32_t instanceId, const std::string& bridgeName);
    static void JSCallMethod(int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const std::string& parameter);
    static void JSSendMethodResult(int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const std::string& resultValue);
    static void JSSendMessage(int32_t instanceId, const std::string& bridgeName, const std::string& data);
    static void JSSendMessageResponse(int32_t instanceId, const std::string& bridgeName, const std::string& data);  
    static void PlatformCallMethod(int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const std::string& parameter);
    static void PlatformSendMethodResult(int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const std::string& result);
    static void PlatformSendMessage(int32_t instanceId, const std::string& bridgeName, const std::string& data);
    static void PlatformSendMessageResponse(int32_t instanceId,
        const std::string& bridgeName, const std::string& data);
    static bool JSBridgeExists(int32_t instanceId, const std::string& bridgeName);
    static void JSCancelMethod(int32_t instanceId, const std::string& bridgeName, const std::string& methodName);

    // for binary codec
    static void JSSendMessageBinary(
        int32_t instanceId, const std::string& bridgeName, const std::vector<uint8_t>& data);
    static void JSCallMethodBinary(int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, const std::vector<uint8_t>& data);
    static void JSSendMethodResultBinary(int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, int errorCode, const std::string& errorMessage,
        std::unique_ptr<std::vector<uint8_t>> result);
    static void PlatformSendMethodResultBinary(int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, int errorCode, const std::string& errorMessage,
        std::unique_ptr<BufferMapping> result);
    static void PlatformCallMethodBinary(int32_t instanceId, const std::string& bridgeName,
        const std::string& methodName, std::unique_ptr<BufferMapping> parameter);
    static void PlatformSendMessageBinary(
        int32_t instanceId, const std::string& bridgeName, std::unique_ptr<BufferMapping> data);

private:
    static std::map<int32_t, std::map<std::string, std::shared_ptr<BridgeReceiver>>> bridgeList_;
    static std::mutex bridgeLock_;

    static std::shared_ptr<BridgeReceiver> FindReceiver(int32_t instanceId, const std::string& bridgeName);
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_MANAGER_H
