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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_PLATFORM_PLUGIN_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_PLATFORM_PLUGIN_JNI_H

#include <memory>
#include <map>

#include "jni.h"

#include "adapter/android/entrance/java/jni/ace_resource_register.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {
class AcePlatformPluginJni {
public:
    AcePlatformPluginJni() = delete;
    ~AcePlatformPluginJni() = delete;

    static bool Register(const std::shared_ptr<JNIEnv>& env);

    static jlong InitResRegister(JNIEnv* env, jobject myObject, jobject resRegister, jint instanceId);

    static RefPtr<AceResourceRegister> GetResRegister(int32_t instanceId);
    static void RegisterSurface(JNIEnv* env, jobject myObject,
        jint instanceId, jlong texture_id, jobject surface);
    static void UnregisterSurface(JNIEnv* env, jobject myObject, jint instanceId, jlong texture_id);
    static void* GetNativeWindow(int32_t instanceId, int64_t textureId);
    static void ReleaseInstance(int32_t instanceId);
    
    static void RegisterTexture(JNIEnv* env, jobject myObject,
        jint instanceId, jlong textureId, jobject surfaceTexture);
    static void UnregisterTexture(JNIEnv* env, jobject myObject, jint instanceId, jlong textureId);

private:
    ACE_DISALLOW_COPY_AND_MOVE(AcePlatformPluginJni);
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_PLATFORM_PLUGIN_JNI_H