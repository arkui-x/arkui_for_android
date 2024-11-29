/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ACCESSIBILITY_EVENT_INFO_H
#define ACCESSIBILITY_EVENT_INFO_H

#include <vector>

#include "accessibility_definitions.h"
#include "accessibility_element_info.h"

namespace OHOS {
namespace Accessibility {
class AccessibilityMemo {
public:
    AccessibilityMemo() {}

    void SetSource(const int64_t componentId);

    int64_t GetViewId() const;

    int64_t GetAccessibilityId() const;

    void SetWindowId(const int32_t windowId);

    int32_t GetWindowId() const;

    int32_t GetCurrentIndex() const;

    void SetCurrentIndex(const int32_t index);

    int32_t GetBeginIndex() const;

    void SetBeginIndex(const int32_t index);

    int32_t GetItemCounts() const;

    void SetItemCounts(const int32_t itemCounts);

    int32_t GetEndIndex() const;

    void SetEndIndex(const int32_t index);

    const std::string& GetComponentType() const;

    void SetComponentType(const std::string& className);

    const std::string& GetBeforeText() const;

    void SetBeforeText(const std::string& beforeText);

    void AddContent(const std::string& content);

    const std::vector<std::string>& GetContentList() const;

    const std::string& GetLatestContent() const;

    void SetLatestContent(const std::string& content);

    const std::string& GetDescription() const;

    void SetDescription(const std::string& contentDescription);

protected:
    int64_t componentId_ = 0;
    int32_t windowId_ = 0;
    int32_t currentIndex_ = 0;
    int32_t beginIndex_ = 0;
    int32_t endIndex_ = 0;
    std::vector<std::string> contents_ {};
    std::string componentType_ = "";
    std::string description_ = "";
    std::string beforeText_ = "";
    std::string latestContent_ = "";
    int64_t elementId_ = 0;
    int32_t itemCounts_ = 0;
};

class AccessibilityEventInfo : public AccessibilityMemo {
public:
    AccessibilityEventInfo() {}

    explicit AccessibilityEventInfo(EventType eventType);

    AccessibilityEventInfo(int32_t windowId, WindowUpdateType windowChangeTypes);

    EventType GetEventType() const;

    WindowsContentChangeTypes GetWindowContentChangeTypes() const;

    void SetWindowContentChangeTypes(const WindowsContentChangeTypes changeTypes);

    WindowUpdateType GetWindowChangeTypes() const;

    void SetWindowChangeTypes(const WindowUpdateType changeTypes);

    void SetEventType(const EventType eventType);

    int64_t GetTimeStamp() const;

    void SetTimeStamp(int64_t timeStamp); // Remained:for send event timestamp

    const std::string& GetBundleName() const;

    void SetBundleName(const std::string& bundleName);

    const std::string& GetNotificationContent() const;

    void SetNotificationContent(const std::string& notificationContent);

    void SetTextMovementStep(const TextMoveUnit granularity);

    TextMoveUnit GetTextMovementStep() const;

    void SetTriggerAction(const ActionType action);

    ActionType GetTriggerAction() const;

    void SetNotificationInfo(const NotificationCategory category);

    NotificationCategory GetNotificationInfo() const;

    void SetGestureType(const GestureType gestureType);

    GestureType GetGestureType() const;

    void SetPageId(const int32_t pageId);

    int32_t GetPageId() const;

    void SetElementInfo(const AccessibilityElementInfo& elementInfo);

    const AccessibilityElementInfo& GetElementInfo() const;

    const std::string& GetTextAnnouncedForAccessibility() const;

    void SetTextAnnouncedForAccessibility(const std::string& textAnnouncedForAccessibility);

    const std::string& GetInspectorKey() const;

    void SetInspectorKey(const std::string& inspectorKey);

    void SetRequestFocusElementId(const int32_t requestFocusElementId);

    int32_t GetRequestFocusElementId() const;

protected:
    EventType eventType_ = TYPE_VIEW_INVALID;
    std::string bundleName_ = "";
    ActionType triggerAction_ = ACCESSIBILITY_ACTION_INVALID;
    int64_t timeStamp_ = 0;
    TextMoveUnit textMoveStep_ = STEP_CHARACTER;
    WindowsContentChangeTypes windowContentChangeTypes_ = CONTENT_CHANGE_TYPE_INVALID;
    WindowUpdateType windowChangeTypes_ = WINDOW_UPDATE_INVALID;
    GestureType gestureType_ = GESTURE_INVALID;
    NotificationCategory category_ = CATEGORY_INVALID;
    int32_t pageId_ = 0;
    std::string notificationContent_ = "";
    AccessibilityElementInfo elementInfo_ {};
    std::string textAnnouncedForAccessibility_ = "";
    std::string inspectorKey_ = "";
    int32_t requestFocusElementId_ = 0;
};
} // namespace Accessibility
} // namespace OHOS
#endif // ACCESSIBILITY_EVENT_INFO_H
