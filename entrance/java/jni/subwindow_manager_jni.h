/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_SUBWINDOW_MANAGER_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_SUBWINDOW_MANAGER_JNI_H

#include "jni.h"
#include <memory>
#include <string>
#include "base/utils/noncopyable.h"
#include "window_option.h"

namespace OHOS::Ace::Platform {

struct SubWindowManagerStruct {
    jobject object;
    jclass clazz;
    jmethodID createSubWindowMethod;
    jmethodID getContentViewMethod;
    jmethodID resizeMethod;
    jmethodID showWindowMethod;
    jmethodID moveWindowToMethod;
    jmethodID destroyWindowMethod;
    jmethodID getWindowIdMethod;
    jmethodID getTopWindowMethod;
};

class SubWindowManagerJni {
public:
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static void CreateSubWindow(std::shared_ptr<OHOS::Rosen::WindowOption> option);
    static jobject GetContentView(const std::string& windowName);
    static uint32_t GetWindowId(const std::string& windowName);
    static bool ShowWindow(const std::string& windowName);
    static bool MoveWindowTo(const std::string& windowName, int32_t x, int32_t y);
    static bool ResizeWindowTo(const std::string& windowName, int32_t width, int32_t height);
    static bool DestroyWindow(const std::string& windowName);
private:
    static SubWindowManagerStruct subWindowManagerStruct_;
    static void SetupSubWindowManager(JNIEnv* env, jobject obj);
    ACE_DISALLOW_COPY_AND_MOVE(SubWindowManagerJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_SUBWINDOW_MANAGER_JNI_H