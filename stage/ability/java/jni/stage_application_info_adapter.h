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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_APPLICATION_INFO_ADAPTER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_APPLICATION_INFO_ADAPTER_H

#include <memory>
#include <mutex>

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class StageApplicationInfoAdapter {
public:
    StageApplicationInfoAdapter();
    ~StageApplicationInfoAdapter();

    static std::shared_ptr<StageApplicationInfoAdapter> GetInstance();
    void SetLocale(const std::string& language, const std::string& country, const std::string& script);
    void GetLocale(std::string& language, std::string& country, std::string& script);

private:
    static std::shared_ptr<StageApplicationInfoAdapter> instance_;
    static std::mutex mutex_;
    std::string language_ { "" };
    std::string country_ { "" };
    std::string script_ { "" };
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_APPLICATION_INFO_ADAPTER_H