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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_CLIPBOARD_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_CLIPBOARD_JNI_H

#include <functional>
#include <memory>

#include "jni.h"

#include "base/thread/task_executor.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {

class ClipboardJni final {
public:
    static bool Register(std::shared_ptr<JNIEnv> env);

    static void NativeInit(JNIEnv* env, jobject object);

    static bool SetData(const std::string& data);

    static bool GetData(
        const std::function<void(const std::string&)>& callback, const WeakPtr<TaskExecutor>& taskExecutor);

    static bool HasData(const std::function<void(const bool)>& callback, const WeakPtr<TaskExecutor>& taskExecutor);

    static bool Clear();
private:
    ClipboardJni() = delete;
    ~ClipboardJni() = delete;

    static void OnJniRegistered();

    ACE_DISALLOW_COPY_AND_MOVE(ClipboardJni);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_CLIPBOARD_JNI_H