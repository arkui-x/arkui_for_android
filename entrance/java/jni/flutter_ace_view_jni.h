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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_FLUTTER_ACE_VIEW_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_FLUTTER_ACE_VIEW_JNI_H

#include "jni.h"

#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {
class FlutterAceViewJni {
public:
    static bool RegisterNatives(JNIEnv* env);

    static jlong CreateAndroidViewHandle(JNIEnv* env, jclass myClass, jobject view, jint instanceId);
    // static jlong CreateOHOSViewHandle(JNIEnv* env, jclass myClass, jobject view, jinit instanceId);
    static void SurfaceCreated(JNIEnv* env, jobject myObject, jlong view, jobject jsurface);
    static void SurfaceChanged(JNIEnv* env, jobject myObject, jlong view, jint width, jint height, jint orientation);
    static void SurfaceDestroyed(JNIEnv* env, jobject myObject, jlong view);
    static void DestroySurfaceHandle(JNIEnv* env, jobject myObject, jlong view);
    static void SetViewportMetrics(JNIEnv* env, jobject myObject, jlong view, jfloat devicePixelRatio,
        jint physicalWidth, jint physicalHeight, jint physicalPaddingTop, jint physicalPaddingRight,
        jint physicalPaddingBottom, jint physicalPaddingLeft, jint physicalViewInsetTop, jint physicalViewInsetRight,
        jint physicalViewInsetBottom, jint physicalViewInsetLeft, jint systemGestureInsetTop,
        jint systemGestureInsetRight, jint systemGestureInsetBottom, jint systemGestureInsetLeft);
    static jboolean DispatchPointerDataPacket(JNIEnv* env, jobject myObject, jlong view, jobject buffer, jint position);
    static jboolean DispatchKeyEvent(JNIEnv* env, jobject myObject, jlong view, jint keyCode, jint action,
        jint repeatTme, jlong timeStamp, jlong timeStampStart, jint metaKey, jint keySource, jint deviceId);
    static void DispatchMouseEvent(JNIEnv* env, jobject myObject, jlong view, jobject buffer, jint position);
    static void RegisterTexture(JNIEnv* env, jobject myObject, jlong view, jlong texture_id, jobject surface_texture);
    static void RegisterSurface(JNIEnv* env, jobject myObject, jlong view, jlong texture_id, jobject surface);
    static void UnregisterSurface(JNIEnv* env, jobject myObject, jlong view, jlong texture_id);
    static void MarkTextureFrameAvailable(JNIEnv* env, jobject myObject, jlong view, jlong texture_id);
    static void UnregisterTexture(JNIEnv* env, jobject myObject, jlong view, jlong texture_id);
    static jlong InitResRegister(JNIEnv* env, jobject myObject, jlong view, jobject resRegister);
    static void InitCacheFilePath(JNIEnv* env, jobject myObject, jlong view, jstring imagePath, jstring filePath);
    static void InitDeviceType(JNIEnv* env, jclass myClass, jint deviceType);
    static jint GetViewBackgoundColor(JNIEnv* env, jobject myObject, jlong view);
    static void SetNativeView(JNIEnv* env, jobject myObject, jlong view, jobject jObject);

private:
    static jlong CreateViewHandle(JNIEnv* env, jclass myClass, jobject view, jint instanceId, int32_t platform);
    static bool RegisterCommonNatives(JNIEnv* env, const jclass myClass);
    ACE_DISALLOW_COPY_AND_MOVE(FlutterAceViewJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_FLUTTER_ACE_VIEW_JNI_H