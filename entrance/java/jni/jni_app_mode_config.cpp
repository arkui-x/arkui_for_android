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

#include "adapter/android/entrance/java/jni/jni_app_mode_config.h"

#include "adapter/android/entrance/java/jni/ace_container_jni.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/entrance/java/jni/jni_registry.h"
#include "adapter/android/stage/ability/java/jni/stage_registry_jni.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
namespace {
const std::string FA_MODE = "fa";
const std::string STAGE_MODE = "stage";
}
bool JniAppModeConfig::Register()
{
    LOGI("JniAppModeConfig register start.");
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeSetAppMode",
            .signature = "(Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetAppMode),
        },
    };

    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("JNI JniAppModeConfig: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/ace/adapter/AppModeConfig");
    if (clazz == nullptr) {
        LOGE("JNI JniAppModeConfig: can't find java class AppModeConfig");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, Ace::ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void JniAppModeConfig::SetAppMode(JNIEnv* env, jclass myclass, jstring str)
{
    LOGI("JniAppModeConfig: SetAppMode");
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }

    auto appMode = env->GetStringUTFChars(str, nullptr);
    if (appMode == FA_MODE) {
        LOGI("Register fa mode jni.");
        if (!OHOS::Ace::Platform::AceContainerJni::Register()) {
            LOGE("JNI Onload: failed to register AceContainer");
            return;
        }
        if (!OHOS::Ace::Platform::JniRegistry::Register()) {
            LOGE("JNI Onload: failed to call JniRegistry");
            return;
        }
    } else if (appMode == STAGE_MODE) {
        LOGI("Register stage mode jni.");
        if (!OHOS::AbilityRuntime::Platform::StageRegistryJni::Register()) {
            LOGE("JNI Onload: failed to register StageRegistryJni");
            return;
        }
        if (!OHOS::Ace::Platform::JniRegistry::Register(true)) {
            LOGE("JNI Onload: failed to call JniRegistry");
            return;
        }
    } else {
        LOGE("%{public}s platform not found, initialization of jni failed", appMode);
    }
}
} // namespace OHOS::Ace::Platform