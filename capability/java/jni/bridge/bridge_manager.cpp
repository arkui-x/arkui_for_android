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
std::map<std::string, std::shared_ptr<BridgeReceiver>> BridgeManager::bridgeList_;
std::mutex BridgeManager::bridgeLock_;

std::shared_ptr<BridgeReceiver> BridgeManager::FindReceiver(const std::string& bridgeName)
{
    std::lock_guard<std::mutex> lock(bridgeLock_);

    auto iter = bridgeList_.find(bridgeName);
    if (iter != bridgeList_.end()) {
        return iter->second;
    }
    return nullptr;
}

bool BridgeManager::JSRegisterBridge(const std::string& bridgeName,
    std::shared_ptr<BridgeReceiver> callback)
{
    if (bridgeName.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(bridgeLock_);
    auto iter = bridgeList_.find(bridgeName);
    if (iter == bridgeList_.end()) {
        bridgeList_[bridgeName] = callback;
        return true;
    }
    return false;
}

void BridgeManager::JSUnRegisterBridge(const std::string& bridgeName)
{
    std::lock_guard<std::mutex> lock(bridgeLock_);
    auto iter = bridgeList_.find(bridgeName);
    if (iter != bridgeList_.end()) {
        bridgeList_.erase(iter);
    }
}

void BridgeManager::JSCallMethod(const std::string& bridgeName,
    const std::string& methodName, const std::string& parameter)
{
    BridgeJni::JSCallMethodJni(bridgeName, methodName, parameter);
}

void BridgeManager::JSSendMethodResult(const std::string& bridgeName,
    const std::string& methodName, const std::string& resultValue)
{
    BridgeJni::JSSendMethodResultJni(bridgeName, methodName, resultValue);
}

void BridgeManager::JSSendMessage(const std::string& bridgeName, const std::string& data)
{
    BridgeJni::JSSendMessageJni(bridgeName, data);
}

void BridgeManager::JSSendMessageResponse(const std::string& bridgeName, const std::string& data)
{
    BridgeJni::JSSendMessageResponseJni(bridgeName, data);
}

void BridgeManager::PlatformCallMethod(const std::string& bridgeName,
    const std::string& methodName, const std::string& parameter)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->callMethodCallback_) {
        receiver->callMethodCallback_(methodName, parameter);
    }
}

void BridgeManager::PlatformSendMethodResult(const std::string& bridgeName,
    const std::string& methodName, const std::string& result)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->methodResultCallback_) {
        receiver->methodResultCallback_(methodName, result);
    }
}

void BridgeManager::PlatformSendMessage(const std::string& bridgeName, const std::string& data)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->sendMessageCallback_) {
        receiver->sendMessageCallback_(data);
    }
}

void BridgeManager::PlatformSendMessageResponse(const std::string& bridgeName, const std::string& data)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->sendMessageResponseCallback_) {
        receiver->sendMessageResponseCallback_(data);
    }
}

bool BridgeManager::JSBridgeExists(const std::string& bridgeName)
{
    return (FindReceiver(bridgeName) != nullptr);
}

void BridgeManager::JSCancelMethod(const std::string& bridgeName, const std::string& methodName)
{
    BridgeJni::JSCancelMethodJni(bridgeName, methodName);
    LOGE("The method was canceled: %{public}s", methodName.c_str());
}
} // namespace OHOS::Ace::Platform
