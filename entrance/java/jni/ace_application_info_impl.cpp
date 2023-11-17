/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/ace_application_info_impl.h"

#include <cstring>

#include "unicode/locid.h"

#include "base/i18n/localization.h"
#include "base/log/ace_trace.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/resource/ace_res_config.h"
#include "base/resource/ace_res_data_struct.h"
#include "core/common/ace_engine.h"

namespace OHOS::Ace::Platform {

AceApplicationInfoImpl::AceApplicationInfoImpl() : object_(nullptr, nullptr) {}

AceApplicationInfoImpl::~AceApplicationInfoImpl() = default;

void AceApplicationInfoImpl::Initialize(JNIEnv* env, jobject info)
{
    if (env == nullptr) {
        LOGW("env is null");
        EventReport::SendAppStartException(AppStartExcepType::JNI_INIT_ERR);
        return;
    }

    object_ = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), info);
    if (object_ == nullptr) {
        LOGW("make global ref failed");
        EventReport::SendAppStartException(AppStartExcepType::JNI_INIT_ERR);
        return;
    }

    const jclass objClass = env->GetObjectClass(object_.get());
    if (objClass == nullptr) {
        LOGE("GetObjectClass return null");
        EventReport::SendAppStartException(AppStartExcepType::JNI_CLASS_ERR);
        return;
    }

    changeLocale_ = env->GetMethodID(objClass, "changeLocale", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (changeLocale_ == nullptr) {
        LOGE("fail to get the method changeLocale id");
        EventReport::SendAppStartException(AppStartExcepType::JNI_CLASS_ERR);
        return;
    }

    getLocaleFallback_ =
        env->GetMethodID(objClass, "getLocaleFallback", "(Ljava/lang/String;[Ljava/lang/String;)Ljava/lang/String;");

    if (getLocaleFallback_ == nullptr) {
        LOGE("fail to get the method getLocaleFallback id");
        EventReport::SendAppStartException(AppStartExcepType::JNI_CLASS_ERR);
        return;
    }

    env->DeleteLocalRef(objClass);
    time(&initiateTimeStamp_);
}

void AceApplicationInfoImpl::SetJsEngineParam(const std::string& key, const std::string& value)
{
    jsEngineParams_[key] = value;
}

std::string AceApplicationInfoImpl::GetJsEngineParam(const std::string& key) const
{
    std::string value;
    auto iter = jsEngineParams_.find(key);
    if (iter != jsEngineParams_.end()) {
        value = iter->second;
    }
    return value;
}

void AceApplicationInfoImpl::ChangeLocale(const std::string& language, const std::string& countryOrRegion)
{
    icu::Locale locale(language.c_str(), countryOrRegion.c_str());

    if (strcmp(locale.getISO3Language(), "") == 0 || strcmp(locale.getISO3Country(), "") == 0) {
        LOGE("locale:[%{public}s-%{public}s] doesn't meet ISO standard.", language.c_str(), countryOrRegion.c_str());
        EventReport::SendInternalException(InternalExcepType::CHANGE_LOCALE_ERR);
        return;
    }

    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("fail to get jni env");
        return;
    }

    jstring jLanguage = env->NewStringUTF(language.c_str());
    jstring jCountryOrRegion = env->NewStringUTF(countryOrRegion.c_str());

    if (jLanguage != nullptr && jCountryOrRegion != nullptr && object_ != nullptr && changeLocale_ != nullptr) {
        env->CallVoidMethod(object_.get(), changeLocale_, jLanguage, jCountryOrRegion);
        if (env->ExceptionCheck()) {
            LOGE("Exception occured, call changeLocale failed");
            EventReport::SendInternalException(InternalExcepType::CHANGE_LOCALE_ERR);
        }
    }

    if (jLanguage != nullptr) {
        env->DeleteLocalRef(jLanguage);
    }

    if (jCountryOrRegion != nullptr) {
        env->DeleteLocalRef(jCountryOrRegion);
    }
}

void AceApplicationInfoImpl::SetLocale(const std::string& language, const std::string& countryOrRegion,
    const std::string& script, const std::string& keywordsAndValues)
{
    language_ = language;
    countryOrRegion_ = countryOrRegion;
    script_ = script;
    keywordsAndValues_ = keywordsAndValues;

    localeTag_ = language;
    if (!script_.empty()) {
        localeTag_.append("-" + script_);
    }

    if (!countryOrRegion_.empty()) {
        localeTag_.append("-" + countryOrRegion_);
    }

    icu::Locale locale(language_.c_str(), countryOrRegion_.c_str());
    isRightToLeft_ = locale.isRightToLeft();
    auto languageList = Localization::GetLanguageList(language_);
    if (languageList.size() == 1) {
        Localization::SetLocale(language_, countryOrRegion_, script_, languageList.front(), keywordsAndValues_);
    } else {
        auto selectLanguage = AceResConfig::GetLocaleFallback(localeTag_, languageList);
        Localization::SetLocale(language_, countryOrRegion_, script_, selectLanguage.front(), keywordsAndValues_);
    }
}

bool AceApplicationInfoImpl::GetBundleInfo(const std::string& packageName, AceBundleInfo& bundleInfo)
{
    return true;
}

AceApplicationInfoImpl& AceApplicationInfoImpl::GetInstance()
{
    static AceApplicationInfoImpl instance;
    return instance;
}

double AceApplicationInfoImpl::GetLifeTime() const
{
    return std::difftime(std::time(nullptr), initiateTimeStamp_);
}

} // namespace OHOS::Ace::Platform

namespace OHOS::Ace {

AceApplicationInfo& AceApplicationInfo::GetInstance()
{
    return Platform::AceApplicationInfoImpl::GetInstance();
}

} // namespace OHOS::Ace
