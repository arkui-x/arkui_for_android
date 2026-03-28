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

#include "adapter/android/capability/java/jni/vibrator/audio_haptic_player_impl.h"

#include <algorithm>
#include <cmath>

#include "adapter/android/capability/java/jni/vibrator/audio_haptic_player_jni.h"
#include "base/log/log.h"

namespace OHOS::Ace::Platform {
namespace {
constexpr float MIN_VOLUME = 0.6f;
constexpr float MAX_VOLUME_BASE = 0.6f;
constexpr float MAX_VOLUME_RANGE = 0.4f;
} // namespace

void AudioHapticPlayerImpl::Preload(const std::string& uri)
{
    AudioHapticPlayerJni::Preload(uri);
}

void AudioHapticPlayerImpl::Play(float volume)
{
    AudioHapticPlayerJni::Play(volume);
}

void AudioHapticPlayerImpl::Release()
{
    AudioHapticPlayerJni::Release();
}

float AudioHapticPlayerImpl::CalculateVolumeBySpeed(size_t speed, size_t speedThreshold, size_t speedMax)
{
    if (speedMax <= speedThreshold) {
        return MIN_VOLUME;
    }
    float userVolume = AudioHapticPlayerJni::GetStreamVolumeDb();
    float maxVolume = MAX_VOLUME_BASE + MAX_VOLUME_RANGE * userVolume;
    float volume = MIN_VOLUME + (maxVolume - MIN_VOLUME) *
                                    ((static_cast<float>(speed) - speedThreshold) / (speedMax - speedThreshold));
    return std::clamp(volume, 0.6f, 1.f);
}
} // namespace OHOS::Ace::Platform
