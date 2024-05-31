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
#include "adapter/android/capability/java/jni/storage/storage_jni.h"

#include "adapter/android/capability/java/jni/storage/storage_proxy_impl.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/json/json_util.h"
#include "base/log/log.h"
#include "core/common/storage/storage_proxy.h"

namespace OHOS::Ace::Platform {
namespace {

static const char* const STORAGE_PLUGIN_CLASS_NAME = "ohos/ace/adapter/capability/storage/PersistentStorageBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(StorageJni::NativeInit) },
};

static const char* const METHOD_SET = "set";
static const char* const METHOD_GET = "get";
static const char* const METHOD_CLEAR = "clear";
static const char* const METHOD_DELETE = "delete";

static const char* const SIGNATURE_SET = "(Ljava/lang/String;Ljava/lang/String;)V";
static const char* const SIGNATURE_GET = "(Ljava/lang/String;)Ljava/lang/String;";
static const char* const SIGNATURE_CLEAR = "()V";
static const char* const SIGNATURE_DELETE = "(Ljava/lang/String;)V";

JniEnvironment::JavaGlobalRef g_jobject(nullptr, nullptr);

struct {
    jmethodID set;
    jmethodID get;
    jmethodID clear;
    jmethodID remove;
} g_pluginClass;

} // namespace

bool StorageJni::Register(std::shared_ptr<JNIEnv> env)
{
    if (!env) {
        LOGW("Storage JNI: env not ready");
        return false;
    }

    jclass cls = env->FindClass(STORAGE_PLUGIN_CLASS_NAME);
    if (!cls) {
        LOGW("Storage JNI: STORAGE_PLUGIN_CLASS_NAME is null");
        return false;
    }

    bool ret = env->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(cls);
    if (!ret) {
        LOGW("Storage JNI: RegisterNatives fail.");
        return false;
    }
    OnJniRegistered();
    return true;
}

void StorageJni::OnJniRegistered()
{
    StorageProxy::GetInstance()->SetDelegate(std::make_unique<StorageProxyImpl>());
}

void StorageJni::NativeInit(JNIEnv* env, jobject jobj)
{
    if (!env) {
        LOGW("Storage JNI: env not ready");
        return;
    }

    g_jobject = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj);

    // Find classes & methods in Java class.
    jclass cls = env->GetObjectClass(jobj);
    if (cls == nullptr) {
        LOGE("Storage JNI: get object class failed");
        return;
    }

    g_pluginClass.set = env->GetMethodID(cls, METHOD_SET, SIGNATURE_SET);
    if (!g_pluginClass.set) {
        LOGW("Storage JNI: setData method not found.");
    }

    g_pluginClass.get = env->GetMethodID(cls, METHOD_GET, SIGNATURE_GET);
    if (!g_pluginClass.get) {
        LOGW("Storage JNI: getData method not found.");
    }

    g_pluginClass.clear = env->GetMethodID(cls, METHOD_CLEAR, SIGNATURE_CLEAR);
    if (!g_pluginClass.clear) {
        LOGW("Storage JNI: clear method not found.");
    }

    g_pluginClass.remove = env->GetMethodID(cls, METHOD_DELETE, SIGNATURE_DELETE);
    if (!g_pluginClass.remove) {
        LOGW("Storage JNI: clear method not found.");
    }

    env->DeleteLocalRef(cls);
}

// Called by C++
void StorageJni::Set(const std::string& key, const std::string& value)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Storage JNI: env not ready");
        return;
    }
    if (!g_jobject || !g_pluginClass.set) {
        return;
    }
    jstring jdata = env->NewStringUTF(key.c_str());
    jstring jvalue = env->NewStringUTF(value.c_str());
    env->CallVoidMethod(g_jobject.get(), g_pluginClass.set, jdata, jvalue);
    if (jdata != nullptr) {
        env->DeleteLocalRef(jdata);
    }
    if (jvalue != nullptr) {
        env->DeleteLocalRef(jvalue);
    }
    if (env->ExceptionCheck()) {
        LOGE("Storage JNI: call Set has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    return;
}

std::string StorageJni::Get(const std::string& data)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Storage JNI: env not ready");
        return "";
    }

    if (!g_jobject || !g_pluginClass.get) {
        return "";
    }

    jstring jdata = env->NewStringUTF(data.c_str());
    std::string result;
    jstring jresult = static_cast<jstring>(env->CallObjectMethod(g_jobject.get(), g_pluginClass.get, jdata));
    if (env->ExceptionCheck()) {
        LOGE("Storage JNI: call Set has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return result;
    }
    const char* content = env->GetStringUTFChars(jresult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jresult, content);
    }

    if (jdata != nullptr) {
        env->DeleteLocalRef(jdata);
    }
    if (jresult != nullptr) {
        env->DeleteLocalRef(jresult);
    }
    return result;
}

void StorageJni::Clear()
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Storage JNI: env not ready");
        return;
    }

    if (!g_jobject || !g_pluginClass.clear) {
        return;
    }

    env->CallVoidMethod(g_jobject.get(), g_pluginClass.clear);
    if (env->ExceptionCheck()) {
        LOGE("Storage JNI: call Clear has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void StorageJni::Delete(const std::string& key)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Storage JNI: env not ready");
        return;
    }

    if (!g_jobject || !g_pluginClass.remove) {
        return;
    }

    jstring jdata = env->NewStringUTF(key.c_str());
    env->CallVoidMethod(g_jobject.get(), g_pluginClass.remove, jdata);
    if (env->ExceptionCheck()) {
        LOGE("Storage JNI: call Delete has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

} // namespace OHOS::Ace::Platform
