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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_ACCESSIBILITY_EVENT_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_ACCESSIBILITY_EVENT_MANAGER_H

#include <string>
#include <vector>
#include "foundation/arkui/ace_engine/interfaces/inner_api/ace_kit/include/ui/base/macros.h"

namespace OHOS::Ace::Framework {

struct AccessibilityAbilityInfo {
    std::string id;
    std::string name;
    std::string bundleName;
    std::vector<std::string> targetBundleNames;
    std::vector<std::string> abilityTypes;
    std::vector<std::string> capabilities;
    std::string description;
    std::vector<std::string> eventTypes;
    bool needHide = false;
    std::string label;
};

struct AccessibilityEventInfo {
    std::string type = "";
    std::string windowUpdateType = "";
    std::string bundleName = "";
    std::string componentType = "";
    std::string description = "";
    std::string triggerAction = "";
    std::string textMoveUnit = "";
    std::string lastContent = "";
    std::string textAnnouncedForAccessibility = "";
    std::string textResourceAnnouncedForAccessibility = "";
    std::string customId = "";
    std::vector<std::string> contents;
    int32_t pageId = 0;
    int32_t beginIndex = 0;
    int32_t currentIndex = 0;
    int32_t endIndex = 0;
    int32_t itemCount = 0;
    int32_t elementId = 0;
};
// Manager for dispatching accessibility events through the node manager
class ACE_EXPORT AccessibilityEventManager {
public:
    // Send an accessibility event via the AccessibilityNodeManager
    static void SendEventInfo(const AccessibilityEventInfo& event);
};
} // namespace OHOS::Ace::Framework
#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_ACCESSIBILITY_EVENT_MANAGER_H