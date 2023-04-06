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

#include "base/log/log.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
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

void ApplicationContextAdapter::SetStageApplication(jobject stageApplication)
{
    LOGI("Set stage application");
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    object_ = Ace::Platform::JniEnvironment::MakeJavaGlobalRef(env, stageApplication);
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
