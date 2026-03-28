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

#ifndef FOUNDATION_ACE_ENGINE_ADAPTER_ANDROID_ENTRANCE_PICKER_PICKER_AUDIO_HAPTIC_FACTORY_H
#define FOUNDATION_ACE_ENGINE_ADAPTER_ANDROID_ENTRANCE_PICKER_PICKER_AUDIO_HAPTIC_FACTORY_H

#include <atomic>
#include <memory>
#include <mutex>

#include "adapter/android/entrance/java/jni/picker/picker_haptic_interface.h"
#include "interfaces/inner_api/ace_kit/include/ui/base/macros.h"

namespace OHOS::Ace::NG {
class ACE_FORCE_EXPORT PickerAudioHapticFactory {
public:
    PickerAudioHapticFactory() = delete;
    ~PickerAudioHapticFactory() = delete;
    static std::shared_ptr<IPickerAudioHaptic> GetInstance(
        const std::string& uri = "", const std::string& effectId = "");

private:
    static std::shared_ptr<IPickerAudioHaptic> instance_;
    static std::mutex mutex_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_ENGINE_ADAPTER_ANDROID_ENTRANCE_PICKER_PICKER_AUDIO_HAPTIC_FACTORY_H
