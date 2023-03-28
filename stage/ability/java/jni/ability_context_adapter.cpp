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

#include "ability_context_adapter.h"

#include <cctype>

#include "base/log/log.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<AbilityContextAdapter> AbilityContextAdapter::instance_ = nullptr;
std::mutex AbilityContextAdapter::mutex_;
AbilityContextAdapter::AbilityContextAdapter() {}

AbilityContextAdapter::~AbilityContextAdapter() {}

std::shared_ptr<AbilityContextAdapter> AbilityContextAdapter::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AbilityContextAdapter>();
        }
    }

    return instance_;
}

void AbilityContextAdapter::AddStageActivity(const std::string& instanceName, jobject stageActivity)
{
    LOGI("Set stage activity");
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    jobjects_.emplace(instanceName, Ace::Platform::JniEnvironment::MakeJavaGlobalRef(env, stageActivity));
}

void AbilityContextAdapter::StartAbility(const std::string& instanceName, const AAFwk::Want& want)
{
    LOGI("Start ability, caller instance name: %{public}s", instanceName.c_str());
    auto finder = jobjects_.find(instanceName);
    if (finder == jobjects_.end()) {
        LOGE("Activity caller is not exist.");
        return;
    }

    jobject stageActivity = finder->second.get();
    if (stageActivity == nullptr) {
        LOGE("stageActivity is nullptr");
        return;
    }

    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    const jclass objClass = env->GetObjectClass(stageActivity);
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return;
    }

    auto startActivityMethod = env->GetMethodID(objClass, "startActivity", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (startActivityMethod == nullptr) {
        LOGE("fail to get the method StartActivity id");
        return;
    }
    env->DeleteLocalRef(objClass);

    auto bundleName = want.GetBundleName();
    auto moduleName = want.GetModuleName();
    auto abilityName = want.GetAbilityName();
    LOGI("bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s", bundleName.c_str(),
        moduleName.c_str(), abilityName.c_str());

    moduleName[0] = std::toupper(moduleName[0]);
    LOGI("moduleName : %{public}s", moduleName.c_str());
    std::string tempName = "Ability";
    std::string activityName =
        bundleName + "." + moduleName + abilityName.replace(abilityName.find(tempName), 7, "Activity");
    LOGI("activityName : %{public}s", activityName.c_str());
    jstring jBundleName = env->NewStringUTF(bundleName.c_str());
    jstring jActivityName = env->NewStringUTF(activityName.c_str());
    if (jBundleName == nullptr || jActivityName == nullptr) {
        LOGE("jBundleName or jActivityName is nullptr");
        return;
    }

    env->CallVoidMethod(stageActivity, startActivityMethod, jBundleName, jActivityName);
    env->DeleteLocalRef(jBundleName);
    env->DeleteLocalRef(jActivityName);
}

void AbilityContextAdapter::TerminateSelf(const std::string& instanceName)
{
    LOGI("Terminate self, caller instance name: %{public}s", instanceName.c_str());
    auto finder = jobjects_.find(instanceName);
    if (finder == jobjects_.end()) {
        LOGE("Activity caller is not exist.");
        return;
    }

    jobject stageActivity = finder->second.get();
    if (stageActivity == nullptr) {
        LOGE("stageActivity is nullptr");
        return;
    }

    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    const jclass objClass = env->GetObjectClass(stageActivity);
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return;
    }

    auto finishMethod = env->GetMethodID(objClass, "finish", "()V");
    if (finishMethod == nullptr) {
        LOGE("fail to get the method finish id");
        return;
    }
    env->DeleteLocalRef(objClass);

    env->CallVoidMethod(stageActivity, finishMethod);
}

void AbilityContextAdapter::RemoveStageActivity(const std::string& instanceName)
{
    LOGI("Remove stage activity, instance name: %{public}s", instanceName.c_str());
    auto finder = jobjects_.find(instanceName);
    if (finder != jobjects_.end()) {
        jobjects_.erase(finder);
    }
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS