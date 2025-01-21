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

#include "accessibility_type_convertor.h"

using namespace OHOS::Accessibility;

namespace OHOS::Ace::Framework {
const char ACCESSIBILITY_FOCUSED_EVENT[] = "accessibilityfocus";
const char ACCESSIBILITY_CLEAR_FOCUS_EVENT[] = "accessibilityclearfocus";
const char TEXT_CHANGE_EVENT[] = "textchange";
const char PAGE_CHANGE_EVENT[] = "pagechange";
const char SCROLL_END_EVENT[] = "scrollend";
const char SCROLL_START_EVENT[] = "scrollstart";
const char MOUSE_HOVER_ENTER[] = "mousehoverenter";
const char MOUSE_HOVER_EXIT[] = "mousehoverexit";
const char ACE_COMPONENT_CHECKBOX[] = "Checkbox";
const char ACE_COMPONENT_CHECKBOXGROUP[] = "CheckboxGroup";
const char ACE_COMPONENT_GAUGE[] = "Gauge";
const char ACE_COMPONENT_MARQUEE[] = "Marquee";
const char ACE_COMPONENT_MENUITEM[] = "MenuItem";
const char ACE_COMPONENT_MENUITEMGROUP[] = "MenuItemGroup";
const char ACE_COMPONENT_MENU[] = "Menu";
const char ACE_COMPONENT_NAVIGATION[] = "Navigation";
const char ACE_COMPONENT_DATEPICKER[] = "DatePicker";
const char ACE_COMPONENT_PROGRESS[] = "Progress";
const char ACE_COMPONENT_RADIO[] = "Radio";
const char ACE_COMPONENT_RATING[] = "Rating";
const char ACE_COMPONENT_SCROLLBAR[] = "ScrollBar";
const char ACE_COMPONENT_SELECT[] = "Select";
const char ACE_COMPONENT_SLIDER[] = "Slider";
const char ACE_COMPONENT_STEPPER[] = "Stepper";
const char ACE_COMPONENT_TEXT[] = "Text";
const char ACE_COMPONENT_TEXTCLOCK[] = "TextClock";
const char ACE_COMPONENT_TEXTPICKER[] = "TextPicker";
const char ACE_COMPONENT_TEXTTIMER[] = "TextTimer";
const char ACE_COMPONENT_TIMEPICKER[] = "TimePicker";
const char ACE_COMPONENT_TOGGLE[] = "Toggle";
const char ACE_COMPONENT_WEB[] = "Web";

static const ActionTable ACTIONTABLE[] = {
    { AceAction::ACTION_CLICK, AndroidActionType::ACTION_CLICK, ActionType::ACCESSIBILITY_ACTION_CLICK },
    { AceAction::ACTION_LONG_CLICK, AndroidActionType::ACTION_LONG_CLICK, ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
    { AceAction::ACTION_SCROLL_FORWARD, AndroidActionType::ACTION_SCROLL_FORWARD,
        ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
    { AceAction::ACTION_SCROLL_BACKWARD, AndroidActionType::ACTION_SCROLL_BACKWARD,
        ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
    { AceAction::ACTION_FOCUS, AndroidActionType::ACTION_FOCUS, ActionType::ACCESSIBILITY_ACTION_FOCUS },
    { AceAction::ACTION_CLEAR_FOCUS, AndroidActionType::ACTION_CLEAR_FOCUS,
        ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
    { AceAction::ACTION_ACCESSIBILITY_FOCUS, AndroidActionType::ACTION_ACCESSIBILITY_FOCUS,
        ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
    { AceAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS, AndroidActionType::ACTION_CLEAR_ACCESSIBILITY_FOCUS,
        ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
    { AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY, AndroidActionType::ACTION_NEXT_AT_MOVEMENT_GRANULARITY,
        ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
    { AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY, AndroidActionType::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY,
        ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
    { AceAction::ACTION_SET_TEXT, AndroidActionType::ACTION_SET_TEXT, ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
    { AceAction::ACTION_COPY, AndroidActionType::ACTION_COPY, ActionType::ACCESSIBILITY_ACTION_COPY },
    { AceAction::ACTION_PASTE, AndroidActionType::ACTION_PASTE, ActionType::ACCESSIBILITY_ACTION_PASTE },
    { AceAction::ACTION_CUT, AndroidActionType::ACTION_CUT, ActionType::ACCESSIBILITY_ACTION_CUT },
    { AceAction::ACTION_SELECT, AndroidActionType::ACTION_SELECT, ActionType::ACCESSIBILITY_ACTION_SELECT },
    { AceAction::ACTION_CLEAR_SELECTION, AndroidActionType::ACTION_CLEAR_SELECTION,
        ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
    { AceAction::ACTION_SET_SELECTION, AndroidActionType::ACTION_SET_SELECTION,
        ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
};

static const char* ACE_COMPONENTS[] = { ACE_COMPONENT_CHECKBOX, ACE_COMPONENT_CHECKBOXGROUP, ACE_COMPONENT_GAUGE,
    ACE_COMPONENT_MARQUEE, ACE_COMPONENT_MENUITEM, ACE_COMPONENT_MENUITEMGROUP, ACE_COMPONENT_MENU,
    ACE_COMPONENT_NAVIGATION, ACE_COMPONENT_DATEPICKER, ACE_COMPONENT_PROGRESS, ACE_COMPONENT_RADIO,
    ACE_COMPONENT_RATING, ACE_COMPONENT_SCROLLBAR, ACE_COMPONENT_SELECT, ACE_COMPONENT_SLIDER, ACE_COMPONENT_STEPPER,
    ACE_COMPONENT_TEXT, ACE_COMPONENT_TEXTCLOCK, ACE_COMPONENT_TEXTPICKER, ACE_COMPONENT_TEXTTIMER,
    ACE_COMPONENT_TIMEPICKER, ACE_COMPONENT_TOGGLE, ACE_COMPONENT_WEB };

AndroidEventType ConvertStrToAndroidEventType(const std::string& type)
{
    static const LinearMapNode<AndroidEventType> eventTypeMap[] = {
        { ACCESSIBILITY_CLEAR_FOCUS_EVENT, AndroidEventType::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED },
        { ACCESSIBILITY_FOCUSED_EVENT, AndroidEventType::TYPE_VIEW_ACCESSIBILITY_FOCUSED },
        { DOM_CLICK, AndroidEventType::TYPE_VIEW_CLICKED },
        { DOM_FOCUS, AndroidEventType::TYPE_VIEW_FOCUSED },
        { DOM_LONG_PRESS, AndroidEventType::TYPE_VIEW_LONG_CLICKED },
        { MOUSE_HOVER_ENTER, AndroidEventType::TYPE_VIEW_HOVER_ENTER },
        { MOUSE_HOVER_EXIT, AndroidEventType::TYPE_VIEW_HOVER_EXIT },
        { PAGE_CHANGE_EVENT, AndroidEventType::TYPE_WINDOW_STATE_CHANGED },
        { SCROLL_END_EVENT, AndroidEventType::TYPE_VIEW_SCROLLED },
        { SCROLL_START_EVENT, AndroidEventType::TYPE_VIEW_TARGETED_BY_SCROLL },
        { DOM_SELECTED, AndroidEventType::TYPE_VIEW_SELECTED },
        { TEXT_CHANGE_EVENT, AndroidEventType::TYPE_VIEW_TEXT_CHANGED },
        { DOM_TOUCH_END, AndroidEventType::TYPE_TOUCH_INTERACTION_END },
        { DOM_TOUCH_START, AndroidEventType::TYPE_TOUCH_INTERACTION_START },
    };
    AndroidEventType eventType = AndroidEventType::TYPE_VIEW_INVALID;
    int64_t idx = BinarySearchFindIndex(eventTypeMap, ArraySize(eventTypeMap), type.c_str());
    if (idx >= 0) {
        eventType = eventTypeMap[idx].value;
    }
    return eventType;
}

AndroidEventType ConvertAndroidEventType(AccessibilityEventType type)
{
    static const LinearEnumMapNode<AccessibilityEventType, AndroidEventType> eventTypeMap[] = {
        { AccessibilityEventType::CLICK, AndroidEventType::TYPE_VIEW_CLICKED },
        { AccessibilityEventType::LONG_PRESS, AndroidEventType::TYPE_VIEW_LONG_CLICKED },
        { AccessibilityEventType::SELECTED, AndroidEventType::TYPE_VIEW_SELECTED },
        { AccessibilityEventType::FOCUS, AndroidEventType::TYPE_VIEW_FOCUSED },
        { AccessibilityEventType::TEXT_CHANGE, AndroidEventType::TYPE_VIEW_TEXT_CHANGED },
        { AccessibilityEventType::HOVER_ENTER_EVENT, AndroidEventType::TYPE_VIEW_HOVER_ENTER },
        { AccessibilityEventType::PAGE_CHANGE, AndroidEventType::TYPE_WINDOW_STATE_CHANGED },
        { AccessibilityEventType::HOVER_EXIT_EVENT, AndroidEventType::TYPE_VIEW_HOVER_EXIT },
        { AccessibilityEventType::CHANGE, AndroidEventType::TYPE_WINDOW_CONTENT_CHANGED },
        { AccessibilityEventType::COMPONENT_CHANGE, AndroidEventType::TYPE_VIEW_TEXT_CHANGED },
        { AccessibilityEventType::SCROLL_END, AndroidEventType::TYPE_VIEW_SCROLLED },
        { AccessibilityEventType::TEXT_SELECTION_UPDATE, AndroidEventType::TYPE_VIEW_TEXT_SELECTION_CHANGED },
        { AccessibilityEventType::ACCESSIBILITY_FOCUSED, AndroidEventType::TYPE_VIEW_ACCESSIBILITY_FOCUSED },
        { AccessibilityEventType::ACCESSIBILITY_FOCUS_CLEARED,
            AndroidEventType::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED },
        { AccessibilityEventType::TEXT_MOVE_UNIT, AndroidEventType::TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY },
        { AccessibilityEventType::SCROLL_START, AndroidEventType::TYPE_VIEW_TARGETED_BY_SCROLL },
        { AccessibilityEventType::PAGE_CLOSE, AndroidEventType::TYPE_PAGE_CLOSE },
        { AccessibilityEventType::ANNOUNCE_FOR_ACCESSIBILITY, AndroidEventType::TYPE_ANNOUNCEMENT },
        { AccessibilityEventType::PAGE_OPEN, AndroidEventType::TYPE_PAGE_OPEN },
    };
    AndroidEventType eventType = AndroidEventType::TYPE_VIEW_INVALID;
    int64_t idx = BinarySearchFindIndex(eventTypeMap, ArraySize(eventTypeMap), type);
    if (idx >= 0) {
        eventType = eventTypeMap[idx].value;
    }
    return eventType;
}

AndroidActionType AceActionConvertAndroid(AceAction aceAction)
{
    for (const auto& item : ACTIONTABLE) {
        if (aceAction == item.aceAction) {
            return item.androidActionType;
        }
    }
    return AndroidActionType::ACTION_INVALID;
}

AceAction AndroidConvertAceAction(AndroidActionType androidActionType)
{
    for (const auto& item : ACTIONTABLE) {
        if (androidActionType == item.androidActionType) {
            return item.aceAction;
        }
    }
    return AceAction::ACTION_NONE;
}

AndroidActionType ActionTypeConvertAndroid(ActionType actionType)
{
    for (const auto& item : ACTIONTABLE) {
        if (actionType == item.actionType) {
            return item.androidActionType;
        }
    }
    return AndroidActionType::ACTION_INVALID;
}

AndroidMovementGranularity TextMoveUnitConvertAndroid(TextMoveUnit moveUnit)
{
    static const TextMoveUnitAndroidTable moveTable[] = {
        { TextMoveUnit::STEP_CHARACTER, AndroidMovementGranularity::MOVEMENT_GRANULARITY_CHARACTER },
        { TextMoveUnit::STEP_WORD, AndroidMovementGranularity::MOVEMENT_GRANULARITY_WORD },
        { TextMoveUnit::STEP_LINE, AndroidMovementGranularity::MOVEMENT_GRANULARITY_LINE },
        { TextMoveUnit::STEP_PAGE, AndroidMovementGranularity::MOVEMENT_GRANULARITY_PAGE },
        { TextMoveUnit::STEP_PARAGRAPH, AndroidMovementGranularity::MOVEMENT_GRANULARITY_PARAGRAPH },
    };
    for (const auto& item : moveTable) {
        if (moveUnit == item.moveUnit) {
            return item.granularity;
        }
    }
    return AndroidMovementGranularity::MOVEMENT_GRANULARITY_INVALID;
}

bool IsComponentInArray(const std::string& componentType)
{
    const auto NUM_COMPONENTS = sizeof(ACE_COMPONENTS) / sizeof(ACE_COMPONENTS[0]);
    for (auto index = 0; index < NUM_COMPONENTS; ++index) {
        if (ACE_COMPONENTS[index] == componentType) {
            return true;
        }
    }
    return false;
}
} // namespace OHOS::Ace::Framework