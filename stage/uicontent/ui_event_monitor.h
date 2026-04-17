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

#ifndef ACE_ANDROID_UI_EVENT_MONITOR_H
#define ACE_ANDROID_UI_EVENT_MONITOR_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include "accessibility_event_info.h"

namespace OHOS::Accessibility {
class AccessibilityEventInfo;
}

namespace OHOS::Ace::Platform {
class UiEventMonitor : public std::enable_shared_from_this<UiEventMonitor> {
public:
    static std::shared_ptr<UiEventMonitor> Create()
    {
        struct MakeSharedEnabler : public UiEventMonitor {
            MakeSharedEnabler() : UiEventMonitor() {}
        };
        return std::make_shared<MakeSharedEnabler>();
    }

    ~UiEventMonitor();

    inline static const std::vector<uint32_t> EVENT_MASK = {
        static_cast<uint32_t>(OHOS::Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT),
        static_cast<uint32_t>(OHOS::Accessibility::EventType::TYPE_PAGE_STATE_UPDATE),
        static_cast<uint32_t>(OHOS::Accessibility::EventType::TYPE_PAGE_CONTENT_UPDATE),
        static_cast<uint32_t>(OHOS::Accessibility::EventType::TYPE_VIEW_SCROLLED_EVENT),
        static_cast<uint32_t>(OHOS::Accessibility::EventType::TYPE_WINDOW_UPDATE),
        static_cast<uint32_t>(OHOS::Accessibility::EventType::TYPE_PAGE_OPEN),
    };

    void OnAccessibilityEvent(const OHOS::Accessibility::AccessibilityEventInfo& eventInfo);

    bool WaitEventIdle(uint32_t idleThresholdMs, uint32_t timeoutMs);

    uint64_t GetLastEventMillis();

    void ResetEventTimer();

    void Init();

private:
    UiEventMonitor();

    static uint64_t GetCurrentMillisecond();

    std::atomic<uint64_t> lastEventMillis_ { 0 };
    std::atomic<int32_t> activeScrollCount_ { 0 };
};
} // namespace OHOS::Ace::Platform

#endif // ACE_ANDROID_UI_EVENT_MONITOR_H
