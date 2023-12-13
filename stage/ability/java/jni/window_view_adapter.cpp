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

#include "window_view_adapter.h"

#include "jni_environment.h"

#include "base/log/log.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<WindowViewAdapter> WindowViewAdapter::instance_ = nullptr;
std::mutex WindowViewAdapter::mutex_;
WindowViewAdapter::WindowViewAdapter() {}

WindowViewAdapter::~WindowViewAdapter() {}

std::shared_ptr<WindowViewAdapter> WindowViewAdapter::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<WindowViewAdapter>();
        }
    }

    return instance_;
}

void WindowViewAdapter::AddWindowView(const std::string& instanceName, jobject windowView)
{
    auto env = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("env is nullptr");
        return;
    }
    jobjects_.emplace(instanceName, Ace::Platform::JniEnvironment::MakeJavaGlobalRef(env, windowView));
}

jobject WindowViewAdapter::GetWindowView(const std::string& instanceName)
{
    LOGI("Get window view, instancename: %{public}s", instanceName.c_str());
    auto finder = jobjects_.find(instanceName);
    if (finder != jobjects_.end()) {
        return finder->second.get();
    }
    return nullptr;
}

std::shared_ptr<JNIEnv> WindowViewAdapter::GetJniEnv()
{
    return Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
}

void WindowViewAdapter::RemoveWindowView(const std::string& instanceName)
{
    LOGI("Remove window view, instancename: %{public}s", instanceName.c_str());
    auto finder = jobjects_.find(instanceName);
    if (finder != jobjects_.end()) {
        jobjects_.erase(finder);
    }
}

std::string WindowViewAdapter::GetWindowName(void* windowView)
{
    if (windowView != nullptr) {
        auto iterator = jobjects_.begin();
        while (iterator != jobjects_.end()) {
            if (iterator->second.get() == windowView) {
                return iterator->first;
            }
            iterator++;
        }
    }
    return std::string("");
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS