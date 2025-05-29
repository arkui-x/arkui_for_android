/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_LOG_INTERFACE_JNI_H
#define FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_LOG_INTERFACE_JNI_H

#include <atomic>
#include <jni.h>
#include <shared_mutex>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
extern std::shared_mutex g_logInterfaceJniLock;
struct LogInterface {
    jobject logger;
    jmethodID d;
    jmethodID i;
    jmethodID w;
    jmethodID e;
    jmethodID f;
};
class LogInterfaceJni final {
public:
    LogInterfaceJni() = delete;
    ~LogInterfaceJni() = delete;

    // JAVA call C++
    static void SetLogger(JNIEnv* env, jobject jobjLogger);

    // C++ call JAVA
    static void PassLogMessage(const int32_t level, const std::string& domain, const std::string& newFmt);
    static LogInterface logInterface_;
};

void StartLogProcessingThread();
void StopLogProcessingThread();
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_LOG_INTERFACE_JNI_H
