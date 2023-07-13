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

#include "adapter/android/entrance/java/jni/ace_env_jni.h"
#include <jni.h>

#include "jni_environment.h"

#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {

bool AceEnvJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = { {
        .name = "nativeSetupFirstFrameHandler",
        .signature = "(I)V",
        .fnPtr = reinterpret_cast<void*>(&SetupFirstFrameHandler),
    } };

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

bool AceEnvJni::SetThreadInfo(int32_t threadId)
{
    if (threadId < 0) {
        return false;
    }
    auto jniEnv = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN_NOLOG(jniEnv, false);

    const jclass clazz = jniEnv->FindClass("ohos/ace/runtime/AceVipThreadUtility");
    CHECK_NULL_RETURN_NOLOG(clazz, false);

    jmethodID setInfoMethod = jniEnv->GetStaticMethodID(clazz, "setHmThreadToRtg", "(II)Z");
    CHECK_NULL_RETURN_NOLOG(setInfoMethod, false);

    jboolean ret = jniEnv->CallStaticBooleanMethod(clazz, setInfoMethod, static_cast<jint>(threadId), 0);
    jniEnv->DeleteLocalRef(clazz);
    if (jniEnv->ExceptionCheck()) {
        LOGE("CallMethod has expection!");
        jniEnv->ExceptionDescribe();
        jniEnv->ExceptionClear();
        return false;
    }
    return ret;
}

void AceEnvJni::SetupFirstFrameHandler(JNIEnv* env, jclass clazz, jint platfrom) {}

} // namespace OHOS::Ace::Platform