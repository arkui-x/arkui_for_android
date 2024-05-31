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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_STORAGE_STORAGE_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_STORAGE_STORAGE_JNI_H

#include <memory>

#include "jni.h"

#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {

class StorageJni final {
public:
    static bool Register(std::shared_ptr<JNIEnv> env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    // Called by C++
    static void Set(const std::string& key, const std::string& value);
    static std::string Get(const std::string& key);
    static void Clear();
    static void Delete(const std::string& key);
private:
    ACE_DISALLOW_COPY_AND_MOVE(StorageJni);

    static void OnJniRegistered();
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_STORAGE_STORAGE_JNI_H