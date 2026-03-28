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

#include "adapter/android/capability/java/jni/vibrator/vibrator_controller_impl.h"

#include "adapter/android/capability/java/jni/vibrator/vibrator_controller_jni.h"

namespace OHOS::Ace::Platform {
VibratorController::VibratorController(const RefPtr<TaskExecutor>& taskExecutor) : taskExecutor_(taskExecutor) {}

void VibratorController::Vibrate(int32_t duration)
{
    if (taskExecutor_) {
        taskExecutor_->PostTask([duration] { VibratorControllerJni::Vibrate(duration); },
            TaskExecutor::TaskType::PLATFORM, "ArkUI-XVibratorControllerVibrateInt");
    }
}

void VibratorController::Vibrate(const std::string& effectId)
{
    if (taskExecutor_) {
        taskExecutor_->PostTask([effectId] { VibratorControllerJni::Vibrate(effectId); },
            TaskExecutor::TaskType::PLATFORM, "ArkUI-XVibratorControllerVibrateString");
    }
}

void VibratorController::Vibrate(const std::string& effectId, float intensity)
{
    if (taskExecutor_) {
        taskExecutor_->PostTask([effectId, intensity] { VibratorControllerJni::Vibrate(effectId, intensity); },
            TaskExecutor::TaskType::PLATFORM, "ArkUI-XVibratorControllerVibrateStringFloat");
    }
}
} // namespace OHOS::Ace::Platform
