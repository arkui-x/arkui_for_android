/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "core/common/vibrator/vibrator_utils.h"

#include <mutex>

#include "adapter/android/capability/java/jni/vibrator/vibrator_controller_impl.h"
#include "base/log/log.h"
#include "frameworks/core/common/container.h"
#include "ui/base/utils/utils.h"

namespace {
constexpr int32_t DEFAULT_VIBRATE_DURATION_MS = 10;

std::unique_ptr<OHOS::Ace::Platform::VibratorController> g_vibratorController = nullptr;
std::mutex g_vibratorControllerMutex;

bool InitVibratorController()
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor == nullptr) {
        LOGE("VibratorUtils: taskExecutor is null, skip vibrator controller initialization");
        return false;
    }

    std::lock_guard<std::mutex> lock(g_vibratorControllerMutex);
    if (g_vibratorController == nullptr) {
        g_vibratorController = std::make_unique<OHOS::Ace::Platform::VibratorController>(taskExecutor);
    }

    return g_vibratorController != nullptr;
}
} // namespace

namespace OHOS::Ace::NG {
void VibratorUtils::StartVibraFeedback()
{
    if (InitVibratorController()) {
        std::lock_guard<std::mutex> lock(g_vibratorControllerMutex);
        CHECK_NULL_VOID(g_vibratorController);
        g_vibratorController->Vibrate(DEFAULT_VIBRATE_DURATION_MS);
    }
}

void VibratorUtils::StartVibraFeedback(const std::string& vibratorType)
{
    CHECK_EQUAL_VOID(vibratorType.empty(), true);

    if (InitVibratorController()) {
        std::lock_guard<std::mutex> lock(g_vibratorControllerMutex);
        CHECK_NULL_VOID(g_vibratorController);
        g_vibratorController->Vibrate(vibratorType);
    }
}

bool VibratorUtils::StartExclusiveVibraFeedback(const char* effectId)
{
    CHECK_NULL_RETURN(effectId, false);

    bool result = false;
    if (InitVibratorController()) {
        std::lock_guard<std::mutex> lock(g_vibratorControllerMutex);
        CHECK_NULL_RETURN(g_vibratorController, false);

        g_vibratorController->Vibrate(std::string(effectId));
        result = true;
    }
    return result;
}

void VibratorUtils::StartViratorDirectly(const std::string& vibratorType)
{
    CHECK_EQUAL_VOID(vibratorType.empty(), true);

    if (InitVibratorController()) {
        std::lock_guard<std::mutex> lock(g_vibratorControllerMutex);
        CHECK_NULL_VOID(g_vibratorController);
        g_vibratorController->Vibrate(vibratorType);
    }
}
} // namespace OHOS::Ace::NG
