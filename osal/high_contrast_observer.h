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

#ifndef HIGH_CONTRAST_OBSERVER_H
#define HIGH_CONTRAST_OBSERVER_H

#include <functional>
#include <mutex>
#include <unordered_map>

namespace OHOS::Ace::Platform {

using HighContrastCallback = std::function<void()>;
using ListenHighContrastCallback = std::function<void(bool)>;

class HighContrastObserver {
public:
    static HighContrastObserver& GetInstance();
    void SubscribeHighContrastChange(int32_t instanceId, HighContrastCallback&& callback);
    void SetListenHighContrastCallback(ListenHighContrastCallback&& callback);
    void UnsubscribeHighContrastChange(int32_t instanceId);
    void OnHighContrastChange(bool newHighContrast);

private:
    HighContrastObserver() = default;
    std::unordered_map<int32_t, HighContrastCallback> callbacks_;
    ListenHighContrastCallback listenHighContrastCallback_;
    bool highContrastState_ = false;
    std::mutex mutex_;
};
} // namespace OHOS::Ace::Platform
#endif // HIGH_CONTRAST_OBSERVER_H
