/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "accessibility_event_manager.h"

#include <map>

#include "core/accessibility/accessibility_manager.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/bridge/common/accessibility/accessibility_node_manager.h"

namespace OHOS::Ace::Framework {

AccessibilityEventType ConvertEventType(const std::string& type)
{
    static const std::map<std::string, AccessibilityEventType> typeMap = {
        { "requestFocusForAccessibility", AccessibilityEventType::HOVER_ENTER_EVENT },
        { "announceForAccessibility", AccessibilityEventType::ANNOUNCE_FOR_ACCESSIBILITY },
    };

    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    }
    return AccessibilityEventType::UNKNOWN;
}

int32_t GetID(const RefPtr<NG::UINode>& node, const std::string& customId)
{
    if (!node) {
        return -1;
    }
    if (node->GetInspectorId().value_or("") == customId) {
        return node->GetAccessibilityId();
    }
    const auto& children = node->GetChildren();
    for (auto& child : children) {
        int32_t nodeId = GetID(child, customId);
        if (nodeId != -1) {
            return nodeId;
        }
    }
    return -1;
}

int32_t ConvertCustomId(const std::string& customId, uint32_t windowId)
{
    auto container = Container::GetByWindowId(windowId);
    if (!container) {
        return -1;
    }
    auto context = container->GetPipelineContext();
    auto pipelineContext = AceType::DynamicCast<NG::PipelineContext>(context);
    if (!pipelineContext) {
        return -1;
    }
    auto root = pipelineContext->GetRootElement();
    return GetID(root, customId);
}

WindowUpdateType ConvertWindowUpdateType(const std::string& type)
{
    static const std::map<std::string, WindowUpdateType> typeMap = {
        { "add", WindowUpdateType::WINDOW_UPDATE_ADDED },
        { "remove", WindowUpdateType::WINDOW_UPDATE_REMOVED },
        { "bounds", WindowUpdateType::WINDOW_UPDATE_BOUNDS },
        { "active", WindowUpdateType::WINDOW_UPDATE_ACTIVE },
        { "focus", WindowUpdateType::WINDOW_UPDATE_FOCUSED },
    };

    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    }
    return WindowUpdateType::WINDOW_UPDATE_INVALID;
}

void AccessibilityEventManager::SendEventInfo(const AccessibilityEventInfo& event)
{
    auto nodeManager = AccessibilityNodeManager::Create();
    if (nodeManager) {
        auto container = Container::GetActive();
        if (!container) {
            return;
        }
        auto pipelineContext = container->GetPipelineContext();
        if (!pipelineContext) {
            return;
        }
        auto windowId = pipelineContext->GetWindowId();
        AccessibilityEvent accessibilityEvent;
        accessibilityEvent.type = ConvertEventType(event.type);
        accessibilityEvent.windowChangeTypes = ConvertWindowUpdateType(event.windowUpdateType);
        accessibilityEvent.componentType = event.componentType;
        accessibilityEvent.windowId = windowId;
        accessibilityEvent.latestContent = event.lastContent;
        accessibilityEvent.startIndex = event.beginIndex;
        accessibilityEvent.currentItemIndex = event.currentIndex;
        accessibilityEvent.endIndex = event.endIndex;
        accessibilityEvent.itemCount = event.itemCount;
        accessibilityEvent.textAnnouncedForAccessibility = event.textAnnouncedForAccessibility;
        accessibilityEvent.stackNodeId = event.elementId;
        accessibilityEvent.nodeId = ConvertCustomId(event.customId, windowId);
        accessibilityEvent.extraEventInfo["sendEventType"] = "pluginsEvent";
        nodeManager->SendAccessibilityAsyncEvent(accessibilityEvent);
    }
}

} // namespace OHOS::Ace::Framework
