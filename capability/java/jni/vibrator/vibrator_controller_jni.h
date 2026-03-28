/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef VIBRATOR_CONTROLLER_JNI_H
#define VIBRATOR_CONTROLLER_JNI_H

#include <memory>

#include "jni.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::Platform {

class VibratorControllerJni final {
public:
    static bool Register(std::shared_ptr<JNIEnv> env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void Vibrate(int32_t duration);
    static void Vibrate(const std::string& effectId);
    static void Vibrate(const std::string& effectId, float intensity);

private:
    VibratorControllerJni() = delete;
    ~VibratorControllerJni() = delete;
    ACE_DISALLOW_COPY_AND_MOVE(VibratorControllerJni);

    static void OnJniRegistered();
};
} // namespace OHOS::Ace::Platform
#endif // VIBRATOR_CONTROLLER_JNI_H
