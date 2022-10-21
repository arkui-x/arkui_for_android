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

#include "adapter/android/entrance/java/jni/jni_environment.h"

#include <sys/prctl.h>

#include "adapter/android/entrance/java/jni/jni_registry.h"
#include "base/log/event_report.h"
#include "base/log/log.h"

namespace OHOS::Ace::Platform {

namespace {

template<typename T>
void DummyRelease(T*)
{}

constexpr int32_t THREAD_NAME_MAX_LENGTH = 16;

} // namespace

JniEnvironment::JniEnvironment()
{
    LOGI("JniEnvironment created.");
}

JniEnvironment::~JniEnvironment() = default;

JniEnvironment JniEnvironment::instance_;

bool JniEnvironment::Initialize(const std::shared_ptr<JavaVM>& javaVM)
{
    if (!javaVM) {
        LOGE("Initialize: java VM is null");
        return false;
    }

    // Save instance of Java VM
    javaVm_ = javaVM;
    return true;
}

// Help to get JNI environment of current thread.
std::shared_ptr<JNIEnv> JniEnvironment::GetJniEnv(JNIEnv* jniEnv, bool isDetach) const
{
    if (jniEnv != nullptr) {
        return std::shared_ptr<JNIEnv>(jniEnv, DummyRelease<JNIEnv>);
    }

    // Get JNI environment of current thread.
    jint retVal = javaVm_->GetEnv(reinterpret_cast<void**>(&jniEnv), version_);
    if (retVal == JNI_OK) {
        return std::shared_ptr<JNIEnv>(jniEnv, DummyRelease<JNIEnv>);
    } else {
        char threadName[THREAD_NAME_MAX_LENGTH] = { 0 };
        prctl(PR_GET_NAME, threadName);
        JavaVMAttachArgs attachArgs { version_, threadName, nullptr };
        jint attachRet = javaVm_->AttachCurrentThread(&jniEnv, &attachArgs);
        if (attachRet != JNI_OK) {
            LOGE("GetJniEnv: Failed to get JNI environment, errCode = %{public}d", attachRet);
            return nullptr;
        }
    }
    if (!isDetach) {
        return std::shared_ptr<JNIEnv>(jniEnv, DummyRelease<JNIEnv>);
    }
    auto detachFunc = [](JNIEnv*) { JniEnvironment::GetInstance().GetVM()->DetachCurrentThread(); };
    return std::shared_ptr<JNIEnv>(jniEnv, detachFunc);
}

// Help to make Java global reference
JniEnvironment::JavaGlobalRef JniEnvironment::MakeJavaGlobalRef(
    const std::shared_ptr<JNIEnv>& jniEnvIn, jobject object, JavaGlobalRefDeleter deleter)
{
    const auto& jniEnv = jniEnvIn ? jniEnvIn : GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("MakeJavaGlobalRef: null JNI environment");
        return JavaGlobalRef(nullptr, nullptr);
    }
    return JavaGlobalRef(jniEnv->NewGlobalRef(object), deleter);
}

// Help to make Java local reference
JniEnvironment::JavaLocalRef JniEnvironment::MakeJavaLocalRef(
    const std::shared_ptr<JNIEnv>& jniEnvIn, jobject object, JavaLocalRefDeleter deleter)
{
    const auto& jniEnv = jniEnvIn ? jniEnvIn : GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("MakeJavaLocalRef: null JNI environment");
        return JavaLocalRef(nullptr, nullptr);
    }
    return JavaLocalRef(jniEnv->NewLocalRef(object), deleter);
}

// Help to make Java weak reference
JniEnvironment::JavaWeakRef JniEnvironment::MakeJavaWeakRef(
    const std::shared_ptr<JNIEnv>& jniEnvIn, jobject object, JavaWeakRefDeleter deleter)
{
    const auto& jniEnv = jniEnvIn ? jniEnvIn : GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("MakeJavaWeakRef: null JNI environment");
        return JavaWeakRef(nullptr, nullptr);
    }
    return JavaWeakRef(jniEnv->NewWeakGlobalRef(object), deleter);
}

void JniEnvironment::DeleteJavaGlobalRef(jobject object)
{
    const auto& jniEnv = GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("DeleteJavaGlobalRef: null JNI environment");
        return;
    }
    jniEnv->DeleteGlobalRef(object);
}

void JniEnvironment::DeleteJavaLocalRef(jobject object)
{
    const auto& jniEnv = GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("DeleteJavaGlobalRef: null JNI environment");
        return;
    }
    jniEnv->DeleteLocalRef(object);
}

void JniEnvironment::DeleteJavaWeakRef(jweak object)
{
    const auto& jniEnv = GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("DeleteJavaWeakRef: null JNI environment");
        return;
    }
    jniEnv->DeleteWeakGlobalRef(object);
}

JniEnvironment& JniEnvironment::GetInstance()
{
    return instance_;
}

} // namespace OHOS::Ace::Platform