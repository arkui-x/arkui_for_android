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
#include "adapter/android/capability/java/jni/vibrator/vibrator_jni.h"

#include "adapter/android/capability/java/jni/vibrator/vibrator_proxy_impl.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/json/json_util.h"
#include "base/log/log.h"
#include "core/common/vibrator/vibrator_proxy.h"

namespace OHOS::Ace::Platform {
namespace {

static const char* const VIBRATOR_PLUGIN_CLASS_NAME = "ohos/ace/adapter/capability/vibrator/VibratorPluginBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(VibratorJni::NativeInit) },
};

static const char* const METHOD_VIBRATE = "vibrate";

static const char* const SIGNATURE_VIBRATE = "(I)V";

static const char* const SPECIFIED_VIBRATE = "(Ljava/lang/String;)V";

JniEnvironment::JavaGlobalRef g_jobject(nullptr, nullptr);

struct {
    jmethodID vibrate;
    jmethodID specifiedVibrate;
} g_pluginClass;

} // namespace

bool VibratorJni::Register(std::shared_ptr<JNIEnv> env)
{
    if (!env) {
        LOGW("Vibrator JNI: env not ready");
        return false;
    }

    jclass cls = env->FindClass(VIBRATOR_PLUGIN_CLASS_NAME);
    if (!cls) {
        LOGW("Vibrator JNI: VIBRATOR_PLUGIN_CLASS_NAME is null");
        return false;
    }

    bool ret = env->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(cls);
    if (!ret) {
        LOGW("Vibrator JNI: RegisterNatives failed.");
        return false;
    }
    OnJniRegistered();
    return true;
}

void VibratorJni::OnJniRegistered()
{
    VibratorProxy::GetInstance().SetDelegate(std::make_unique<VibratorProxyImpl>());
}

void VibratorJni::NativeInit(JNIEnv* env, jobject jobj)
{
    if (!env) {
        LOGW("Vibrator JNI: env not ready");
        return;
    }

    g_jobject = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj);

    // Find classes & methods in Java class.
    jclass cls = env->GetObjectClass(jobj);
    if (cls == nullptr) {
        LOGE("Vibrator JNI: get object class failed");
        return;
    }

    g_pluginClass.vibrate = env->GetMethodID(cls, METHOD_VIBRATE, SIGNATURE_VIBRATE);
    if (!g_pluginClass.vibrate) {
        LOGW("Vibrator JNI: vibrate method not found.");
    }

    g_pluginClass.specifiedVibrate = env->GetMethodID(cls, METHOD_VIBRATE, SPECIFIED_VIBRATE);
    if (!g_pluginClass.specifiedVibrate) {
        LOGW("Vibrator JNI: vibrate method not found.");
    }

    env->DeleteLocalRef(cls);
}

// Called by C++
void VibratorJni::Vibrate(int32_t duration)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Vibrator JNI: env not ready");
        return;
    }

    if (!g_jobject || !g_pluginClass.vibrate) {
        LOGE("Vibrator JNI: null return.");
        return;
    }

    env->CallVoidMethod(g_jobject.get(), g_pluginClass.vibrate, duration);
    if (env->ExceptionCheck()) {
        LOGE("Vibrator JNI: occurred in Vibrate");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

// Called by C++
void VibratorJni::Vibrate(const std::string& effectId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Vibrator JNI: env not ready");
        return;
    }

    if (!g_jobject || !g_pluginClass.specifiedVibrate) {
        LOGE("Vibrator JNI: null return.");
        return;
    }

    jstring jParams = env->NewStringUTF(effectId.c_str());
    env->CallVoidMethod(g_jobject.get(), g_pluginClass.specifiedVibrate, jParams);

    if (env->ExceptionCheck()) {
        LOGE("Vibrator JNI: occurred in Vibrate");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    env->DeleteLocalRef(jParams);
}

} // namespace OHOS::Ace::Platform
