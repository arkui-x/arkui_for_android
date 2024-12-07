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

#ifndef ACCESSIBILITY_TYPE_CONVERTOR_H
#define ACCESSIBILITY_TYPE_CONVERTOR_H

#include <cstdint>

#include "accessibility_definitions.h"
#include "base/utils/linear_map.h"
#include "base/utils/utils.h"
#include "core/accessibility/accessibility_utils.h"
#include "frameworks/bridge/common/dom/dom_type.h"

namespace OHOS::Ace::Framework {
enum class AndroidEventType : int32_t {
    TYPE_VIEW_INVALID = 0,
    TYPE_VIEW_CLICKED = 1 << 0,
    TYPE_VIEW_LONG_CLICKED = 1 << 1,
    TYPE_VIEW_SELECTED = 1 << 2,
    TYPE_VIEW_FOCUSED = 1 << 3,
    TYPE_VIEW_TEXT_CHANGED = 1 << 4,
    TYPE_WINDOW_STATE_CHANGED = 1 << 5,
    TYPE_NOTIFICATION_STATE_CHANGED = 1 << 6,
    TYPE_VIEW_HOVER_ENTER = 1 << 7,
    TYPE_VIEW_HOVER_EXIT = 1 << 8,
    TYPE_TOUCH_EXPLORATION_GESTURE_START = 1 << 9,
    TYPE_TOUCH_EXPLORATION_GESTURE_END = 1 << 10,
    TYPE_WINDOW_CONTENT_CHANGED = 1 << 11,
    TYPE_VIEW_SCROLLED = 1 << 12,
    TYPE_VIEW_TEXT_SELECTION_CHANGED = 1 << 13,
    TYPE_ANNOUNCEMENT = 1 << 14,
    TYPE_VIEW_ACCESSIBILITY_FOCUSED = 1 << 15,
    TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED = 1 << 16,
    TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY = 1 << 17,
    TYPE_GESTURE_DETECTION_START = 1 << 18,
    TYPE_GESTURE_DETECTION_END = 1 << 19,
    TYPE_TOUCH_INTERACTION_START = 1 << 20,
    TYPE_TOUCH_INTERACTION_END = 1 << 21,
    TYPE_WINDOWS_CHANGED = 1 << 22,
    TYPE_VIEW_CONTEXT_CLICKED = 1 << 23,
    TYPE_ASSIST_READING_CONTEXT = 1 << 24,
    TYPE_SPEECH_STATE_CHANGE = 1 << 25,
    TYPE_VIEW_TARGETED_BY_SCROLL = 1 << 26,
    //add extra
    TYPE_PAGE_OPEN = 2049,
    TYPE_PAGE_CLOSE = 2050,
};

enum class AndroidActionType : int32_t {
    ACTION_INVALID = 0,
    ACTION_FOCUS = 1 << 0,
    ACTION_CLEAR_FOCUS = 1 << 1,
    ACTION_SELECT = 1 << 2,
    ACTION_CLEAR_SELECTION = 1 << 3,
    ACTION_CLICK = 1 << 4,
    ACTION_LONG_CLICK = 1 << 5,
    ACTION_ACCESSIBILITY_FOCUS = 1 << 6,
    ACTION_CLEAR_ACCESSIBILITY_FOCUS = 1 << 7,
    ACTION_NEXT_AT_MOVEMENT_GRANULARITY = 1 << 8,
    ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY = 1 << 9,
    ACTION_NEXT_HTML_ELEMENT = 1 << 10,
    ACTION_PREVIOUS_HTML_ELEMENT = 1 << 11,
    ACTION_SCROLL_FORWARD = 1 << 12,
    ACTION_SCROLL_BACKWARD = 1 << 13,
    ACTION_COPY = 1 << 14,
    ACTION_PASTE = 1 << 15,
    ACTION_CUT = 1 << 16,
    ACTION_SET_SELECTION = 1 << 17,
    ACTION_EXPAND = 1 << 18,
    ACTION_COLLAPSE = 1 << 19,
    ACTION_DISMISS = 1 << 20,
    ACTION_SET_TEXT = 1 << 21,
};

enum class AndroidMovementGranularity : int32_t {
    MOVEMENT_GRANULARITY_INVALID = 0,
    MOVEMENT_GRANULARITY_CHARACTER = 1 << 0,
    MOVEMENT_GRANULARITY_WORD = 1 << 1,
    MOVEMENT_GRANULARITY_LINE = 1 << 2,
    MOVEMENT_GRANULARITY_PARAGRAPH = 1 << 3,
    MOVEMENT_GRANULARITY_PAGE = 1 << 4,
};

struct ActionTable {
    AceAction aceAction;
    AndroidActionType androidActionType;
    OHOS::Accessibility::ActionType actionType;
};

struct TextMoveUnitAndroidTable {
    OHOS::Accessibility::TextMoveUnit moveUnit;
    AndroidMovementGranularity granularity;
};

AndroidEventType ConvertStrToAndroidEventType(const std::string& type);
AndroidEventType ConvertAndroidEventType(AccessibilityEventType type);
AndroidMovementGranularity TextMoveUnitConvertAndroid(OHOS::Accessibility::TextMoveUnit moveUnit);
AndroidActionType AceActionConvertAndroid(AceAction aceAction);
AceAction AndroidConvertAceAction(AndroidActionType action);
AndroidActionType ActionTypeConvertAndroid(OHOS::Accessibility::ActionType actionType);
bool IsComponentInArray(const std::string& componentType);
} // namespace OHOS::Ace::Framework
#endif // ACCESSIBILITY_TYPE_CONVERTOR_H