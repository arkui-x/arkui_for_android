/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "ability_loader_jni.h"

#include "app_main.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/utils/utils.h"
#include "base/log/log.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {

bool AbilityLoaderJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    LOGI("AbilityLoaderJni register start.");
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeDispatchOnCreate",
            .signature = "(Ljava/lang/String;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnCreate),
        },
        {
            .name = "nativeDispatchOnDestroy",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnDestroy),
        },
    };
    if (!env) {
        LOGE("JNI AbilityLoader: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/stage/ability/adapter/AbilityLoader");
    if (clazz == nullptr) {
        LOGE("JNI AbilityLoader: can't find java class AbilityLoader");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, Ace::ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void AbilityLoaderJni::DispatchOnCreate(JNIEnv* env, jclass myclass, jstring instanceName, jstring params)
{
    LOGI("AbilityLoaderJni DispatchOnCreate is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto name = env->GetStringUTFChars(instanceName, nullptr);
    if (name == nullptr) {
        LOGE("instanceName is nullptr");
        return;
    }
    auto parameters = env->GetStringUTFChars(params, nullptr);
    if (parameters == nullptr) {
        LOGE("params is nullptr");
        env->ReleaseStringUTFChars(instanceName, name);
        return;
    }
    AppMain::GetInstance()->DispatchOnCreate(name, parameters);
    env->ReleaseStringUTFChars(instanceName, name);
    env->ReleaseStringUTFChars(params, parameters);
}

void AbilityLoaderJni::DispatchOnDestroy(JNIEnv* env, jclass myclass, jstring instanceName)
{
    LOGI("AbilityLoaderJni DispatchOnDestroy is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto name = env->GetStringUTFChars(instanceName, nullptr);
    if (name == nullptr) {
        LOGE("instanceName is nullptr");
        return;
    }
    AppMain::GetInstance()->DispatchOnDestroy(name);
    env->ReleaseStringUTFChars(instanceName, name);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
