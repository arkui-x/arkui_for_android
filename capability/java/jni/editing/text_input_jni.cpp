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

#include "adapter/android/capability/java/jni/editing/text_input_jni.h"

#include <unordered_map>

#include "adapter/android/capability/java/jni/editing/text_input_client_handler.h"
#include "adapter/android/capability/java/jni/editing/text_input_plugin.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/json/json_util.h"
#include "base/log/log.h"
#include "core/common/ime/text_input_proxy.h"

namespace OHOS::Ace::Platform {
namespace {

static const char* const TEXT_INPUT_PLUGIN_CLASS_NAME = "ohos/ace/adapter/capability/editing/TextInputPluginBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "(I)V", reinterpret_cast<void*>(TextInputJni::NativeInit) },
    { "updateEditingState", "(ILjava/lang/String;)V", reinterpret_cast<void*>(TextInputJni::UpdateEditingState) },
    { "performAction", "(II)V", reinterpret_cast<void*>(TextInputJni::PerformAction) },
    { "updateInputFilterErrorText", "(ILjava/lang/String;)V", reinterpret_cast<void*>(TextInputJni::UpdateInputFilterErrorText) },
};

static const char* const METHOD_SET_CLIENT = "setTextInputClient";
static const char* const METHOD_CLEAR_CLIENT = "clearTextInputClient";
static const char* const METHOD_SET_EDITING_STATE = "setTextInputEditingState";
static const char* const METHOD_SHOW = "showTextInput";
static const char* const METHOD_HIDE = "hideTextInput";

static const char* const SIGNATURE_SET_CLIENT = "(ILjava/lang/String;)V";
static const char* const SIGNATURE_CLEAR_CLIENT = "()V";
static const char* const SIGNATURE_SET_EDITING_STATE = "(Ljava/lang/String;)V";
static const char* const SIGNATURE_SHOW = "(Z)V";
static const char* const SIGNATURE_HIDE = "()V";

std::unordered_map<jint, JniEnvironment::JavaGlobalRef> g_jobjects;

struct {
    jmethodID setClient;
    jmethodID clearClient;
    jmethodID setEditingState;
    jmethodID showTextInput;
    jmethodID hideTextInput;
} g_pluginClass;

} // namespace

bool TextInputJni::needFireChangeEvent_ = true;

bool TextInputJni::Register(std::shared_ptr<JNIEnv> env)
{
    if (!env) {
        LOGW("TextInput JNI: env is null");
        return false;
    }
    jclass cls = env->FindClass(TEXT_INPUT_PLUGIN_CLASS_NAME);
    if (!cls) {
        LOGW("TextInput JNI: TextInputPluginBase not found");
        return false;
    }

    bool ret = env->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(cls);
    if (!ret) {
        LOGW("TextInput JNI: RegisterNatives failed.");
        return false;
    }
    OnJniRegistered();
    return true;
}

void TextInputJni::OnJniRegistered()
{
    TextInputProxy::GetInstance().SetDelegate(std::make_unique<TextInputPlugin>());
}

void TextInputJni::NativeInit(JNIEnv* env, jobject jobj, jint instanceId)
{
    if (env == nullptr) {
        LOGW("TextInput JNI: env is null");
        return;
    }

    if (jobj == nullptr) {
        LOGW("TextInput JNI: jobj is null");
        return;
    }
    auto result = g_jobjects.try_emplace(
        instanceId, JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj));
    if (!result.second) {
        LOGW("TextInput JNI: instance already exist.");
        return;
    }
    // Find classes & methods in Java class.
    jclass cls = env->GetObjectClass(jobj);
    if (cls == nullptr) {
        LOGE("TextInput JNI: get object class failed");
        return;
    }

    jclass superCls = env->GetSuperclass(cls);
    if (superCls == nullptr) {
        LOGE("TextInput JNI: get super class failed");
        return;
    }

    g_pluginClass.setClient = env->GetMethodID(superCls, METHOD_SET_CLIENT, SIGNATURE_SET_CLIENT);
    if (!g_pluginClass.setClient) {
        LOGW("TextInput JNI: setClient method not found.");
    }

    g_pluginClass.clearClient = env->GetMethodID(superCls, METHOD_CLEAR_CLIENT, SIGNATURE_CLEAR_CLIENT);
    if (!g_pluginClass.clearClient) {
        LOGW("TextInput JNI: clearClient method not found.");
    }

    g_pluginClass.setEditingState = env->GetMethodID(superCls, METHOD_SET_EDITING_STATE, SIGNATURE_SET_EDITING_STATE);
    if (!g_pluginClass.setEditingState) {
        LOGW("TextInput JNI: setEditingState method not found.");
    }

    g_pluginClass.showTextInput = env->GetMethodID(cls, METHOD_SHOW, SIGNATURE_SHOW);
    if (!g_pluginClass.showTextInput) {
        LOGW("TextInput JNI: showTextInput method not found.");
    }

    g_pluginClass.hideTextInput = env->GetMethodID(cls, METHOD_HIDE, SIGNATURE_HIDE);
    if (!g_pluginClass.hideTextInput) {
        LOGW("TextInput JNI: hideTextInput method not found.");
    }

    env->DeleteLocalRef(cls);
}

// Java -> C++
void TextInputJni::UpdateEditingState(JNIEnv* env, jclass clazz, jint clientId, jstring editingState)
{
    if (env == nullptr) {
        LOGW("TextInput JNI: env is null");
        return;
    }

    if (!editingState) {
        LOGW("TextInput JNI: Editing state is null");
        return;
    }
    const char* content = env->GetStringUTFChars(editingState, nullptr);
    if (content == nullptr) {
        // May have OutOfMemoryError.
        LOGW("TextInput JNI: Failed get string from jstring.");
        return;
    }

    auto json = JsonUtil::ParseJsonString(content);
    env->ReleaseStringUTFChars(editingState, content);

    if (!json) {
        LOGW("TextInput JNI: Invalid editing state json data.");
        return;
    }

    // Using shared_ptr rather than unique_ptr, because unique_ptr has problems on transiting by lambda.
    auto value = std::make_shared<TextEditingValue>();
    value->ParseFromJson(*json);
    TextInputClientHandler::GetInstance().UpdateEditingValue(clientId, value, needFireChangeEvent_);
    needFireChangeEvent_ = true;
}

void TextInputJni::PerformAction(JNIEnv* env, jclass clazz, jint clientId, jint actionValue)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }

    TextInputAction action = CastToTextInputAction(actionValue);
    TextInputClientHandler::GetInstance().PerformAction(clientId, action);
}

void TextInputJni::UpdateInputFilterErrorText(JNIEnv* env, jclass clazz, jint clientId, jstring errorText)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }

    if (!errorText) {
        LOGW("TextInput JNI: Error Text is null");
        return;
    }
    const char* content = env->GetStringUTFChars(errorText, nullptr);
    if (content == nullptr) {
        // May have OutOfMemoryError.
        LOGW("TextInput JNI: Failed get string from jstring.");
        return;
    }
    TextInputClientHandler::GetInstance().UpdateInputFilterErrorText(clientId, content);
}

bool TextInputJni::ShowTextInput(bool isFocusViewChanged, int32_t instanceId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("TextInput JNI: env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.showTextInput) {
        return false;
    }

    env->CallVoidMethod(jobject->second.get(), g_pluginClass.showTextInput, isFocusViewChanged);
    return true;
}

bool TextInputJni::SetClient(const int32_t clientId, const TextInputConfiguration& config, int32_t instanceId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("TextInput JNI: env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.setClient) {
        return false;
    }

    jstring jConfig = env->NewStringUTF(config.ToJsonString().c_str());
    env->CallVoidMethod(jobject->second.get(), g_pluginClass.setClient, clientId, jConfig);
    if (jConfig) {
        env->DeleteLocalRef(jConfig);
    }
    if (env->ExceptionCheck()) {
        LOGE("TextInput JNI: call SetClient has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool TextInputJni::HideTextInput(int32_t instanceId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("TextInput JNI: env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.hideTextInput) {
        return false;
    }

    env->CallVoidMethod(jobject->second.get(), g_pluginClass.hideTextInput);
    if (env->ExceptionCheck()) {
        LOGE("TextInput JNI: call HideTextInput has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool TextInputJni::SetEditingState(const TextEditingValue& state, int32_t instanceId, bool needFireChangeEvent)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("TextInput JNI: env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.setEditingState) {
        return false;
    }

    needFireChangeEvent_ = needFireChangeEvent;
    jstring jState = env->NewStringUTF(state.ToJsonString().c_str());
    env->CallVoidMethod(jobject->second.get(), g_pluginClass.setEditingState, jState);
    if (jState) {
        env->DeleteLocalRef(jState);
    }
    if (env->ExceptionCheck()) {
        LOGE("TextInput JNI: call SetEditingState has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool TextInputJni::ClearClient(int32_t instanceId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("TextInput JNI: env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.clearClient) {
        return false;
    }

    env->CallVoidMethod(jobject->second.get(), g_pluginClass.clearClient);
    if (env->ExceptionCheck()) {
        LOGE("TextInput JNI: call ClearClient has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

void TextInputJni::ReleaseInstance(int32_t instanceId)
{
    g_jobjects.erase(instanceId);
}

} // namespace OHOS::Ace::Platform
