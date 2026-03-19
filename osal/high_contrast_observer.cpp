/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "high_contrast_observer.h"

namespace OHOS::Ace::Platform {

HighContrastObserver& HighContrastObserver::GetInstance()
{
    static HighContrastObserver instance;
    return instance;
}

void HighContrastObserver::SubscribeHighContrastChange(int32_t instanceId, HighContrastCallback&& callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[instanceId] = std::move(callback);
}

void HighContrastObserver::SetListenHighContrastCallback(ListenHighContrastCallback&& callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    listenHighContrastCallback_ = std::move(callback);
    if (listenHighContrastCallback_) {
        listenHighContrastCallback_(highContrastState_);
    }
    for (auto& pair : callbacks_) {
        if (pair.second) {
            pair.second();
        }
    }
}

void HighContrastObserver::UnsubscribeHighContrastChange(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.erase(instanceId);
}

void HighContrastObserver::OnHighContrastChange(bool newHighContrast)
{
    std::lock_guard<std::mutex> lock(mutex_);
    highContrastState_ = newHighContrast;
    if (listenHighContrastCallback_) {
        listenHighContrastCallback_(highContrastState_);
    }
    for (auto& pair : callbacks_) {
        if (pair.second) {
            pair.second();
        }
    }
}
} // namespace OHOS::Ace::Platform
