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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_CONTAINER_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_CONTAINER_JNI_H

#include "jni.h"

namespace OHOS::Ace::Platform {

enum class HandlePageType { RUN_PAGE, PUSH_PAGE };

class AceContainerJni {
public:
    AceContainerJni() = delete;
    ~AceContainerJni() = delete;

    static bool Register();

    static void CreateContainer(JNIEnv* env, jclass clazz, jint instanceId, jint type, jobject callback, jstring name);
    static void DestroyContainer(JNIEnv* env, jclass clazz, jint instanceId);
    static jboolean RunPage(JNIEnv* env, jclass clazz, jint instanceId, jstring content, jstring params);
    static jboolean PushPage(JNIEnv* env, jclass clazz, jint instanceId, jstring content, jstring params);
    static jboolean NativeOnBackPressed(JNIEnv* env, jclass clazz, jint instanceId);
    static void NativeOnShow(JNIEnv* env, jclass clazz, jint instanceId);
    static void NativeOnHide(JNIEnv* env, jclass clazz, jint instanceId);
    static void NativeOnConfigurationUpdated(JNIEnv* env, jclass clazz, jint instanceId, jstring data);
    // static void NativeOnWindowDisplayModeChanged(
    //     JNIEnv* env, jclass clazz, jint instanceId, jboolean isShowInMultiWindow, jstring data);
    static void NativeOnActive(JNIEnv* env, jclass clazz, jint instanceId);
    static void NativeOnInactive(JNIEnv* env, jclass clazz, jint instanceId);
    static void NativeOnNewRequest(JNIEnv* env, jclass clazz, jint instanceId, jstring data);
    static void NativeOnMemoryLevel(JNIEnv* env, jclass clazz, jint instanceId, jint level);
    static void AddAssetPath(JNIEnv* env, jclass clazz, jint instanceId, jobject assetManager, jstring path);
    static void SetView(
        JNIEnv* env, jclass clazz, jint instanceId, jlong nativePtr, jfloat density, jint width, jint height);
    static void SetFontScale(JNIEnv* env, jclass clazz, jint instanceId, jfloat fontScale);
    static void SetWindowStyle(JNIEnv* env, jclass clazz, jint instanceId, jint windowModal, jint colorScheme);
    static void SetSemiModalCustomStyle(JNIEnv* env, jclass clazz, jint instanceId, jint modalHeight, jint modalColor);
    static void SetColorMode(JNIEnv* env, jclass clazz, jint instanceId, jint colorMode);
    static void SetHostClassName(JNIEnv* env, jclass clazz, jint instanceId, jstring hostClassName);
    static void InitDeviceInfo(JNIEnv* env, jclass clazz, jint instanceId, jint deviceWidth, jint deviceHeight,
        jint orientation, jfloat density, jboolean isRound, jint mcc, jint mnc);
    static void InitResourceManager(JNIEnv* env, jclass clazz, jint instanceId, jint themeId, jstring path);

private:
    static jboolean HandlePage(JNIEnv* env, jint instanceId, jstring content, jstring params, HandlePageType type);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_CONTAINER_JNI_H