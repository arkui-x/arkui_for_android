/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "stage_fragment_delegate_jni.h"

#include "ability_context_adapter.h"
#include "app_main.h"
#include "window_view_adapter.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/utils/utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
bool StageFragmentDelegateJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    LOGI("StageFragmentDelegateJni register start.");
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeAttachFragment",
            .signature = "(Ljava/lang/String;Lohos/stage/ability/adapter/StageFragment;)V",
            .fnPtr = reinterpret_cast<void*>(&AttachFragment),
        },
        {
            .name = "nativeDispatchOnCreate",
            .signature = "(Ljava/lang/String;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnCreate),
        },
        {
            .name = "nativeSetWindowView",
            .signature = "(Ljava/lang/String;Lohos/ace/adapter/WindowViewInterface;)V",
            .fnPtr = reinterpret_cast<void*>(&SetWindowView),
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
        {
            .name = "nativeDispatchOnDestroy",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnDestroy),
        },
    };
    if (env == NULL) {
        LOGE("Env is null.");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/stage/ability/adapter/StageFragmentDelegate");
    if (clazz == nullptr) {
        LOGE("Can't find java class StageFragmentDelegate.");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, Ace::ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void StageFragmentDelegateJni::AttachFragment(JNIEnv* env, jclass myclass, jstring jinstanceName, jobject object)
{
    if (env == nullptr) {
        LOGE("JNI StageFragmentDelegate: null java env");
        return;
    }

    auto instanceName = env->GetStringUTFChars(jinstanceName, nullptr);
    if (instanceName == nullptr) {
        LOGE("instanceName is nullptr");
        return;
    }
    AbilityContextAdapter::GetInstance()->AddStageActivity(instanceName, object);
    env->ReleaseStringUTFChars(jinstanceName, instanceName);
}

void StageFragmentDelegateJni::SetWindowView(JNIEnv* env, jclass myclass, jstring str, jobject jwindowView)
{
    LOGI("JNI StageFragmentDelegate: SetWindowView is called.");
    if (env == nullptr) {
        LOGE("Env is nullptr.");
        return;
    }

    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName == nullptr) {
        LOGE("InstanceName is nullptr.");
        return;
    }
    WindowViewAdapter::GetInstance()->AddWindowView(instanceName, jwindowView);
    env->ReleaseStringUTFChars(str, instanceName);
}

void StageFragmentDelegateJni::DispatchOnCreate(JNIEnv* env, jclass myclass, jstring str, jstring params)
{
    LOGI("JNI StageFragmentDelegate: DispatchOnCreate is called.");
    if (env == nullptr) {
        LOGE("Env is nullptr.");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    auto parameters = env->GetStringUTFChars(params, nullptr);
    if (instanceName != nullptr && parameters != nullptr) {
        AppMain::GetInstance()->DispatchOnCreate(instanceName, parameters);
        env->ReleaseStringUTFChars(str, instanceName);
        env->ReleaseStringUTFChars(params, parameters);
    }
}

void StageFragmentDelegateJni::DispatchOnForeground(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("JNI StageFragmentDelegate: DispatchOnForeground is called.");
    if (env == nullptr) {
        LOGE("Env is nullptr.");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName != nullptr) {
        AppMain::GetInstance()->DispatchOnForeground(instanceName);
        env->ReleaseStringUTFChars(str, instanceName);
    }
}

void StageFragmentDelegateJni::DispatchOnBackground(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("JNI StageFragmentDelegate: DispatchOnBackground is called.");
    if (env == nullptr) {
        LOGE("Env is nullptr.");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName != nullptr) {
        AppMain::GetInstance()->DispatchOnBackground(instanceName);
        env->ReleaseStringUTFChars(str, instanceName);
    }
}

void StageFragmentDelegateJni::DispatchOnDestroy(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("JNI StageFragmentDelegate: DispatchOnDestroy is called.");
    if (env == nullptr) {
        LOGE("Env is nullptr.");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName != nullptr) {
        AppMain::GetInstance()->DispatchOnDestroy(instanceName);
        env->ReleaseStringUTFChars(str, instanceName);
    }
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS