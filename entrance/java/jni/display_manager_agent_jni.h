/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DISPLAY_MANAGER_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DISPLAY_MANAGER_JNI_H

#include "jni.h"
#include <memory>
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {

struct DisplayManagerAgentJniStruct {
    jobject object;
    jclass clazz;
    jmethodID isFoldableMethod;
    jmethodID getFoldStatusMethod;
    jmethodID registerDisplayListenerMethod;
    jmethodID unregisterDisplayListenerMethod;
};

class DisplayManagerAgentJni {
public:
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static bool IsFoldable();
    static uint32_t GetFoldStatus();
    static void RegisterDisplayListener();
    static void UnregisterDisplayListener();
    static void OnDisplayAdded(JNIEnv* env, jobject obj, jint displayId);
    static void OnDisplayRemoved(JNIEnv* env, jobject obj, jint displayId);
    static void OnDisplayChanged(JNIEnv* env, jobject obj, jint displayId);
    static void FoldStatusChangeCallback(const std::string& instanceName, int32_t foldStatus);

private:
    static DisplayManagerAgentJniStruct displayManagerAgentJniStruct_;
    static void SetupDisplayManagerAgent(JNIEnv* env, jobject obj);
    ACE_DISALLOW_COPY_AND_MOVE(DisplayManagerAgentJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DISPLAY_MANAGER_JNI_H