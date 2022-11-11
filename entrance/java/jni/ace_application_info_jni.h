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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_APPLICATION_INFO_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_APPLICATION_INFO_JNI_H

#include <memory>

#include "jni.h"

namespace OHOS::Ace::Platform {

class AceApplicationInfoJni {
public:
    AceApplicationInfoJni() = delete;
    ~AceApplicationInfoJni() = delete;

    static bool Register(const std::shared_ptr<JNIEnv>& env);

    static void NativeSetPackageInfo(
        JNIEnv* env, jclass myClass, jstring packageName, jint uid, jboolean isDebug, jboolean needDebugBreakpoint);
    static void NativeInitialize(JNIEnv* env, jclass myclass, jobject object);
    static void NativeLocaleChanged(JNIEnv* env, jclass myClass, jstring language, jstring countryOrRegion,
        jstring script, jstring keywordsAndValues);
    static void NativeSetUserID(JNIEnv* env, jclass myClass, jint userId);

    static void NativeSetAccessibilityEnabled(JNIEnv* env, jclass myClass, jboolean enabled);

    static void NativeSetProcessName(JNIEnv* env, jclass myClass, jstring processName);
    static void NativeSetupIcuRes(JNIEnv* env, jclass myClass, jstring icuData);
};

} // namespace OHOS::Ace::Platform

#endif