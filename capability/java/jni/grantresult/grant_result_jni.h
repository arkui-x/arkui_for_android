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

#ifndef FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_GRANT_RESULT_JNI_H
#define FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_GRANT_RESULT_JNI_H

#include <memory>

#include "jni.h"

namespace OHOS::Ace::Platform {
class GrantResultJni {
public:
    GrantResultJni() = delete;
    ~GrantResultJni() = delete;

    static bool Register(std::shared_ptr<JNIEnv> env);
    static void OnRequestPremissionCallback(JNIEnv* env, jobject obj, jobjectArray stringArray, jintArray intArray);
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_GRANT_RESULT_JNI_H