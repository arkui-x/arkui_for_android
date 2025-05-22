/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "display_manager_agent.h"
#include "display_manager_agent_jni.h"
#include "stage_activity_delegate_jni.h"

#include <new>
#include <parcel.h>
#include "base/log/log.h"
#include "base/utils/utils.h"

using namespace OHOS::Ace;
using namespace OHOS::Ace::Platform;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AbilityRuntime::Platform;

namespace OHOS::Rosen {

std::shared_ptr<DisplayManagerAgent> DisplayManagerAgent::instance_ = nullptr;
std::mutex DisplayManagerAgent::mutex_;

DisplayManagerAgent::DisplayManagerAgent() {}

DisplayManagerAgent::~DisplayManagerAgent() {}

std::shared_ptr<DisplayManagerAgent> DisplayManagerAgent::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<DisplayManagerAgent>();
        }
    }
    return instance_;
}

bool DisplayManagerAgent::IsFoldable() const
{
    return DisplayManagerAgentJni::IsFoldable();
}

uint32_t DisplayManagerAgent::GetFoldStatus() const
{
    return DisplayManagerAgentJni::GetFoldStatus();
}

void DisplayManagerAgent::RegisterDisplayListener()
{
    DisplayManagerAgentJni::RegisterDisplayListener();
}

void DisplayManagerAgent::UnregisterDisplayListener()
{
    DisplayManagerAgentJni::UnregisterDisplayListener();
}

std::string DisplayManagerAgent::RegisterFoldStatusListener()
{
    return StageActivityDelegateJni::OnFoldStatusChange();
}

void DisplayManagerAgent::UnRegisterFoldStatusListener()
{
    StageActivityDelegateJni::OffFoldStatusChange();
}

} // namespace OHOS::Rosen