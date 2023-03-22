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

#ifndef FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_RECEIVER_H
#define FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_RECEIVER_H

#include <functional>
#include <string>

#include "base/utils/macros.h"

namespace OHOS::Ace::Platform {
using AceCallMethodCallback = std::function<void(const std::string& methodName, const std::string& parameter)>;
using AceMethodResultCallback = std::function<void(const std::string& methodName, const std::string& resultValue)>;
using AceSendMessageCallback = std::function<void(const std::string& data)>;
using AceSendMessageResponseCallback = std::function<void(const std::string& data)>;

struct ACE_EXPORT BridgeReceiver {
    AceCallMethodCallback callMethodCallback_ = nullptr;
    AceMethodResultCallback methodResultCallback_ = nullptr;
    AceSendMessageCallback sendMessageCallback_ = nullptr;
    AceSendMessageResponseCallback sendMessageResponseCallback_ = nullptr;
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BRIDGE_RECEIVER_H
