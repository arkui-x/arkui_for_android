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

#include "application_context_adapter.h"

#include <cctype>
#include "ability_manager_errors.h"
#include "base/log/log.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
const std::string ABILITY_NAME = "Ability";
const std::string ACTIVITY_NAME = "Activity";
} // namespace
std::shared_ptr<ApplicationContextAdapter> ApplicationContextAdapter::instance_ = nullptr;
std::mutex ApplicationContextAdapter::mutex_;
ApplicationContextAdapter::ApplicationContextAdapter() : object_(nullptr, nullptr) {}

ApplicationContextAdapter::~ApplicationContextAdapter() {}

std::shared_ptr<ApplicationContextAdapter> ApplicationContextAdapter::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<ApplicationContextAdapter>();
        }
    }

    return instance_;
}

int32_t ApplicationContextAdapter::StartAbility(const AAFwk::Want& want)
{
    LOGI("AbilityDelegator:start ability with application.");
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return AAFwk::INNER_ERR;
    }

    const jclass objClass = env->GetObjectClass(object_.get());
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

    auto bundleName = want.GetBundleName();
    auto moduleName = want.GetModuleName();
    auto abilityName = want.GetAbilityName();
    LOGI("AbilityDelegator:bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s", bundleName.c_str(),
        moduleName.c_str(), abilityName.c_str());
    
    if (!moduleName.empty()) {
        moduleName[0] = std::toupper(moduleName[0]);
    }
    LOGI("AbilityDelegator:moduleName : %{public}s", moduleName.c_str());
    std::string activityName = bundleName + "." + moduleName + abilityName + ACTIVITY_NAME;

    LOGI("AbilityDelegator:activityName : %{public}s", activityName.c_str());
    jstring jBundleName = env->NewStringUTF(bundleName.c_str());
    jstring jActivityName = env->NewStringUTF(activityName.c_str());
    jstring jParams = env->NewStringUTF(want.ToJson().c_str());
    if (jBundleName == nullptr || jActivityName == nullptr || jParams == nullptr) {
        LOGE("jBundleName or jActivityName or jParams is nullptr");
        return AAFwk::INNER_ERR;
    }

    auto result = env->CallIntMethod(object_.get(), startActivityMethod, jBundleName, jActivityName, jParams);
    env->DeleteLocalRef(jBundleName);
    env->DeleteLocalRef(jActivityName);
    
    if (result != ERR_OK) {
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    return ERR_OK;
}

void ApplicationContextAdapter::SetStageApplicationDelegate(jobject stageApplicationDelegate)
{
    LOGI("Set stage application");
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    object_ = Ace::Platform::JniEnvironment::MakeJavaGlobalRef(env, stageApplicationDelegate);
    if (object_ == nullptr) {
        LOGE("object_ is nullptr");
        return;
    }

    const jclass objClass = env->GetObjectClass(object_.get());
    if (objClass == nullptr) {
        LOGE("objClass is nullptr");
        return;
    }

    getRunningProcessInfoMethod_ =
        env->GetMethodID(objClass, "getRunningProcessInfo", "()Ljava/lang/Object;");
    if (getRunningProcessInfoMethod_ == nullptr) {
        LOGE("fail to get the method StartActivity id");
        return;
    }

    env->DeleteLocalRef(objClass);
}

std::string ApplicationContextAdapter::ConvertJstringToString(std::shared_ptr<JNIEnv> env, jclass cls, jstring jstr)
{
    std::string result;
    const char* content = env->GetStringUTFChars(jstr, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jstr, content);
    }
    if (jstr != nullptr) {
        env->DeleteLocalRef(jstr);
    }
    return result;
}

int32_t ApplicationContextAdapter::FinishUserTest()
{
    LOGI("Finish user test.");
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return AAFwk::INNER_ERR;
    }

    const jclass objClass = env->GetObjectClass(object_.get());
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return AAFwk::INNER_ERR;
    }

    jmethodID finishUserTestMethod = env->GetMethodID(objClass, "finishUserTest", "()I");
    if (finishUserTestMethod == nullptr) {
        LOGE("fail to get the method finishUserTest id");
        return AAFwk::INNER_ERR;
    }

    env->DeleteLocalRef(objClass);
    env->CallIntMethod(object_.get(), finishUserTestMethod);
    return ERR_OK;
}

std::string ApplicationContextAdapter::GetTopAbility()
{
    LOGI("Get top ability.");
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return "";
    }

    const jclass objClass = env->GetObjectClass(object_.get());
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return "";
    }

    jmethodID getTopActivityMethod = env->GetMethodID(objClass, "getTopActivity", "()Ljava/lang/String;");
    if (getTopActivityMethod == nullptr) {
        LOGE("fail to get the method getTopActivity id");
        return "";
    }

    env->DeleteLocalRef(objClass);

    jstring result = (jstring) env->CallObjectMethod(object_.get(), getTopActivityMethod);
    const char* topAbility = env->GetStringUTFChars(result, NULL);
    return topAbility;
}

void ApplicationContextAdapter::Print(std::string msg)
{
    LOGI("Print the message.");
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    const jclass objClass = env->GetObjectClass(object_.get());
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        return;
    }

    jmethodID printMethod = env->GetMethodID(objClass, "print", "(Ljava/lang/String;)V");
    if (printMethod == nullptr) {
        LOGE("fail to get the method print id");
        return;
    }

    env->DeleteLocalRef(objClass);

    jstring jMsg = env->NewStringUTF(msg.c_str());
    env->CallVoidMethod(object_.get(), printMethod, jMsg);
}

std::vector<RunningProcessInfo> ApplicationContextAdapter::GetRunningProcessInformation()
{
    LOGI("Get running process information.");
    std::vector<RunningProcessInfo> infos;
    if (getRunningProcessInfoMethod_ == nullptr) {
        LOGE("getRunningProcessInfoMethod_ is nullptr");
        return infos;
    }
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return infos;
    }
    jobject jData = env->CallObjectMethod(object_.get(), getRunningProcessInfoMethod_);

    jclass listClass = env->FindClass("java/util/List");
    if (listClass == nullptr) {
        LOGE("listClass is nullptr");
        return infos;
    }
    jmethodID listGetMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    if (listGetMethod == nullptr) {
        LOGE("listGetMethod is nullptr");
        return infos;
    }
    jmethodID listSizeMethod = env->GetMethodID(listClass, "size", "()I");
    if (listSizeMethod == nullptr) {
        LOGE("listSizeMethod is nullptr");
        return infos;
    }
    int32_t listSize = env->CallIntMethod(jData, listSizeMethod);
    for (int32_t index = 0; index < listSize; index++) {
        RunningProcessInfo info;
        jobject object = env->CallObjectMethod(jData, listGetMethod, index);
        if (object == nullptr) {
            LOGE("object is nullptr");
            continue;
        }
        jclass objClass = env->GetObjectClass(object);
        if (objClass == nullptr) {
            LOGE("objClass is nullptr");
            continue;
        }
        jfieldID processNameId = env->GetFieldID(objClass, "processName", "Ljava/lang/String;");
        if (processNameId == nullptr) {
            LOGE("processNameId is nullptr");
            continue;
        }
        jstring jProcessName = (jstring)env->GetObjectField(object, processNameId);
        if (jProcessName == nullptr) {
            LOGE("jProcessName is nullptr");
            continue;
        }
        info.processName = ConvertJstringToString(env, objClass, jProcessName);
        jfieldID pidId = env->GetFieldID(objClass, "pid", "I");
        if (pidId == nullptr) {
            LOGE("pidId is nullptr");
            continue;
        }
        info.pid = env->GetIntField(object, pidId);
        jfieldID pkgListId = env->GetFieldID(objClass, "pkgList", "Ljava/lang/Object;");
        if (pkgListId == nullptr) {
            LOGE("pkgListId is nullptr");
            continue;
        }
        jobject pkglist = env->GetObjectField(object, pkgListId);
        if (pkglist == nullptr) {
            LOGE("pkglist is nullptr");
            continue;
        }
        int32_t pkglistSize = env->CallIntMethod(pkglist, listSizeMethod);
        for (int32_t pkgIndex = 0; pkgIndex < pkglistSize; pkgIndex++) {
            jstring jPackageName = (jstring)env->CallObjectMethod(pkglist, listGetMethod, pkgIndex);
            if (jPackageName == nullptr) {
                LOGE("jPackageName is nullptr");
                continue;
            }
            std::string bundleName = ConvertJstringToString(env, objClass, jPackageName);
            info.bundleNames.emplace_back(bundleName);
        }
        infos.emplace_back(info);
    }
    return infos;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
