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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JAVA_EVENT_CALLBACK_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JAVA_EVENT_CALLBACK_H

#include <memory>
#include <string>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/utils/macros.h"

namespace OHOS::Ace::Platform {
class ACE_EXPORT JavaEventCallback {
public:
    explicit JavaEventCallback(jobject object);
    ~JavaEventCallback() = default;
    bool Initialize();
    bool OnEvent(int32_t pageId, const std::string& eventId, const std::string& param, std::string& result) const;
    void OnFinish() const;
    void OnStatusBarBgColorChanged(uint32_t color);

private:
    std::shared_ptr<JNIEnv> env_;
    JniEnvironment::JavaGlobalRef object_;
    jmethodID onEventMethodId_ = nullptr;
    jmethodID onFinishMethodId_ = nullptr;
    jmethodID onStatusBarBgColorMethodId_ = nullptr;
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JAVA_EVENT_CALLBACK_H