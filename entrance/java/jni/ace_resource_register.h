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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_RESOURCE_REGISTER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_RESOURCE_REGISTER_H

#include <cstdint>
#include <vector>

#include "jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/thread/task_executor.h"
#include "core/common/platform_res_register.h"

namespace OHOS::Ace::Platform {

class AceResourceRegister final : public PlatformResRegister {
public:
    AceResourceRegister(jobject object, int32_t id);
    ~AceResourceRegister() override = default;

    static void OnCallEvent(JNIEnv* env, jclass clazz, jlong resRegisterPtr, jstring evnetId, jstring param);

    int64_t CreateResource(const std::string& resourceType, const std::string& param) override;
    bool ReleaseResource(const std::string& resourceHash) override;
    bool OnMethodCall(const std::string& method, const std::string& param, std::string& result) override;
    bool Initialize(JNIEnv* env);

private:
    static void CreateResouceRegister(JNIEnv* env, jclass clazz, jobject object);
    bool IsRunOnPlatfromThread();

    JniEnvironment::JavaGlobalRef object_;
    jmethodID registerResourceMethod_ = nullptr;
    jmethodID releaseResourceMethod_ = nullptr;
    jmethodID onCallMethod_ = nullptr;
    int32_t instanceId_ = 0;
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_RESOURCE_REGISTER_H