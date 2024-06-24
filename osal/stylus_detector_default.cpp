/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "core/common/stylus/stylus_detector_default.h"

#include "frameworks/base/log/log_wrapper.h"
namespace OHOS::Ace {
StylusDetectorDefault* StylusDetectorDefault::GetInstance()
{
    static StylusDetectorDefault instance;
    return &instance;
}

bool StylusDetectorDefault::IsEnable()
{
    return isEnable_;
}

bool StylusDetectorDefault::RegisterStylusInteractionListener(
    const std::string& bundleName, const std::shared_ptr<IStylusDetectorCallback>& callback)
{
    return false;
}

void StylusDetectorDefault::UnRegisterStylusInteractionListener(const std::string& bundleName) {}

bool StylusDetectorDefault::Notify(const NotifyInfo& notifyInfo)
{
    defaultNodeId_ = 0;
    return false;
}

void StylusDetectorDefault::ExecuteCommand(const std::vector<std::string>& params) {}
} // namespace OHOS::Ace