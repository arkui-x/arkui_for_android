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

    auto bridgeIter = bridgeList_.find(bridgeName);
    if (bridgeIter != bridgeList_.end()) {
        return bridgeIter->second;
    }

    LOGE("Not found JsBridge, bridgeName is %{public}s", bridgeName.c_str());
    return nullptr;
}

bool BridgeManager::JSRegisterBridge(std::shared_ptr<BridgeReceiver> bridgeReceiver)
{
    if (bridgeReceiver == nullptr || bridgeReceiver->bridgeName_.empty()) {
        LOGE("JSRegisterBridge failed, bridgeReceiver is null or bridgeName is empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bridgeLock_);
    auto bridgeIter = bridgeList_.find(bridgeReceiver->bridgeName_);
    if (bridgeIter == bridgeList_.end()) {
        bridgeList_[bridgeReceiver->bridgeName_] = bridgeReceiver;
        LOGI("JSRegisterBridge success, bridgeName: %{public}s", bridgeReceiver->bridgeName_.c_str());
        BridgeJni::JSOnRegisterResultJni(bridgeReceiver->bridgeName_, bridgeReceiver->bridgeType_, true);
        return true;
    } else {
        // Bridge already exists, update it.
        bridgeList_[bridgeReceiver->bridgeName_] = bridgeReceiver;
        LOGI("JSRegisterBridge updated existing bridge: %{public}s", bridgeReceiver->bridgeName_.c_str());
        BridgeJni::JSOnRegisterResultJni(bridgeReceiver->bridgeName_, bridgeReceiver->bridgeType_, true);
        return true;
    }
}

void BridgeManager::JSUnRegisterBridge(const std::string& bridgeName)
{
    std::lock_guard<std::mutex> lock(bridgeLock_);
    auto bridgeIter = bridgeList_.find(bridgeName);
    if (bridgeIter != bridgeList_.end()) {
        bridgeList_.erase(bridgeIter);
        LOGI("JSUnRegisterBridge success, bridgeName: %{public}s", bridgeName.c_str());
    }
    BridgeJni::JSOnRegisterResultJni(bridgeName, 0, false);
}

void BridgeManager::JSCallMethod(
    const std::string& bridgeName, const std::string& methodName, const std::string& parameter)
{
    BridgeJni::JSCallMethodJni(bridgeName, methodName, parameter);
}

void BridgeManager::JSCallMethodBinary(
    const std::string& bridgeName, const std::string& methodName, const std::vector<uint8_t>& data)
{
    BridgeJni::JSCallMethodBinaryJni(bridgeName, methodName, data);
}

std::string BridgeManager::JSCallMethodSync(
    const std::string& bridgeName, const std::string& methodName, const std::string& parameter)
{
    return BridgeJni::JSCallMethodSyncJni(bridgeName, methodName, parameter);
}

BinaryResultHolder BridgeManager::JSCallMethodBinarySync(
    const std::string& bridgeName, const std::string& methodName, const std::vector<uint8_t>& data)
{
    return BridgeJni::JSCallMethodBinarySyncJni(bridgeName, methodName, data);
}

void BridgeManager::JSSendMethodResult(
    const std::string& bridgeName, const std::string& methodName, const std::string& resultValue)
{
    BridgeJni::JSSendMethodResultJni(bridgeName, methodName, resultValue);
}

void BridgeManager::JSSendMethodResultBinary(const std::string& bridgeName,
    const std::string& methodName, int errorCode, const std::string& errorMessage,
    std::unique_ptr<std::vector<uint8_t>> result)
{
    if (result == nullptr) {
        BridgeJni::JSSendMethodResultBinaryJni(bridgeName, methodName, errorCode, errorMessage, nullptr);
    } else {
        BridgeJni::JSSendMethodResultBinaryJni(
            bridgeName, methodName, errorCode, errorMessage, std::move(result));
    }
}

void BridgeManager::JSSendMessage(const std::string& bridgeName, const std::string& data)
{
    BridgeJni::JSSendMessageJni(bridgeName, data);
}

void BridgeManager::JSSendMessageBinary(
    const std::string& bridgeName, const std::vector<uint8_t>& data)
{
    BridgeJni::JSSendMessageBinaryJni(bridgeName, data);
}

void BridgeManager::JSSendMessageResponse(const std::string& bridgeName, const std::string& data)
{
    BridgeJni::JSSendMessageResponseJni(bridgeName, data);
}

void BridgeManager::PlatformCallMethod(
    const std::string& bridgeName, const std::string& methodName, const std::string& parameter)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->callMethodCallback_) {
        receiver->callMethodCallback_(methodName, parameter);
    }
}

void BridgeManager::PlatformCallMethodBinary(const std::string& bridgeName,
    const std::string& methodName, std::unique_ptr<BufferMapping> parameter)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->callMethodBinaryCallback_) {
        receiver->callMethodBinaryCallback_(methodName, std::move(parameter));
    }
}

std::unique_ptr<BufferMapping> BridgeManager::PlatformCallMethodSyncBinary(const std::string& bridgeName,
    const std::string& methodName, std::unique_ptr<BufferMapping> parameter, int32_t& errorCode)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->callMethodSyncBinaryCallback_) {
        return receiver->callMethodSyncBinaryCallback_(methodName, std::move(parameter), errorCode);
    }
    LOGE("PlatformCallMethodSyncBinary: BridgeReceiver is null, bridgeName=%{public}s, method=%{public}s",
        bridgeName.c_str(), methodName.c_str());
    return nullptr;
}

void BridgeManager::PlatformSendMethodResult(
    const std::string& bridgeName, const std::string& methodName, const std::string& result)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->methodResultCallback_) {
        receiver->methodResultCallback_(methodName, result);
    }
}

void BridgeManager::PlatformSendMethodResultBinary(const std::string& bridgeName,
    const std::string& methodName, int errorCode, const std::string& errorMessage,
    std::unique_ptr<BufferMapping> result)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->methodResultBinaryCallback_) {
        receiver->methodResultBinaryCallback_(methodName, errorCode, errorMessage, std::move(result));
    }
}

void BridgeManager::PlatformSendMessage(const std::string& bridgeName, const std::string& data)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->sendMessageCallback_) {
        receiver->sendMessageCallback_(data);
    }
}

void BridgeManager::PlatformSendMessageBinary(
    const std::string& bridgeName, std::unique_ptr<BufferMapping> data)
{
    auto receiver = FindReceiver(bridgeName);
    if (receiver && receiver->sendMessageBinaryCallback_) {
        receiver->sendMessageBinaryCallback_(std::move(data));
    }
}

void BridgeManager::PlatformSendMessageResponse(
    const std::string& bridgeName, const std::string& data)
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
}

std::string BridgeManager::PlatformCallMethodSync(
    const std::string& bridgeName, const std::string& methodName, const std::string& parameter)
{
    LOGD("BridgeManager::PlatformCallMethodSync called for bridge '%{public}s', method '%{public}s'",
        bridgeName.c_str(), methodName.c_str());

    if (bridgeName.empty() || methodName.empty()) {
        LOGE("PlatformCallMethodSync: Bridge name or method name is empty");
        return "{\"errorCode\":1, \"errorMessage\":\"Null parameter error!\", \"result\":null}";
    }

    auto receiver = FindReceiver(bridgeName);
    std::string result = receiver->callMethodSyncCallback_(methodName, parameter);
    return result;
}

int BridgeManager::GetBridgeType(const std::string& bridgeName)
{
    auto receiver = FindReceiver(bridgeName);
    if (!receiver) {
        return -1;
    }
    return receiver->bridgeType_;
}
} // namespace OHOS::Ace::Platform
