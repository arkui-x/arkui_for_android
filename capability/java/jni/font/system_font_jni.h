/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_SYSTEM_FONT_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_SYSTEM_FONT_JNI_H

#include <memory>
#include <string>
#include "jni.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {

typedef struct FontInfoAndroidTag {
    std::string path;
    std::string name;
    uint32_t weight = 0;
    bool italic = false;
} FontInfoAndroid;

class SystemFontJni final {
public:
    static bool Register(std::shared_ptr<JNIEnv> env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    // Called by C++
    static void GetSystemFontInfo(std::vector<FontInfoAndroid>& fontInfos);
private:
    static void OnJniRegistered();
    static void InitFontInfo(JNIEnv* env);
    static void ConvertFontInfo(jobjectArray jObjectArray, std::vector<FontInfoAndroid>& fontInfos);

    ACE_DISALLOW_COPY_AND_MOVE(SystemFontJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_SYSTEM_FONT_JNI_H
