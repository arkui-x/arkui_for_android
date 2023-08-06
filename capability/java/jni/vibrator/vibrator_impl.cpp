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

#include "adapter/android/capability/java/jni/vibrator/vibrator_impl.h"

#include "adapter/android/capability/java/jni/vibrator/vibrator_jni.h"

namespace OHOS::Ace::Platform {

VibratorImpl::VibratorImpl(const RefPtr<TaskExecutor>& taskExecutor) : Vibrator(taskExecutor) {}

void VibratorImpl::Vibrate(int32_t duration)
{
    if (taskExecutor_) {
        taskExecutor_->PostTask([duration] { VibratorJni::Vibrate(duration); }, TaskExecutor::TaskType::PLATFORM);
    }
}

void VibratorImpl::Vibrate(const std::string& effectId)
{
    if (taskExecutor_) {
        taskExecutor_->PostTask([effectId] { VibratorJni::Vibrate(effectId); }, TaskExecutor::TaskType::PLATFORM);
    }
}


} // namespace OHOS::Ace::Platform
