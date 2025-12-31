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

/**
 * @brief Accessibility ability information structure.
 * 
 * Contains metadata about an accessibility ability/service.
 */
struct AccessibilityAbilityInfo {
    std::string id;                            ///< Unique identifier for the ability
    std::string name;                          ///< Display name of the ability
    std::string bundleName;                    ///< Bundle name containing the ability
    std::vector<std::string> targetBundleNames; ///< Bundles this ability can access
    std::vector<std::string> abilityTypes;     ///< Types of accessibility abilities
    std::vector<std::string> capabilities;     ///< Capabilities of the ability
    std::string description;                   ///< Description of the ability
    std::vector<std::string> eventTypes;       ///< Event types the ability handles
    bool needHide = false;                     ///< Whether the ability should be hidden
    std::string label;                         ///< UI label for the ability
};

/**
 * @brief Accessibility event information structure.
 * 
 * Contains all data needed to construct and send an accessibility event.
 */
struct AccessibilityEventInfo {
    // Event type and categorization
    std::string type;                          ///< Type of accessibility event
    std::string windowUpdateType;              ///< Type of window update (if applicable)
    std::string bundleName;                    ///< Bundle name where event originated
    std::string componentType;                 ///< Type of UI component that generated the event
    std::string description;                   ///< Description of the event
    std::string triggerAction;                 ///< Action that triggered the event
    std::string textMoveUnit;                  ///< Unit for text navigation events
    
    // Content and text information
    std::string lastContent;                   ///< Previous content before change
    std::string textAnnouncedForAccessibility; ///< Text to be announced by screen reader
    std::string textResourceAnnouncedForAccessibility; ///< Resource ID for text to announce
    std::string customId;                      ///< Custom identifier for the element
    std::vector<std::string> contents;         ///< Additional content information
    
    // Navigation and position information
    int32_t pageId = -1;                       ///< Page identifier (-1 indicates invalid)
    int32_t beginIndex = -1;                   ///< Starting index for range operations
    int32_t currentIndex = -1;                 ///< Current position/index
    int32_t endIndex = -1;                     ///< Ending index for range operations
    int32_t itemCount = 0;                     ///< Total number of items
    int32_t elementId = -1;                    ///< Element identifier (-1 indicates invalid)
};

/**
 * @brief Manager for dispatching accessibility events.
 * 
 * This class provides a static interface for sending accessibility events
 * through the accessibility system. It acts as a bridge between UI components
 * and the underlying accessibility framework.
 */
class ACE_EXPORT AccessibilityEventManager {
public:
    /**
     * @brief Send an accessibility event.
     * 
     * Constructs and dispatches an accessibility event based on the provided
     * event information. The event will be sent asynchronously through the
     * AccessibilityNodeManager.
     * 
     * @param event The accessibility event information to send.
     */
    static void SendEventInfo(const AccessibilityEventInfo& event);
    
    // Note: The class is intentionally minimal with only static methods
    // to serve as a simple facade for accessibility event dispatch.
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_ACCESSIBILITY_EVENT_MANAGER_H