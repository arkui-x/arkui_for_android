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

#include "jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/log/ace_trace.h"
#include "jni_app_mode_config.h"

namespace {

template<typename T>
void DummyRelease(T*)
{}

const jint RET_FAIL = -1;

}

// This is called by the VM when the shared library is first loaded.
jint JNI_OnLoad(JavaVM* vm, void*)
{
    if (vm == nullptr) {
        LOGE("JNI Onload: JavaVm is null");
        return RET_FAIL;
    }

    OHOS::Ace::AceScopedTrace aceScopedTrace("JNI_OnLoad");
    std::shared_ptr<JavaVM> javaVm(vm, DummyRelease<JavaVM>);
    if (!OHOS::Ace::Platform::JniEnvironment::GetInstance().Initialize(javaVm)) {
        LOGE("JNI Onload: failed to initialize JniEnvironment");
        return RET_FAIL;
    }

    if (!OHOS::Ace::Platform::JniAppModeConfig::Register()) {
        LOGE("JNI Onload: failed to register JniAppModeConfig");
        return RET_FAIL;
    }

    LOGI("JNI Onload: sharedlibrary has been loaded successsfully!");
    return OHOS::Ace::Platform::JniEnvironment::GetInstance().Version();
}
