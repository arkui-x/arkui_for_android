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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DOWNLOAD_MANAGER_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DOWNLOAD_MANAGER_JNI_H

#include <memory>

#include "jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"

namespace OHOS::Ace::Platform {

class DownloadManagerJni {
public:
    DownloadManagerJni() = delete;
    ~DownloadManagerJni() = delete;

    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static bool Download(const std::string& url, std::vector<uint8_t>& dataOut);

private:
    static Platform::JniEnvironment::JavaGlobalRef downloadManagerClass_;
    static jmethodID downloadMethod_;
};

}

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_DOWNLOAD_MANAGER_JNI_H