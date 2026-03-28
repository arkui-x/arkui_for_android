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

#ifndef FOUNDATION_ACE_ENGINE_ADAPTER_OHOS_ENTRANCE_PICKER_PICKER_AUDIO_HAPTIC_CONTROLLER_H
#define FOUNDATION_ACE_ENGINE_ADAPTER_OHOS_ENTRANCE_PICKER_PICKER_AUDIO_HAPTIC_CONTROLLER_H

#include <condition_variable>
#include <cmath>
#include <deque>
#include <mutex>
#include <thread>

#include "adapter/android/entrance/java/jni/picker/picker_haptic_interface.h"
#include "adapter/android/capability/java/jni/vibrator/audio_haptic_player_impl.h"
#include "adapter/android/capability/java/jni/vibrator/vibrator_controller_impl.h"
#include "core/components/common/layout/screen_system_manager.h"
#include "core/gestures/velocity_tracker.h"
#include "frameworks/base/memory/ace_type.h"

namespace OHOS::Ace::NG {

class PickerHapticController : public IPickerAudioHaptic {
public:
    enum class ThreadStatus {
        NONE,
        START,
        READY,
        PLAYING,
        PLAY_ONCE,
    };

    PickerHapticController(const std::string& uri, const std::string& effectId) noexcept;
    ~PickerHapticController() noexcept;
    void Play(size_t speed) override;
    void PlayOnce() override;
    void Stop() override;
    void HandleDelta(double dy) override;

private:
    void ThreadLoop();
    void ThreadRelease();
    void InitPlayThread();
    bool IsThreadReady();
    bool IsThreadPlaying();
    bool IsThreadPlayOnce();
    bool IsThreadNone();
    double ConvertPxToMillimeters(double px) const;
    size_t GetCurrentSpeedInMm();
    int8_t GetPlayStatus();

    bool isHapticCanLoopPlay_ = false;
    bool isInHapticLoop_ = false;
    bool isLoopReadyToStop_ = false;
    ThreadStatus playThreadStatus_ = ThreadStatus::NONE;
    std::recursive_mutex threadMutex_;
    std::condition_variable_any threadCv_;
    VelocityTracker velocityTracker_;
    int32_t effectSourceId_ = -1;
    size_t absSpeedInMm_ = 0;
    uint64_t  lastHandleDeltaTime_ = 0;
    std::deque<double> recentSpeeds_;
    std::string effectiveEffectId_;
    std::unique_ptr<std::thread> playThread_ = nullptr;
    std::unique_ptr<Platform::AudioHapticPlayerImpl> audioHapticPlayer_;
    std::unique_ptr<Platform::VibratorController> vibratorController_;
    ACE_DISALLOW_COPY_AND_MOVE(PickerHapticController);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_ENGINE_ADAPTER_OHOS_ENTRANCE_PICKER_PICKER_AUDIO_HAPTIC_CONTROLLER_H
