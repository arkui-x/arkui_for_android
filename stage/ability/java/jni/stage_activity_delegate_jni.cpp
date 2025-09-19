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

#include "stage_activity_delegate_jni.h"

#include "ability_context_adapter.h"
#include "app_main.h"
#include "window_view_adapter.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/utils/utils.h"
#include "display_manager_agent_jni.h"

using namespace OHOS::Ace::Platform;
namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
StageActivityStruct StageActivityDelegateJni::stageActivityStruct_;

bool StageActivityDelegateJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    LOGI("StageActivityDelegateJni register start.");
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeAttachStageActivity",
            .signature = "(Ljava/lang/String;Lohos/stage/ability/adapter/StageActivity;)V",
            .fnPtr = reinterpret_cast<void*>(&AttachStageActivity),
        },
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
            .name = "nativeDispatchOnNewWant",
            .signature = "(Ljava/lang/String;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnNewWant),
        },
        {
            .name = "nativeSetWindowView",
            .signature = "(Ljava/lang/String;Lohos/ace/adapter/WindowViewInterface;)V",
            .fnPtr = reinterpret_cast<void*>(&SetWindowView),
        },
        {
            .name = "nativeCreateAbilityDelegator",
            .signature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&CreateAbilityDelegator),
        },
        {
            .name = "nativeDispatchOnAbilityResult",
            .signature = "(Ljava/lang/String;IILjava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&DispatchOnAbilityResult),
        },
        {
            .name = "nativeFoldStatusChangeCallback",
            .signature = "(Ljava/lang/String;I)V",
            .fnPtr = reinterpret_cast<void*>(&FoldStatusChangeCallback),
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

void StageActivityDelegateJni::AttachStageActivity(JNIEnv* env, jclass myclass, jstring jinstanceName, jobject object)
{
    if (env == nullptr) {
        LOGE("JNI StageActivityDelegate: null java env");
        return;
    }

    auto instanceName = env->GetStringUTFChars(jinstanceName, nullptr);
    if (instanceName == nullptr) {
        LOGE("instanceName is nullptr");
        return;
    }
    AbilityContextAdapter::GetInstance()->AddStageActivity(instanceName, object);
    env->ReleaseStringUTFChars(jinstanceName, instanceName);

    SetStageActivityStruct(env, object);
}

void StageActivityDelegateJni::DispatchOnCreate(JNIEnv* env, jclass myclass, jstring str, jstring params)
{
    LOGI("JNI DispatchOnCreate is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
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

void StageActivityDelegateJni::DispatchOnDestroy(JNIEnv* env, jclass myclass, jstring str)
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

    AbilityContextAdapter::GetInstance()->RemoveStageActivity(instanceName);
}

void StageActivityDelegateJni::DispatchOnForeground(JNIEnv* env, jclass myclass, jstring str)
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

void StageActivityDelegateJni::DispatchOnBackground(JNIEnv* env, jclass myclass, jstring str)
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

void StageActivityDelegateJni::DispatchOnNewWant(JNIEnv* env, jclass myclass, jstring str, jstring params)
{
    LOGI("JNI DispatchOnNewWant is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    auto parameters = env->GetStringUTFChars(params, nullptr);
    if (instanceName != nullptr && parameters != nullptr) {
        AppMain::GetInstance()->DispatchOnNewWant(instanceName, parameters);
        env->ReleaseStringUTFChars(str, instanceName);
        env->ReleaseStringUTFChars(params, parameters);
    }
}

void StageActivityDelegateJni::SetWindowView(JNIEnv* env, jclass myclass, jstring str, jobject jwindowView)
{
    LOGI("JNI SetWindowView is called.");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    auto instanceName = env->GetStringUTFChars(str, nullptr);
    if (instanceName == nullptr) {
        LOGE("instanceName is nullptr");
        return;
    }
    WindowViewAdapter::GetInstance()->AddWindowView(instanceName, jwindowView);
    env->ReleaseStringUTFChars(str, instanceName);
}

void StageActivityDelegateJni::CreateAbilityDelegator(JNIEnv* env, jclass myclass, jstring jbundleName,
    jstring jmoduleName, jstring jtestName, jstring jtimeout)
{
    auto bundleName = env->GetStringUTFChars(jbundleName, nullptr);
    auto moduleName = env->GetStringUTFChars(jmoduleName, nullptr);
    auto testName = env->GetStringUTFChars(jtestName, nullptr);
    auto timeout = env->GetStringUTFChars(jtimeout, nullptr);
    if (bundleName == nullptr) {
        LOGE("bundleName is nullptr");
        return;
    }
    if (moduleName == nullptr) {
        LOGE("moduleName is nullptr");
        return;
    }
    if (testName == nullptr) {
        LOGE("testName is nullptr");
        return;
    }
    AppMain::GetInstance()->PrepareAbilityDelegator(bundleName, moduleName, testName, timeout);
}

void StageActivityDelegateJni::DispatchOnAbilityResult(
    JNIEnv* env, jclass myclass, jstring str, jint requestCode, jint resultCode, jstring resultWantParams)
{
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    auto instanceName = env->GetStringUTFChars(str, nullptr);
    auto parameters = env->GetStringUTFChars(resultWantParams, nullptr);
    if (instanceName != nullptr && parameters != nullptr) {
        AppMain::GetInstance()->DispatchOnAbilityResult(instanceName, requestCode, resultCode, parameters);
        env->ReleaseStringUTFChars(str, instanceName);
        env->ReleaseStringUTFChars(resultWantParams, parameters);
    }
}

void StageActivityDelegateJni::SetStageActivityStruct(JNIEnv* env, jobject object)
{
    jclass clazz = env->GetObjectClass(object);
    stageActivityStruct_.object = env->NewGlobalRef(object);
    stageActivityStruct_.clazz = (jclass)env->NewGlobalRef(clazz);
    stageActivityStruct_.onFoldStatusChangeMethod = env->GetMethodID(clazz,
        "onFoldStatusChange", "()Ljava/lang/String;");
    stageActivityStruct_.offFoldStatusChangeMethod = env->GetMethodID(clazz, "offFoldStatusChange", "()V");
}

std::string StageActivityDelegateJni::OnFoldStatusChange()
{
    JNIEnv* env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("StageActivityDelegateJni:: env is NULL");
    }
    jstring instanceName = static_cast<jstring>(env->CallObjectMethod(stageActivityStruct_.object,
        stageActivityStruct_.onFoldStatusChangeMethod));
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    const char *instance = env->GetStringUTFChars(instanceName, nullptr);
    if (instance == nullptr) {
        return "";
    }
    std::string instanceStr(instance);
    env->ReleaseStringUTFChars(instanceName, instance);
    return instanceStr;
}

void StageActivityDelegateJni::OffFoldStatusChange()
{
    JNIEnv* env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("StageActivityDelegateJni:: env is NULL");
    }
    env->CallVoidMethod(stageActivityStruct_.object, stageActivityStruct_.offFoldStatusChangeMethod);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void StageActivityDelegateJni::FoldStatusChangeCallback(JNIEnv* env, jclass myclass,
                                                        jstring jinstanceName, jint foldStatus)
{
    auto instanceName = env->GetStringUTFChars(jinstanceName, nullptr);
    if (instanceName == nullptr) {
        LOGE("instanceName is nullptr");
        return;
    }
    DisplayManagerAgentJni::FoldStatusChangeCallback(instanceName, (int32_t)foldStatus);
    env->ReleaseStringUTFChars(jinstanceName, instanceName);
}

} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS