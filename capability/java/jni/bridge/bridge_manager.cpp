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

#include "adapter/android/capability/java/jni/bridge/bridge_manager.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
std::map<int32_t, std::map<std::string, std::shared_ptr<BridgeReceiver>>> BridgeManager::bridgeList_;
std::mutex BridgeManager::bridgeLock_;

std::shared_ptr<BridgeReceiver> BridgeManager::FindReceiver(int32_t instanceId, const std::string& bridgeName)
{
    std::lock_guard<std::mutex> lock(bridgeLock_);

    auto instanceIdIter = bridgeList_.find(instanceId);
    if (instanceIdIter != bridgeList_.end()) {
        auto bridgeIter = instanceIdIter->second.find(bridgeName);
        if (bridgeIter != instanceIdIter->second.end()) {
            return bridgeIter->second;
        }
    }
    return nullptr;
}

bool BridgeManager::JSRegisterBridge(int32_t instanceId, std::shared_ptr<BridgeReceiver> callback)
{
    if (callback == nullptr || instanceId <= 0 || callback->bridgeName_.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(bridgeLock_);
    auto instanceIdIter = bridgeList_.find(instanceId);
    if (instanceIdIter == bridgeList_.end()) {
        std::map<std::string, std::shared_ptr<BridgeReceiver>> brideList;
        brideList[callback->bridgeName_] = callback;
        bridgeList_[instanceId] = brideList;
        return true;
    } else {
        auto bridgeIter = instanceIdIter->second.find(callback->bridgeName_);
        if (bridgeIter == instanceIdIter->second.end()) {
            instanceIdIter->second[callback->bridgeName_] = callback;
            return true;
        }
    }
    return false;
}

void BridgeManager::JSUnRegisterBridge(int32_t instanceId, const std::string& bridgeName)
{
    std::lock_guard<std::mutex> lock(bridgeLock_);
    auto instanceIdIter = bridgeList_.find(instanceId);
    if (instanceIdIter == bridgeList_.end()) {
        return;
    }

    auto bridgeIter = instanceIdIter->second.find(bridgeName);
    if (bridgeIter != instanceIdIter->second.end()) {
        instanceIdIter->second.erase(bridgeIter);
    }
    if (!instanceIdIter->second.empty()) {
        bridgeList_.erase(instanceIdIter);
    }
}

void BridgeManager::JSCallMethod(int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const std::string& parameter)
{
    BridgeJni::JSCallMethodJni(instanceId, bridgeName, methodName, parameter);
}

void BridgeManager::JSCallMethodBinary(int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const std::vector<uint8_t>& data)
{
    BridgeJni::JSCallMethodBinaryJni(instanceId, bridgeName, methodName, data);
}

void BridgeManager::JSSendMethodResult(int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const std::string& resultValue)
{
     BridgeJni::JSSendMethodResultJni(instanceId, bridgeName, methodName, resultValue);
}

void BridgeManager::JSSendMethodResultBinary(int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, int errorCode, const std::string& errorMessage,
    std::unique_ptr<std::vector<uint8_t>> result)
{
    if (result == nullptr) {
        BridgeJni::JSSendMethodResultBinaryJni(instanceId, bridgeName, methodName, errorCode, errorMessage, nullptr);
    } else {
        BridgeJni::JSSendMethodResultBinaryJni(
            instanceId, bridgeName, methodName, errorCode, errorMessage, std::move(result));
    }
}

void BridgeManager::JSSendMessage(int32_t instanceId, const std::string& bridgeName, const std::string& data)
{
    BridgeJni::JSSendMessageJni(instanceId, bridgeName, data);
}

void BridgeManager::JSSendMessageBinary(int32_t instanceId, const std::string& bridgeName, const std::vector<uint8_t>& data)
{
    BridgeJni::JSSendMessageBinaryJni(instanceId, bridgeName, data);
}

void BridgeManager::JSSendMessageResponse(int32_t instanceId, const std::string& bridgeName, const std::string& data)
{
    BridgeJni::JSSendMessageResponseJni(instanceId, bridgeName, data);
}

void BridgeManager::PlatformCallMethod(int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const std::string& parameter)
{
    auto receiver = FindReceiver(instanceId, bridgeName);
    if (receiver && receiver->callMethodCallback_) {
        receiver->callMethodCallback_(methodName, parameter);
    }
}

void BridgeManager::PlatformCallMethodBinary(int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, std::unique_ptr<BufferMapping> parameter)
{
    auto receiver = FindReceiver(instanceId, bridgeName);
    if (receiver && receiver->callMethodBinaryCallback_) {
        receiver->callMethodBinaryCallback_(methodName, std::move(parameter));
    }
}

void BridgeManager::PlatformSendMethodResult(int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, const std::string& result)
{
    auto receiver = FindReceiver(instanceId, bridgeName);
    if (receiver && receiver->methodResultCallback_) {
        receiver->methodResultCallback_(methodName, result);
    }
}

void BridgeManager::PlatformSendMethodResultBinary(int32_t instanceId, const std::string& bridgeName,
    const std::string& methodName, int errorCode, const std::string& errorMessage,
    std::unique_ptr<BufferMapping> result)
{
    auto receiver = FindReceiver(instanceId, bridgeName);
    if (receiver && receiver->methodResultBinaryCallback_) {
        receiver->methodResultBinaryCallback_(methodName, errorCode, errorMessage, std::move(result));
    }
}

void BridgeManager::PlatformSendMessage(int32_t instanceId, const std::string& bridgeName, const std::string& data)
{
    auto receiver = FindReceiver(instanceId, bridgeName);
    if (receiver && receiver->sendMessageCallback_) {
        receiver->sendMessageCallback_(data);
    }
}

void BridgeManager::PlatformSendMessageBinary(
    int32_t instanceId, const std::string& bridgeName, std::unique_ptr<BufferMapping> data)
{
    auto receiver = FindReceiver(instanceId, bridgeName);
    if (receiver && receiver->sendMessageBinaryCallback_) {
        receiver->sendMessageBinaryCallback_(std::move(data));
    }
}

void BridgeManager::PlatformSendMessageResponse(int32_t instanceId,
    const std::string& bridgeName, const std::string& data)
{
    auto receiver = FindReceiver(instanceId, bridgeName);
    if (receiver && receiver->sendMessageResponseCallback_) {
        receiver->sendMessageResponseCallback_(data);
    }
}

bool BridgeManager::JSBridgeExists(int32_t instanceId, const std::string& bridgeName)
{
    return (FindReceiver(instanceId, bridgeName) != nullptr);
}

void BridgeManager::JSCancelMethod(int32_t instanceId, const std::string& bridgeName, const std::string& methodName)
{
    BridgeJni::JSCancelMethodJni(instanceId, bridgeName, methodName);
    LOGE("The method was canceled: %{public}s", methodName.c_str());
}
} // namespace OHOS::Ace::Platform
