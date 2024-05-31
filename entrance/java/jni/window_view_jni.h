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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_WINDOW_VIEW_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_WINDOW_VIEW_JNI_H

#include "jni.h"

#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {
class WindowViewJni {
public:
    static bool RegisterNatives(JNIEnv* env);

    static void SurfaceCreated(JNIEnv* env, jobject myObject, jlong window, jobject jsurface);
    static void SurfaceChanged(JNIEnv* env, jobject myObject, jlong window, jint width, jint height, jfloat density);
    static void KeyboardHeightChanged(JNIEnv* env, jobject myObject, jlong window, jint height);
    static void SurfaceDestroyed(JNIEnv* env, jobject myObject, jlong window);
    static void RegisterWindow(JNIEnv* env, void* window, jobject windowView);
    static void UnRegisterWindow(JNIEnv* env, jobject windowView);

    static void OnWindowFocusChanged(JNIEnv* env, jobject myObject, jlong window, jboolean hasWindowFocus);
    static void Foreground(JNIEnv* env, jobject myObject, jlong window);
    static void Background(JNIEnv* env, jobject myObject, jlong window);
    static void Destroy(JNIEnv* env, jobject myObject, jlong window);

    static jboolean BackPressed(JNIEnv* env, jobject myObject, jlong window);
    static jboolean DispatchPointerDataPacket(
        JNIEnv* env, jobject myObject, jlong window, jobject buffer, jint position);
    static jboolean DispatchMouseDataPacket(
        JNIEnv* env, jobject myObject, jlong window, jobject buffer, jint position);
    static jboolean DispatchKeyEvent(JNIEnv* env, jobject myObject, jlong window, jint keyCode, jint action,
        jint repeatTime, jlong timeStamp, jlong timeStampStart, jint source, jint deviceId, jint metaKey);

private:
    static bool RegisterCommonNatives(JNIEnv* env, const jclass myClass);
    ACE_DISALLOW_COPY_AND_MOVE(WindowViewJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_WINDOW_VIEW_JNI_H