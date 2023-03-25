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
AbilityContextAdapter::AbilityContextAdapter() : object_(nullptr, nullptr) {}

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

void AbilityContextAdapter::SetStageActivity(jobject stageActivity)
{
    LOGI("Set stage activity");
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    object_ = Ace::Platform::JniEnvironment::MakeJavaGlobalRef(env, stageActivity);
    if (object_ == nullptr) {
        LOGE("make global ref failed");
        return;
    }

    const jclass objClass = env->GetObjectClass(object_.get());
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return;
    }

    startActivityMethod_ = env->GetMethodID(objClass, "startActivity", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (startActivityMethod_ == nullptr) {
        LOGE("fail to get the method StartActivity id");
        return;
    }

    finishMethod_ = env->GetMethodID(objClass, "finish", "()V");
    if (finishMethod_ == nullptr) {
        LOGE("fail to get the method finish id");
        return;
    }

    env->DeleteLocalRef(objClass);
}

void AbilityContextAdapter::StartAbility(const AAFwk::Want& want)
{
    LOGI("Start ability");
    if (startActivityMethod_ == nullptr) {
        LOGE("startActivityMethod_ is nullptr");
        return;
    }

    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    auto bundleName = want.GetBundleName();
    LOGI("bundleName : %{public}s", bundleName.c_str());
    auto moduleName = want.GetModuleName();
    LOGI("moduleName : %{public}s", moduleName.c_str());
    auto abilityName = want.GetAbilityName();
    LOGI("abilityName : %{public}s", abilityName.c_str());

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

    env->CallVoidMethod(object_.get(), startActivityMethod_, jBundleName, jActivityName);
    env->DeleteLocalRef(jBundleName);
    env->DeleteLocalRef(jActivityName);
}

void AbilityContextAdapter::TerminateSelf()
{
    if (finishMethod_ == nullptr) {
        LOGE("finishMethod_ is nullptr");
        return;
    }

    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    env->CallVoidMethod(object_.get(), finishMethod_);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS