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

#include "stage_application_delegate_jni.h"

#include "ability_context_adapter.h"
#include "app_main.h"
#include "application_context_adapter.h"
#include "foundation/arkui/ace_engine/adapter/android/entrance/java/jni/apk_asset_provider.h"
#include "stage_application_info_adapter.h"
#include "stage_asset_provider.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
bool StageApplicationDelegateJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    LOGI("StageApplicationDelegateJni register start.");
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeAttachStageApplicationDelegate",
            .signature = "(Lohos/stage/ability/adapter/StageApplicationDelegate;)V",
            .fnPtr = reinterpret_cast<void*>(&AttachStageApplicationDelegate),
        },
        {
            .name = "nativeSetAssetManager",
            .signature = "(Ljava/lang/Object;)V",
            .fnPtr = reinterpret_cast<void*>(&SetNativeAssetManager),
        },
        {
            .name = "nativeSetHapPath",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetHapPath),
        },
        {
            .name = "nativeSetPackageName",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetPackageName),
        },
        {
            .name = "nativeLaunchApplication",
            .signature = "(Z)V",
            .fnPtr = reinterpret_cast<void*>(&LaunchApplication),
        },
        {
            .name = "nativeSetAssetsFileRelativePath",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetAssetsFileRelativePath),
        },
        {
            .name = "nativeSetCacheDir",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetCacheDir),
        },
        {
            .name = "nativeSetFileDir",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetFileDir),
        },
        {
            .name = "nativeSetAppLibDir",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetAppLibDir),
        },
        {
            .name = "nativeSetResourcesFilePrefixPath",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetResourcesFilePrefixPath),
        },
        {
            .name = "nativeSetPidAndUid",
            .signature = "(II)V",
            .fnPtr = reinterpret_cast<void*>(&SetPidAndUid),
        },
        {
            .name = "nativeInitConfiguration",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&InitConfiguration),
        },
        {
            .name = "nativeOnConfigurationChanged",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&OnConfigurationChanged),
        },
        {
            .name = "nativeSetLocale",
            .signature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetLocale),
        } };

    if (!env) {
        LOGE("JNI StageApplicationDelegate: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/stage/ability/adapter/StageApplicationDelegate");
    if (clazz == nullptr) {
        LOGE("JNI StageApplicationDelegate: can't find java class StageApplicationDelegate");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, Ace::ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void StageApplicationDelegateJni::SetNativeAssetManager(JNIEnv* env, jclass myclass, jobject assetManager)
{
    LOGI("Set native asset manager.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    StageAssetProvider::GetInstance()->SetAssetManager(env, assetManager);
}

void StageApplicationDelegateJni::SetHapPath(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("Set hap path");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto hapPath = env->GetStringUTFChars(str, nullptr);
    if (hapPath != nullptr) {
        StageAssetProvider::GetInstance()->SetAppPath(hapPath);
        env->ReleaseStringUTFChars(str, hapPath);
    }
}

void StageApplicationDelegateJni::SetPackageName(JNIEnv* env, jclass myclass, jstring packageName)
{
    LOGI("Set package name");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto platformBundleName = env->GetStringUTFChars(packageName, nullptr);
    if (platformBundleName != nullptr) {
        AbilityContextAdapter::GetInstance()->SetPlatformBundleName(platformBundleName);
        env->ReleaseStringUTFChars(packageName, platformBundleName);
    }
}

void StageApplicationDelegateJni::SetAssetsFileRelativePath(JNIEnv* env, jclass myclass, jstring str)
{
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto path = env->GetStringUTFChars(str, nullptr);
    if (path != nullptr) {
        StageAssetProvider::GetInstance()->SetAssetsFileRelativePaths(path);
        env->ReleaseStringUTFChars(str, path);
    }
}

void StageApplicationDelegateJni::LaunchApplication(JNIEnv* env, jclass clazz, jboolean isCopyNativeLibs)
{
    LOGI("Launch application");
    AppMain::GetInstance()->LaunchApplication(isCopyNativeLibs);
}

void StageApplicationDelegateJni::SetCacheDir(JNIEnv* env, jclass myclass, jstring str)
{
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto cacheDir = env->GetStringUTFChars(str, nullptr);
    if (cacheDir != nullptr) {
        StageAssetProvider::GetInstance()->SetCacheDir(cacheDir);
        env->ReleaseStringUTFChars(str, cacheDir);
    }
}

void StageApplicationDelegateJni::SetFileDir(JNIEnv* env, jclass myclass, jstring str)
{
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto filesDir = env->GetStringUTFChars(str, nullptr);
    if (filesDir != nullptr) {
        StageAssetProvider::GetInstance()->SetFileDir(filesDir);
        env->ReleaseStringUTFChars(str, filesDir);
    }
}

void StageApplicationDelegateJni::SetAppLibDir(JNIEnv* env, jclass myclass, jstring str)
{
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto filesDir = env->GetStringUTFChars(str, nullptr);
    if (filesDir != nullptr) {
        StageAssetProvider::GetInstance()->SetAppLibDir(filesDir);
        env->ReleaseStringUTFChars(str, filesDir);
    }
}

void StageApplicationDelegateJni::SetResourcesFilePrefixPath(JNIEnv* env, jclass myclass, jstring str)
{
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto path = env->GetStringUTFChars(str, nullptr);
    if (path != nullptr) {
        StageAssetProvider::GetInstance()->SetResourcesFilePrefixPath(path);
        env->ReleaseStringUTFChars(str, path);
    }
}

void StageApplicationDelegateJni::SetPidAndUid(JNIEnv* env, jclass myclass, jint pid, jint uid)
{
    AppMain::GetInstance()->SetPidAndUid(pid, uid);
}

void StageApplicationDelegateJni::InitConfiguration(JNIEnv* env, jclass myclass, jstring data)
{
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto jsonConfiguration = env->GetStringUTFChars(data, nullptr);
    if (jsonConfiguration != nullptr) {
        AppMain::GetInstance()->InitConfiguration(jsonConfiguration);
        env->ReleaseStringUTFChars(data, jsonConfiguration);
    }
}

void StageApplicationDelegateJni::OnConfigurationChanged(JNIEnv* env, jclass myclass, jstring data)
{
    LOGI("JNI OnConfigurationChanged is called.");
    if (env == nullptr) {
        LOGE("OnConfigurationChanged env is nullptr");
        return;
    }
    auto jsonConfiguration = env->GetStringUTFChars(data, nullptr);
    if (jsonConfiguration == nullptr) {
        LOGE("OnConfigurationChanged jsonConfiguration is nullptr");
        return;
    }
    AppMain::GetInstance()->OnConfigurationUpdate(jsonConfiguration);
    env->ReleaseStringUTFChars(data, jsonConfiguration);
}

void StageApplicationDelegateJni::SetLocale(
    JNIEnv* env, jclass myclass, jstring jlanguage, jstring jcountry, jstring jscript)
{
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    auto language = env->GetStringUTFChars(jlanguage, nullptr);
    if (language == nullptr) {
        LOGE("language is nullptr");
        return;
    }

    auto country = env->GetStringUTFChars(jcountry, nullptr);
    if (country == nullptr) {
        LOGE("country is nullptr");
        return;
    }

    auto script = env->GetStringUTFChars(jscript, nullptr);
    if (script == nullptr) {
        LOGE("script is nullptr");
        return;
    }

    StageApplicationInfoAdapter::GetInstance()->SetLocale(language, country, script);
}

void StageApplicationDelegateJni::AttachStageApplicationDelegate(JNIEnv* env, jclass myclass, jobject object)
{
    LOGI("Attach Stage Application.");
    if (env == nullptr) {
        LOGE("JNI JniStageApplicationDelegate: null java env");
        return;
    }
    ApplicationContextAdapter::GetInstance()->SetStageApplicationDelegate(object);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS