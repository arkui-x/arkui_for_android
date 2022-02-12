/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/ace_env_jni.h"

#include "flutter/shell/platform/android/platform_view_android.h"

#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {

bool AceEnvJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeSetupFirstFrameHandler",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&SetupFirstFrameHandler),
        }
    };

    if (!env) {
        LOGE("JNI AceEnv: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/ace/adapter/AceEnv");
    if (clazz == nullptr) {
        LOGE("JNI AceEnv: can't find java class AceEnv");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void AceEnvJni::SetupFirstFrameHandler(JNIEnv* env, jclass clazz, jint platfrom)
{
    if (!flutter::PlatformViewAndroid::RegisterOnFirstFrame(env, platfrom)) {
        LOGE("JNI AceEnv: register OnFirstFrame failed");
    }
}

} // OHOS::Ace::Platform