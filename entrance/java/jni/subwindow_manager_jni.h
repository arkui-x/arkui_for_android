/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <memory>
#include <string>

#include "jni.h"
#include "window_option.h"

#include "base/utils/noncopyable.h"

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
    jmethodID setBackgroundColorMethod;
    jmethodID setAppScreenBrightnessMethod;
    jmethodID getAppScreenBrightnessMethod;
    jmethodID setKeepScreenOnMethod;
    jmethodID isKeepScreenOnMethod;
    jmethodID requestOrientationMethod;
    jmethodID setStatusBarStatusMethod;
    jmethodID setActionBarStatusMethod;
    jmethodID isWindowShowingMethod;
    jmethodID setWindowLayoutFullScreenMethod;
    jmethodID setNavigationBarStatusMethod;
    jmethodID setNavigationIndicatorStatusMethod;
    jmethodID getStatusBarHeightMethod;
    jmethodID getCutoutBarHeightMethod;
    jmethodID getNavigationBarHeightMethod;
    jmethodID getGestureBarHeightMethod;
};

class SubWindowManagerJni {
public:
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static bool CreateSubWindow(std::shared_ptr<OHOS::Rosen::WindowOption> option);
    static jobject GetContentView(const std::string& windowName);
    static uint32_t GetWindowId(const std::string& windowName);
    static bool ShowWindow(const std::string& windowName);
    static bool MoveWindowTo(const std::string& windowName, int32_t x, int32_t y);
    static bool ResizeWindowTo(const std::string& windowName, int32_t width, int32_t height);
    static bool DestroyWindow(const std::string& windowName);
    static bool SetBackgroundColor(uint32_t color);
    static bool SetAppScreenBrightness(float brightness);
    static float GetAppScreenBrightness();
    static bool SetKeepScreenOn(bool keepScreenOn);
    static bool IsKeepScreenOn();
    static bool RequestOrientation(Rosen::Orientation orientation);
    static bool SetStatusBarStatus(bool hide);
    static bool SetActionBarStatus(bool hide);
    static bool IsWindowShowing(const std::string& name);
    static bool SetWindowLayoutFullScreen(bool isFullScreen);
    static bool SetNavigationBarStatus(bool hide);
    static bool SetNavigationIndicatorStatus(bool hide);
    static uint32_t GetStatusBarHeight();
    static uint32_t getCutoutBarHeight();
    static uint32_t GetNavigationBarHeight();
    static uint32_t GetNavigationIndicatorHeight();

private:
    static SubWindowManagerStruct subWindowManagerStruct_;
    static void SetupSubWindowManager(JNIEnv* env, jobject obj);
    ACE_DISALLOW_COPY_AND_MOVE(SubWindowManagerJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_SUBWINDOW_MANAGER_JNI_H