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

#include "ability_manager_errors.h"
#include "application_context_adapter.h"
#include "base/log/log.h"
#include "base/utils/string_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
const std::string ABILITY_NAME = "Ability";
const std::string ACTIVITY_NAME = "Activity";
} // namespace

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

int32_t AbilityContextAdapter::StartAbility(const std::string& instanceName, const AAFwk::Want& want)
{
    LOGI("Start ability, caller instance name: %{public}s", instanceName.c_str());
    auto finder = jobjects_.find(instanceName);
    if (finder == jobjects_.end()) {
        LOGE("Activity caller is not exist.");
        return AAFwk::INNER_ERR;
    }

    jobject stageActivity = finder->second.get();
    if (stageActivity == nullptr) {
        LOGE("stageActivity is nullptr");
        return AAFwk::INNER_ERR;
    }

    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return AAFwk::INNER_ERR;
    }

    const jclass objClass = env->GetObjectClass(stageActivity);
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return AAFwk::INNER_ERR;
    }

    auto startActivityMethod =
        env->GetMethodID(objClass, "startActivity", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    if (startActivityMethod == nullptr) {
        LOGE("fail to get the method StartActivity id");
        return AAFwk::INNER_ERR;
    }
    env->DeleteLocalRef(objClass);

    std::string bundleName;
    std::string activityName;
    std::string wantParams;
    ParseWant(want, bundleName, activityName, wantParams);

    jstring jBundleName = env->NewStringUTF(bundleName.c_str());
    jstring jActivityName = env->NewStringUTF(activityName.c_str());
    jstring jParams = env->NewStringUTF(wantParams.c_str());
    if (jBundleName == nullptr || jActivityName == nullptr || jParams == nullptr) {
        LOGE("jBundleName or jActivityName or jParams is nullptr");
        return AAFwk::INNER_ERR;
    }

    auto result = env->CallIntMethod(stageActivity, startActivityMethod, jBundleName, jActivityName, jParams);
    env->DeleteLocalRef(jBundleName);
    env->DeleteLocalRef(jActivityName);
    env->DeleteLocalRef(jParams);

    if (result != ERR_OK) {
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t AbilityContextAdapter::DoAbilityForeground(const std::string &fullName)
{
    LOGI("Do ability foreground, caller full name: %{public}s", fullName.c_str());
    std::string instanceName = ApplicationContextAdapter::GetInstance()->GetTopAbility();
    auto finder = jobjects_.find(instanceName);
    if (finder == jobjects_.end()) {
        LOGE("Activity caller is not exist.");
        return AAFwk::INNER_ERR;
    }
    jobject stageActivity = finder->second.get();
    if (stageActivity == nullptr) {
        LOGE("stageActivity is nullptr");
        return AAFwk::INNER_ERR;
    }
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return AAFwk::INNER_ERR;
    }
    const jclass objClass = env->GetObjectClass(stageActivity);
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return AAFwk::INNER_ERR;
    }
    auto doActivityForegroundMethod = env->GetMethodID(objClass, "doActivityForeground",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    if (doActivityForegroundMethod == nullptr) {
        LOGE("fail to get the method doActivityForeground id");
        return AAFwk::INNER_ERR;
    }
    env->DeleteLocalRef(objClass);
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    GetThreeElement(fullName, bundleName, moduleName, abilityName);
    jstring jBundleName = env->NewStringUTF(bundleName.c_str());
    jstring jModuleName = env->NewStringUTF(moduleName.c_str());
    jstring jActivityName = env->NewStringUTF(abilityName.c_str());
    if (jBundleName == nullptr || jModuleName == nullptr || jActivityName == nullptr) {
        LOGE("jBundleName or jModuleName or jActivityName is nullptr");
        return AAFwk::INNER_ERR;
    }
    auto result = env->CallIntMethod(stageActivity, doActivityForegroundMethod, jBundleName, jModuleName,
        jActivityName);
    env->DeleteLocalRef(jBundleName);
    env->DeleteLocalRef(jModuleName);
    env->DeleteLocalRef(jActivityName);

    if (result != ERR_OK) {
        LOGE("DoAbilityForeground INVALID_PARAMETERS_ERR");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

void AbilityContextAdapter::GetThreeElement(const std::string &fullName, std::string &bundleName,
    std::string &moduleName, std::string &abilityName)
{
    std::vector<std::string> vecList;
    Ace::StringUtils::StringSplitter(fullName, ':', vecList);
    int32_t VEC_LENS = 3;
    size_t lens = vecList.size();
    if (lens != VEC_LENS) {
        LOGE("Enter DoAbilityForeground StringSplit fullName size not equal three!");
        return;
    }
    int abilityNameIndex = 2;
    bundleName = vecList[0];
    moduleName = vecList[1];
    abilityName = vecList[abilityNameIndex];
    LOGI("bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s", bundleName.c_str(),
        moduleName.c_str(), abilityName.c_str());
    
    if (!moduleName.empty()) {
        moduleName[0] = std::toupper(moduleName[0]);
    }
    abilityName.append(ACTIVITY_NAME);
}

int32_t AbilityContextAdapter::DoAbilityBackground(const std::string &fullName)
{
    LOGI("Do ability background");
    std::string activityName;
    std::string instanceName = ApplicationContextAdapter::GetInstance()->GetTopAbility();

    auto pos = instanceName.find(fullName);
    if (pos != std::string::npos) {
        activityName = instanceName;
    } else {
        LOGI("Do ability background, alrady background %{public}s", fullName.c_str());
        return ERR_OK;
    }
    LOGI("DoAbilityBackground get activityName, activityName: %{public}s", activityName.c_str());
    auto finder = jobjects_.find(activityName);
    if (finder == jobjects_.end()) {
        LOGE("Activity caller is not exist.");
        return AAFwk::INNER_ERR;
    }

    jobject stageActivity = finder->second.get();
    if (stageActivity == nullptr) {
        LOGE("stageActivity is nullptr");
        return AAFwk::INNER_ERR;
    }

    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return AAFwk::INNER_ERR;
    }

    const jclass objClass = env->GetObjectClass(stageActivity);
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return AAFwk::INNER_ERR;
    }

    auto doActivityBackgroundMethod = env->GetMethodID(objClass, "doActivityBackground", "()I");
    if (doActivityBackgroundMethod == nullptr) {
        LOGE("fail to get the method doActivityBackground id");
        return AAFwk::INNER_ERR;
    }
    env->DeleteLocalRef(objClass);
    auto result = env->CallIntMethod(stageActivity, doActivityBackgroundMethod);
    if (result != ERR_OK) {
        LOGE("DoAbilityBackground INVALID_PARAMETERS_ERR");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
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

int32_t AbilityContextAdapter::StartAbilityForResult(
    const std::string& instanceName, const AAFwk::Want& want, int32_t requestCode)
{
    LOGI("Start ability, caller instance name: %{public}s", instanceName.c_str());
    auto finder = jobjects_.find(instanceName);
    if (finder == jobjects_.end()) {
        LOGE("Activity caller is not exist.");
        return AAFwk::INNER_ERR;
    }

    jobject stageActivity = finder->second.get();
    if (stageActivity == nullptr) {
        LOGE("stageActivity is nullptr");
        return AAFwk::INNER_ERR;
    }

    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return AAFwk::INNER_ERR;
    }

    const jclass objClass = env->GetObjectClass(stageActivity);
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return AAFwk::INNER_ERR;
    }

    auto startActivityForResultMethod = env->GetMethodID(
        objClass, "startActivityForResult", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I");
    if (startActivityForResultMethod == nullptr) {
        LOGE("fail to get the method startActivityForResult id");
        return AAFwk::INNER_ERR;
    }
    env->DeleteLocalRef(objClass);

    std::string bundleName;
    std::string activityName;
    std::string wantParams;
    ParseWant(want, bundleName, activityName, wantParams);

    jstring jBundleName = env->NewStringUTF(bundleName.c_str());
    jstring jActivityName = env->NewStringUTF(activityName.c_str());
    jstring jParams = env->NewStringUTF(wantParams.c_str());
    if (jBundleName == nullptr || jActivityName == nullptr || jParams == nullptr) {
        LOGE("jBundleName or jActivityName or jParams is nullptr");
        return AAFwk::INNER_ERR;
    }
    auto result = env->CallIntMethod(
        stageActivity, startActivityForResultMethod, jBundleName, jActivityName, jParams, requestCode);
    env->DeleteLocalRef(jBundleName);
    env->DeleteLocalRef(jActivityName);
    env->DeleteLocalRef(jParams);

    if (result != ERR_OK) {
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t AbilityContextAdapter::TerminateAbilityWithResult(
    const std::string& instanceName, const AAFwk::Want& resultWant, int32_t resultCode)
{
    LOGI("Terminate self, caller instance name: %{public}s", instanceName.c_str());
    auto finder = jobjects_.find(instanceName);
    if (finder == jobjects_.end()) {
        LOGE("Activity caller is not exist.");
        return AAFwk::INNER_ERR;
    }

    jobject stageActivity = finder->second.get();
    if (stageActivity == nullptr) {
        LOGE("stageActivity is nullptr");
        return AAFwk::INNER_ERR;
    }

    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return AAFwk::INNER_ERR;
    }

    const jclass objClass = env->GetObjectClass(stageActivity);
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return AAFwk::INNER_ERR;
    }

    auto terminateActivityWithResultMethod =
        env->GetMethodID(objClass, "terminateActivityWithResult", "(Ljava/lang/String;I)V");
    if (terminateActivityWithResultMethod == nullptr) {
        LOGE("fail to get the method terminateActivityWithResultMethod id");
        return AAFwk::INNER_ERR;
    }
    env->DeleteLocalRef(objClass);

    jstring jResultWantParams = env->NewStringUTF(resultWant.ToJson().c_str());
    env->CallVoidMethod(stageActivity, terminateActivityWithResultMethod, jResultWantParams, resultCode);

    env->DeleteLocalRef(jResultWantParams);
    return ERR_OK;
}

void AbilityContextAdapter::ParseWant(
    const AAFwk::Want& want, std::string& bundleName, std::string& activityName, std::string& wantParams)
{
    bundleName = want.GetBundleName();
    auto moduleName = want.GetModuleName();
    auto abilityName = want.GetAbilityName();
    LOGI("bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s", bundleName.c_str(),
        moduleName.c_str(), abilityName.c_str());

    if (!moduleName.empty()) {
        moduleName[0] = std::toupper(moduleName[0]);
    }
    LOGI("moduleName : %{public}s", moduleName.c_str());

    activityName = bundleName + "." + moduleName + abilityName + ACTIVITY_NAME;
    LOGI("activityName : %{public}s", activityName.c_str());

    wantParams = want.ToJson();
    LOGI("wantParams : %{public}s", wantParams.c_str());
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
