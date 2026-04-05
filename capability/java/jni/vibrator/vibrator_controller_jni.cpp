/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "adapter/android/capability/java/jni/vibrator/vibrator_controller_jni.h"

#include "adapter/android/capability/java/jni/vibrator/vibrator_proxy_impl.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/json/json_util.h"
#include "base/log/log.h"
#include "core/common/vibrator/vibrator_proxy.h"
#include "ui/base/utils/utils.h"

namespace OHOS::Ace::Platform {
namespace {
static const char* const VIBRATOR_PLUGIN_CLASS_NAME = "ohos/ace/adapter/capability/vibrator/VibratorPluginBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(VibratorControllerJni::NativeInit) },
};

static const char* const METHOD_VIBRATE = "vibrate";

static const char* const SIGNATURE_VIBRATE = "(I)V";

static const char* const SPECIFIED_VIBRATE = "(Ljava/lang/String;)V";

static const char* const INTENSITY_VIBRATE = "(Ljava/lang/String;F)V";

JniEnvironment::JavaGlobalRef g_jobject(nullptr, nullptr);

struct {
    jmethodID vibrate;
    jmethodID specifiedVibrate;
    jmethodID intensityVibrate;
} g_pluginClass;
} // namespace

bool VibratorControllerJni::Register(std::shared_ptr<JNIEnv> env)
{
    CHECK_NULL_RETURN(env, false);

    jclass cls = env->FindClass(VIBRATOR_PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);

    bool ret = env->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(cls);
    OnJniRegistered();
    return ret;
}

void VibratorControllerJni::OnJniRegistered()
{
    VibratorProxy::GetInstance().SetDelegate(std::make_unique<VibratorProxyImpl>());
}

void VibratorControllerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(jobj);

    g_jobject = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.vibrate = env->GetMethodID(cls, METHOD_VIBRATE, SIGNATURE_VIBRATE);
    g_pluginClass.specifiedVibrate = env->GetMethodID(cls, METHOD_VIBRATE, SPECIFIED_VIBRATE);
    g_pluginClass.intensityVibrate = env->GetMethodID(cls, METHOD_VIBRATE, INTENSITY_VIBRATE);
    env->DeleteLocalRef(cls);
}

void VibratorControllerJni::Vibrate(int32_t duration)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_jobject);
    CHECK_NULL_VOID(g_pluginClass.vibrate);

    env->CallVoidMethod(g_jobject.get(), g_pluginClass.vibrate, duration);
    if (env->ExceptionCheck()) {
        LOGE("VibratorController JNI: occurred in Vibrate");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void VibratorControllerJni::Vibrate(const std::string& effectId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_jobject);
    CHECK_NULL_VOID(g_pluginClass.specifiedVibrate);

    if (effectId.empty()) {
        LOGE("VibratorController JNI: effectId is empty, cannot vibrate");
        return;
    }

    jstring jParams = env->NewStringUTF(effectId.c_str());
    CHECK_NULL_VOID(jParams);

    env->CallVoidMethod(g_jobject.get(), g_pluginClass.specifiedVibrate, jParams);
    env->DeleteLocalRef(jParams);

    if (env->ExceptionCheck()) {
        LOGE("VibratorController JNI: occurred in Vibrate");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void VibratorControllerJni::Vibrate(const std::string& effectId, float intensity)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_jobject);
    CHECK_NULL_VOID(g_pluginClass.intensityVibrate);

    if (effectId.empty()) {
        LOGE("VibratorController JNI: effectId is empty, cannot vibrate with intensity");
        return;
    }

    jstring jParams = env->NewStringUTF(effectId.c_str());
    CHECK_NULL_VOID(jParams);

    env->CallVoidMethod(g_jobject.get(), g_pluginClass.intensityVibrate, jParams, intensity);
    env->DeleteLocalRef(jParams);
    if (env->ExceptionCheck()) {
        LOGE("VibratorController JNI: occurred in Vibrate with intensity");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
} // namespace OHOS::Ace::Platform
