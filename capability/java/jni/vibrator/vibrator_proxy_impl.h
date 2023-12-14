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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_VIBRATOR_VIBRATOR_PROXY_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_VIBRATOR_VIBRATOR_PROXY_IMPL_H

#include "base/utils/noncopyable.h"
#include "core/common/vibrator/vibrator_interface.h"

namespace OHOS::Ace::Platform {

class VibratorProxyImpl final : public VibratorInterface {
public:
    VibratorProxyImpl() = default;
    ~VibratorProxyImpl() = default;

    RefPtr<Vibrator> GetVibrator(const RefPtr<TaskExecutor>& taskExecutor) override;

    ACE_DISALLOW_COPY_AND_MOVE(VibratorProxyImpl);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_VIBRATOR_VIBRATOR_PROXY_IMPL_H
