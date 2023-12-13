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

#include "adapter/android/capability/java/jni/plugin/plugin_manager_jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
namespace {

const char PLUGIN_MANAGER_PLUGIN_CLASS_NAME[] = "ohos/ace/adapter/capability/plugin/PluginManager";

const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(PluginManagerJni::NativeInit) },
    { "nativeRegister", "(JLjava/lang/String;)V", reinterpret_cast<void*>(PluginManagerJni::NativeRegister) },
};

const char METHOD_INIT_PLUGIN[] = "initPlugin";
const char METHOD_REGISTER_PLUGIN[] = "registerPlugin";

const char SIGNATURE_INIT_PLUGIN[] = "(Ljava/lang/String;)V";
const char SIGNATURE_REGISTER_PLUGIN[] = "(JLjava/lang/String;)V";

JniEnvironment::JavaGlobalRef g_jobject(nullptr, nullptr);

struct {
    jmethodID initPlugin;
    jmethodID registerPlugin;
} g_pluginClass;

} // namespace

bool PluginManagerJni::Register(std::shared_ptr<JNIEnv> env)
{
    CHECK_NULL_RETURN(env, false);
    jclass cls = env->FindClass(PLUGIN_MANAGER_PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);

    bool ret = env->RegisterNatives(cls, METHODS, ArraySize(METHODS)) == 0;
    env->DeleteLocalRef(cls);
    if (!ret) {
        LOGW("PluginManager JNI: RegisterNatives failed.");
        return false;
    }
    return true;
}

void PluginManagerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_jobject = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj);

    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.initPlugin = env->GetMethodID(cls, METHOD_INIT_PLUGIN, SIGNATURE_INIT_PLUGIN);
    CHECK_NULL_VOID(g_pluginClass.initPlugin);

    g_pluginClass.registerPlugin = env->GetMethodID(cls, METHOD_REGISTER_PLUGIN, SIGNATURE_REGISTER_PLUGIN);
    CHECK_NULL_VOID(g_pluginClass.registerPlugin);

    env->DeleteLocalRef(cls);
}

void PluginManagerJni::NativeRegister(JNIEnv* env, jobject jobj, jlong jPluginRegisterFunc, jstring jPluginPackageName)
{
    CHECK_NULL_VOID(env);
    bool status = reinterpret_cast<RegisterCallback>(jPluginRegisterFunc)(env);
    if (!status) {
        LOGW("PluginManager NativeRegister JNI: register failed");
        return;
    }

    const char* pluginPackageNameStr = env->GetStringUTFChars(jPluginPackageName, nullptr);
    if (pluginPackageNameStr != nullptr) {
        auto pluginPackageName = std::string(pluginPackageNameStr);
        InitPlugin(pluginPackageName);
    }
    env->ReleaseStringUTFChars(jPluginPackageName, pluginPackageNameStr);
}

void PluginManagerJni::RegisterPlugin(RegisterCallback pluginFunc, const std::string& pluginPackageName)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_jobject);
    CHECK_NULL_VOID(g_pluginClass.registerPlugin);

    jlong jPluginFunc = reinterpret_cast<int64_t>(pluginFunc);
    jstring jPluginPackageName = env->NewStringUTF(pluginPackageName.c_str());
    env->CallVoidMethod(g_jobject.get(), g_pluginClass.registerPlugin, jPluginFunc, jPluginPackageName);

    if (env->ExceptionCheck()) {
        LOGE("PluginManager JNI: registerPlugin failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    env->DeleteLocalRef(jPluginPackageName);
}

void PluginManagerJni::InitPlugin(const std::string& packageName)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_jobject);
    CHECK_NULL_VOID(g_pluginClass.initPlugin);

    jstring jParams = env->NewStringUTF(packageName.c_str());
    env->CallVoidMethod(g_jobject.get(), g_pluginClass.initPlugin, jParams);

    if (env->ExceptionCheck()) {
        LOGE("PluginManager JNI: initPlugin failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    env->DeleteLocalRef(jParams);
}

} // namespace OHOS::Ace::Platform
