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

#include "adapter/android/entrance/java/jni/picker/picker_haptic_controller.h"

#include <algorithm>

#include "frameworks/core/common/container.h"

namespace OHOS::Ace::NG {
namespace {
using std::chrono_literals::operator""s;
using std::chrono_literals::operator""ms;
const std::string AUDIO_URI = "arkui-x/systemres/resources/base/media/timepicker.wav";
const std::string EFFECT_ID_NAME = "haptic.slide";
constexpr std::chrono::milliseconds DEFAULT_DELAY(40);
constexpr std::chrono::milliseconds EXTENDED_DELAY(50);
constexpr float HAPTIC_INTENSITY_BASE = 50.f;
constexpr float HAPTIC_INTENSITY_MAX = 98.f;
constexpr float HAPTIC_INTENSITY_MIN = 50.f;
constexpr float HAPTIC_SPEED_FACTOR = 0.01f;
constexpr size_t SPEED_MAX = 5000;
constexpr size_t SPEED_PLAY_ONCE = 0;
constexpr size_t SPEED_THRESHOLD = 1560;
constexpr size_t TREND_COUNT = 3;
} // namespace

PickerHapticController::PickerHapticController(const std::string& uri, const std::string& effectId) noexcept
{
    std::string effectiveUri = uri.empty() ? AUDIO_URI : uri;
    effectiveEffectId_ = effectId.empty() ? EFFECT_ID_NAME : effectId;
    audioHapticPlayer_ = std::make_unique<Platform::AudioHapticPlayerImpl>();
    audioHapticPlayer_->Preload(effectiveUri);
    auto taskExecutor = Container::CurrentTaskExecutor();
    if (taskExecutor) {
        vibratorController_ = std::make_unique<Platform::VibratorController>(taskExecutor);
    }
    InitPlayThread();
}

PickerHapticController::~PickerHapticController() noexcept
{
    ThreadRelease();
    if (audioHapticPlayer_) {
        audioHapticPlayer_->Release();
    }
}

void PickerHapticController::ThreadRelease()
{
    if (playThread_) {
        {
            std::lock_guard<std::recursive_mutex> guard(threadMutex_);
            playThreadStatus_ = ThreadStatus::NONE;
        }
        threadCv_.notify_one();
        playThread_->join();
        playThread_.reset();
    }
}

bool PickerHapticController::IsThreadReady()
{
    std::lock_guard<std::recursive_mutex> guard(threadMutex_);
    return playThreadStatus_ == ThreadStatus::READY;
}

bool PickerHapticController::IsThreadPlaying()
{
    std::lock_guard<std::recursive_mutex> guard(threadMutex_);
    return playThreadStatus_ == ThreadStatus::PLAYING;
}

bool PickerHapticController::IsThreadPlayOnce()
{
    std::lock_guard<std::recursive_mutex> guard(threadMutex_);
    return playThreadStatus_ == ThreadStatus::PLAY_ONCE;
}

bool PickerHapticController::IsThreadNone()
{
    std::lock_guard<std::recursive_mutex> guard(threadMutex_);
    return playThreadStatus_ == ThreadStatus::NONE;
}

void PickerHapticController::InitPlayThread()
{
    ThreadRelease();
    playThreadStatus_ = ThreadStatus::START;
    playThread_ = std::make_unique<std::thread>(&PickerHapticController::ThreadLoop, this);
    if (playThread_) {
        playThreadStatus_ = ThreadStatus::READY;
    } else {
        playThreadStatus_ = ThreadStatus::NONE;
    }
}

void PickerHapticController::ThreadLoop()
{
    while (!IsThreadNone()) {
        {
            std::unique_lock<std::recursive_mutex> lock(threadMutex_);
            threadCv_.wait(lock, [this]() { return IsThreadPlaying() || IsThreadPlayOnce() || IsThreadNone(); });
            if (IsThreadNone()) {
                return;
            }
        }
        isInHapticLoop_ = true;
        if (audioHapticPlayer_) {
            float volume = audioHapticPlayer_->CalculateVolumeBySpeed(absSpeedInMm_, SPEED_THRESHOLD, SPEED_MAX);
            audioHapticPlayer_->Play(volume);
            float haptic = absSpeedInMm_ * HAPTIC_SPEED_FACTOR + HAPTIC_INTENSITY_BASE;
            haptic = std::clamp(haptic, HAPTIC_INTENSITY_MIN, HAPTIC_INTENSITY_MAX);
            if (vibratorController_) {
                vibratorController_->Vibrate(effectiveEffectId_, haptic);
            }
        }

        {
            auto startTime = std::chrono::high_resolution_clock::now();
            std::unique_lock<std::recursive_mutex> lock(threadMutex_);
            std::chrono::milliseconds delayTime = DEFAULT_DELAY;
            if (IsThreadPlayOnce() && isLoopReadyToStop_) {
                delayTime = EXTENDED_DELAY;
            }
            threadCv_.wait_until(lock, startTime + delayTime);
            if (IsThreadPlayOnce() || isLoopReadyToStop_) {
                playThreadStatus_ = ThreadStatus::READY;
            }
        }
        isInHapticLoop_ = false;
    }
}

void PickerHapticController::Play(size_t speed)
{
    if (!playThread_) {
        InitPlayThread();
    }
    bool needNotify = !IsThreadPlaying() && !IsThreadPlayOnce();
    {
        std::lock_guard<std::recursive_mutex> guard(threadMutex_);
        absSpeedInMm_ = speed;
        playThreadStatus_ = ThreadStatus::PLAYING;
    }
    if (needNotify) {
        threadCv_.notify_one();
    }
}

void PickerHapticController::PlayOnce()
{
    if (IsThreadPlaying()) {
        return;
    }
    if (!playThread_) {
        InitPlayThread();
    }

    bool needNotify = !IsThreadPlaying() && !IsThreadPlayOnce();
    {
        std::lock_guard<std::recursive_mutex> guard(threadMutex_);
        playThreadStatus_ = ThreadStatus::PLAY_ONCE;
        absSpeedInMm_ = SPEED_PLAY_ONCE;
    }
    if (needNotify) {
        threadCv_.notify_one();
    }
}

void PickerHapticController::Stop()
{
    {
        std::lock_guard<std::recursive_mutex> guard(threadMutex_);
        playThreadStatus_ = ThreadStatus::READY;
    }
    threadCv_.notify_one();
    lastHandleDeltaTime_ = 0;
}

void PickerHapticController::HandleDelta(double dy)
{
    uint64_t currentTime = GetMilliseconds();
    uint64_t intervalTime = currentTime - lastHandleDeltaTime_;
    CHECK_EQUAL_VOID(intervalTime, 0);

    lastHandleDeltaTime_ = currentTime;
    auto scrollSpeed = std::abs(ConvertPxToMillimeters(dy) / intervalTime) * 1000;
    if (scrollSpeed > SPEED_MAX) {
        scrollSpeed = SPEED_MAX;
    }
    recentSpeeds_.push_back(scrollSpeed);
    if (recentSpeeds_.size() > TREND_COUNT) {
        recentSpeeds_.pop_front();
    }

    if (!isInHapticLoop_ && isLoopReadyToStop_) {
        isLoopReadyToStop_ = false;
        playThreadStatus_ = ThreadStatus::READY;
        PlayOnce();
    } else if (isHapticCanLoopPlay_ && GetPlayStatus() == 1) {
        Play(scrollSpeed);
    } else if (GetPlayStatus() == -1 && IsThreadPlaying() && !isLoopReadyToStop_) {
        isLoopReadyToStop_ = true;
        isHapticCanLoopPlay_ = false;
        recentSpeeds_.clear();
        absSpeedInMm_ = scrollSpeed;
    }
}

double PickerHapticController::ConvertPxToMillimeters(double px) const
{
    auto& manager = ScreenSystemManager::GetInstance();
    const double density = manager.GetDensity();
    return density == 0.0 ? 0.0 : (px / density);
}

size_t PickerHapticController::GetCurrentSpeedInMm()
{
    double velocityInPixels = velocityTracker_.GetVelocity().GetVelocityY();
    return std::abs(ConvertPxToMillimeters(velocityInPixels));
}

int8_t PickerHapticController::GetPlayStatus()
{
    if (recentSpeeds_.size() < TREND_COUNT) {
        return 0;
    }
    bool allAbove = true;
    bool allBelow = true;
    for (size_t i = 0; i < TREND_COUNT; ++i) {
        const double speed = recentSpeeds_[i];
        if (speed <= SPEED_THRESHOLD) {
            allAbove = false;
        }
        if (speed >= SPEED_THRESHOLD) {
            allBelow = false;
        }
    }
    return allAbove ? 1 : (allBelow ? -1 : 0);
}
} // namespace OHOS::Ace::NG
