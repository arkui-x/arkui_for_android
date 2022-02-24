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

#include "adapter/android/entrance/java/jni/jni_registry.h"

#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/shell/platform/android/platform_view_android.h"
#include "flutter/shell/platform/android/vsync_waiter_android.h"

#include "adapter/android/capability/java/jni/clipboard/clipboard_jni.h"
#include "adapter/android/capability/java/jni/editing/text_input_jni.h"
#include "adapter/android/entrance/java/jni/ace_application_info_jni.h"
#include "adapter/android/entrance/java/jni/ace_env_jni.h"
#include "adapter/android/entrance/java/jni/dump_helper_jni.h"
#include "adapter/android/entrance/java/jni/flutter_ace_view_jni.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"

namespace OHOS::Ace::Platform {

bool JniRegistry::Register()
{
    // Get JNI environment of current thread.
    auto jniEnv = JniEnvironment::GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("JNI Initialize: failed to get JNI environment");
        return false;
    }

    if (!AceEnvJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register AceEnvJni");
        return false;
    }

    if (!DumpHelperJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register DumpHelper");
        return false;
    }

    if (!AceApplicationInfoJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register AceApplicationInfo");
        return false;
    }

    fml::jni::InitJavaVM(JniEnvironment::GetInstance().GetVM().get());

    if (!FlutterAceViewJni::RegisterNatives(jniEnv.get())) {
        LOGE("JNI Initialize: failed to register FlutterAceView");
        return false;
    }

    if (!flutter::VsyncWaiterAndroid::Register(jniEnv.get())) {
        LOGE("JNI Initialize: failed to register VsyncWaiterAndroid");
        return false;
    }

    if (!flutter::PlatformViewAndroid::Register(jniEnv.get())) {
        LOGE("JNI Initialize: failed to register PlatformViewAndroid");
        return false;
    }

    if (!AceApplicationInfoJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register AceApplicationInfo");
        return false;
    }

    // register capability plugins
    if (!ClipboardJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register ClipboardJni");
        return false;
    }

    if (!TextInputJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register TextInputJni");
        return false;
    }

    return true;
}

} // namespace OHOS::Ace::Platform
