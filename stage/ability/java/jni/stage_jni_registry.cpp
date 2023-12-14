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

#include "stage_jni_registry.h"

#include "stage_activity_delegate_jni.h"
#include "stage_application_delegate_jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
bool StageJniRegistry::Register()
{
    auto jniEnv = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("JNI Initialize: failed to get JNI environment");
        return false;
    }

    if (!StageApplicationDelegateJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register StageApplicationDelegateJni");
        return false;
    }

    if (!StageActivityDelegateJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register StageActivityDelegateJni");
        return false;
    }
    return true;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
