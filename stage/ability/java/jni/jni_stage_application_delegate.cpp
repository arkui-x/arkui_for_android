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

#include "jni_stage_application_delegate.h"

#include "app_main.h"
#include "foundation/arkui/ace_engine/adapter/android/entrance/java/jni/apk_asset_provider.h"
#include "stage_asset_provider.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
bool JniStageApplicationDelegate::Register(const std::shared_ptr<JNIEnv>& env)
{
    LOGI("JniStageApplicationDelegate register start.");
    static const JNINativeMethod methods[] = {
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
            .name = "nativeLaunchApplication",
            .signature = "()V",
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
    };

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

void JniStageApplicationDelegate::SetNativeAssetManager(JNIEnv* env, jclass myclass, jobject assetManager)
{
    LOGI("Set native asset manager.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    StageAssetProvider::GetInstance()->SetAssetManager(env, assetManager);
}

void JniStageApplicationDelegate::SetHapPath(JNIEnv* env, jclass myclass, jstring str)
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

void JniStageApplicationDelegate::SetAssetsFileRelativePath(JNIEnv* env, jclass myclass, jstring str)
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

void JniStageApplicationDelegate::LaunchApplication(JNIEnv* env, jclass clazz)
{
    LOGI("Launch application");
    AppMain::GetInstance()->LaunchApplication();
}

void JniStageApplicationDelegate::SetCacheDir(JNIEnv* env, jclass myclass, jstring str)
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

void JniStageApplicationDelegate::SetFileDir(JNIEnv* env, jclass myclass, jstring str)
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
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS