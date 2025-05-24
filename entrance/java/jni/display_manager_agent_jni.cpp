/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "display_manager_agent_jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "display_manager.h"

namespace OHOS::Ace::Platform {

static constexpr int32_t ADD = 0;
static constexpr int32_t REMOVE = 1;
static constexpr int32_t CHANGE = 2;

DisplayManagerAgentJniStruct DisplayManagerAgentJni::displayManagerAgentJniStruct_;

bool DisplayManagerAgentJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeSetupDisplayManagerAgent",
            .signature = "()V",
            .fnPtr = reinterpret_cast<void*>(&SetupDisplayManagerAgent),
        },
        {
            .name = "nativeOnDisplayAdded",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&OnDisplayAdded),
        },
        {
            .name = "nativeOnDisplayRemoved",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&OnDisplayRemoved),
        },
        {
            .name = "nativeOnDisplayChanged",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&OnDisplayChanged),
        }
    };

    if (!env) {
        LOGE("JNI Window: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/ace/adapter/DisplayManagerAgent");
    if (clazz == nullptr) {
        LOGE("JNI: can't find java class DisplayManagerAgent");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void DisplayManagerAgentJni::SetupDisplayManagerAgent(JNIEnv* env, jobject obj)
{
    LOGI("DisplayManagerAgentJni::SetupDisplayManagerAgent called");

    jclass clazz = env->GetObjectClass(obj);
    displayManagerAgentJniStruct_.object = env->NewGlobalRef(obj);
    displayManagerAgentJniStruct_.clazz = (jclass)env->NewGlobalRef(clazz);
    displayManagerAgentJniStruct_.isFoldableMethod = env->GetMethodID(clazz, "isFoldable", "()Z");
    displayManagerAgentJniStruct_.getFoldStatusMethod = env->GetMethodID(clazz, "getFoldStatus", "()I");
    displayManagerAgentJniStruct_.registerDisplayListenerMethod = env->GetMethodID(clazz, "registerDisplayListener",
        "()V");
    displayManagerAgentJniStruct_.unregisterDisplayListenerMethod = env->GetMethodID(clazz,
        "unregisterDisplayListener", "()V");
}

bool DisplayManagerAgentJni::IsFoldable()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayManagerAgentJni::IsFoldable env is NULL");
        return -1;
    }

    jboolean isFoldable = env->CallBooleanMethod(displayManagerAgentJniStruct_.object,
        displayManagerAgentJniStruct_.isFoldableMethod);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return (bool)isFoldable;
}

uint32_t DisplayManagerAgentJni::GetFoldStatus()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayManagerAgentJni::GetFoldStatus env is NULL");
        return -1;
    }

    jint foldStatus = env->CallIntMethod(displayManagerAgentJniStruct_.object,
        displayManagerAgentJniStruct_.getFoldStatusMethod);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return (uint32_t)foldStatus;
}

void DisplayManagerAgentJni::RegisterDisplayListener()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayManagerAgentJni::RegisterDisplayListener env is NULL");
    }
    env->CallVoidMethod(displayManagerAgentJniStruct_.object,
        displayManagerAgentJniStruct_.registerDisplayListenerMethod);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void DisplayManagerAgentJni::UnregisterDisplayListener()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayManagerAgentJni::UnregisterDisplayListener env is NULL");
    }
    env->CallVoidMethod(displayManagerAgentJniStruct_.object,
        displayManagerAgentJniStruct_.unregisterDisplayListenerMethod);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void DisplayManagerAgentJni::OnDisplayAdded(JNIEnv* env, jobject obj, jint displayId)
{
    Rosen::DisplayManager::GetInstance().NotifyDisplayListenerCallback(ADD, (uint64_t)displayId);
}

void DisplayManagerAgentJni::OnDisplayRemoved(JNIEnv* env, jobject obj, jint displayId)
{
    Rosen::DisplayManager::GetInstance().NotifyDisplayListenerCallback(REMOVE, (uint64_t)displayId);
}

void DisplayManagerAgentJni::OnDisplayChanged(JNIEnv* env, jobject obj, jint displayId)
{
    Rosen::DisplayManager::GetInstance().NotifyDisplayListenerCallback(CHANGE, (uint64_t)displayId);
}

void DisplayManagerAgentJni::FoldStatusChangeCallback(const std::string& instanceName, int32_t foldStatus)
{
    Rosen::DisplayManager::GetInstance().NotifyFoldStatusChangeCallback(instanceName, foldStatus);
}

} // namespace OHOS::Ace::Platform
