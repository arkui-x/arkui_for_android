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

#include "adapter/android/entrance/java/jni/jni_registry.h"

#include "flutter/fml/platform/android/jni_util.h"

#include "adapter/android/capability/java/jni/bridge/bridge_jni.h"
#include "adapter/android/capability/java/jni/clipboard/clipboard_jni.h"
#include "adapter/android/capability/java/jni/editing/text_input_jni.h"
#include "adapter/android/capability/java/jni/environment/environment_jni.h"
#include "adapter/android/capability/java/jni/grantresult/grant_result_jni.h"
#include "adapter/android/capability/java/jni/storage/storage_jni.h"
#include "adapter/android/capability/java/jni/vibrator/vibrator_jni.h"
#include "adapter/android/capability/java/jni/plugin/plugin_manager_jni.h"
#include "adapter/android/entrance/java/jni/ace_platform_plugin_jni.h"
#include "adapter/android/entrance/java/jni/download_manager_jni.h"
#include "adapter/android/entrance/java/jni/dump_helper_jni.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/entrance/java/jni/window_view_jni.h"
#include "adapter/android/entrance/java/jni/subwindow_manager_jni.h"
#include "adapter/android/entrance/java/jni/display_info_jni.h"
#include "adapter/android/entrance/java/jni/web_adapter_jni.h"
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

    if (!DumpHelperJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register DumpHelper");
        return false;
    }

    fml::jni::InitJavaVM(JniEnvironment::GetInstance().GetVM().get());

    if (!WindowViewJni::RegisterNatives(jniEnv.get())) {
        LOGE("JNI Initialize: failed to register WindowViewJni");
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

    if (!EnvironmentJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register EnvironmentJni");
        return false;
    }

    if (!StorageJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register StorageJni");
        return false;
    }

    if (!VibratorJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register VibratorJni");
        return false;
    }

    if (!PluginManagerJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register PluginManagerJni");
        return false;
    }

    if (!DownloadManagerJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register DownloadManager");
        return false;
    }

    if (!BridgeJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register BridgeJni");
        return false;
    }

    if (!GrantResultJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register GrantResultJni");
        return false;
    }

    if (!AcePlatformPluginJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register AcePlatformPluginJni");
        return false;
    }

    if (!GrantResultJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register GrantResultJni");
        return false;
    }

    if (!SubWindowManagerJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register GrantResultJni");
        return false;
    }

    if (!DisplayInfoJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register GrantResultJni");
        return false;
    }

    if (!WebAdapterJni::Register(jniEnv)) {
        LOGE("JNI Initialize: failed to register WebAdapterJni");
        return false;
    }

    return true;
}

} // namespace OHOS::Ace::Platform
