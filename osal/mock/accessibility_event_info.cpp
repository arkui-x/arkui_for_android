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

#include "accessibility_event_info.h"

#include <cinttypes>

#include "hilog.h"

namespace OHOS {
namespace Accessibility {
void AccessibilityMemo::SetSource(const int64_t componentId)
{
    HILOG_DEBUG("componentId[%{public}" PRId64 "]", componentId);
    componentId_ = componentId;
    elementId_ = componentId;
}

int64_t AccessibilityMemo::GetViewId() const
{
    HILOG_DEBUG("componentId_[%{public}" PRId64 "]", componentId_);
    return componentId_;
}

int64_t AccessibilityMemo::GetAccessibilityId() const
{
    HILOG_DEBUG("called] elementId_[%{public}" PRId64 "]", elementId_);
    return elementId_;
}

void AccessibilityMemo::SetWindowId(const int32_t windowId)
{
    HILOG_DEBUG("windowId[%{public}d]", windowId);
    windowId_ = windowId;
}

int32_t AccessibilityMemo::GetWindowId() const
{
    HILOG_DEBUG("windowId_[%{public}d]", windowId_);
    return windowId_;
}

int32_t AccessibilityMemo::GetCurrentIndex() const
{
    HILOG_DEBUG("currentIndex_[%{public}d]", currentIndex_);
    return currentIndex_;
}

void AccessibilityMemo::SetCurrentIndex(const int32_t index)
{
    currentIndex_ = index;
    HILOG_DEBUG("currentIndex_[%{public}d]", currentIndex_);
}

int32_t AccessibilityMemo::GetBeginIndex() const
{
    HILOG_DEBUG("beginIndex_[%{public}d]", beginIndex_);
    return beginIndex_;
}

void AccessibilityMemo::SetBeginIndex(const int32_t index)
{
    beginIndex_ = index;
    HILOG_DEBUG("beginIndex_[%{public}d]", beginIndex_);
}

int32_t AccessibilityMemo::GetEndIndex() const
{
    HILOG_DEBUG("endIndex_[%{public}d]", endIndex_);
    return endIndex_;
}

void AccessibilityMemo::SetEndIndex(const int32_t index)
{
    endIndex_ = index;
    HILOG_DEBUG("endIndex_[%{public}d]", endIndex_);
}

const std::string& AccessibilityMemo::GetComponentType() const
{
    HILOG_DEBUG("componentType_[%{public}s]", componentType_.c_str());
    return componentType_;
}

void AccessibilityMemo::SetComponentType(const std::string& className)
{
    componentType_ = className;
    HILOG_DEBUG("componentType_[%{public}s]", componentType_.c_str());
}

const std::string& AccessibilityMemo::GetBeforeText() const
{
    HILOG_DEBUG("beforeText_[%{public}s]", beforeText_.c_str());
    return beforeText_;
}

void AccessibilityMemo::SetBeforeText(const std::string& beforeText)
{
    beforeText_ = beforeText;
    HILOG_DEBUG("beforeText_[%{public}s]", beforeText_.c_str());
}

void AccessibilityMemo::AddContent(const std::string& content)
{
    contents_.push_back(content);
    HILOG_DEBUG("content[%{public}s]", content.c_str());
}

const std::vector<std::string>& AccessibilityMemo::GetContentList() const
{
    return contents_;
}

const std::string& AccessibilityMemo::GetLatestContent() const
{
    HILOG_DEBUG("latestContent_[%{public}s]", latestContent_.c_str());
    return latestContent_;
}

void AccessibilityMemo::SetLatestContent(const std::string& content)
{
    latestContent_ = content;
    HILOG_DEBUG("latestContent_[%{public}s]", latestContent_.c_str());
}

const std::string& AccessibilityMemo::GetDescription() const
{
    HILOG_DEBUG("description_[%{public}s]", description_.c_str());
    return description_;
}

void AccessibilityMemo::SetDescription(const std::string& contentDescription)
{
    description_ = contentDescription;
    HILOG_DEBUG("description_[%{public}s]", description_.c_str());
}

void AccessibilityMemo::SetItemCounts(const int32_t itemCounts)
{
    itemCounts_ = itemCounts;
    HILOG_DEBUG("itemCounts_[%{public}d]", itemCounts_);
}

int32_t AccessibilityMemo::GetItemCounts() const
{
    HILOG_DEBUG("itemCounts_[%{public}d]", itemCounts_);
    return itemCounts_;
}

EventType AccessibilityEventInfo::GetEventType() const
{
    HILOG_DEBUG("eventType_[%{public}d]", eventType_);
    return eventType_;
}

WindowsContentChangeTypes AccessibilityEventInfo::GetWindowContentChangeTypes() const
{
    HILOG_DEBUG("windowType_[%{public}d]", windowContentChangeTypes_);
    return windowContentChangeTypes_;
}

void AccessibilityEventInfo::SetWindowContentChangeTypes(const WindowsContentChangeTypes changeTypes)
{
    windowContentChangeTypes_ = changeTypes;
    HILOG_DEBUG("windowType_[%{public}d]", windowContentChangeTypes_);
}

WindowUpdateType AccessibilityEventInfo::GetWindowChangeTypes() const
{
    HILOG_DEBUG("windowType_[%{public}d]", windowChangeTypes_);
    return windowChangeTypes_;
}

void AccessibilityEventInfo::SetWindowChangeTypes(const WindowUpdateType changeTypes)
{
    windowChangeTypes_ = changeTypes;
    HILOG_DEBUG("windowType_[%{public}d]", windowChangeTypes_);
}

void AccessibilityEventInfo::SetEventType(const EventType eventType)
{
    eventType_ = EventType(eventType);
    HILOG_DEBUG("eventType_[%{public}d]", eventType_);
}

int64_t AccessibilityEventInfo::GetTimeStamp() const
{
    HILOG_DEBUG("timeStamp:%{public}" PRId64 "", timeStamp_);
    return timeStamp_;
}

void AccessibilityEventInfo::SetTimeStamp(int64_t timeStamp)
{
    timeStamp_ = timeStamp;
    HILOG_DEBUG("timeStamp:%{public}" PRId64 "", timeStamp_);
}

const std::string& AccessibilityEventInfo::GetBundleName() const
{
    HILOG_DEBUG("bundleName_[%{public}s]", bundleName_.c_str());
    return bundleName_;
}

void AccessibilityEventInfo::SetBundleName(const std::string& bundleName)
{
    HILOG_DEBUG("previous bundleName[%{public}s] new bundleName[%{public}s]", bundleName_.c_str(), bundleName.c_str());
    bundleName_ = bundleName;
}

const std::string& AccessibilityEventInfo::GetNotificationContent() const
{
    HILOG_DEBUG("notificationContent_[%{public}s]", notificationContent_.c_str());
    return notificationContent_;
}

void AccessibilityEventInfo::SetNotificationContent(const std::string& notificationContent)
{
    notificationContent_ = notificationContent;
    HILOG_DEBUG("notificationContent_[%{public}s]", notificationContent_.c_str());
}

const std::string& AccessibilityEventInfo::GetTextAnnouncedForAccessibility() const
{
    HILOG_DEBUG("textAnnouncedForAccessibility_[%{public}s]", textAnnouncedForAccessibility_.c_str());
    return textAnnouncedForAccessibility_;
}

void AccessibilityEventInfo::SetTextAnnouncedForAccessibility(const std::string& textAnnouncedForAccessibility)
{
    textAnnouncedForAccessibility_ = textAnnouncedForAccessibility;
    HILOG_DEBUG("textAnnouncedForAccessibility_[%{public}s]", textAnnouncedForAccessibility_.c_str());
}

const std::string& AccessibilityEventInfo::GetInspectorKey() const
{
    HILOG_DEBUG("inspectorKey_[%{public}s]", inspectorKey_.c_str());
    return inspectorKey_;
}

void AccessibilityEventInfo::SetInspectorKey(const std::string& inspectorKey)
{
    inspectorKey_ = inspectorKey;
    HILOG_DEBUG("inspectorKey_[%{public}s]", inspectorKey_.c_str());
}

void AccessibilityEventInfo::SetTextMovementStep(const TextMoveUnit granularity)
{
    textMoveStep_ = granularity;
    HILOG_DEBUG("textMoveStep_[%{public}d]", textMoveStep_);
}

TextMoveUnit AccessibilityEventInfo::GetTextMovementStep() const
{
    HILOG_DEBUG("textMoveStep_[%{public}d]", textMoveStep_);
    return textMoveStep_;
}

void AccessibilityEventInfo::SetTriggerAction(const ActionType action)
{
    triggerAction_ = action;
    HILOG_DEBUG("triggerAction_[%{public}d]", triggerAction_);
}

ActionType AccessibilityEventInfo::GetTriggerAction() const
{
    HILOG_DEBUG("triggerAction_[%{public}d]", triggerAction_);
    return triggerAction_;
}

AccessibilityEventInfo::AccessibilityEventInfo(int32_t windowId, WindowUpdateType windowChangeTypes)
    : eventType_(TYPE_WINDOW_UPDATE), windowChangeTypes_(windowChangeTypes)
{
    HILOG_DEBUG();
    SetWindowId(windowId);
}

AccessibilityEventInfo::AccessibilityEventInfo(EventType eventType) : eventType_(eventType)
{
    HILOG_DEBUG("eventType_[%{public}d]", eventType_);
}

void AccessibilityEventInfo::SetNotificationInfo(const NotificationCategory category)
{
    category_ = category;
    HILOG_DEBUG("category_[%{public}d]", category_);
}

void AccessibilityEventInfo::SetGestureType(const GestureType gestureType)
{
    gestureType_ = gestureType;
    HILOG_DEBUG("gestureType_[%{public}d]", gestureType_);
}

GestureType AccessibilityEventInfo::GetGestureType() const
{
    HILOG_DEBUG("gestureType_[%{public}d]", gestureType_);
    return gestureType_;
}

NotificationCategory AccessibilityEventInfo::GetNotificationInfo() const
{
    HILOG_DEBUG("category_[%{public}d]", category_);
    return category_;
}

void AccessibilityEventInfo::SetPageId(const int32_t pageId)
{
    HILOG_DEBUG("pageId[%{public}d]", pageId);
    pageId_ = pageId;
}

int32_t AccessibilityEventInfo::GetPageId() const
{
    HILOG_DEBUG("pageId_[%{public}d]", pageId_);
    return pageId_;
}

void AccessibilityEventInfo::SetElementInfo(const AccessibilityElementInfo& elementInfo)
{
    HILOG_DEBUG("set elementInfo");
    elementInfo_ = elementInfo;
}

const AccessibilityElementInfo& AccessibilityEventInfo::GetElementInfo() const
{
    HILOG_DEBUG("get elementInfo");
    return elementInfo_;
}

void AccessibilityEventInfo::SetRequestFocusElementId(const int32_t requestFocusElementId)
{
    HILOG_DEBUG("requestFocusElementId[%{public}d]", requestFocusElementId);
    requestFocusElementId_ = requestFocusElementId;
}

int32_t AccessibilityEventInfo::GetRequestFocusElementId() const
{
    HILOG_DEBUG("requestFocusElementId_[%{public}d]", requestFocusElementId_);
    return requestFocusElementId_;
}
} // namespace Accessibility
} // namespace OHOS