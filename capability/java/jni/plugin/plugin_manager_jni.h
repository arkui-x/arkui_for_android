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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_PLUGIN_MANAGER_PLUGIN_MANAGER_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_PLUGIN_MANAGER_PLUGIN_MANAGER_JNI_H

#include <functional>
#include <memory>

#include "jni.h"

#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {

using RegisterCallback = bool (*)(void*);

class ACE_EXPORT PluginManagerJni final {
public:
    static bool Register(std::shared_ptr<JNIEnv> env);
    static void RegisterPlugin(RegisterCallback pluginFunc, const std::string& packageName);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    // Called by Native->Java->Native
    static void NativeRegister(JNIEnv* env, jobject jobj, jlong jPluginRegisterFunc, jstring jPluginPackageName);

private:
    PluginManagerJni() = delete;
    ~PluginManagerJni() = delete;

    static void InitPlugin(const std::string& packageName);

    ACE_DISALLOW_COPY_AND_MOVE(PluginManagerJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_PLUGIN_MANAGER_PLUGIN_MANAGER_JNI_H
