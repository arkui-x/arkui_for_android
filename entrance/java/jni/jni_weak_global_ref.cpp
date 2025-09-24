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

#include "jni_weak_global_ref.h"

namespace OHOS::Ace::Platform {
JniWeakGlobalRef::JniWeakGlobalRef(JNIEnv* env, jobject obj) : weakGlobalRef_(nullptr)
{
    if (env && obj) {
        weakGlobalRef_ = env->NewWeakGlobalRef(obj);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            weakGlobalRef_ = nullptr;
        }
    }
}

JniWeakGlobalRef::~JniWeakGlobalRef()
{
    Clear();
}

JniWeakGlobalRef::JniWeakGlobalRef(const JniWeakGlobalRef& other) : weakGlobalRef_(nullptr)
{
    CopyFrom(other);
}

JniWeakGlobalRef& JniWeakGlobalRef::operator=(const JniWeakGlobalRef& other)
{
    if (this != &other) {
        CopyFrom(other);
    }
    return *this;
}

JniEnvironment::JavaLocalRef JniWeakGlobalRef::GetLocalRef(JNIEnv* env) const
{
    if (!env || !weakGlobalRef_ || env->IsSameObject(weakGlobalRef_, nullptr)) {
        return JniEnvironment::JavaLocalRef(nullptr, JniEnvironment::DeleteJavaLocalRef);
    }
    jobject javaObject = env->NewLocalRef(weakGlobalRef_);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return JniEnvironment::JavaLocalRef(nullptr, JniEnvironment::DeleteJavaLocalRef);
    }
    if (!javaObject) {
        return JniEnvironment::JavaLocalRef(nullptr, JniEnvironment::DeleteJavaLocalRef);
    }
    auto envShared = JniEnvironment::GetInstance().GetJniEnv(env, false);
    return JniEnvironment::MakeJavaLocalRef(envShared, javaObject, JniEnvironment::DeleteJavaLocalRef);
}

void JniWeakGlobalRef::Clear()
{
    if (weakGlobalRef_) {
        auto env = JniEnvironment::GetInstance().GetJniEnv();
        if (env) {
            env->DeleteWeakGlobalRef(weakGlobalRef_);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
            }
        }
        weakGlobalRef_ = nullptr;
    }
}

void JniWeakGlobalRef::CopyFrom(const JniWeakGlobalRef& other)
{
    if (&other == this) {
        return;
    }
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        return;
    }
    if (weakGlobalRef_) {
        env->DeleteWeakGlobalRef(weakGlobalRef_);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
    }
    weakGlobalRef_ = other.weakGlobalRef_ ? env->NewWeakGlobalRef(other.weakGlobalRef_) : nullptr;
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}
} // namespace OHOS::Ace::Platform