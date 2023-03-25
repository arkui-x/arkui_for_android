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

    static void SurfaceCreated(JNIEnv* env, jobject myObject, jlong view, jobject jsurface);
    static void SurfaceChanged(JNIEnv* env, jobject myObject, jlong view, jint width, jint height);
    static void SurfaceDestroyed(JNIEnv* env, jobject myObject, jlong view);
    static void RegisterWindow(JNIEnv* env, void* window, jobject windowView);
    static void UnRegisterWindow(JNIEnv* env, jobject windowView);

private:
    static bool RegisterCommonNatives(JNIEnv* env, const jclass myClass);
    ACE_DISALLOW_COPY_AND_MOVE(WindowViewJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_WINDOW_VIEW_JNI_H