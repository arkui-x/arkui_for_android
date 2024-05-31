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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_ACTIVITY_DELEGATE_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_ACTIVITY_DELEGATE_JNI_H

#include <memory>

#include "jni.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class StageActivityDelegateJni {
public:
    StageActivityDelegateJni() = delete;
    ~StageActivityDelegateJni() = delete;

    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static void AttachStageActivity(JNIEnv* env, jclass myclass, jstring jinstanceName, jobject object);
    static void DispatchOnCreate(JNIEnv* env, jclass myclass, jstring str, jstring params);
    static void DispatchOnDestroy(JNIEnv* env, jclass myclass, jstring str);
    static void DispatchOnForeground(JNIEnv* env, jclass myclass, jstring str);
    static void DispatchOnBackground(JNIEnv* env, jclass myclass, jstring str);
    static void DispatchOnNewWant(JNIEnv* env, jclass myclass, jstring str);
    static void SetWindowView(JNIEnv* env, jclass myclass, jstring str, jobject jwindowView);
    static void CreateAbilityDelegator(JNIEnv* env, jclass myclass, jstring jbundleName,
        jstring jmoduleName, jstring jtestName, jstring timeout);
    static void DispatchOnAbilityResult(
        JNIEnv* env, jclass myclass, jstring str, jint requestCode, jint resultCode, jstring resultWantParams);
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_ACTIVITY_DELEGATE_JNI_H