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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_PLATFORM_EVENT_CALLBACK_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_PLATFORM_EVENT_CALLBACK_H

#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {
class PlatformEventCallback {
public:
    PlatformEventCallback() = default;
    virtual ~PlatformEventCallback() = default;
    virtual void OnFinish() const = 0;
    virtual void OnStartAbility(const std::string& address) {};
    virtual void OnStatusBarBgColorChanged(uint32_t color) = 0;

private:
    ACE_DISALLOW_COPY_AND_MOVE(PlatformEventCallback);
};
} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_PLATFORM_EVENT_CALLBACK_H
