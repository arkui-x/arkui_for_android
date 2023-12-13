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

#include "stage_application_info_adapter.h"

#include "base/log/log.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<StageApplicationInfoAdapter> StageApplicationInfoAdapter::instance_ = nullptr;
std::mutex StageApplicationInfoAdapter::mutex_;

StageApplicationInfoAdapter::StageApplicationInfoAdapter() {}

StageApplicationInfoAdapter::~StageApplicationInfoAdapter() {}

std::shared_ptr<StageApplicationInfoAdapter> StageApplicationInfoAdapter::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<StageApplicationInfoAdapter>();
        }
    }

    return instance_;
}

void StageApplicationInfoAdapter::SetLocale(
    const std::string& language, const std::string& country, const std::string& script)
{
    LOGI("SetLocale, language: %{public}s, country: %{public}s, script: %{public}s", language.c_str(), country.c_str(),
        script.c_str());
    language_ = language;
    country_ = country;
    script_ = script;
}

void StageApplicationInfoAdapter::GetLocale(std::string& language, std::string& country, std::string& script)
{
    language = language_;
    country = country_;
    script = script_;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS