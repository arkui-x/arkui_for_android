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
bool AceEnvJni::SetThreadInfo(int32_t threadId)
{
    if (threadId < 0) {
        return false;
    }
    auto jniEnv = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN(jniEnv, false);

    const jclass clazz = jniEnv->FindClass("ohos/ace/runtime/AceVipThreadUtility");
    if (!clazz || jniEnv->ExceptionCheck()) {
        LOGI("no thread uility, clear exception.");
        jniEnv->ExceptionDescribe();
        jniEnv->ExceptionClear();
        return false;
    }

    jmethodID setInfoMethod = jniEnv->GetStaticMethodID(clazz, "setHmThreadToRtg", "(II)Z");
    if (!setInfoMethod || jniEnv->ExceptionCheck()) {
        jniEnv->ExceptionDescribe();
        jniEnv->ExceptionClear();
        return false;
    }

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
} // namespace OHOS::Ace::Platform