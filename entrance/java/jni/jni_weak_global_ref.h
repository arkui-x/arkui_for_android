/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JNI_WEAK_GLOBAL_REF_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JNI_WEAK_GLOBAL_REF_H

#include <jni.h>

#include "adapter/android/entrance/java/jni/jni_environment.h"

namespace OHOS::Ace::Platform {
class JniWeakGlobalRef {
public:
    JniWeakGlobalRef() : weakGlobalRef_(nullptr) {}
    JniWeakGlobalRef(JNIEnv* env, jobject obj);
    ~JniWeakGlobalRef();
    JniWeakGlobalRef(const JniWeakGlobalRef& other);
    JniWeakGlobalRef& operator=(const JniWeakGlobalRef& other);
    JniEnvironment::JavaLocalRef GetLocalRef(JNIEnv* env) const;
    void Clear();

private:
    void CopyFrom(const JniWeakGlobalRef& other);
    jweak weakGlobalRef_;
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JNI_WEAK_GLOBAL_REF_H