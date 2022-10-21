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
#include "adapter/android/capability/java/jni/environment/environment_jni.h"

#include "adapter/android/capability/java/jni/environment/environment_proxy_impl.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/json/json_util.h"
#include "base/log/log.h"
#include "core/common/environment/environment_proxy.h"

namespace OHOS::Ace::Platform {
namespace {

static const char* const ENVIRONMENT_PLUGIN_CLASS_NAME = "ohos/ace/adapter/capability/environment/EnvironmentBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(EnvironmentJni::NativeInit) },
};

static const char* const METHOD_GET_ACCESSIBILITY_ENABLED = "getAccessibilityEnabled";

static const char* const SIGNATURE_GET_ACCESSIBILITY_ENABLED = "()Ljava/lang/String;";

// JniEnvironment is used in JVM, and EnvironmentJni is for plugin. Make sure use the correct name.
JniEnvironment::JavaGlobalRef g_jobject(nullptr, nullptr);

struct {
    jmethodID getAccessibilityEnabled;
} g_pluginClass;

} // namespace

bool EnvironmentJni::Register(std::shared_ptr<JNIEnv> env)
{
    if (!env) {
        LOGW("Environment JNI: env not ready");
        return false;
    }

    jclass cls = env->FindClass(ENVIRONMENT_PLUGIN_CLASS_NAME);
    if (!cls) {
        LOGW("Environment JNI: ENVIRONMENT_PLUGIN_CLASS_NAME is null");
        return false;
    }

    bool ret = env->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(cls);
    if (!ret) {
        LOGW("Environment JNI: RegisterNatives failed.");
        return false;
    }
    OnJniRegistered();
    return true;
}

void EnvironmentJni::OnJniRegistered()
{
    EnvironmentProxy::GetInstance()->SetDelegate(std::make_unique<EnvironmentProxyImpl>());
}

void EnvironmentJni::NativeInit(JNIEnv* env, jobject jobj)
{
    if (!env) {
        LOGW("Environment JNI: env not ready");
        return;
    }

    g_jobject = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj);

    // Find classes & methods in Java class.
    jclass cls = env->GetObjectClass(jobj);
    if (cls == nullptr) {
        LOGE("Environment JNI: get object class failed");
        return;
    }
    g_pluginClass.getAccessibilityEnabled =
        env->GetMethodID(cls, METHOD_GET_ACCESSIBILITY_ENABLED, SIGNATURE_GET_ACCESSIBILITY_ENABLED);
    if (!g_pluginClass.getAccessibilityEnabled) {
        LOGW("Environment JNI: method not found.");
    }
    env->DeleteLocalRef(cls);
}

// Called by C++
std::string EnvironmentJni::GetAccessibilityEnabled()
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Environment JNI: env not ready");
        return "";
    }

    if (!g_jobject || !g_pluginClass.getAccessibilityEnabled) {
        return "";
    }
    std::string result;
    jstring jResult =
        static_cast<jstring>(env->CallObjectMethod(g_jobject.get(), g_pluginClass.getAccessibilityEnabled));
    if (env->ExceptionCheck()) {
        LOGE("Environment JNI: call GetAccessibilityEnabled has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return result;
    }
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
    return result;
}

} // namespace OHOS::Ace::Platform
