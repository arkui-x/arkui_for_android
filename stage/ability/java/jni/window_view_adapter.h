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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_WINDOW_VIEW_ADAPTER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_WINDOW_VIEW_ADAPTER_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "jni.h"
#include "jni_environment.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class WindowViewAdapter {
public:
    WindowViewAdapter();
    ~WindowViewAdapter();

    static std::shared_ptr<WindowViewAdapter> GetInstance();
    void AddWindowView(const std::string& instanceName, jobject windowView);
    jobject GetWindowView(const std::string& instanceName);
    std::shared_ptr<JNIEnv> GetJniEnv();
    void RemoveWindowView(const std::string& instanceName);
    std::string GetWindowName(void* windowView);

private:
    static std::shared_ptr<WindowViewAdapter> instance_;
    static std::mutex mutex_;
    std::unordered_map<std::string, Ace::Platform::JniEnvironment::JavaGlobalRef> jobjects_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_WINDOW_VIEW_ADAPTER_H