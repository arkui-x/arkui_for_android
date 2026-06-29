/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DISPLAY_INFO_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DISPLAY_INFO_JNI_H

#include <cstdint>
#include "jni.h"
#include <memory>
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {

struct DisplayInfoStruct {
    jobject object;
    jclass clazz;
    jmethodID getDisplayIdMethod;
    jmethodID getOrentationMethod;
    jmethodID getWidthMethod;
    jmethodID getHeightMethod;
    jmethodID getRefreshRateMethod;
    jmethodID getDensityPixelsMethod;
    jmethodID getDensityDpiMethod;
    jmethodID getScaledDensityMethod;
    jmethodID getXDpiMethod;
    jmethodID getYDpiMethod;
    jmethodID getRotationMethod;
    jmethodID getDisplayIdWithIdMethod;
    jmethodID getRotationWithIdMethod;
    jmethodID getOrentationWithIdMethod;
    jmethodID getWidthWithIdMethod;
    jmethodID getHeightWithIdMethod;
    jmethodID getRefreshRateWithIdMethod;
    jmethodID getDensityPixelsWithIdMethod;
    jmethodID getDensityDpiWithIdMethod;
    jmethodID getScaledDensityWithIdMethod;
    jmethodID getXDpiWithIdMethod;
    jmethodID getYDpiWithIdMethod;
};

class DisplayInfoJni {
public:
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static int getDisplayId();
    static int32_t getDisplayWidth();
    static int32_t getDisplayHeight();
    static int getOrentation();
    static float getRefreshRate();
    static float getDensityPixels();
    static int getDensityDpi();
    static float getScaledDensity();
    static float getXDpi();
    static float getYDpi();
    static uint32_t GetRotation();
    static int GetDisplayId(int displayId);
    static uint32_t GetRotation(int displayId);
    static int32_t GetDisplayWidth(int displayId);
    static int32_t GetDisplayHeight(int displayId);
    static int GetOrentation(int displayId);
    static float GetRefreshRate(int displayId);
    static float GetDensityPixels(int displayId);
    static int GetDensityDpi(int displayId);
    static float GetScaledDensity(int displayId);
    static float GetXDpi(int displayId);
    static float GetYDpi(int displayId);

private:
    static DisplayInfoStruct displayInfoStruct_;
    static void SetupDisplayInfo(JNIEnv* env, jobject obj);
    static void ReleaseDisplayInfo(JNIEnv* env, jobject obj);
    ACE_DISALLOW_COPY_AND_MOVE(DisplayInfoJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DISPLAY_INFO_JNI_H