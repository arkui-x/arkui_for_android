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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_JNI_H

#include <memory>

#include "jni.h"

#include "base/utils/noncopyable.h"

namespace OHOS::Ace {

struct TextInputConfiguration;
struct TextEditingValue;

namespace Platform {

class TextInputJni final {
public:
    static bool Register(std::shared_ptr<JNIEnv> env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj, jint instanceId);
    static void UpdateEditingState(JNIEnv* env, jclass clazz, jint inputClientId, jstring editingState);
    static void PerformAction(JNIEnv* env, jclass clazz, jint clientId, jint actionValue);
    static void UpdateInputFilterErrorText(JNIEnv* env, jclass clazz, jint inputClientId, jstring errorText);

    // Called by C++
    static bool SetClient(int32_t clientId, const TextInputConfiguration& config, const int32_t instanceId);
    static bool ShowTextInput(bool isFocusViewChanged, int32_t instanceId);
    static bool HideTextInput(int32_t instanceId);
    static bool SetEditingState(const TextEditingValue& state, int32_t instanceId, bool needFireChangeEvent = true);
    static bool ClearClient(int32_t instanceId);
    static void ReleaseInstance(int32_t instanceId);

private:
    static void OnJniRegistered();
    static bool needFireChangeEvent_;

    TextInputJni() = delete;
    ~TextInputJni() = delete;
    ACE_DISALLOW_COPY_AND_MOVE(TextInputJni);
};

} // namespace Platform

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_COMMON_JNI_EDITING_TEXT_INPUT_JNI_H