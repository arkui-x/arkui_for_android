/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

static const char* const TEXT_INPUT_PLUGIN_CLASS_NAME = "ohos/ace/capability/editing/TextInputPluginBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "(I)V", reinterpret_cast<void*>(TextInputJni::NativeInit) },
    { "updateEditingState", "(ILjava/lang/String;)V", reinterpret_cast<void*>(TextInputJni::UpdateEditingState) },
    { "performAction", "(II)V", reinterpret_cast<void*>(TextInputJni::PerformAction) },
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
        LOGW("env is null");
        return false;
    }
    jclass cls = env->FindClass(TEXT_INPUT_PLUGIN_CLASS_NAME);
    if (!cls) {
        LOGW("cls TEXT_INPUT_PLUGIN_CLASS_NAME is null");
        return false;
    }

    bool ret = env->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(cls);
    if (!ret) {
        LOGW("TextInputJni RegisterNatives fail.");
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
        LOGW("env is null");
        return;
    }

    if (jobj == nullptr) {
        LOGW("jobj is null");
        return;
    }
    auto result = g_jobjects.try_emplace(
        instanceId, JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj));
    if (!result.second) {
        LOGW("instance already exist.");
        return;
    }
    // Find classes & methods in Java class.
    jclass cls = env->GetObjectClass(jobj);
    if (cls == nullptr) {
        LOGE("get object class failed");
        return;
    }

    jclass superCls = env->GetSuperclass(cls);
    if (superCls == nullptr) {
        LOGE("get super class failed");
        return;
    }

    g_pluginClass.setClient = env->GetMethodID(superCls, METHOD_SET_CLIENT, SIGNATURE_SET_CLIENT);
    if (!g_pluginClass.setClient) {
        LOGW("setClient method is not exists.");
    }

    g_pluginClass.clearClient = env->GetMethodID(superCls, METHOD_CLEAR_CLIENT, SIGNATURE_CLEAR_CLIENT);
    if (!g_pluginClass.clearClient) {
        LOGW("clearClient method is not exists.");
    }

    g_pluginClass.setEditingState = env->GetMethodID(superCls, METHOD_SET_EDITING_STATE, SIGNATURE_SET_EDITING_STATE);
    if (!g_pluginClass.setEditingState) {
        LOGW("setEditingState method is not exists.");
    }

    g_pluginClass.showTextInput = env->GetMethodID(cls, METHOD_SHOW, SIGNATURE_SHOW);
    if (!g_pluginClass.showTextInput) {
        LOGW("showTextInput method is not exists.");
    }

    g_pluginClass.hideTextInput = env->GetMethodID(cls, METHOD_HIDE, SIGNATURE_HIDE);
    if (!g_pluginClass.hideTextInput) {
        LOGW("hideTextInput method is not exists.");
    }

    env->DeleteLocalRef(cls);
}

// Java -> C++
void TextInputJni::UpdateEditingState(JNIEnv* env, jclass clazz, jint clientId, jstring editingState)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }

    if (!editingState) {
        LOGW("Editing state is null");
        return;
    }
    const char* content = env->GetStringUTFChars(editingState, nullptr);
    if (content == nullptr) {
        // May have OutOfMemoryError.
        LOGW("Failed get string from jstring.");
        return;
    }

    auto json = JsonUtil::ParseJsonString(content);
    env->ReleaseStringUTFChars(editingState, content);

    if (!json) {
        LOGW("Invalid editing state json data.");
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

bool TextInputJni::ShowTextInput(bool isFocusViewChanged, int32_t instanceId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("jni env not ready");
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
        LOGW("jni env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.setClient) {
        return false;
    }

    jstring jconfig = env->NewStringUTF(config.ToJsonString().c_str());
    env->CallVoidMethod(jobject->second.get(), g_pluginClass.setClient, clientId, jconfig);
    if (jconfig) {
        env->DeleteLocalRef(jconfig);
    }
    return true;
}

bool TextInputJni::HideTextInput(int32_t instanceId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("jni env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.hideTextInput) {
        return false;
    }

    env->CallVoidMethod(jobject->second.get(), g_pluginClass.hideTextInput);
    return true;
}

bool TextInputJni::SetEditingState(const TextEditingValue& state, int32_t instanceId, bool needFireChangeEvent)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("jni env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.setEditingState) {
        return false;
    }

    needFireChangeEvent_ = needFireChangeEvent;
    jstring jstate = env->NewStringUTF(state.ToJsonString().c_str());
    env->CallVoidMethod(jobject->second.get(), g_pluginClass.setEditingState, jstate);
    if (jstate) {
        env->DeleteLocalRef(jstate);
    }
    return true;
}

bool TextInputJni::ClearClient(int32_t instanceId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("jni env not ready");
        return false;
    }

    auto jobject = g_jobjects.find(instanceId);
    if (jobject == g_jobjects.end() || !g_pluginClass.clearClient) {
        return false;
    }

    env->CallVoidMethod(jobject->second.get(), g_pluginClass.clearClient);
    return true;
}

void TextInputJni::ReleaseInstance(int32_t instanceId)
{
    g_jobjects.erase(instanceId);
}

} // namespace OHOS::Ace::Platform
