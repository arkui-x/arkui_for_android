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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_ABILITY_CONTEXT_ADAPTER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_ABILITY_CONTEXT_ADAPTER_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include "jni.h"
#include "jni_environment.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class AbilityContextAdapter {
public:
    AbilityContextAdapter();
    ~AbilityContextAdapter();

    static std::shared_ptr<AbilityContextAdapter> GetInstance();
    void AddStageActivity(const std::string& instanceName, jobject stageActivity);
    void RemoveStageActivity(const std::string& instanceName);
    int32_t StartAbility(const std::string& instanceName, const AAFwk::Want& want);
    int32_t DoAbilityForeground(const std::string &fullName);
    int32_t DoAbilityBackground(const std::string &fullName);
    void TerminateSelf(const std::string& instanceName);
    void GetThreeElement(const std::string &fullName, std::string &bundleName,
        std::string &moduleName, std::string &abilityName);
    void GetNewFullName(const std::string &fullName, std::string &newFullName);
    int32_t StartAbilityForResult(
        const std::string& instanceName, const AAFwk::Want& want, int32_t requestCode);
    int32_t TerminateAbilityWithResult(
        const std::string& instanceName, const AAFwk::Want& resultWant, int32_t resultCode);
    void SetPlatformBundleName(const std::string& platformBundleName);
    std::string GetPlatformBundleName();
private:
    void ParseWant(
        const AAFwk::Want& want, std::string& bundleName, std::string& activityName, std::string& wantParams);
    static std::shared_ptr<AbilityContextAdapter> instance_;
    static std::mutex mutex_;
    std::unordered_map<std::string, Ace::Platform::JniEnvironment::JavaGlobalRef> jobjects_;
    std::string platformBundleName_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_ABILITY_CONTEXT_ADAPTER_H