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

#include "adapter/android/entrance/java/jni/ace_platform_plugin_jni.h"

#include "flutter/fml/platform/android/jni_weak_ref.h"
#include "adapter/android/entrance/java/jni/ace_resource_register.h"
#include "adapter/android/entrance/java/jni/flutter_ace_view.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
bool AcePlatformPluginJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeInitResRegister",
            .signature = "(JLohos/ace/adapter/AceResourceRegister;)J",
            .fnPtr = reinterpret_cast<void*>(&AcePlatformPluginJni::InitResRegister),
        }
    };

    if (!env) {
        LOGE("JNIEnv is null when register AcePlatformPluginJni jni！");
        return false;
    }

    const jclass myClass = env->FindClass("ohos/ace/adapter/AcePlatformPlugin");
    if (myClass == nullptr) {
        LOGE("Failed to find the AcePlatformPlugin Class");
        return false;
    }

    return env->RegisterNatives(myClass, methods, ArraySize(methods)) == 0;
}

jlong AcePlatformPluginJni::InitResRegister(JNIEnv* env, jobject myObject, jlong view, jobject resRegister)
{
    if (env == nullptr) {
        LOGE("env is null");
        return 0;
    }
    auto aceResRegister = Referenced::MakeRefPtr<AceResourceRegister>(resRegister);
    if (!aceResRegister->Initialize(env)) {
        LOGE("Failed to initialize the AcerResourceRegister");
        return 0;
    }
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr == nullptr) {
        LOGE("viewPtr is null");
        return 0;
    }
    viewPtr->SetPlatformResRegister(aceResRegister);
    return PointerToJavaLong(AceType::RawPtr(aceResRegister));
}
} // namespace OHOS::Ace::Platform