/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/ace_application_info_jni.h"

#include "unicode/locid.h"

#include "adapter/android/entrance/java/jni/ace_application_info_impl.h"
#include "base/i18n/localization.h"
#include "base/log/ace_trace.h"
#include "base/log/event_report.h"
#include "base/log/log.h"

namespace OHOS::Ace::Platform {

bool AceApplicationInfoJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeSetPackageInfo",
            .signature = "(Ljava/lang/String;IZZ)V",
            .fnPtr = reinterpret_cast<void*>(&NativeSetPackageInfo),
        },
        {
            .name = "nativeInitialize",
            .signature = "(Lohos/ace/adapter/AceApplicationInfo;)V",
            .fnPtr = reinterpret_cast<void*>(&NativeInitialize),
        },
        {
            .name = "nativeLocaleChanged",
            .signature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&NativeLocaleChanged),
        },
        {
            .name = "nativeSetUserId",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&NativeSetUserID),
        },
        {
            .name = "nativeSetProcessName",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&NativeSetProcessName),
        },
        {
            .name = "nativeSetupIcuRes",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&NativeSetupIcuRes),
        },
        {
            .name = "nativeSetAccessibilityEnabled",
            .signature = "(Z)V",
            .fnPtr = reinterpret_cast<void*>(&NativeSetAccessibilityEnabled),
        },
    };

    if (!env) {
        LOGE("JNIEnv is null when register Ace ApplicationInfo jni！");
        return false;
    }

    const jclass myClass = env->FindClass("ohos/ace/adapter/AceApplicationInfo");
    if (myClass == nullptr) {
        LOGE("Failed to find the AceApplicationInfo Class");
        return false;
    }

    return env->RegisterNatives(myClass, methods, ArraySize(methods)) == 0;
}

void AceApplicationInfoJni::NativeSetPackageInfo(
    JNIEnv* env, jclass myClass, jstring packageName, jint uid, jboolean isDebug, jboolean needDebugBreakPoint)
{
    if (!env) {
        LOGW("env is null");
        return;
    }
    auto pkgStr = env->GetStringUTFChars(packageName, nullptr);
    if (pkgStr != nullptr) {
        AceApplicationInfo::GetInstance().SetPackageName(pkgStr);
        env->ReleaseStringUTFChars(packageName, pkgStr);
    }
    AceApplicationInfo::GetInstance().SetUid(uid);
    AceApplicationInfoImpl::GetInstance().SetDebug(isDebug, needDebugBreakPoint);
}

void AceApplicationInfoJni::NativeLocaleChanged(
    JNIEnv* env, jclass myClass, jstring language, jstring countryOrRegion, jstring script, jstring keywordsAndValues)
{
    ACE_SCOPED_TRACE("nativeLocaleChanged");

    if (env == nullptr) {
        LOGW("env is nullptr");
        return;
    }

    auto countryOrRegionStr = env->GetStringUTFChars(countryOrRegion, nullptr);
    auto languageStr = env->GetStringUTFChars(language, nullptr);
    auto scriptStr = env->GetStringUTFChars(script, nullptr);
    auto keywordsAndValuesStr = env->GetStringUTFChars(keywordsAndValues, nullptr);

    if (countryOrRegionStr != nullptr && languageStr != nullptr && scriptStr != nullptr &&
        keywordsAndValuesStr != nullptr) {
        std::string countryOrRegionID(countryOrRegionStr);
        std::string languageID(languageStr);
        std::string scriptID(scriptStr);
        std::string keywordsAndValuesID(keywordsAndValuesStr);
        AceApplicationInfoImpl::GetInstance().SetLocale(languageID, countryOrRegionID, scriptID, keywordsAndValuesID);
    }

    if (countryOrRegionStr != nullptr) {
        env->ReleaseStringUTFChars(countryOrRegion, countryOrRegionStr);
    }

    if (languageStr != nullptr) {
        env->ReleaseStringUTFChars(language, languageStr);
    }

    if (scriptStr != nullptr) {
        env->ReleaseStringUTFChars(script, scriptStr);
    }

    if (keywordsAndValuesStr != nullptr) {
        env->ReleaseStringUTFChars(keywordsAndValues, keywordsAndValuesStr);
    }
}

void AceApplicationInfoJni::NativeInitialize(JNIEnv* env, jclass myClass, jobject object)
{
    AceApplicationInfoImpl::GetInstance().Initialize(env, object);
}

void AceApplicationInfoJni::NativeSetupIcuRes(JNIEnv* env, jclass myClass, jstring icuData)
{
    auto icuDataStr = env->GetStringUTFChars(icuData, nullptr);
    if (icuDataStr == nullptr) {
        return;
    }
    u_setDataDirectory(icuDataStr);
    env->ReleaseStringUTFChars(icuData, icuDataStr);
}

void AceApplicationInfoJni::NativeSetUserID(JNIEnv* env, jclass myClass, jint userId)
{
    AceApplicationInfoImpl::GetInstance().SetUserId(userId);
}

void AceApplicationInfoJni::NativeSetAccessibilityEnabled(JNIEnv* env, jclass myClass, jboolean enabled)
{
    AceApplicationInfoImpl::GetInstance().SetAccessibilityEnabled(enabled);
}

void AceApplicationInfoJni::NativeSetProcessName(JNIEnv* env, jclass myClass, jstring processName)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }

    auto processNameStr = env->GetStringUTFChars(processName, nullptr);
    if (processNameStr != nullptr) {
        AceApplicationInfoImpl::GetInstance().SetProcessName(processNameStr);
        env->ReleaseStringUTFChars(processName, processNameStr);
    }
}

} // namespace OHOS::Ace::Platform