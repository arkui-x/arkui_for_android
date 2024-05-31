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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_APPLICATION_CONTEXT_ADAPTER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_APPLICATION_CONTEXT_ADAPTER_H

#include <memory>
#include <mutex>

#include "jni.h"
#include "jni_environment.h"
#include "running_process_info.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class ApplicationContextAdapter {
public:
    ApplicationContextAdapter();
    ~ApplicationContextAdapter();

    static std::shared_ptr<ApplicationContextAdapter> GetInstance();
    void SetStageApplicationDelegate(jobject stageApplicationDelegate);
    std::vector<RunningProcessInfo> GetRunningProcessInformation();
    std::string GetTopAbility();
    int32_t StartAbility(const AAFwk::Want& want);
    void Print(std::string msg);
    int32_t FinishUserTest();

private:
    std::string ConvertJstringToString(std::shared_ptr<JNIEnv> env, jclass cls, jstring jstr);
    static std::shared_ptr<ApplicationContextAdapter> instance_;
    static std::mutex mutex_;
    Ace::Platform::JniEnvironment::JavaGlobalRef object_;
    jmethodID getRunningProcessInfoMethod_ = nullptr;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_APPLICATION_CONTEXT_ADAPTER_H
