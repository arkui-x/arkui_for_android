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

#include "jni_stage_activity_delegate.h"

#include "app_main.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/utils/utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
bool JniStageActivityDelegate::Register(const std::shared_ptr<JNIEnv>& env)
{
    LOGI("JniStageActivityDelegate register start.");
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeDispatchOnCreate",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnCreate),
        },
        {
            .name = "nativeDispatchOnDestroy",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnDestroy),
        },
        {
            .name = "nativeDispatchOnForeground",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnForeground),
        },
        {
            .name = "nativeDispatchOnBackground",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnBackground),
        },
    };

    if (!env) {
        LOGE("JNI StageActivityDelegate: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/stage/ability/adapter/StageActivityDelegate");
    if (clazz == nullptr) {
        LOGE("JNI StageActivityDelegate: can't find java class StageActivityDelegate");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, Ace::ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void JniStageActivityDelegate::DispatchOnCreate(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("JNI DispatchOnCreate is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName != nullptr) {
        AppMain::GetInstance()->DispatchOnCreate(instanceName);
        env->ReleaseStringUTFChars(str, instanceName);
    }
}

void JniStageActivityDelegate::DispatchOnDestroy(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("JNI DispatchOnDestroy is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName != nullptr) {
        AppMain::GetInstance()->DispatchOnDestroy(instanceName);
        env->ReleaseStringUTFChars(str, instanceName);
    }
}

void JniStageActivityDelegate::DispatchOnForeground(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("JNI DispatchOnForeground is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName != nullptr) {
        AppMain::GetInstance()->DispatchOnForeground(instanceName);
        env->ReleaseStringUTFChars(str, instanceName);
    }
}

void JniStageActivityDelegate::DispatchOnBackground(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("JNI DispatchOnBackground is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName != nullptr) {
        AppMain::GetInstance()->DispatchOnBackground(instanceName);
        env->ReleaseStringUTFChars(str, instanceName);
    }
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS