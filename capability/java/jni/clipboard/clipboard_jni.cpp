/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "adapter/android/capability/java/jni/clipboard/clipboard_jni.h"

#include "adapter/android/capability/java/jni/clipboard/clipboard_proxy_impl.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/json/json_util.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "core/common/clipboard/clipboard_proxy.h"

namespace OHOS::Ace::Platform {

namespace {

static const char CLIPBOARD_PLUGIN_CLASS_NAME[] = "ohos/ace/adapter/capability/clipboard/ClipboardPluginBase";

static const JNINativeMethod METHODS[] = {
    { .name = "nativeInit", .signature = "()V", .fnPtr = reinterpret_cast<void*>(ClipboardJni::NativeInit) },
};

static const char METHOD_SET_DATA[] = "setData";
static const char METHOD_GET_DATA[] = "getData";
static const char METHOD_HAS_DATA[] = "hasData";
static const char METHOD_CLEAR[] = "clear";
static const char METHOD_IS_MULTI_TYPE_DATA[] = "isMultiTypeData";
static const char METHOD_SET_MULTI_TYPE_DATA[] = "setMultiTypeData";
static const char METHOD_GET_MULTI_TYPE_DATA[] = "getMultiTypeData";

static const char SIGNATURE_SET_DATA[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_GET_DATA[] = "()Ljava/lang/String;";
static const char SIGNATURE_HAS_DATA[] = "()Z";
static const char SIGNATURE_CLEAR[] = "()V";
static const char SIGNATURE_IS_MULTI_TYPE_DATA[] = "()Z";
static const char SIGNATURE_SET_MULTI_TYPE_DATA[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_GET_MULTI_TYPE_DATA[] = "()Ljava/lang/String;";

JniEnvironment::JavaGlobalRef g_clipboardObj(nullptr, nullptr);

struct {
    jmethodID setData;
    jmethodID getData;
    jmethodID hasData;
    jmethodID clear;
    jmethodID isMultiTypeData;
    jmethodID setMultiTypeData;
    jmethodID getMultiTypeData;
} g_pluginMethods;

} // namespace

bool ClipboardJni::Register(std::shared_ptr<JNIEnv> env)
{
    if (!env) {
        LOGW("Clipborad JNI: null env");
        return false;
    }
    jclass clazz = env->FindClass(CLIPBOARD_PLUGIN_CLASS_NAME);
    if (clazz == nullptr) {
        LOGW("Clipborad JNI: class not found");
        return false;
    }

    bool ret = env->RegisterNatives(clazz, METHODS, ArraySize(METHODS)) == 0;
    env->DeleteLocalRef(clazz);

    if (!ret) {
        LOGE("Clipborad JNI: register failed");
        return false;
    }

    OnJniRegistered();
    return true;
}

void ClipboardJni::OnJniRegistered()
{
    ClipboardProxy::GetInstance()->SetDelegate(std::make_unique<ClipboardProxyImpl>());
}

void ClipboardJni::NativeInit(JNIEnv* env, jobject object)
{
    if (!env) {
        LOGW("Clipborad JNI Init: null env");
        return;
    }
    g_clipboardObj = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), object);
    jclass clazz = env->GetObjectClass(object);
    if (clazz == nullptr) {
        LOGE("Clipborad JNI Init: class not found");
        return;
    }

    g_pluginMethods.setData = env->GetMethodID(clazz, METHOD_SET_DATA, SIGNATURE_SET_DATA);
    if (!g_pluginMethods.setData) {
        LOGW("Clipborad JNI Init: setData method not found");
    }

    g_pluginMethods.getData = env->GetMethodID(clazz, METHOD_GET_DATA, SIGNATURE_GET_DATA);
    if (!g_pluginMethods.getData) {
        LOGW("Clipborad JNI Init: getData method not found");
    }

    g_pluginMethods.hasData = env->GetMethodID(clazz, METHOD_HAS_DATA, SIGNATURE_HAS_DATA);
    if (!g_pluginMethods.hasData) {
        LOGW("Clipborad JNI Init: hasData method not found");
    }

    g_pluginMethods.clear = env->GetMethodID(clazz, METHOD_CLEAR, SIGNATURE_CLEAR);
    if (!g_pluginMethods.clear) {
        LOGW("Clipborad JNI Init: clear method not found");
    }

    g_pluginMethods.isMultiTypeData = env->GetMethodID(clazz, METHOD_IS_MULTI_TYPE_DATA, SIGNATURE_IS_MULTI_TYPE_DATA);
    if (!g_pluginMethods.isMultiTypeData) {
        LOGW("Clipborad JNI Init: isMultiTypeData method not found");
    }

    g_pluginMethods.setMultiTypeData =
        env->GetMethodID(clazz, METHOD_SET_MULTI_TYPE_DATA, SIGNATURE_SET_MULTI_TYPE_DATA);
    if (!g_pluginMethods.setMultiTypeData) {
        LOGW("Clipborad JNI Init: setMultiTypeData method not found");
    }

    g_pluginMethods.getMultiTypeData =
        env->GetMethodID(clazz, METHOD_GET_MULTI_TYPE_DATA, SIGNATURE_GET_MULTI_TYPE_DATA);
    if (!g_pluginMethods.getMultiTypeData) {
        LOGW("Clipborad JNI Init: getMultiTypeData method not found");
    }
    env->DeleteLocalRef(clazz);
}

bool ClipboardJni::SetData(const std::string& data)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboardObj || !g_pluginMethods.setData) {
        LOGW("Clipborad JNI: null setData method");
        return false;
    }
    jstring jData = env->NewStringUTF(data.c_str());
    env->CallVoidMethod(g_clipboardObj.get(), g_pluginMethods.setData, jData);
    if (jData != nullptr) {
        env->DeleteLocalRef(jData);
    }
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call setData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool ClipboardJni::GetData(
    const std::function<void(const std::string&)>& callback, const WeakPtr<TaskExecutor>& taskExecutor)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboardObj || !g_pluginMethods.getData) {
        LOGW("Clipborad JNI: null getData method");
        return false;
    }

    std::string result;
    jstring jData = static_cast<jstring>(env->CallObjectMethod(g_clipboardObj.get(), g_pluginMethods.getData));
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call getData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    const char* content = env->GetStringUTFChars(jData, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jData, content);
    }
    if (jData != nullptr) {
        env->DeleteLocalRef(jData);
    }

    auto executor = taskExecutor.Upgrade();
    if (executor) {
        executor->PostTask(
            [callback, result] {
                if (callback) {
                    callback(result);
                }
            },
            TaskExecutor::TaskType::UI, "ArkUI-XClipboardJniGetData");
    }
    return true;
}

std::string ClipboardJni::GetData()
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN(env, "");
    CHECK_NULL_RETURN(g_clipboardObj, "");
    CHECK_NULL_RETURN(g_pluginMethods.getData, "");
    std::string result = "";
    jstring jData = static_cast<jstring>(env->CallObjectMethod(g_clipboardObj.get(), g_pluginMethods.getData));
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call getData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return "";
    }
    const char* content = env->GetStringUTFChars(jData, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jData, content);
    }
    if (jData != nullptr) {
        env->DeleteLocalRef(jData);
    }

    return result;
}

bool ClipboardJni::HasData(const std::function<void(const bool)>& callback, const WeakPtr<TaskExecutor>& taskExecutor)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboardObj || !g_pluginMethods.hasData) {
        LOGW("Clipborad JNI: null hasData method");
        return false;
    }

    bool jResult = static_cast<bool>(env->CallBooleanMethod(g_clipboardObj.get(), g_pluginMethods.hasData));
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call hasData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    auto executor = taskExecutor.Upgrade();
    if (executor) {
        executor->PostTask(
            [callback, jResult] {
                if (callback) {
                    callback(jResult);
                }
            },
            TaskExecutor::TaskType::UI, "ArkUI-XClipboardJniHasData");
    }
    return true;
}

bool ClipboardJni::HasPasteData()
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_clipboardObj, false);
    CHECK_NULL_RETURN(g_pluginMethods.hasData, false);

    bool jResult = static_cast<bool>(env->CallBooleanMethod(g_clipboardObj.get(), g_pluginMethods.hasData));
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call HasPasteData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return jResult;
}

bool ClipboardJni::Clear()
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboardObj || !g_pluginMethods.clear) {
        LOGW("Clipborad JNI: null clear method");
        return false;
    }

    env->CallVoidMethod(g_clipboardObj.get(), g_pluginMethods.clear);
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call clear has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool ClipboardJni::IsMultiTypeData()
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_clipboardObj, false);
    CHECK_NULL_RETURN(g_pluginMethods.isMultiTypeData, false);
    bool jResult = static_cast<bool>(env->CallBooleanMethod(g_clipboardObj.get(), g_pluginMethods.isMultiTypeData));
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call IsMultiTypeData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return jResult;
}

std::string ClipboardJni::GetMultiTypeData()
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN(env, "");
    CHECK_NULL_RETURN(g_clipboardObj, "");
    CHECK_NULL_RETURN(g_pluginMethods.getMultiTypeData, "");
    std::string result = "";
    jstring jData = static_cast<jstring>(env->CallObjectMethod(g_clipboardObj.get(), g_pluginMethods.getMultiTypeData));
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call GetMultiTypeData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return "";
    }
    const char* content = env->GetStringUTFChars(jData, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jData, content);
    }
    if (jData != nullptr) {
        env->DeleteLocalRef(jData);
    }

    return result;
}

void ClipboardJni::SetMultiTypeData(const std::string& data)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_clipboardObj);
    CHECK_NULL_VOID(g_pluginMethods.setMultiTypeData);
    jstring jData = env->NewStringUTF(data.c_str());
    env->CallVoidMethod(g_clipboardObj.get(), g_pluginMethods.setMultiTypeData, jData);
    if (jData != nullptr) {
        env->DeleteLocalRef(jData);
    }
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call SetMultiTypeData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}
} // namespace OHOS::Ace::Platform
