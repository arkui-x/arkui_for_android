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

#include "adapter/android/stage/uicontent/ui_event_monitor.h"

#include <chrono>
#include <thread>

#include "adapter/android/osal/js_accessibility_manager.h"
#include "base/log/log.h"

namespace OHOS::Ace::Platform {
UiEventMonitor::UiEventMonitor()
{
    ResetEventTimer();
}

UiEventMonitor::~UiEventMonitor() {}

void UiEventMonitor::Init()
{
    std::weak_ptr<UiEventMonitor> weakThis = shared_from_this();
    OHOS::Ace::Framework::JsAccessibilityManager::AddUiTestAccessibilityRequest();
    OHOS::Ace::Framework::JsAccessibilityManager::SetUiTestEventCallback(
        [weakThis](const OHOS::Accessibility::AccessibilityEventInfo& info) {
            if (auto monitor = weakThis.lock()) {
                monitor->OnAccessibilityEvent(info);
            }
        });
}

uint64_t UiEventMonitor::GetCurrentMillisecond()
{
    using namespace std::chrono;
    auto now = steady_clock::now();
    return static_cast<uint64_t>(duration_cast<milliseconds>(now.time_since_epoch()).count());
}

void UiEventMonitor::OnAccessibilityEvent(const OHOS::Accessibility::AccessibilityEventInfo& eventInfo)
{
    auto eventType = static_cast<uint32_t>(eventInfo.GetEventType());
    if (eventInfo.GetEventType() == OHOS::Accessibility::EventType::TYPE_VIEW_SCROLLED_START) {
        activeScrollCount_++;
    }
    if (eventInfo.GetEventType() == OHOS::Accessibility::EventType::TYPE_VIEW_SCROLLED_EVENT) {
        int32_t currentScrollCount = activeScrollCount_.load();
        while (currentScrollCount > 0) {
            if (activeScrollCount_.compare_exchange_weak(currentScrollCount, currentScrollCount - 1)) {
                break;
            }
        }
    }

    for (auto watchedType : EVENT_MASK) {
        if (eventType == watchedType) {
            lastEventMillis_.store(GetCurrentMillisecond());
            break;
        }
    }
}

bool UiEventMonitor::WaitEventIdle(uint32_t idleThresholdMs, uint32_t timeoutMs)
{
    uint64_t startMs = GetCurrentMillisecond();
    static constexpr auto sliceMs = 10;
    uint64_t currentMs = GetCurrentMillisecond();
    while (currentMs - startMs < timeoutMs) {
        if (activeScrollCount_.load() > 0) {
            lastEventMillis_.store(currentMs);
        }
        if (lastEventMillis_.load() <= 0) {
            lastEventMillis_.store(currentMs);
        }
        if (currentMs - lastEventMillis_.load() >= idleThresholdMs) {
            if (activeScrollCount_.load() == 0) {
                return true;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sliceMs));
        currentMs = GetCurrentMillisecond();
    }
    return false;
}

uint64_t UiEventMonitor::GetLastEventMillis()
{
    if (lastEventMillis_.load() == 0) {
        lastEventMillis_.store(GetCurrentMillisecond());
    }
    return lastEventMillis_.load();
}

void UiEventMonitor::ResetEventTimer()
{
    lastEventMillis_.store(GetCurrentMillisecond());
}
} // namespace OHOS::Ace::Platform
