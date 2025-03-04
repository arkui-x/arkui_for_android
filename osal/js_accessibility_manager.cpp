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

#include "js_accessibility_manager.h"

#include <algorithm>
#include <variant>

#include "accessibility_type_convertor.h"
#include "adapter/android/entrance/java/jni/js_accessibility_manager_jni.h"
#include "adapter/android/osal/mock/accessible_ability_operator_callback_impl.h"
#include "base/log/ace_trace.h"
#include "base/log/dump_log.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/linear_map.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/pipeline_context.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/bridge/common/dom/dom_type.h"
#include "frameworks/core/components_ng/pattern/ui_extension/ui_extension_manager.h"
#include "nlohmann/json.hpp"

using namespace std;
using namespace OHOS::Accessibility;
using Json = nlohmann::json;
using namespace OHOS::Ace::Platform;

namespace OHOS::Ace::Framework {
namespace {
const char DUMP_ORDER[] = "-accessibility";
const char DUMP_INSPECTOR[] = "-inspector";
const char ACCESSIBILITY_FOCUSED_EVENT[] = "accessibilityfocus";
const char ACCESSIBILITY_CLEAR_FOCUS_EVENT[] = "accessibilityclearfocus";
const char TEXT_CHANGE_EVENT[] = "textchange";
const char PAGE_CHANGE_EVENT[] = "pagechange";
const char SCROLL_END_EVENT[] = "scrollend";
const char SCROLL_START_EVENT[] = "scrollstart";
const char MOUSE_HOVER_ENTER[] = "mousehoverenter";
const char MOUSE_HOVER_EXIT[] = "mousehoverexit";
const char LIST_TAG[] = "List";
const char SIDEBARCONTAINER_TAG[] = "SideBarContainer";
const char STRING_DIR_FORWARD[] = "forward";
const char STRING_DIR_BACKWARD[] = "backward";
constexpr int32_t INVALID_PARENT_ID = -2100000;
constexpr int32_t DEFAULT_PARENT_ID = 2100000;
constexpr int32_t ROOT_STACK_BASE = 1100000;
constexpr int32_t ROOT_DECOR_BASE = 3100000;
constexpr int32_t CARD_NODE_ID_RATION = 10000;
constexpr int32_t CARD_ROOT_NODE_ID_RATION = 1000;
constexpr int32_t CARD_BASE = 100000;
const std::string ACTION_ARGU_SCROLL_STUB = "scrolltype";

struct ActionTable {
    AceAction aceAction;
    ActionType action;
};

struct FillEventInfoParam {
    int64_t elementId;
    int64_t stackNodeId;
    uint32_t windowId;
};

struct AccessibilityActionParam {
    RefPtr<NG::AccessibilityProperty> accessibilityProperty;
    std::string setTextArgument = "";
    int32_t setSelectionStart = -1;
    int32_t setSelectionEnd = -1;
    bool setSelectionDir = false;
    int32_t setCursorIndex = -1;
    TextMoveUnit moveUnit = TextMoveUnit::STEP_CHARACTER;
    AccessibilityScrollType scrollType = AccessibilityScrollType::SCROLL_DEFAULT;
};

struct FindAccessibilityFocusArg {
    int64_t uiExtensionOffset;
    int64_t currentFocusNodeId;
};

const std::map<Accessibility::ActionType, std::function<bool(const AccessibilityActionParam& param)>> ACTIONS = {
    { ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionScrollForward(param.scrollType);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionScrollBackward(param.scrollType);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SET_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionSetText(param.setTextArgument);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SET_SELECTION,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionSetSelection(param.setSelectionStart,
                                                                      param.setSelectionEnd, param.setSelectionDir);
        } },
    { ActionType::ACCESSIBILITY_ACTION_COPY,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionCopy(); } },
    { ActionType::ACCESSIBILITY_ACTION_CUT,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionCut(); } },
    { ActionType::ACCESSIBILITY_ACTION_PASTE,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionPaste(); } },
    { ActionType::ACCESSIBILITY_ACTION_CLICK,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionClick(); } },
    { ActionType::ACCESSIBILITY_ACTION_LONG_CLICK,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionLongClick(); } },
    { ActionType::ACCESSIBILITY_ACTION_SELECT,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionSelect(); } },
    { ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionClearSelection(); } },
    { ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionMoveText(static_cast<int32_t>(param.moveUnit), true);
        } },
    { ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionMoveText(static_cast<int32_t>(param.moveUnit), false);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionSetIndex(static_cast<int32_t>(param.setCursorIndex));
        } },
};

bool IsExtensionComponent(const RefPtr<NG::UINode>& node)
{
    return node && (node->GetTag() == V2::UI_EXTENSION_COMPONENT_ETS_TAG
        || node->GetTag() == V2::EMBEDDED_COMPONENT_ETS_TAG
        || node->GetTag() == V2::ISOLATED_COMPONENT_ETS_TAG);
}

bool IsIsolatedComponent(const RefPtr<NG::UINode>& node)
{
    return node && (node->GetTag() == V2::ISOLATED_COMPONENT_ETS_TAG);
}

bool IsUIExtensionShowPlaceholder(const RefPtr<NG::UINode>& node)
{
    CHECK_NULL_RETURN(node, true);
    if (node->GetTag() == V2::ISOLATED_COMPONENT_ETS_TAG) {
        return false;
    }

    return true;
}

Accessibility::EventType ConvertStrToEventType(const std::string& type)
{
    // static linear map must be sorted by key.
    static const LinearMapNode<Accessibility::EventType> eventTypeMap[] = {
        { ACCESSIBILITY_CLEAR_FOCUS_EVENT, Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED_EVENT },
        { ACCESSIBILITY_FOCUSED_EVENT, Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUSED_EVENT },
        { DOM_CLICK, Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT },
        { DOM_FOCUS, Accessibility::EventType::TYPE_VIEW_FOCUSED_EVENT },
        { DOM_LONG_PRESS, Accessibility::EventType::TYPE_VIEW_LONG_CLICKED_EVENT },
        { MOUSE_HOVER_ENTER, Accessibility::EventType::TYPE_VIEW_HOVER_ENTER_EVENT },
        { MOUSE_HOVER_EXIT, Accessibility::EventType::TYPE_VIEW_HOVER_EXIT_EVENT },
        { PAGE_CHANGE_EVENT, Accessibility::EventType::TYPE_PAGE_STATE_UPDATE },
        { SCROLL_END_EVENT, Accessibility::EventType::TYPE_VIEW_SCROLLED_EVENT },
        { SCROLL_START_EVENT, Accessibility::EventType::TYPE_VIEW_SCROLLED_START },
        { DOM_SELECTED, Accessibility::EventType::TYPE_VIEW_SELECTED_EVENT },
        { TEXT_CHANGE_EVENT, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { DOM_TOUCH_END, Accessibility::EventType::TYPE_TOUCH_END },
        { DOM_TOUCH_START, Accessibility::EventType::TYPE_TOUCH_BEGIN },
    };
    Accessibility::EventType eventType = Accessibility::EventType::TYPE_VIEW_INVALID;
    int64_t idx = BinarySearchFindIndex(eventTypeMap, ArraySize(eventTypeMap), type.c_str());
    if (idx >= 0) {
        eventType = eventTypeMap[idx].value;
    }
    return eventType;
}

Accessibility::EventType ConvertAceEventType(AccessibilityEventType type)
{
    static const LinearEnumMapNode<AccessibilityEventType, Accessibility::EventType> eventTypeMap[] = {
        { AccessibilityEventType::CLICK, Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT },
        { AccessibilityEventType::LONG_PRESS, Accessibility::EventType::TYPE_VIEW_LONG_CLICKED_EVENT },
        { AccessibilityEventType::SELECTED, Accessibility::EventType::TYPE_VIEW_SELECTED_EVENT },
        { AccessibilityEventType::FOCUS, Accessibility::EventType::TYPE_VIEW_FOCUSED_EVENT },
        { AccessibilityEventType::TEXT_CHANGE, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { AccessibilityEventType::HOVER_ENTER_EVENT, Accessibility::EventType::TYPE_VIEW_HOVER_ENTER_EVENT },
        { AccessibilityEventType::PAGE_CHANGE, Accessibility::EventType::TYPE_PAGE_STATE_UPDATE },
        { AccessibilityEventType::HOVER_EXIT_EVENT, Accessibility::EventType::TYPE_VIEW_HOVER_EXIT_EVENT },
        { AccessibilityEventType::CHANGE, Accessibility::EventType::TYPE_PAGE_CONTENT_UPDATE },
        { AccessibilityEventType::COMPONENT_CHANGE, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { AccessibilityEventType::SCROLL_END, Accessibility::EventType::TYPE_VIEW_SCROLLED_EVENT },
        { AccessibilityEventType::TEXT_SELECTION_UPDATE,
            Accessibility::EventType::TYPE_VIEW_TEXT_SELECTION_UPDATE_EVENT },
        { AccessibilityEventType::ACCESSIBILITY_FOCUSED,
            Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUSED_EVENT },
        { AccessibilityEventType::ACCESSIBILITY_FOCUS_CLEARED,
            Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED_EVENT },
        { AccessibilityEventType::TEXT_MOVE_UNIT, Accessibility::EventType::TYPE_VIEW_TEXT_MOVE_UNIT_EVENT },
        { AccessibilityEventType::SCROLL_START, Accessibility::EventType::TYPE_VIEW_SCROLLED_START },
        { AccessibilityEventType::PAGE_CLOSE, Accessibility::EventType::TYPE_PAGE_CLOSE },
        { AccessibilityEventType::PAGE_OPEN, Accessibility::EventType::TYPE_PAGE_OPEN },
    };
    Accessibility::EventType eventType = Accessibility::EventType::TYPE_VIEW_INVALID;
    int64_t idx = BinarySearchFindIndex(eventTypeMap, ArraySize(eventTypeMap), type);
    if (idx >= 0) {
        eventType = eventTypeMap[idx].value;
    }
    return eventType;
}

ActionType ConvertAceAction(AceAction aceAction)
{
    static const ActionTable actionTable[] = {
        { AceAction::ACTION_CLICK, ActionType::ACCESSIBILITY_ACTION_CLICK },
        { AceAction::ACTION_LONG_CLICK, ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
        { AceAction::ACTION_SCROLL_FORWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
        { AceAction::ACTION_SCROLL_BACKWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
        { AceAction::ACTION_FOCUS, ActionType::ACCESSIBILITY_ACTION_FOCUS },
        { AceAction::ACTION_CLEAR_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
        { AceAction::ACTION_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
        { AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
        { AceAction::ACTION_SET_TEXT, ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
        { AceAction::ACTION_COPY, ActionType::ACCESSIBILITY_ACTION_COPY },
        { AceAction::ACTION_PASTE, ActionType::ACCESSIBILITY_ACTION_PASTE },
        { AceAction::ACTION_CUT, ActionType::ACCESSIBILITY_ACTION_CUT },
        { AceAction::ACTION_SELECT, ActionType::ACCESSIBILITY_ACTION_SELECT },
        { AceAction::ACTION_CLEAR_SELECTION, ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
        { AceAction::ACTION_SET_SELECTION, ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
        { AceAction::ACTION_SET_CURSOR_POSITION, ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION },
    };
    for (const auto& item : actionTable) {
        if (aceAction == item.aceAction) {
            return item.action;
        }
    }
    return ActionType::ACCESSIBILITY_ACTION_INVALID;
}

inline RangeInfo ConvertAccessibilityValue(const AccessibilityValue& value)
{
    return RangeInfo(value.min, value.max, value.current);
}

int64_t ConvertToCardAccessibilityId(int64_t nodeId, int64_t cardId, int64_t rootNodeId)
{
    // result is integer total ten digits, top five for agp virtualViewId, end five for ace nodeId,
    // for example agp virtualViewId is 32, ace nodeId is 1000001, convert to result is 00032 10001.
    int64_t result = 0;
    if (nodeId == rootNodeId + ROOT_STACK_BASE) {
        // for example agp virtualViewId is 32 root node is 2100000, convert to result is 00032 21000.
        result = cardId * CARD_BASE + (static_cast<int64_t>(nodeId / CARD_BASE)) * CARD_ROOT_NODE_ID_RATION +
                 nodeId % CARD_BASE;
    } else {
        result = cardId * CARD_BASE + (static_cast<int64_t>(nodeId / DOM_ROOT_NODE_ID_BASE)) * CARD_NODE_ID_RATION +
                 nodeId % DOM_ROOT_NODE_ID_BASE;
    }
    return result;
}

void UpdateAccessibilityNodePosition(const RefPtr<AccessibilityNode>& node, AccessibilityElementInfo& nodeInfo,
    const RefPtr<JsAccessibilityManager>& manager, int windowId)
{
    int leftTopX = static_cast<int>(node->GetLeft()) + manager->GetWindowLeft(node->GetWindowId());
    int leftTopY = static_cast<int>(node->GetTop()) + manager->GetWindowTop(node->GetWindowId());
    int rightBottomX = leftTopX + static_cast<int>(node->GetWidth());
    int rightBottomY = leftTopY + static_cast<int>(node->GetHeight());
    if (manager->isOhosHostCard()) {
        int64_t id = ConvertToCardAccessibilityId(node->GetNodeId(), manager->GetCardId(), manager->GetRootNodeId());
        nodeInfo.SetAccessibilityId(id);
        if (node->GetParentId() == -1) {
            nodeInfo.SetParent(-1);
        } else {
            nodeInfo.SetParent(
                ConvertToCardAccessibilityId(node->GetParentId(), manager->GetCardId(), manager->GetRootNodeId()));
        }
        leftTopX = static_cast<int>(node->GetLeft() + manager->GetCardOffset().GetX());
        leftTopY = static_cast<int>(node->GetTop() + manager->GetCardOffset().GetY());
        rightBottomX = leftTopX + static_cast<int>(node->GetWidth());
        rightBottomY = leftTopY + static_cast<int>(node->GetHeight());
    } else {
        if (node->GetTag() == SIDEBARCONTAINER_TAG) {
            Rect sideBarRect = node->GetRect();
            for (const auto& childNode : node->GetChildList()) {
                sideBarRect = sideBarRect.CombineRect(childNode->GetRect());
            }
            leftTopX = static_cast<int>(sideBarRect.Left()) + manager->GetWindowLeft(node->GetWindowId());
            leftTopY = static_cast<int>(sideBarRect.Top()) + manager->GetWindowTop(node->GetWindowId());
            rightBottomX = static_cast<int>(sideBarRect.Right()) + manager->GetWindowLeft(node->GetWindowId());
            rightBottomY = static_cast<int>(sideBarRect.Bottom()) + manager->GetWindowTop(node->GetWindowId());
        }
        nodeInfo.SetComponentId(static_cast<int64_t>(node->GetNodeId()));
        nodeInfo.SetParent(static_cast<int64_t>(node->GetParentId()));
    }
    if (node->GetParentId() == -1) {
        const auto& children = node->GetChildList();
        if (!children.empty()) {
            auto lastChildNode = manager->GetAccessibilityNodeById(children.back()->GetNodeId());
            if (lastChildNode) {
                rightBottomX = leftTopX + static_cast<int>(lastChildNode->GetWidth());
                rightBottomY = leftTopY + static_cast<int>(lastChildNode->GetHeight());
            }
        }
        nodeInfo.SetParent(INVALID_PARENT_ID);
    }
    Accessibility::Rect bounds(leftTopX, leftTopY, rightBottomX, rightBottomY);
    nodeInfo.SetRectInScreen(bounds);
}

bool IsImportantForAccessibility(const std::string& aceLevel, const std::string& componentType)
{
    bool bRet = false;
    if (aceLevel == NG::AccessibilityProperty::Level::YES_STR) {
        bRet = true;
    }
    if (aceLevel == NG::AccessibilityProperty::Level::AUTO) {
        bRet = IsComponentInArray(componentType);
    }
    return bRet;
}

void UpdateAccessibilityNodeSupport(const RefPtr<AccessibilityNode>& node, AccessibilityElementInfo& nodeInfo,
    const RefPtr<JsAccessibilityManager>& manager, int windowId)
{
    if (node->GetTag() == LIST_TAG) {
        nodeInfo.SetItemCounts(node->GetListItemCounts());
        nodeInfo.SetBeginIndex(node->GetListBeginIndex());
        nodeInfo.SetEndIndex(node->GetListEndIndex());
    }
    if (node->GetIsPassword()) {
        std::string strStar(node->GetText().size(), '*');
        nodeInfo.SetContent(strStar);
    } else {
        nodeInfo.SetContent(node->GetText());
    }

    auto supportAceActions = node->GetSupportAction();
    std::vector<ActionType> actions(supportAceActions.size());

    for (auto it = supportAceActions.begin(); it != supportAceActions.end(); ++it) {
        AccessibleAction action(ConvertAceAction(*it), "ace");
        nodeInfo.AddAction(action);
    }

    if (node->GetImportantForAccessibility() == NG::AccessibilityProperty::Level::YES_STR) {
        actions.emplace_back(ActionType::ACCESSIBILITY_ACTION_FOCUS);
        nodeInfo.SetCheckable(true);
    } else if (node->GetImportantForAccessibility() == NG::AccessibilityProperty::Level::NO_STR ||
               node->GetImportantForAccessibility() == NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS) {
        nodeInfo.SetVisible(false);
    }

    bool isImportant = IsImportantForAccessibility(node->GetImportantForAccessibility(), node->GetTag());
    nodeInfo.SetImportantForAccessibility(isImportant);
}

void UpdateAccessibilityNodeInfo(const RefPtr<AccessibilityNode>& node, AccessibilityElementInfo& nodeInfo,
    const RefPtr<JsAccessibilityManager>& manager, int windowId)
{
    UpdateAccessibilityNodePosition(node, nodeInfo, manager, windowId);
    if (node->GetNodeId() == 0) {
        nodeInfo.SetParent(INVALID_PARENT_ID);
    }
    nodeInfo.SetPagePath(manager->GetPagePath());
    nodeInfo.SetWindowId(windowId);
    nodeInfo.SetChecked(node->GetCheckedState());
    nodeInfo.SetEnabled(node->GetEnabledState());
    nodeInfo.SetFocused(node->GetFocusedState());
    nodeInfo.SetSelected(node->GetSelectedState());
    nodeInfo.SetCheckable(node->GetCheckableState());
    nodeInfo.SetClickable(node->GetClickableState());
    nodeInfo.SetFocusable(node->GetFocusableState());
    nodeInfo.SetScrollable(node->GetScrollableState());
    nodeInfo.SetLongClickable(node->GetLongClickableState());
    nodeInfo.SetEditable(node->GetEditable());
    nodeInfo.SetPluraLineSupported(node->GetIsMultiLine());
    nodeInfo.SetPassword(node->GetIsPassword());
    nodeInfo.SetTextLengthLimit(node->GetMaxTextLength());
    nodeInfo.SetSelectedBegin(node->GetTextSelectionStart());
    nodeInfo.SetSelectedEnd(node->GetTextSelectionEnd());
    nodeInfo.SetVisible(node->GetShown() && node->GetVisible());
    nodeInfo.SetHint(node->GetHintText());
    std::string accessibilityLabel = node->GetAccessibilityLabel();
    nodeInfo.SetLabeled(atol(accessibilityLabel.c_str()));
    nodeInfo.SetError(node->GetErrorText());
    nodeInfo.SetComponentResourceId(node->GetJsComponentId());
    nodeInfo.SetInspectorKey(node->GetJsComponentId());
    RangeInfo rangeInfo = ConvertAccessibilityValue(node->GetAccessibilityValue());
    nodeInfo.SetRange(rangeInfo);
    nodeInfo.SetInputType(static_cast<int>(node->GetTextInputType()));
    nodeInfo.SetComponentType(node->GetTag());
    GridInfo gridInfo(
        node->GetCollectionInfo().rows, node->GetCollectionInfo().columns, (nodeInfo.IsPluraLineSupported() ? 0 : 1));
    nodeInfo.SetGrid(gridInfo);
    nodeInfo.SetAccessibilityFocus(node->GetAccessibilityFocusedState());
    nodeInfo.SetPageId(node->GetPageId());

    int32_t row = node->GetCollectionItemInfo().row;
    int32_t column = node->GetCollectionItemInfo().column;
    GridItemInfo gridItemInfo(row, row, column, column, false);
    gridItemInfo.SetSelected(nodeInfo.IsSelected());
    nodeInfo.SetGridItem(gridItemInfo);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    UpdateAccessibilityNodeSupport(node, nodeInfo, manager, windowId);

    manager->UpdateNodeChildIds(node);
    for (const auto& child : node->GetChildIds()) {
        nodeInfo.AddChild(child);
    }
}

void UpdateCacheInfo(std::list<AccessibilityElementInfo>& infos, uint32_t mode, const RefPtr<AccessibilityNode>& node,
    const RefPtr<JsAccessibilityManager>& jsAccessibilityManager, int windowId)
{
    // parent
    uint32_t umode = mode;
    if (umode & static_cast<uint32_t>(PREFETCH_PREDECESSORS)) {
        if (node->GetParentId() != -1 && node->GetParentId() != DEFAULT_PARENT_ID) {
            AccessibilityElementInfo parentNodeInfo;
            UpdateAccessibilityNodeInfo(node->GetParentNode(), parentNodeInfo, jsAccessibilityManager, windowId);
            infos.emplace_back(parentNodeInfo);
        }
    }
    // sister/brothers
    if ((umode & static_cast<uint32_t>(PREFETCH_SIBLINGS)) &&
        (node->GetParentId() != -1 &&
        node->GetParentId() != DEFAULT_PARENT_ID)) {
        for (const auto& item : node->GetParentNode()->GetChildList()) {
            if (node->GetNodeId() != item->GetNodeId()) {
                AccessibilityElementInfo siblingNodeInfo;
                UpdateAccessibilityNodeInfo(item, siblingNodeInfo, jsAccessibilityManager, windowId);
                infos.emplace_back(siblingNodeInfo);
            }
        }
    }
    // children
    if (umode & static_cast<uint32_t>(PREFETCH_CHILDREN)) {
        for (const auto& item : node->GetChildList()) {
            AccessibilityElementInfo childNodeInfo;
            UpdateAccessibilityNodeInfo(item, childNodeInfo, jsAccessibilityManager, windowId);
            infos.emplace_back(childNodeInfo);
        }
    }

    // get all children
    if (umode & static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN)) {
        std::list<RefPtr<AccessibilityNode>> children;
        for (const auto& item : node->GetChildList()) {
            children.emplace_back(item);
        }

        while (!children.empty()) {
            auto parent = children.front();
            children.pop_front();
            AccessibilityElementInfo childNodeInfo;
            UpdateAccessibilityNodeInfo(parent, childNodeInfo, jsAccessibilityManager, windowId);
            infos.push_back(childNodeInfo);
            for (const auto& item : parent->GetChildList()) {
                children.emplace_back(item);
            }
        }
    }
}

void SortAccessibilityInfosByBreadth(std::list<AccessibilityElementInfo>& infos,
    std::list<int64_t>& accessibilityIdQueue, std::list<AccessibilityElementInfo>& output)
{
    while (!accessibilityIdQueue.empty()) {
        auto accessibilityId = accessibilityIdQueue.front();
        accessibilityIdQueue.pop_front();
        for (std::list<AccessibilityElementInfo>::iterator info = infos.begin(); info != infos.end(); ++info) {
            if (accessibilityId != info->GetAccessibilityId()) {
                continue;
            }
            for (auto& child : info->GetChildIds()) {
                accessibilityIdQueue.emplace_back(child);
            }
            output.emplace_back(*info);
            infos.erase(info);
            break;
        }
        SortAccessibilityInfosByBreadth(infos, accessibilityIdQueue, output);
    }
}

void SortExtensionAccessibilityInfo(std::list<AccessibilityElementInfo>& infos, int64_t rootAccessibilityId)
{
    auto input = infos;
    infos.clear();
    std::list<int64_t> accessibilityIdQueue;
    accessibilityIdQueue.emplace_back(rootAccessibilityId);
    SortAccessibilityInfosByBreadth(input, accessibilityIdQueue, infos);
}

void ConvertExtensionAccessibilityId(AccessibilityElementInfo& info, const RefPtr<NG::FrameNode>& extensionNode,
    int64_t uiExtensionOffset, AccessibilityElementInfo& parentInfo)
{
    auto extensionAbilityId =
            extensionNode->WrapExtensionAbilityId(uiExtensionOffset, info.GetAccessibilityId());
    info.SetAccessibilityId(extensionAbilityId);
    auto parentNodeId =
        extensionNode->WrapExtensionAbilityId(uiExtensionOffset, info.GetParentNodeId());
    info.SetParent(parentNodeId);
    auto childIds = info.GetChildIds();
    for (auto& child : childIds) {
        info.RemoveChild(child);
        info.AddChild(extensionNode->WrapExtensionAbilityId(uiExtensionOffset, child));
    }
    if (V2::ROOT_ETS_TAG == info.GetComponentType()) {
        for (auto& child : info.GetChildIds()) {
            parentInfo.AddChild(child);
        }
    }
}

void ConvertExtensionAccessibilityNodeId(std::list<AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& extensionNode, int64_t uiExtensionOffset,
    AccessibilityElementInfo& parentInfo)
{
    CHECK_NULL_VOID(extensionNode);
    for (auto& accessibilityElementInfo : infos) {
        ConvertExtensionAccessibilityId(accessibilityElementInfo, extensionNode, uiExtensionOffset, parentInfo);
    }
    for (auto& accessibilityElementInfo : infos) {
        if (std::find(parentInfo.GetChildIds().begin(), parentInfo.GetChildIds().end(),
            accessibilityElementInfo.GetAccessibilityId()) != parentInfo.GetChildIds().end()) {
            accessibilityElementInfo.SetParent(extensionNode->GetAccessibilityId());
        }
    }
}

inline std::string BoolToString(bool tag)
{
    return tag ? "true" : "false";
}

std::string ConvertInputTypeToString(AceTextCategory type)
{
    switch (type) {
        case AceTextCategory::INPUT_TYPE_DEFAULT:
            return "INPUT_TYPE_DEFAULT";
        case AceTextCategory::INPUT_TYPE_TEXT:
            return "INPUT_TYPE_TEXT";
        case AceTextCategory::INPUT_TYPE_EMAIL:
            return "INPUT_TYPE_EMAIL";
        case AceTextCategory::INPUT_TYPE_DATE:
            return "INPUT_TYPE_DATE";
        case AceTextCategory::INPUT_TYPE_TIME:
            return "INPUT_TYPE_TIME";
        case AceTextCategory::INPUT_TYPE_NUMBER:
            return "INPUT_TYPE_NUMBER";
        case AceTextCategory::INPUT_TYPE_PASSWORD:
            return "INPUT_TYPE_PASSWORD";
        case AceTextCategory::INPUT_TYPE_PHONENUMBER:
            return "INPUT_TYPE_PHONENUMBER";
        case AceTextCategory::INPUT_TYPE_USER_NAME:
            return "INPUT_TYPE_USER_NAME";
        case AceTextCategory::INPUT_TYPE_NEW_PASSWORD:
            return "INPUT_TYPE_NEW_PASSWORD";
        default:
            return "illegal input type";
    }
}

bool FindAccessibilityFocus(const RefPtr<AccessibilityNode>& node, RefPtr<AccessibilityNode>& resultNode)
{
    CHECK_NULL_RETURN(node, false);
    if (node->GetAccessibilityFocusedState()) {
        resultNode = node;
        return true;
    }
    if (!node->GetChildList().empty()) {
        for (const auto& item : node->GetChildList()) {
            if (resultNode != nullptr) {
                return true;
            }
            if (FindAccessibilityFocus(item, resultNode)) {
                return true;
            }
        }
    }

    return false;
}

void FindFocusedExtensionElementInfoNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityElementInfo& info)
{
    if (NG::UI_EXTENSION_OFFSET_MIN < (searchParam.uiExtensionOffset + 1)) {
        node->FindFocusedExtensionElementInfoNG(
            searchParam.nodeId, searchParam.mode, searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR, info);
    } else {
        info.SetValidElement(false);
    }
}

void SetUiExtensionAbilityParentIdForFocus(const RefPtr<NG::UINode>& uiExtensionNode,
    const int64_t uiExtensionOffset, Accessibility::AccessibilityElementInfo& info)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiExtensionNode);
    if (frameNode) {
        auto parentId = info.GetParentNodeId();
        AccessibilityElementInfo parentInfo;
        ConvertExtensionAccessibilityId(info, frameNode, uiExtensionOffset, parentInfo);
        if (parentId == NG::UI_EXTENSION_ROOT_ID) {
            info.SetParent(frameNode->GetAccessibilityId());
        }
    }
}

RefPtr<NG::FrameNode> GetFramenodeByAccessibilityId(const RefPtr<NG::FrameNode>& root, int64_t id);

RefPtr<NG::FrameNode> FindAccessibilityFocus(const RefPtr<NG::UINode>& node, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    FindAccessibilityFocusArg findAccessibilityFocusArg)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (frameNode) {
        if (frameNode->GetRenderContext()->GetAccessibilityFocus().value_or(false)) {
            auto node = GetFramenodeByAccessibilityId(frameNode, findAccessibilityFocusArg.currentFocusNodeId);
            return frameNode;
        }
    }
    if (node->GetChildren(true).empty()) {
        return nullptr;
    }
    for (const auto& child : node->GetChildren(true)) {
        auto extensionNode = AceType::DynamicCast<NG::FrameNode>(child);
        if (IsIsolatedComponent(child) && extensionNode && !IsUIExtensionShowPlaceholder(extensionNode) &&
            (extensionNode->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
            (((extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
                 (NG::UI_EXTENSION_OFFSET_MAX == findAccessibilityFocusArg.uiExtensionOffset)) ||
            (extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX))) {
            SearchParameter transferSearchParam { NG::UI_EXTENSION_ROOT_ID, "", focusType,
                findAccessibilityFocusArg.uiExtensionOffset };
            OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(transferSearchParam, extensionNode, info);
            if (info.GetAccessibilityId() < 0) {
                continue;
            }
            SetUiExtensionAbilityParentIdForFocus(extensionNode, findAccessibilityFocusArg.uiExtensionOffset, info);
            return extensionNode;
        }
        auto result = FindAccessibilityFocus(child, focusType, info, context, findAccessibilityFocusArg);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

bool FindInputFocus(const RefPtr<AccessibilityNode>& node, RefPtr<AccessibilityNode>& resultNode)
{
    CHECK_NULL_RETURN(node, false);
    if (!node->GetFocusedState() && (node->GetParentId() != -1)) {
        return false;
    }
    if (node->GetFocusedState()) {
        resultNode = node;
    }
    if (!node->GetChildList().empty()) {
        for (const auto& item : node->GetChildList()) {
            if (FindInputFocus(item, resultNode)) {
                return true;
            }
        }
    }
    return node->GetFocusedState();
}

RefPtr<NG::FrameNode> FindInputFocus(const RefPtr<NG::UINode>& node, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const int64_t uiExtensionOffset,
    const RefPtr<PipelineBase>& context)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    CHECK_NULL_RETURN(frameNode, nullptr);
    if (!(frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsCurrentFocus() : false)) {
        return nullptr;
    }
    if (frameNode->GetFocusHub()->IsChild()) {
        if (frameNode->IsInternal()) {
            return frameNode->GetFocusParent();
        }
        return frameNode;
    }
    auto focusHub = frameNode->GetFocusHub();
    RefPtr<NG::FrameNode> target;
    focusHub->AnyChildFocusHub([&target, &info, context, focusType, uiExtensionOffset](
                                   const RefPtr<NG::FocusHub>& focusChild) {
        auto extensionNode = focusChild->GetFrameNode();
        if ((extensionNode && IsIsolatedComponent(extensionNode)) &&
            !IsUIExtensionShowPlaceholder(extensionNode) &&
            (extensionNode->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
            (((extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
            (NG::UI_EXTENSION_OFFSET_MAX == uiExtensionOffset)) ||
            (extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX))) {
            SearchParameter transferSearchParam {NG::UI_EXTENSION_ROOT_ID, "", focusType, uiExtensionOffset};
            OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(
                transferSearchParam, extensionNode, info);
            if (info.GetAccessibilityId() < 0) {
                return false;
            }
            SetUiExtensionAbilityParentIdForFocus(extensionNode, uiExtensionOffset, info);
            target = extensionNode;
            return true;
        }
        target = FindInputFocus(focusChild->GetFrameNode(), focusType, info, uiExtensionOffset, context);
        return target ? true : false;
    });
    return target;
}

bool FindFrameNodeByAccessibilityId(int64_t id, const std::list<RefPtr<NG::UINode>>& children,
    std::queue<NG::UINode*>& nodes, RefPtr<NG::FrameNode>& result)
{
    NG::FrameNode* frameNode = nullptr;
    for (const auto& child : children) {
        frameNode = AceType::DynamicCast<NG::FrameNode>(Referenced::RawPtr(child));
        if (frameNode != nullptr && !frameNode->CheckAccessibilityLevelNo()) {
            if (frameNode->GetAccessibilityId() == id) {
                result = AceType::DynamicCast<NG::FrameNode>(child);
                return true;
            }
        }
        nodes.push(Referenced::RawPtr(child));
    }
    return false;
}

RefPtr<NG::FrameNode> GetFramenodeByAccessibilityId(const RefPtr<NG::FrameNode>& root, int64_t id)
{
    CHECK_NULL_RETURN(root, nullptr);
    if (root->GetAccessibilityId() == id) {
        return root;
    }
    std::queue<NG::UINode*> nodes;
    nodes.push(Referenced::RawPtr(root));
    RefPtr<NG::FrameNode> frameNodeResult = nullptr;

    while (!nodes.empty()) {
        auto current = nodes.front();
        nodes.pop();
        if (current->HasVirtualNodeAccessibilityProperty()) {
            auto fnode = AceType::DynamicCast<NG::FrameNode>(current);
            auto property = fnode->GetAccessibilityProperty<NG::AccessibilityProperty>();
            const auto& children = std::list<RefPtr<NG::UINode>> { property->GetAccessibilityVirtualNode() };
            if (FindFrameNodeByAccessibilityId(id, children, nodes, frameNodeResult)) {
                return frameNodeResult;
            }
        } else {
            const auto& children = current->GetChildren(true);
            if (FindFrameNodeByAccessibilityId(id, children, nodes, frameNodeResult)) {
                return frameNodeResult;
            }
        }
    }
    return nullptr;
}

void GetFrameNodeChildren(const RefPtr<NG::UINode>& uiNode, std::vector<int64_t>& children, int32_t pageId)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        if (!frameNode->IsFirstVirtualNode()) {
            CHECK_NULL_VOID(frameNode->IsActive());
        }
        if (uiNode->GetTag() == "stage") {
        } else if (uiNode->GetTag() == "page") {
            if (uiNode->GetPageId() != pageId) {
                return;
            }
        } else if (!frameNode->IsInternal() || frameNode->IsFirstVirtualNode()) {
            children.emplace_back(uiNode->GetAccessibilityId());
            return;
        }
    }

    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
        if (uiVirtualNode != nullptr) {
            auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
            if (virtualNode != nullptr) {
                GetFrameNodeChildren(virtualNode, children, pageId);
                return;
            }
        }
    }

    for (const auto& frameChild : uiNode->GetChildren(true)) {
        GetFrameNodeChildren(frameChild, children, pageId);
    }
}

void GetFrameNodeChildren(
    const RefPtr<NG::UINode>& uiNode, std::list<RefPtr<NG::FrameNode>>& children, int32_t pageId = -1)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
        CHECK_NULL_VOID(frameNode->IsActive());
        if (uiNode->GetTag() == "page") {
            if (pageId != -1 && uiNode->GetPageId() != pageId) {
                return;
            }
        } else if (!frameNode->IsInternal() && uiNode->GetTag() != "stage") {
            children.emplace_back(frameNode);
            return;
        }
    }

    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
        if (uiVirtualNode != nullptr) {
            auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
            if (virtualNode != nullptr) {
                GetFrameNodeChildren(virtualNode, children, pageId);
                return;
            }
        }
    }

    for (const auto& frameChild : uiNode->GetChildren(true)) {
        GetFrameNodeChildren(frameChild, children, pageId);
    }
}

int64_t GetParentId(const RefPtr<NG::UINode>& uiNode)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        if (AceType::DynamicCast<NG::FrameNode>(uiNode)->IsFirstVirtualNode()) {
            auto weakNode = AceType::DynamicCast<NG::FrameNode>(uiNode)->GetVirtualNodeParent();
            auto refNode = weakNode.Upgrade();
            return refNode == nullptr ? INVALID_PARENT_ID : refNode->GetAccessibilityId();
        }
    }
    auto parent = uiNode->GetParent();
    while (parent) {
        if (AceType::InstanceOf<NG::FrameNode>(parent)) {
            if ((parent->GetTag() == V2::PAGE_ETS_TAG) || (parent->GetTag() == V2::STAGE_ETS_TAG) ||
                AceType::DynamicCast<NG::FrameNode>(parent)->CheckAccessibilityLevelNo()) {
                parent = parent->GetParent();
                continue;
            }
            return parent->GetAccessibilityId();
        }
        parent = parent->GetParent();
    }
    return INVALID_PARENT_ID;
}

void FillElementInfo(int64_t elementId, AccessibilityElementInfo& elementInfo, const RefPtr<PipelineBase>& context,
    const RefPtr<JsAccessibilityManager>& jsAccessibilityManager, const FillEventInfoParam& param)
{
    std::list<AccessibilityElementInfo> elementInfos;
    int32_t mode = 0;
    CHECK_NULL_VOID(jsAccessibilityManager);
    jsAccessibilityManager->SearchElementInfoByAccessibilityIdNG(
        elementId, mode, elementInfos, context, NG::UI_EXTENSION_OFFSET_MAX);
    if (elementInfos.empty()) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "Element infos is empty. Find element infos failed.");
        return;
    }
    elementInfo = elementInfos.front();
    elementInfo.SetWindowId(param.windowId);
}

void FillEventInfo(const RefPtr<NG::FrameNode>& node,
                   AccessibilityEventInfo& eventInfo,
                   const RefPtr<PipelineBase>& context,
                   const RefPtr<JsAccessibilityManager>& jsAccessibilityManager,
                   const FillEventInfoParam& param)
{
    CHECK_NULL_VOID(node);
    eventInfo.SetComponentType(node->GetTag());
    eventInfo.SetPageId(node->GetPageId());
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    eventInfo.AddContent(accessibilityProperty->GetGroupText());
    eventInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    eventInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    eventInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    AccessibilityElementInfo elementInfo;
    FillElementInfo(param.elementId, elementInfo, context, jsAccessibilityManager, param);
    elementInfo.SetNavDestinationId(param.stackNodeId);
    eventInfo.SetElementInfo(elementInfo);
}

void FillEventInfo(const RefPtr<AccessibilityNode>& node, AccessibilityEventInfo& eventInfo)
{
    eventInfo.SetComponentType(node->GetTag());
    if (node->GetTag() == LIST_TAG) {
        eventInfo.SetItemCounts(node->GetListItemCounts());
        eventInfo.SetBeginIndex(node->GetListBeginIndex());
        eventInfo.SetEndIndex(node->GetListEndIndex());
    }
    eventInfo.SetPageId(node->GetPageId());
    eventInfo.AddContent(node->GetText());
    eventInfo.SetLatestContent(node->GetText());
}

inline bool IsPopupSupported(const RefPtr<NG::PipelineContext>& pipeline, int64_t nodeId)
{
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    if (overlayManager) {
        return overlayManager->HasPopupInfo(nodeId);
    }
    return false;
}

void SetAccessibilityFocusAction(AccessibilityElementInfo& nodeInfo, const char* tag)
{
    if (nodeInfo.HasAccessibilityFocus()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS, tag);
        nodeInfo.AddAction(action);
    } else {
        AccessibleAction action(ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, tag);
        nodeInfo.AddAction(action);
    }
}

void UpdateSupportAction(const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo)
{
    CHECK_NULL_VOID(node);
    if (nodeInfo.IsFocusable()) {
        if (nodeInfo.IsFocused()) {
            AccessibleAction action(ACCESSIBILITY_ACTION_CLEAR_FOCUS, "ace");
            nodeInfo.AddAction(action);
        } else {
            AccessibleAction action(ACCESSIBILITY_ACTION_FOCUS, "ace");
            nodeInfo.AddAction(action);
        }
    }

    auto eventHub = node->GetEventHub<NG::EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto gestureEventHub = eventHub->GetGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    nodeInfo.SetClickable(gestureEventHub->IsAccessibilityClickable());
    if (gestureEventHub->IsAccessibilityClickable()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_CLICK, "ace");
        nodeInfo.AddAction(action);
    }
    nodeInfo.SetLongClickable(gestureEventHub->IsAccessibilityLongClickable());
    if (gestureEventHub->IsAccessibilityLongClickable()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_LONG_CLICK, "ace");
        nodeInfo.AddAction(action);
    }
}
} // namespace

void JsAccessibilityManager::UpdateAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo)
{
    CHECK_NULL_VOID(node);
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    if (accessibilityProperty->HasAccessibilityRole()) {
        nodeInfo.SetComponentType(accessibilityProperty->GetAccessibilityRole());
    }

    if (accessibilityProperty->HasUserTextValue()) {
        nodeInfo.SetContent(accessibilityProperty->GetUserTextValue());
    } else {
        nodeInfo.SetContent(accessibilityProperty->GetGroupText());
    }
    nodeInfo.SetAccessibilityText(accessibilityProperty->GetAccessibilityText());
    if (accessibilityProperty->HasRange()) {
        RangeInfo rangeInfo = ConvertAccessibilityValue(accessibilityProperty->GetAccessibilityValue());
        nodeInfo.SetRange(rangeInfo);
    }
    nodeInfo.SetHint(accessibilityProperty->GetHintText());
    nodeInfo.SetAccessibilityGroup(accessibilityProperty->IsAccessibilityGroup());
    nodeInfo.SetAccessibilityLevel(accessibilityProperty->GetAccessibilityLevel());
    nodeInfo.SetTextType(accessibilityProperty->GetTextType());
    nodeInfo.SetTextLengthLimit(accessibilityProperty->GetTextLengthLimit());
    nodeInfo.SetOffset(accessibilityProperty->GetScrollOffSet());

    UpdateAccessibilityElementInfoBase(node, nodeInfo, accessibilityProperty);
    UpdateAccessibilityElementInfoImproved(node, nodeInfo, accessibilityProperty);
    UpdateAccessibilityElementInfoImportant(node, nodeInfo, accessibilityProperty);
}

void JsAccessibilityManager::UpdateAccessibilityElementInfoBase(const RefPtr<NG::FrameNode>& node,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::AccessibilityProperty>& accessibilityProperty)
{
    auto context = node->GetRenderContext();
    if (context != nullptr) {
        nodeInfo.SetZIndex(context->GetZIndex().value_or(0));
        nodeInfo.SetOpacity(context->GetOpacity().value_or(1));
        nodeInfo.SetBackgroundColor(context->GetBackgroundColor().value_or(Color::TRANSPARENT).ToString());
        nodeInfo.SetBackgroundImage(context->GetBackgroundImage().value_or(ImageSourceInfo("")).ToString());
        if (context->GetForeground() != nullptr) {
            nodeInfo.SetBlur(context->GetForeground()->propBlurRadius.value_or(Dimension(0)).ToString());
        }
    }
    auto eventHub = node->GetEventHub<NG::EventHub>();
    if (eventHub != nullptr) {
       nodeInfo.SetHitTestBehavior(NG::GestureEventHub::GetHitTestModeStr(eventHub->GetGestureEventHub()));
    }
    if (accessibilityProperty->HasUserDisabled()) {
        nodeInfo.SetEnabled(!accessibilityProperty->IsUserDisabled());
    }
    if (accessibilityProperty->HasUserCheckedType()) {
        nodeInfo.SetChecked(accessibilityProperty->GetUserCheckedType());
    } else {
        nodeInfo.SetChecked(accessibilityProperty->IsChecked());
    }
    if (accessibilityProperty->HasUserSelected()) {
        nodeInfo.SetSelected(accessibilityProperty->IsUserSelected());
    } else {
        nodeInfo.SetSelected(accessibilityProperty->IsSelected());
    }
    nodeInfo.SetPassword(accessibilityProperty->IsPassword());
    nodeInfo.SetPluraLineSupported(accessibilityProperty->IsMultiLine());
    nodeInfo.SetHinting(accessibilityProperty->IsHint());
    nodeInfo.SetDescriptionInfo(accessibilityProperty->GetAccessibilityDescription());
}

void JsAccessibilityManager::UpdateAccessibilityElementInfoImproved(const RefPtr<NG::FrameNode>& node,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::AccessibilityProperty>& accessibilityProperty)
{
    if (accessibilityProperty->HasUserCurrentValue()) {
        nodeInfo.SetCurrentIndex(accessibilityProperty->GetUserCurrentValue());
    } else {
        nodeInfo.SetCurrentIndex(accessibilityProperty->GetCurrentIndex());
    }
    if (accessibilityProperty->HasUserMinValue()) {
        nodeInfo.SetBeginIndex(accessibilityProperty->GetUserMinValue());
    } else {
        nodeInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    }
    if (accessibilityProperty->HasUserMaxValue()) {
        nodeInfo.SetEndIndex(accessibilityProperty->GetUserMaxValue());
    } else {
        nodeInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    }

    auto tag = node->GetTag();
    if (tag == V2::TOAST_ETS_TAG || tag == V2::POPUP_ETS_TAG || tag == V2::DIALOG_ETS_TAG ||
        tag == V2::ACTION_SHEET_DIALOG_ETS_TAG || tag == V2::ALERT_DIALOG_ETS_TAG || tag == V2::MENU_ETS_TAG ||
        tag == "SelectMenu") {
        nodeInfo.SetLiveRegion(1);
    }
    nodeInfo.SetContentInvalid(accessibilityProperty->GetContentInvalid());
    nodeInfo.SetError(accessibilityProperty->GetErrorText());
    nodeInfo.SetSelectedBegin(accessibilityProperty->GetTextSelectionStart());
    nodeInfo.SetSelectedEnd(accessibilityProperty->GetTextSelectionEnd());
    nodeInfo.SetInputType(static_cast<int>(accessibilityProperty->GetTextInputType()));
    nodeInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    nodeInfo.SetChildTreeIdAndWinId(accessibilityProperty->GetChildTreeId(), accessibilityProperty->GetChildWindowId());
    if (nodeInfo.GetComponentType() == "FormComponent") {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", Set: %{public}d %{public}d",
            nodeInfo.GetAccessibilityId(), accessibilityProperty->GetChildTreeId(),
            accessibilityProperty->GetChildWindowId());
    }
    if (nodeInfo.GetWindowId() == static_cast<int32_t>(windowId_)) {
        nodeInfo.SetBelongTreeId(treeId_);
        nodeInfo.SetParentWindowId(parentWindowId_);
    } else {
        nodeInfo.SetBelongTreeId(0);
        nodeInfo.SetParentWindowId(0);
    }

    GridInfo gridInfo(accessibilityProperty->GetCollectionInfo().rows,
        accessibilityProperty->GetCollectionInfo().columns, accessibilityProperty->GetCollectionInfo().selectMode);
    nodeInfo.SetGrid(gridInfo);
    ExtraElementInfo extraElementInfo {};
    accessibilityProperty->GetAllExtraElementInfo(extraElementInfo);
    nodeInfo.SetExtraElement(extraElementInfo);
}

void JsAccessibilityManager::UpdateAccessibilityElementInfoImportant(const RefPtr<NG::FrameNode>& node,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::AccessibilityProperty>& accessibilityProperty)
{
    int32_t row = accessibilityProperty->GetCollectionItemInfo().row;
    int32_t column = accessibilityProperty->GetCollectionItemInfo().column;
    int32_t rowSpan = accessibilityProperty->GetCollectionItemInfo().rowSpan;
    int32_t columnSpan = accessibilityProperty->GetCollectionItemInfo().columnSpan;
    bool heading = accessibilityProperty->GetCollectionItemInfo().heading;
    GridItemInfo gridItemInfo(row, rowSpan, column, columnSpan, heading);
    gridItemInfo.SetSelected(nodeInfo.IsSelected());
    nodeInfo.SetGridItem(gridItemInfo);

    SetAccessibilityFocusAction(nodeInfo, "ace");
    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetCheckable(accessibilityProperty->IsCheckable());
        nodeInfo.SetScrollable(accessibilityProperty->IsScrollable());
        nodeInfo.SetEditable(accessibilityProperty->IsEditable());
        nodeInfo.SetDeletable(accessibilityProperty->IsDeletable());
        UpdateSupportAction(node, nodeInfo);
        accessibilityProperty->ResetSupportAction();
        auto supportAceActions = accessibilityProperty->GetSupportAction();
        for (auto it = supportAceActions.begin(); it != supportAceActions.end(); ++it) {
            AccessibleAction action(ConvertAceAction(*it), "ace");
            nodeInfo.AddAction(action);
        }
    }
    const std::string& componentType = accessibilityProperty->HasAccessibilityRole()
                                           ? accessibilityProperty->GetAccessibilityRole()
                                           : nodeInfo.GetComponentType();
    bool isImportant = IsImportantForAccessibility(accessibilityProperty->GetAccessibilityLevel(), componentType);
    nodeInfo.SetImportantForAccessibility(isImportant);
}

namespace {
void UpdateChildrenOfAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty, AccessibilityElementInfo& nodeInfo)
{
    if (!IsExtensionComponent(node) || IsUIExtensionShowPlaceholder(node)) {
        std::vector<int64_t> children;
        for (const auto& item : node->GetChildren(true)) {
            GetFrameNodeChildren(item, children, commonProperty.pageId);
        }
        auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
        auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
        if (uiVirtualNode != nullptr) {
            auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
            if (virtualNode != nullptr) {
                children.clear();
                GetFrameNodeChildren(virtualNode, children, commonProperty.pageId);
            }
        }
        for (const auto& child : children) {
            nodeInfo.AddChild(child);
        }
    }
}
}

void JsAccessibilityManager::UpdateVirtualNodeChildAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeParentInfo, AccessibilityElementInfo& nodeInfo,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(node);
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());

    nodeInfo.SetEnabled(node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true);
    nodeInfo.SetFocused(node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false);
    nodeInfo.SetAccessibilityFocus(node->GetRenderContext()->GetAccessibilityFocus().value_or(false));
    nodeInfo.SetInspectorKey(node->GetInspectorId().value_or(""));
    nodeInfo.SetVisible(node->IsVisible());
    if (node->IsVisible()) {
        auto virtualNodeRect = node->GetTransformRectRelativeToWindow();
        int32_t left = nodeParentInfo.GetRectInScreen().GetLeftTopXScreenPostion() + virtualNodeRect.GetX();
        int32_t top = nodeParentInfo.GetRectInScreen().GetLeftTopYScreenPostion() + virtualNodeRect.GetY();
        int32_t right = nodeParentInfo.GetRectInScreen().GetLeftTopXScreenPostion() + virtualNodeRect.GetX() +
            virtualNodeRect.Width();
        int32_t bottom = nodeParentInfo.GetRectInScreen().GetLeftTopYScreenPostion() + virtualNodeRect.GetY() +
            virtualNodeRect.Height();
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(commonProperty.pagePath);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetFocusHub() ? node->GetFocusHub()->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(node->GetInspectorId().value_or(""));
    UpdateAccessibilityElementInfo(node, nodeInfo);
}

void JsAccessibilityManager::UpdateVirtualNodeAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& parent, const RefPtr<NG::FrameNode>& node,
    const CommonProperty& commonProperty, AccessibilityElementInfo& nodeInfo,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(parent);
    CHECK_NULL_VOID(node);
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());

    nodeInfo.SetEnabled(node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true);
    nodeInfo.SetFocused(node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false);
    nodeInfo.SetAccessibilityFocus(node->GetRenderContext()->GetAccessibilityFocus().value_or(false));
    nodeInfo.SetInspectorKey(node->GetInspectorId().value_or(""));
    nodeInfo.SetVisible(node->IsVisible());
    if (node->IsVisible()) {
        auto virtualNodeRect = node->GetTransformRectRelativeToWindow();
        auto parentRect = parent->GetTransformRectRelativeToWindow();
        auto left = parentRect.Left() + commonProperty.windowLeft;
        auto top = parentRect.Top() + commonProperty.windowTop;
        auto right = parentRect.Left() + virtualNodeRect.Width() + commonProperty.windowLeft;
        if (virtualNodeRect.Width() > (parentRect.Right() - parentRect.Left())) {
            right = parentRect.Right() + commonProperty.windowLeft;
        }
        auto bottom = parentRect.Top() + virtualNodeRect.Height() + commonProperty.windowTop;
        if (virtualNodeRect.Height() > (parentRect.Bottom() - parentRect.Top())) {
            bottom = parentRect.Bottom() + commonProperty.windowTop;
        }
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(commonProperty.pagePath);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetFocusHub() ? node->GetFocusHub()->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(node->GetInspectorId().value_or(""));
    UpdateAccessibilityElementInfo(node, nodeInfo);
}

void JsAccessibilityManager::UpdateAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(node);
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());

    nodeInfo.SetEnabled(node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true);
    nodeInfo.SetFocused(node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false);
    nodeInfo.SetAccessibilityFocus(node->GetRenderContext()->GetAccessibilityFocus().value_or(false));
    nodeInfo.SetInspectorKey(node->GetInspectorId().value_or(""));
    nodeInfo.SetVisible(node->IsVisible());
    if (node->IsAccessibilityVirtualNode()) {
        auto rect = node->GetVirtualNodeTransformRectRelativeToWindow();
        auto left = rect.Left() + commonProperty.windowLeft;
        auto top = rect.Top() + commonProperty.windowTop;
        auto right = rect.Right() + commonProperty.windowLeft;
        auto bottom = rect.Bottom() + commonProperty.windowTop;
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    } else if (node->IsVisible()) {
        auto rect = node->GetTransformRectRelativeToWindow();
        auto left = rect.Left() + commonProperty.windowLeft;
        auto top = rect.Top() + commonProperty.windowTop;
        auto right = rect.Right() + commonProperty.windowLeft;
        auto bottom = rect.Bottom() + commonProperty.windowTop;
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(commonProperty.pagePath);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetFocusHub() ? node->GetFocusHub()->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(node->GetInspectorId().value_or(""));
    UpdateAccessibilityElementInfo(node, nodeInfo);
}

namespace {
std::list<AccessibilityElementInfo> SearchExtensionElementInfoByAccessibilityIdNG(
    int64_t elementId, int32_t mode, const RefPtr<NG::FrameNode>& node, int64_t offset)
{
    std::list<AccessibilityElementInfo> extensionElementInfo;
    if (NG::UI_EXTENSION_OFFSET_MIN < (offset + 1)) {
        node->SearchExtensionElementInfoByAccessibilityIdNG(elementId, mode,
            offset / NG::UI_EXTENSION_ID_FACTOR, extensionElementInfo);
    }
    return extensionElementInfo;
}

void UpdateUiExtensionParentIdForFocus(const RefPtr<NG::FrameNode>& rootNode, const int64_t uiExtensionOffset,
    Accessibility::AccessibilityElementInfo& info)
{
    if ((uiExtensionOffset != NG::UI_EXTENSION_OFFSET_MAX) && (info.GetComponentType() != V2::ROOT_ETS_TAG) &&
        (info.GetParentNodeId() == rootNode->GetAccessibilityId())) {
            info.SetParent(NG::UI_EXTENSION_ROOT_ID);
    }
}

void GetChildrenFromFrameNode(const RefPtr<NG::FrameNode>& node,
    std::list<std::variant<RefPtr<NG::FrameNode>, int64_t>>& children, int32_t pageId)
{
    std::list<RefPtr<NG::FrameNode>> frameNodeChildren;
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
    if (uiVirtualNode != nullptr) {
        auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
        if (virtualNode != nullptr) {
            GetFrameNodeChildren(virtualNode, frameNodeChildren, pageId);
        }
    } else {
        for (const auto& item : node->GetChildren(true)) {
            GetFrameNodeChildren(item, frameNodeChildren, pageId);
        }
    }
    while (!frameNodeChildren.empty()) {
        children.emplace_back(frameNodeChildren.front());
        frameNodeChildren.pop_front();
    }
}
} // namespace

void JsAccessibilityManager::UpdateVirtualNodeInfo(std::list<AccessibilityElementInfo>& infos,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::UINode>& uiVirtualNode, const CommonProperty& commonProperty,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    AccessibilityElementInfo virtualInfo;
    for (const auto& item : uiVirtualNode->GetChildren(true)) {
        auto frameNodeChild = AceType::DynamicCast<NG::FrameNode>(item);
        if (frameNodeChild == nullptr) {
            continue;
        }
        UpdateVirtualNodeChildAccessibilityElementInfo(frameNodeChild, commonProperty,
            nodeInfo, virtualInfo, ngPipeline);
        virtualInfo.SetParent(uiVirtualNode->GetAccessibilityId());
        nodeInfo.AddChild(frameNodeChild->GetAccessibilityId());
        UpdateVirtualNodeInfo(infos, virtualInfo, item, commonProperty, ngPipeline);
        infos.push_back(virtualInfo);
    }
}

namespace {
void SearchExtensionElementInfoNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
    Accessibility::AccessibilityElementInfo& parentInfo)
{
    auto extensionElementInfos = SearchExtensionElementInfoByAccessibilityIdNG(
        searchParam.nodeId, searchParam.mode, node, searchParam.uiExtensionOffset);
    if (extensionElementInfos.size() > 0) {
        auto rootParentId = extensionElementInfos.front().GetParentNodeId();
        ConvertExtensionAccessibilityNodeId(extensionElementInfos, node, searchParam.uiExtensionOffset, parentInfo);
        if (rootParentId == NG::UI_EXTENSION_ROOT_ID) {
            extensionElementInfos.front().SetParent(node->GetAccessibilityId());
        }
        if (parentInfo.GetComponentType() == V2::ISOLATED_COMPONENT_ETS_TAG) {
            auto windowId = parentInfo.GetWindowId();
            for (auto& info : extensionElementInfos) {
                info.SetWindowId(windowId);
            }
        }
        for (auto& info : extensionElementInfos) {
            infos.push_back(info);
        }
    }
}

bool IsNodeInRoot(const RefPtr<NG::FrameNode>& node, const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_RETURN(node, false);
    CHECK_NULL_RETURN(ngPipeline, false);
    auto rect = node->GetTransformRectRelativeToWindow();
    auto root = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(root, false);
    auto rootRect = root->GetTransformRectRelativeToWindow();
    return LessNotEqual(rect.GetX(), rootRect.GetX() + rootRect.Width());
}
} // namespace

void JsAccessibilityManager::UpdateCacheInfoNG(std::list<AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    const RefPtr<NG::PipelineContext>& ngPipeline, const SearchParameter& searchParam)
{
    uint32_t umode = searchParam.mode;
    std::list<std::variant<RefPtr<NG::FrameNode>, int64_t>> children;
    // get all children
    if (!(umode & static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN))) {
        return;
    }
    GetChildrenFromFrameNode(node, children, commonProperty.pageId);
    while (!children.empty()) {
        std::variant<RefPtr<NG::FrameNode>, int64_t> parent = children.front();
        children.pop_front();

        if (parent.index() == 0) {
            CacheParam cacheParam { commonProperty, ngPipeline, searchParam };
            UpdateCacheInfoNG4FrameNode(infos, parent, cacheParam, children);
        } else if (parent.index() == 1) {
            // Handle the parent when its type is WebNode
            AccessibilityElementInfo nodeInfo;
            int64_t intParent = std::get<1>(parent);
            auto frameNode = GetFramenodeByAccessibilityId(node, intParent);
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, ngPipeline);
            infos.push_back(nodeInfo);
        }
    }
}

void JsAccessibilityManager::UpdateCacheInfoNG4FrameNode(std::list<Accessibility::AccessibilityElementInfo>& infos,
    const std::variant<RefPtr<NG::FrameNode>, int64_t>& parent,
    const CacheParam &cacheParam,
    std::list<std::variant<RefPtr<NG::FrameNode>, int64_t>>& children)
{
    // Handle the parent when its type is FrameNode
    AccessibilityElementInfo nodeInfo;

    RefPtr<NG::FrameNode> frameNodeParent = std::get<0>(parent);
    auto accessibilityProperty = frameNodeParent->GetAccessibilityProperty<NG::AccessibilityProperty>();
    auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
    UpdateAccessibilityElementInfo(frameNodeParent, cacheParam.commonProperty, nodeInfo, cacheParam.ngPipeline);
    if (nodeInfo.GetComponentType() == V2::WEB_ETS_TAG && !IsNodeInRoot(frameNodeParent, cacheParam.ngPipeline)) {
        return;
    }
    if (uiVirtualNode != nullptr) {
        auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
        if (virtualNode == nullptr) {
            return;
        }
        AccessibilityElementInfo virtualInfo;
        UpdateVirtualNodeAccessibilityElementInfo(frameNodeParent, virtualNode,
            cacheParam.commonProperty, virtualInfo, cacheParam.ngPipeline);
        virtualInfo.SetParent(frameNodeParent->GetAccessibilityId());
        auto childIds = nodeInfo.GetChildIds();
        for (auto& child : childIds) {
            nodeInfo.RemoveChild(child);
        }
        nodeInfo.AddChild(virtualNode->GetAccessibilityId());
        auto uiParentNode = AceType::DynamicCast<NG::UINode>(frameNodeParent);
        if (!uiVirtualNode->GetChildren(true).empty()) {
            UpdateVirtualNodeInfo(infos, virtualInfo, uiVirtualNode, cacheParam.commonProperty, cacheParam.ngPipeline);
        }
        infos.push_back(virtualInfo);
        infos.push_back(nodeInfo);
        return;
    }
    if (!IsExtensionComponent(frameNodeParent) || IsUIExtensionShowPlaceholder(frameNodeParent)) {
        infos.push_back(nodeInfo);
        GetChildrenFromFrameNode(frameNodeParent, children, cacheParam.commonProperty.pageId);
        return;
    }
    if (!((frameNodeParent->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
        (((frameNodeParent->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
        (NG::UI_EXTENSION_OFFSET_MAX == cacheParam.searchParam.uiExtensionOffset)) ||
        (frameNodeParent->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX)))) {
        return;
    }
    auto transferParam = cacheParam.searchParam;
    transferParam.nodeId = NG::UI_EXTENSION_ROOT_ID;
    SearchExtensionElementInfoNG(transferParam, frameNodeParent, infos, nodeInfo);

    infos.push_back(nodeInfo);
}

namespace {
bool CanAccessibilityFocusedNG(const RefPtr<NG::FrameNode>& node)
{
    CHECK_NULL_RETURN(node, false);
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    auto level = accessibilityProperty->GetAccessibilityLevel();
    return !node->IsRootNode() &&
           node->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) == VisibleType::VISIBLE &&
           level != NG::AccessibilityProperty::Level::NO_STR &&
           level != NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS;
}

bool RequestFocus(RefPtr<NG::FrameNode>& frameNode)
{
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    return focusHub->RequestFocusImmediately();
}

bool LostFocus(const RefPtr<NG::FrameNode>& frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    focusHub->LostFocus();
    return true;
}

bool ActClick(RefPtr<NG::FrameNode>& frameNode)
{
    auto gesture = frameNode->GetEventHub<NG::EventHub>()->GetGestureEventHub();
    CHECK_NULL_RETURN(gesture, false);
    return gesture->ActClick();
}

bool ActLongClick(RefPtr<NG::FrameNode>& frameNode)
{
    auto gesture = frameNode->GetEventHub<NG::EventHub>()->GetGestureEventHub();
    CHECK_NULL_RETURN(gesture, false);
    return gesture->ActLongClick();
}

void ClearAccessibilityFocus(const RefPtr<NG::FrameNode>& root, int64_t focusNodeId)
{
    auto oldFocusNode = GetFramenodeByAccessibilityId(root, focusNodeId);
    CHECK_NULL_VOID(oldFocusNode);
    bool isAccessibilityVirtualNode = oldFocusNode->IsAccessibilityVirtualNode();
    RefPtr<NG::RenderContext> renderContext = nullptr;
    if (isAccessibilityVirtualNode) {
        auto parentUinode = oldFocusNode->GetVirtualNodeParent().Upgrade();
        CHECK_NULL_VOID(parentUinode);
        auto parentFrame = AceType::DynamicCast<NG::FrameNode>(parentUinode);
        CHECK_NULL_VOID(parentFrame);
        renderContext = parentFrame->GetRenderContext();
    } else {
        renderContext = oldFocusNode->GetRenderContext();
    }
    CHECK_NULL_VOID(renderContext);
    if (oldFocusNode->GetTag() != V2::WEB_CORE_TAG) {
        if (isAccessibilityVirtualNode) {
            renderContext->UpdateAccessibilityFocus(false, oldFocusNode->GetAccessibilityId());
        } else {
            renderContext->UpdateAccessibilityFocus(false);
        }
    }
}

bool ActAccessibilityFocus(int64_t elementId, RefPtr<NG::FrameNode>& frameNode, RefPtr<NG::PipelineContext>& context,
    int64_t& currentFocusNodeId, bool isNeedClear)
{
    CHECK_NULL_RETURN(frameNode, false);
    bool isAccessibilityVirtualNode = frameNode->IsAccessibilityVirtualNode();
    RefPtr<NG::RenderContext> renderContext = nullptr;
    if (isAccessibilityVirtualNode) {
        auto parentUinode = frameNode->GetVirtualNodeParent().Upgrade();
        CHECK_NULL_RETURN(parentUinode, false);
        auto parentFrame = AceType::DynamicCast<NG::FrameNode>(parentUinode);
        CHECK_NULL_RETURN(parentFrame, false);
        renderContext = parentFrame->GetRenderContext();
    } else {
        renderContext = frameNode->GetRenderContext();
    }
    CHECK_NULL_RETURN(renderContext, false);
    if (isNeedClear) {
        if (elementId != currentFocusNodeId) {
            return false;
        }
        renderContext->UpdateAccessibilityFocus(false);
        currentFocusNodeId = -1;
        return true;
    }
    if (elementId == currentFocusNodeId) {
        return false;
    }
    Framework::ClearAccessibilityFocus(context->GetRootElement(), currentFocusNodeId);
    if (isAccessibilityVirtualNode) {
        auto rect = frameNode->GetTransformRectRelativeToWindow();
        NG::RectT<int32_t> rectInt { static_cast<int32_t>(rect.Left()), static_cast<int32_t>(rect.Top()),
            static_cast<int32_t>(rect.Width()), static_cast<int32_t>(rect.Height()) };
        renderContext->UpdateAccessibilityFocusRect(rectInt);
        renderContext->UpdateAccessibilityFocus(true, frameNode->GetAccessibilityId());
    } else {
        renderContext->ResetAccessibilityFocusRect();
        renderContext->UpdateAccessibilityFocus(true);
    }
    currentFocusNodeId = frameNode->GetAccessibilityId();
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    accessibilityProperty->OnAccessibilityFocusCallback(true);
    return true;
}

inline string GetSupportAction(const std::unordered_set<AceAction>& supportAceActions)
{
    std::string actionForDump;
    for (const auto& action : supportAceActions) {
        if (!actionForDump.empty()) {
            actionForDump.append(",");
        }
        actionForDump.append(std::to_string(static_cast<int32_t>(action)));
    }
    return actionForDump;
}

static std::string ConvertActionTypeToString(ActionType action)
{
    switch (action) {
        case ActionType::ACCESSIBILITY_ACTION_FOCUS:
            return "ACCESSIBILITY_ACTION_FOCUS";
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS:
            return "ACCESSIBILITY_ACTION_CLEAR_FOCUS";
        case ActionType::ACCESSIBILITY_ACTION_SELECT:
            return "ACCESSIBILITY_ACTION_SELECT";
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION:
            return "ACCESSIBILITY_ACTION_CLEAR_SELECTION";
        case ActionType::ACCESSIBILITY_ACTION_CLICK:
            return "ACCESSIBILITY_ACTION_CLICK";
        case ActionType::ACCESSIBILITY_ACTION_LONG_CLICK:
            return "ACCESSIBILITY_ACTION_LONG_CLICK";
        case ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS:
            return "ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS";
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS:
            return "ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS";
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD:
            return "ACCESSIBILITY_ACTION_SCROLL_FORWARD";
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD:
            return "ACCESSIBILITY_ACTION_SCROLL_BACKWARD";
        case ActionType::ACCESSIBILITY_ACTION_COPY:
            return "ACCESSIBILITY_ACTION_COPY";
        case ActionType::ACCESSIBILITY_ACTION_PASTE:
            return "ACCESSIBILITY_ACTION_PASTE";
        case ActionType::ACCESSIBILITY_ACTION_CUT:
            return "ACCESSIBILITY_ACTION_CUT";
        case ActionType::ACCESSIBILITY_ACTION_SET_SELECTION:
            return "ACCESSIBILITY_ACTION_SET_SELECTION";
        case ActionType::ACCESSIBILITY_ACTION_SET_TEXT:
            return "ACCESSIBILITY_ACTION_SET_TEXT";
        case ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT:
            return "ACCESSIBILITY_ACTION_NEXT_TEXT";
        case ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT:
            return "ACCESSIBILITY_ACTION_PREVIOUS_TEXT";
        case ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION:
            return "ACCESSIBILITY_ACTION_SET_CURSOR_POSITION";
        default:
            return "ACCESSIBILITY_ACTION_INVALID";
    }
}

static AceAction ConvertAccessibilityAction(ActionType accessibilityAction)
{
    static const ActionTable actionTable[] = {
        { AceAction::ACTION_CLICK, ActionType::ACCESSIBILITY_ACTION_CLICK },
        { AceAction::ACTION_LONG_CLICK, ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
        { AceAction::ACTION_SCROLL_FORWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
        { AceAction::ACTION_SCROLL_BACKWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
        { AceAction::ACTION_FOCUS, ActionType::ACCESSIBILITY_ACTION_FOCUS },
        { AceAction::ACTION_CLEAR_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
        { AceAction::ACTION_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
        { AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
        { AceAction::ACTION_SET_TEXT, ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
        { AceAction::ACTION_COPY, ActionType::ACCESSIBILITY_ACTION_COPY },
        { AceAction::ACTION_PASTE, ActionType::ACCESSIBILITY_ACTION_PASTE },
        { AceAction::ACTION_CUT, ActionType::ACCESSIBILITY_ACTION_CUT },
        { AceAction::ACTION_SELECT, ActionType::ACCESSIBILITY_ACTION_SELECT },
        { AceAction::ACTION_CLEAR_SELECTION, ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
        { AceAction::ACTION_SET_SELECTION, ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
        { AceAction::ACTION_SET_CURSOR_POSITION, ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION },
    };
    for (const auto& item : actionTable) {
        if (accessibilityAction == item.action) {
            return item.aceAction;
        }
    }
    return AceAction::ACTION_NONE;
}

inline string ChildernToString(const vector<int64_t>& children, int32_t treeId)
{
    std::string ids;
    for (auto child : children) {
        if (!ids.empty()) {
            ids.append(",");
        }
        int64_t childId = child;

        JsAccessibilityManager::SetSplicElementIdTreeId(treeId, childId);
        ids.append(std::to_string(childId));
    }
    return ids;
}

void GenerateAccessibilityEventInfo(const AccessibilityEvent& accessibilityEvent, AccessibilityEventInfo& eventInfo)
{
    Accessibility::EventType type = Accessibility::EventType::TYPE_VIEW_INVALID;
    if (accessibilityEvent.type != AccessibilityEventType::UNKNOWN) {
        type = ConvertAceEventType(accessibilityEvent.type);
    } else {
        type = ConvertStrToEventType(accessibilityEvent.eventType);
    }

    if (type == Accessibility::EventType::TYPE_VIEW_INVALID) {
        return;
    }

    eventInfo.SetTimeStamp(GetMicroTickCount());
    eventInfo.SetBeforeText(accessibilityEvent.beforeText);
    eventInfo.SetLatestContent(accessibilityEvent.latestContent);
    eventInfo.SetWindowChangeTypes(static_cast<Accessibility::WindowUpdateType>(accessibilityEvent.windowChangeTypes));
    eventInfo.SetWindowContentChangeTypes(
        static_cast<Accessibility::WindowsContentChangeTypes>(accessibilityEvent.windowContentChangeTypes));
    eventInfo.SetSource(accessibilityEvent.nodeId);
    eventInfo.SetEventType(type);
    eventInfo.SetCurrentIndex(static_cast<int>(accessibilityEvent.currentItemIndex));
    eventInfo.SetItemCounts(static_cast<int>(accessibilityEvent.itemCount));
    eventInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());
}
} // namespace

void JsAccessibilityManager::UpdateVirtualNodeFocus()
{
}

JsAccessibilityManager::~JsAccessibilityManager()
{
    UnsubscribeStateObserver();

    DeregisterInteractionOperation();
}

bool JsAccessibilityManager::SubscribeStateObserver()
{
    if (!stateObserver_) {
        stateObserver_ = std::make_shared<JsAccessibilityStateObserver>();
    }

    stateObserver_->SetAccessibilityManager(WeakClaim(this));
    stateObserver_->SetPipeline(context_);
    void* ptr = stateObserver_.get();
    bool bRet = JsAccessibilityManagerJni::RegisterJsAccessibilityStateObserver(ptr, windowId_);
    return bRet;
}

bool JsAccessibilityManager::UnsubscribeStateObserver()
{
    bool bRet = JsAccessibilityManagerJni::UnregisterJsAccessibilityStateObserver(windowId_);
    return bRet;
}

void JsAccessibilityManager::InitializeCallback()
{
    if (IsRegister()) {
        return;
    }

    auto pipelineContext = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(pipelineContext);
    windowId_ = pipelineContext->GetWindowId();

    bool isEnabled = JsAccessibilityManagerJni::isEnabled(windowId_);
    AceApplicationInfo::GetInstance().SetAccessibilityEnabled(isEnabled);

    if (pipelineContext->IsFormRender() || pipelineContext->IsJsCard() || pipelineContext->IsJsPlugin()) {
        return;
    }

    SubscribeStateObserver();
    if (isEnabled) {
        RegisterInteractionOperation(windowId_);
    }
}

std::vector<int> ConvertAceActionToAD(std::vector<Accessibility::AccessibleAction>& actionList)
{
    std::vector<int> actionListAD;
    for (auto action : actionList) {
        auto aceAction = ConvertAccessibilityAction(action.GetActionType());
        auto androidAction = static_cast<int>(AceActionConvertAndroid(aceAction));
        actionListAD.emplace_back(androidAction);
    }
    return actionListAD;
}
void AddEventInfoJsonImporved(Json& eventInfoJson, AccessibilityElementInfo& elementInfo)
{
    eventInfoJson["IsCheckable"] = elementInfo.IsCheckable();
    eventInfoJson["IsChecked"] = elementInfo.IsChecked();
    eventInfoJson["IsFocusable"] = elementInfo.IsFocusable();
    eventInfoJson["IsFocused"] = elementInfo.IsFocused();
    eventInfoJson["IsVisible"] = elementInfo.IsVisible();
    eventInfoJson["HasAccessibilityFocus"] = elementInfo.HasAccessibilityFocus();
    eventInfoJson["IsSelected"] = elementInfo.IsSelected();
    eventInfoJson["IsClickable"] = elementInfo.IsClickable();
    eventInfoJson["IsLongClickable"] = elementInfo.IsLongClickable();
    eventInfoJson["IsEnabled"] = elementInfo.IsEnabled();
    eventInfoJson["IsPassword"] = elementInfo.IsPassword();
    eventInfoJson["IsScrollable"] = elementInfo.IsScrollable();
    eventInfoJson["IsEditable"] = elementInfo.IsEditable();
    eventInfoJson["IsEssential"] = elementInfo.IsEssential();
    eventInfoJson["BundleName"] = elementInfo.GetBundleName();
    eventInfoJson["ComponentType"] = elementInfo.GetComponentType();
    eventInfoJson["Content"] = elementInfo.GetContent();
    eventInfoJson["Hint"] = elementInfo.GetHint();
    eventInfoJson["DescriptionInfo"] = elementInfo.GetDescriptionInfo();
    eventInfoJson["ComponentResourceId"] = elementInfo.GetComponentResourceId();
    eventInfoJson["AccessibilityId"] = elementInfo.GetAccessibilityId();
    eventInfoJson["SelectedBegin"] = elementInfo.GetSelectedBegin();
    eventInfoJson["SelectedEnd"] = elementInfo.GetSelectedEnd();
    eventInfoJson["PageId"] = elementInfo.GetPageId();
    std::vector<std::string> contentList;
    elementInfo.GetContentList(contentList);
    Json contentListJson = contentList;
    eventInfoJson["ContentList"] = contentListJson.dump();
    eventInfoJson["AccessibilityText"] = elementInfo.GetAccessibilityText();
    eventInfoJson["Offset"] = elementInfo.GetOffset();
    eventInfoJson["LiveRegion"] = elementInfo.GetLiveRegion();
    eventInfoJson["ParentNodeId"] = elementInfo.GetParentNodeId();
    eventInfoJson["GirdInfoRows"] = elementInfo.GetGrid().GetRowCount();
    eventInfoJson["GirdInfoColumns"] = elementInfo.GetGrid().GetColumnCount();
    AndroidMovementGranularity granularity = TextMoveUnitConvertAndroid(elementInfo.GetTextMovementStep());
    eventInfoJson["TextMoveStep"] = static_cast<int32_t>(granularity);
    eventInfoJson["InputType"] = elementInfo.GetInputType();
    std::vector<AccessibleAction> actionList = elementInfo.GetActionList();
    std::vector<int> actionListAD = ConvertAceActionToAD(actionList);
    Json actionListJson = actionListAD;
    eventInfoJson["ActionList"] = actionListJson.dump();
    GridItemInfo gridItem = elementInfo.GetGridItem();
    eventInfoJson["IsHeading"] = gridItem.IsHeading();
    std::vector<int64_t> childIDs = elementInfo.GetChildIds();
    Json childIDsJson = childIDs;
    eventInfoJson["childIDs"] = childIDsJson.dump();
}

bool JsAccessibilityManager::SendAccessibilitySyncEvent(
    const AccessibilityEvent& accessibilityEvent, AccessibilityEventInfo eventInfo)
{
    if (!IsRegister()) {
        return false;
    }

    if (!JsAccessibilityManagerJni::isEnabled(eventInfo.GetWindowId())) {
        return false;
    }

    int64_t elementId = eventInfo.GetAccessibilityId();
    SetSplicElementIdTreeId(treeId_, elementId);
    eventInfo.SetSource(elementId);
    AccessibilityElementInfo elementInfo = eventInfo.GetElementInfo();
    UpdateElementInfoTreeId(elementInfo);
    eventInfo.SetElementInfo(elementInfo);

    Json eventInfoJson = {};
    eventInfoJson["packageName"] = eventInfo.GetBundleName();
    eventInfoJson["source"] = eventInfo.GetAccessibilityId();
    eventInfoJson["currentIndex"] = eventInfo.GetCurrentIndex();
    eventInfoJson["beginIndex"] = eventInfo.GetBeginIndex();
    eventInfoJson["endIndex"] = eventInfo.GetEndIndex();
    eventInfoJson["itemCounts"] = eventInfo.GetItemCounts();
    eventInfoJson["beforeText"] = eventInfo.GetBeforeText();
    eventInfoJson["windowId"] = eventInfo.GetWindowId();
    eventInfoJson["latestContent"] = eventInfo.GetLatestContent();
    eventInfoJson["contentChangeType"] = static_cast<uint32_t>(eventInfo.GetWindowContentChangeTypes());
    AndroidActionType action = ActionTypeConvertAndroid(eventInfo.GetTriggerAction());
    eventInfoJson["actionType"] = static_cast<uint32_t>(action);
    eventInfoJson["TextLengthLimit"] = elementInfo.GetTextLengthLimit();
    eventInfoJson["ParentNodeId"] = elementInfo.GetParentNodeId();
    AddEventInfoJsonImporved(eventInfoJson, elementInfo);

    AndroidEventType eventType = AndroidEventType::TYPE_VIEW_INVALID;
    if (accessibilityEvent.type != AccessibilityEventType::UNKNOWN) {
        eventType = ConvertAndroidEventType(accessibilityEvent.type);
    } else {
        eventType = ConvertStrToAndroidEventType(accessibilityEvent.eventType);
    }
    int32_t androidEventType = static_cast<int32_t>(eventType);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "send accessibility eventType:%{public}s androidEvent:%{public}d  windowContentChangeTypes:%{public}d "
        "windowChangeTypes:%{public}d accessibilityId:%{public}" PRId64,
        accessibilityEvent.eventType.c_str(), androidEventType, accessibilityEvent.windowContentChangeTypes,
        accessibilityEvent.windowChangeTypes, eventInfo.GetAccessibilityId());
    return JsAccessibilityManagerJni::SendAccessibilityEvent(
        elementId, androidEventType, eventInfoJson.dump().c_str(), eventInfo.GetWindowId());
}

void JsAccessibilityManager::FillEventInfoWithNode(
    const RefPtr<NG::FrameNode>& node,
    AccessibilityEventInfo& eventInfo,
    const RefPtr<NG::PipelineContext>& context,
    int64_t elementId)
{
    CHECK_NULL_VOID(node);
    if (node->GetTag() == V2::WEB_CORE_TAG) {
        FillEventInfo(
            node, eventInfo, context, Claim(this), FillEventInfoParam { elementId, -1, eventInfo.GetWindowId() });
        return;
    }
    eventInfo.SetComponentType(node->GetTag());
    eventInfo.SetPageId(node->GetPageId());
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    eventInfo.AddContent(accessibilityProperty->GetGroupText());
    eventInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    eventInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    eventInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    AccessibilityElementInfo elementInfo;

    CommonProperty commonProperty;
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    GenerateCommonProperty(context, commonProperty, mainContext);
    UpdateAccessibilityElementInfo(node, commonProperty, elementInfo, context);
    elementInfo.SetWindowId(eventInfo.GetWindowId());
    eventInfo.SetElementInfo(elementInfo);
}

void JsAccessibilityManager::SendEventToAccessibilityWithNode(
    const AccessibilityEvent& accessibilityEvent, const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context)
{
    CHECK_NULL_VOID(node);
    CHECK_NULL_VOID(context);
    int32_t windowId = static_cast<int32_t>(context->GetFocusWindowId());
    if (windowId == 0) {
        return;
    }
    if (!AceType::InstanceOf<NG::FrameNode>(node)) {
        return;
    }
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    CHECK_NULL_VOID(frameNode);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

    if ((!frameNode->IsActive()) || frameNode->CheckAccessibilityLevelNo()) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "node: %{public}" PRId64 ", is not active or level is no",
            frameNode->GetAccessibilityId());
        return;
    }

    AccessibilityEventInfo eventInfo;

    if (accessibilityEvent.type != AccessibilityEventType::PAGE_CHANGE || accessibilityEvent.windowId == 0) {
        eventInfo.SetWindowId(windowId);
    } else {
        eventInfo.SetWindowId(accessibilityEvent.windowId);
    }
    FillEventInfoWithNode(frameNode, eventInfo, ngPipeline, accessibilityEvent.nodeId);
    if ((ngPipeline != nullptr) && (ngPipeline->IsFormRender())) {
        eventInfo.SetWindowId(static_cast<int32_t>(GetWindowId()));
    }
    GenerateAccessibilityEventInfo(accessibilityEvent, eventInfo);

    SendAccessibilitySyncEvent(accessibilityEvent, eventInfo);
}

void GetRealEventWindowId(
    const AccessibilityEvent& accessibilityEvent, const RefPtr<NG::PipelineContext>& ngPipeline, uint32_t& windowId)
{
    if ((accessibilityEvent.type == AccessibilityEventType::PAGE_CHANGE && accessibilityEvent.windowId != 0) ||
        accessibilityEvent.windowChangeTypes == WINDOW_UPDATE_ADDED) {
        windowId = accessibilityEvent.windowId;
        return;
    }
    if ((ngPipeline != nullptr) && (ngPipeline->IsFormRender())) {
        return;
    }
    windowId = ngPipeline->GetFocusWindowId();
}
void JsAccessibilityManager::SendAccessibilityAsyncEvent(const AccessibilityEvent& accessibilityEvent)
{
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    int32_t windowId = static_cast<int32_t>(context->GetFocusWindowId());
    if (windowId == 0) {
        return;
    }
    RefPtr<NG::PipelineContext> ngPipeline;
    AccessibilityEventInfo eventInfo;
    uint32_t realWindowId = GetWindowId();
    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        RefPtr<NG::FrameNode> node;
        ngPipeline = FindPipelineByElementId(accessibilityEvent.nodeId, node);
        CHECK_NULL_VOID(ngPipeline);
        CHECK_NULL_VOID(node);
        GetRealEventWindowId(accessibilityEvent, ngPipeline, realWindowId);
        FillEventInfo(node, eventInfo, ngPipeline, Claim(this),
            FillEventInfoParam {
                accessibilityEvent.nodeId, accessibilityEvent.stackNodeId, realWindowId });
        eventInfo.SetWindowId(realWindowId);
    } else {
        ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
        auto node = GetAccessibilityNodeFromPage(accessibilityEvent.nodeId);
        CHECK_NULL_VOID(node);
        FillEventInfo(node, eventInfo);
        eventInfo.SetWindowId(windowId);
    }
    if (accessibilityEvent.type == AccessibilityEventType::PAGE_CHANGE && accessibilityEvent.windowId != 0) {
        eventInfo.SetWindowId(accessibilityEvent.windowId);
    }
    if ((ngPipeline != nullptr) && (ngPipeline->IsFormRender())) {
        eventInfo.SetWindowId(static_cast<int32_t>(GetWindowId()));
    }

    GenerateAccessibilityEventInfo(accessibilityEvent, eventInfo);

    SendAccessibilitySyncEvent(accessibilityEvent, eventInfo);
}

void JsAccessibilityManager::UpdateNodeChildIds(const RefPtr<AccessibilityNode>& node)
{
    CHECK_NULL_VOID(node);
    node->ActionUpdateIds();
    const auto& children = node->GetChildList();
    std::vector<int32_t> childrenVec;
    auto cardId = GetCardId();
    auto rootNodeId = GetRootNodeId();

    // get last stack children to barrier free service.
    if ((node->GetNodeId() == GetRootNodeId() + ROOT_STACK_BASE) && !children.empty() && !IsDeclarative()) {
        UpdateSpecialChildren(node, children, childrenVec, cardId, rootNodeId);
    } else {
        UpdateRegularChildren(node, children, childrenVec, cardId, rootNodeId);
    }
    node->SetChildIds(childrenVec);
}

void JsAccessibilityManager::UpdateSpecialChildren(const RefPtr<AccessibilityNode>& node,
    const std::list<RefPtr<AccessibilityNode>>& children, std::vector<int32_t>& childrenVec, int32_t cardId,
    int32_t rootNodeId)
{
    auto lastChildNodeId = children.back()->GetNodeId();
    if (isOhosHostCard()) {
        childrenVec.emplace_back(ConvertToCardAccessibilityId(lastChildNodeId, cardId, rootNodeId));
    } else {
        childrenVec.emplace_back(lastChildNodeId);
        for (const auto& child : children) {
            if (child->GetNodeId() == ROOT_DECOR_BASE - 1) {
                childrenVec.emplace_back(child->GetNodeId());
                break;
            }
        }
    }
}

void JsAccessibilityManager::UpdateRegularChildren(const RefPtr<AccessibilityNode>& node,
    const std::list<RefPtr<AccessibilityNode>>& children, std::vector<int32_t>& childrenVec, int32_t cardId,
    int32_t rootNodeId)
{
    childrenVec.resize(children.size());
    if (isOhosHostCard()) {
        std::transform(children.begin(), children.end(), childrenVec.begin(),
            [cardId, rootNodeId](const RefPtr<AccessibilityNode>& child) {
                return ConvertToCardAccessibilityId(child->GetNodeId(), cardId, rootNodeId);
            });
    } else {
        std::transform(children.begin(), children.end(), childrenVec.begin(),
            [](const RefPtr<AccessibilityNode>& child) { return child->GetNodeId(); });
    }
}

void JsAccessibilityManager::ProcessParameters(
    ActionType op, const std::vector<std::string>& params, std::map<std::string, std::string>& paramsMap)
{
    if (op == ActionType::ACCESSIBILITY_ACTION_SET_TEXT) {
        if (params.size() == EVENT_DUMP_PARAM_LENGTH_UPPER) {
            paramsMap = { { ACTION_ARGU_SET_TEXT, params[EVENT_DUMP_ACTION_PARAM_INDEX] } };
        }
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) {
        paramsMap[ACTION_ARGU_SELECT_TEXT_START] = "-1";
        paramsMap[ACTION_ARGU_SELECT_TEXT_END] = "-1";
        paramsMap[ACTION_ARGU_SELECT_TEXT_INFORWARD] = STRING_DIR_BACKWARD;
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER) {
            paramsMap[ACTION_ARGU_SELECT_TEXT_START] = params[EVENT_DUMP_ACTION_PARAM_INDEX];
        }
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER + 1) {
            paramsMap[ACTION_ARGU_SELECT_TEXT_END] = params[EVENT_DUMP_ACTION_PARAM_INDEX + 1];
        }
        // 2 means params number Offset
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER + 2) {
            // 2 means params number Offset
            paramsMap[ACTION_ARGU_SELECT_TEXT_INFORWARD] = params[EVENT_DUMP_ACTION_PARAM_INDEX + 2];
        }
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT || op == ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT) {
        if (params.size() == EVENT_DUMP_PARAM_LENGTH_UPPER) {
            paramsMap[ACTION_ARGU_MOVE_UNIT] = std::to_string(TextMoveUnit::STEP_CHARACTER);
        }
        paramsMap[ACTION_ARGU_MOVE_UNIT] = std::to_string(TextMoveUnit::STEP_CHARACTER);
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION) {
        paramsMap[ACTION_ARGU_SET_OFFSET] = params[EVENT_DUMP_ACTION_PARAM_INDEX];
    }

    if ((op == ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD) ||
        (op == ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD)) {
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER) {
            paramsMap = { { ACTION_ARGU_SCROLL_STUB, params[EVENT_DUMP_ACTION_PARAM_INDEX] } };
        }
    }
}

RefPtr<NG::PipelineContext> JsAccessibilityManager::GetPipelineByWindowId(uint32_t windowId)
{
    auto mainPipeline = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
    if (mainPipeline != nullptr && mainPipeline->GetWindowId() == windowId) {
        return mainPipeline;
    }
    for (auto subPipelineWeak : GetSubPipelineContexts()) {
        auto subContextNG = AceType::DynamicCast<NG::PipelineContext>(subPipelineWeak.Upgrade());
        if (subContextNG != nullptr && subContextNG->GetWindowId() == windowId) {
            return subContextNG;
        }
    }
    if (GetWindowId() == windowId) {
        return mainPipeline;
    }
    return nullptr;
}

void JsAccessibilityManager::HandleComponentPostBinding()
{
    for (auto targetIter = nodeWithTargetMap_.begin(); targetIter != nodeWithTargetMap_.end();) {
        auto nodeWithTarget = targetIter->second.Upgrade();
        if (!nodeWithTarget) {
            nodeWithTargetMap_.erase(targetIter++);
            continue;
        }
        auto idNodeIter = nodeWithIdMap_.find(targetIter->first);
        if (nodeWithTarget->GetTag() != ACCESSIBILITY_TAG_POPUP || idNodeIter == nodeWithIdMap_.end()) {
            ++targetIter;
            continue;
        }
        auto nodeWithId = idNodeIter->second.Upgrade();
        if (nodeWithId) {
            nodeWithId->SetAccessibilityHint(nodeWithTarget->GetText());
        } else {
            nodeWithIdMap_.erase(idNodeIter);
        }
        ++targetIter;
    }
    // clear the disabled node in the maps
    for (auto idItem = nodeWithIdMap_.begin(); idItem != nodeWithIdMap_.end();) {
        if (!idItem->second.Upgrade()) {
            nodeWithIdMap_.erase(idItem++);
        } else {
            ++idItem;
        }
    }
}

RefPtr<AccessibilityNodeManager> AccessibilityNodeManager::Create()
{
    return AceType::MakeRefPtr<JsAccessibilityManager>();
}

RefPtr<PipelineBase> JsAccessibilityManager::GetPipelineByWindowId(const int32_t windowId)
{
    auto context = context_.Upgrade();
    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        CHECK_NULL_RETURN(context, nullptr);
        if (context->GetWindowId() == static_cast<uint32_t>(windowId)) {
            return context;
        }
        if (GetWindowId() == static_cast<uint32_t>(windowId)) {
            return context;
        }
        for (auto& subContext : GetSubPipelineContexts()) {
            context = subContext.Upgrade();
            CHECK_NULL_RETURN(context, nullptr);
            if (context->GetWindowId() == static_cast<uint32_t>(windowId)) {
                return context;
            }
        }
        return nullptr;
    } else {
        return context;
    }
}

void JsAccessibilityManager::SearchElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context, int64_t uiExtensionOffset)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);

    AccessibilityElementInfo nodeInfo;
    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }

    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext);
    auto node = GetFramenodeByAccessibilityId(rootNode, nodeId);
    CHECK_NULL_VOID(node);
    UpdateAccessibilityElementInfo(node, commonProperty, nodeInfo, ngPipeline);
    if (IsExtensionComponent(node) && !IsUIExtensionShowPlaceholder(node)) {
        SearchParameter param { -1, "", mode, uiExtensionOffset };
        SearchExtensionElementInfoNG(param, node, infos, nodeInfo);
    }
    infos.push_back(nodeInfo);
    SearchParameter param { nodeId, "", mode, uiExtensionOffset };
    UpdateCacheInfoNG(infos, node, commonProperty, ngPipeline, param);
    SortExtensionAccessibilityInfo(infos, nodeInfo.GetAccessibilityId());
    if ((infos.size() > 0) && (uiExtensionOffset != NG::UI_EXTENSION_OFFSET_MAX) &&
        (infos.front().GetComponentType() != V2::ROOT_ETS_TAG) &&
        (infos.front().GetParentNodeId() == rootNode->GetAccessibilityId())) {
            infos.front().SetParent(NG::UI_EXTENSION_ROOT_ID);
    }
}

void JsAccessibilityManager::FindFocusedElementInfoNG(int64_t elementId, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const int64_t uiExtensionOffset)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);

    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }
    auto node = GetFramenodeByAccessibilityId(rootNode, nodeId);
    if (!node) {
        return info.SetValidElement(false);
    }
    if (IsExtensionComponent(node) && !IsUIExtensionShowPlaceholder(node)) {
        SearchParameter transferSearchParam { NG::UI_EXTENSION_ROOT_ID, "", focusType, uiExtensionOffset };
        OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(transferSearchParam, node, info);
        return SetUiExtensionAbilityParentIdForFocus(node, uiExtensionOffset, info);
    }
    RefPtr<NG::FrameNode> resultNode;
    if (focusType == FOCUS_TYPE_ACCESSIBILITY) {
        FindAccessibilityFocusArg findAccessibilityFocusArg;
        findAccessibilityFocusArg.uiExtensionOffset = uiExtensionOffset;
        findAccessibilityFocusArg.currentFocusNodeId = currentFocusNodeId_;
        resultNode = FindAccessibilityFocus(node, focusType, info, context, findAccessibilityFocusArg);
    }
    if (focusType == FOCUS_TYPE_INPUT) {
        resultNode = FindInputFocus(node, focusType, info, uiExtensionOffset, context);
    }
    if ((!resultNode) || (IsExtensionComponent(resultNode) && !IsUIExtensionShowPlaceholder(resultNode))) {
        return;
    }
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext);
    UpdateAccessibilityElementInfo(resultNode, commonProperty, info, ngPipeline);
    UpdateUiExtensionParentIdForFocus(rootNode, uiExtensionOffset, info);
}

RefPtr<NG::FrameNode> JsAccessibilityManager::FindNodeFromPipeline(
    const WeakPtr<PipelineBase>& context, const int64_t elementId)
{
    auto pipeline = context.Upgrade();
    CHECK_NULL_RETURN(pipeline, nullptr);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(rootNode, nullptr);

    int64_t nodeId = elementId;
    // accessibility use -1 for first search to get root node
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }

    auto node = GetFramenodeByAccessibilityId(rootNode, nodeId);
    if (node) {
        return node;
    }
    return nullptr;
}

RefPtr<NG::PipelineContext> JsAccessibilityManager::FindPipelineByElementId(
    const int64_t elementId, RefPtr<NG::FrameNode>& node)
{
    node = FindNodeFromPipeline(context_, elementId);
    if (node) {
        auto context = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
        return context;
    }
    for (auto subContext : GetSubPipelineContexts()) {
        node = FindNodeFromPipeline(subContext, elementId);
        if (node) {
            auto context = AceType::DynamicCast<NG::PipelineContext>(subContext.Upgrade());
            return context;
        }
    }
    return nullptr;
}

void JsAccessibilityManager::JsInteractionOperation::ExecuteAction(const int64_t elementId, const int32_t action,
    const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", action: %{public}d", elementId, action);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;

    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    AndroidActionType androidAction = static_cast<AndroidActionType>(action);
    AceAction aceAction = AndroidConvertAceAction(androidAction);
    ActionType actionInfo = ConvertAceAction(aceAction);
    ActionParam param { actionInfo, actionArguments };
    jsAccessibilityManager->ExecuteAction(splitElementId, param, requestId, callback, windowId_);
}

bool JsAccessibilityManager::AccessibilityActionEvent(const ActionType& action,
    const std::map<std::string, std::string>& actionArguments, const RefPtr<AccessibilityNode>& node,
    const RefPtr<PipelineContext>& context)
{
    if (!node || !context) {
        return false;
    }
    ContainerScope scope(context->GetInstanceId());
    switch (action) {
        case ActionType::ACCESSIBILITY_ACTION_CLICK: {
            return ActionClick(actionArguments, node, context);
        }
        case ActionType::ACCESSIBILITY_ACTION_LONG_CLICK: {
            return ActionLongClick(actionArguments, node, context);
        }
        case ActionType::ACCESSIBILITY_ACTION_SET_TEXT: {
            return ActionSetText(actionArguments, node, context);
        }
        case ActionType::ACCESSIBILITY_ACTION_FOCUS: {
            return ActionFocus(actionArguments, node, context);
        }
        case ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS: {
            return RequestAccessibilityFocus(node);
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
            return ClearAccessibilityFocus(node);
        }
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD: {
            return node->ActionScrollForward();
        }
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD: {
            return node->ActionScrollBackward();
        }
        default:
            return false;
    }
}

bool JsAccessibilityManager::ActionClick(const std::map<std::string, std::string>& actionArguments,
    const RefPtr<AccessibilityNode>& node, const RefPtr<PipelineContext>& context)
{
    node->SetClicked(true);
    if (!node->GetClickEventMarker().IsEmpty()) {
#ifndef NG_BUILD
        context->SendEventToFrontend(node->GetClickEventMarker());
#endif
        node->ActionClick();
        return true;
    }
    return node->ActionClick();
}

bool JsAccessibilityManager::ActionLongClick(const std::map<std::string, std::string>& actionArguments,
    const RefPtr<AccessibilityNode>& node, const RefPtr<PipelineContext>& context)
{
    if (!node->GetLongPressEventMarker().IsEmpty()) {
#ifndef NG_BUILD
        context->SendEventToFrontend(node->GetLongPressEventMarker());
#endif
        node->ActionLongClick();
        return true;
    }
    return node->ActionLongClick();
}

bool JsAccessibilityManager::ActionSetText(const std::map<std::string, std::string>& actionArguments,
    const RefPtr<AccessibilityNode>& node, const RefPtr<PipelineContext>& context)
{
    if (!node->GetSetTextEventMarker().IsEmpty()) {
#ifndef NG_BUILD
        context->SendEventToFrontend(node->GetSetTextEventMarker());
#endif
        node->ActionSetText(actionArguments.find(ACTION_ARGU_SET_TEXT)->second);
        return true;
    }
    return node->ActionSetText(actionArguments.find(ACTION_ARGU_SET_TEXT)->second);
}

bool JsAccessibilityManager::ActionFocus(const std::map<std::string, std::string>& actionArguments,
    const RefPtr<AccessibilityNode>& node, const RefPtr<PipelineContext>& context)
{
#ifndef NG_BUILD
    context->AccessibilityRequestFocus(std::to_string(node->GetNodeId()));
#endif
    if (!node->GetFocusEventMarker().IsEmpty()) {
#ifndef NG_BUILD
        context->SendEventToFrontend(node->GetFocusEventMarker());
#endif
        node->ActionFocus();
        return true;
    }
    return node->ActionFocus();
}

void JsAccessibilityManager::SendActionEvent(const Accessibility::ActionType& action, int64_t nodeId)
{
    static std::unordered_map<Accessibility::ActionType, std::string> actionToStr {
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK, DOM_CLICK },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_LONG_CLICK, DOM_LONG_PRESS },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_FOCUS, DOM_FOCUS },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, ACCESSIBILITY_FOCUSED_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS, ACCESSIBILITY_CLEAR_FOCUS_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD, SCROLL_END_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD, SCROLL_END_EVENT },
    };
    if (actionToStr.find(action) == actionToStr.end()) {
        return;
    }
    AccessibilityEvent accessibilityEvent;
    accessibilityEvent.eventType = actionToStr[action];
    accessibilityEvent.nodeId = static_cast<int64_t>(nodeId);
    SendAccessibilityAsyncEvent(accessibilityEvent);
}

void stringToLower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char& c) {
        return std::tolower(c);
    });
}

bool conversionDirection(std::string dir)
{
    stringToLower(dir);
    if (dir.compare(STRING_DIR_FORWARD) == 0) {
        return true;
    }

    return false;
}

inline void getArgumentByKey(const std::map<std::string, std::string>& actionArguments, const std::string& checkKey,
    int32_t& argument)
{
    auto iter = actionArguments.find(checkKey);
    if (iter != actionArguments.end()) {
        std::stringstream strArguments;
        strArguments << iter->second;
        strArguments >> argument;
    }
}

void PrepareActionSetSelection(AccessibilityActionParam& param,
    const std::map<std::string, std::string>& actionArguments, RefPtr<NG::AccessibilityProperty>& accessibilityProperty)
{
    int start = -1;
    int end = -1;
    std::string dir = STRING_DIR_BACKWARD;
    auto iter = actionArguments.find(ACTION_ARGU_SELECT_TEXT_START);
    if (iter != actionArguments.end()) {
        std::stringstream str_start;
        str_start << iter->second;
        str_start >> start;
    }
    iter = actionArguments.find(ACTION_ARGU_SELECT_TEXT_END);
    if (iter != actionArguments.end()) {
        std::stringstream str_end;
        str_end << iter->second;
        str_end >> end;
    }
    iter = actionArguments.find(ACTION_ARGU_SELECT_TEXT_INFORWARD);
    if (iter != actionArguments.end()) {
        dir = iter->second;
    }
    param.setSelectionStart = start;
    param.setSelectionEnd = end;
    param.setSelectionDir = conversionDirection(dir);
}

void PrepareActionSetText(AccessibilityActionParam& param, const std::map<std::string, std::string>& actionArguments,
    RefPtr<NG::AccessibilityProperty>& accessibilityProperty)
{
    auto iter = actionArguments.find(ACTION_ARGU_SET_TEXT);
    if (iter != actionArguments.end()) {
        param.setTextArgument = iter->second;
    }
}

void PrepareActionNextAndPreText(AccessibilityActionParam& param,
    const std::map<std::string, std::string>& actionArguments, RefPtr<NG::AccessibilityProperty>& accessibilityProperty)
{
    int moveUnit = TextMoveUnit::STEP_CHARACTER;
    auto iter = actionArguments.find(ACTION_ARGU_MOVE_UNIT);
    if (iter != actionArguments.end()) {
        std::stringstream str_moveUnit;
        str_moveUnit << iter->second;
        str_moveUnit >> moveUnit;
    }
    param.moveUnit = static_cast<TextMoveUnit>(moveUnit);
}

void PrepareActionSetCursor(AccessibilityActionParam& param, const std::map<std::string, std::string>& actionArguments,
    RefPtr<NG::AccessibilityProperty>& accessibilityProperty)
{
    auto iter = actionArguments.find(ACTION_ARGU_SET_OFFSET);
    int32_t position = -1;
    if (iter != actionArguments.end()) {
        std::stringstream strPosition;
        strPosition << iter->second;
        strPosition >> position;
    }
    param.setCursorIndex = position;
}

void PrepareActionScroll(AccessibilityActionParam& param, const std::map<std::string, std::string>& actionArguments,
    RefPtr<NG::AccessibilityProperty>& accessibilityProperty)
{
    int32_t scrollType = static_cast<int32_t>(AccessibilityScrollType::SCROLL_DEFAULT);
    getArgumentByKey(actionArguments, ACTION_ARGU_SCROLL_STUB, scrollType);
    if ((scrollType < static_cast<int32_t>(AccessibilityScrollType::SCROLL_DEFAULT)) ||
        (scrollType > static_cast<int32_t>(AccessibilityScrollType::SCROLL_MAX_TYPE))) {
        scrollType = static_cast<int32_t>(AccessibilityScrollType::SCROLL_DEFAULT);
    }
    param.scrollType = static_cast<AccessibilityScrollType>(scrollType);
}

bool ActAccessibilityAction(Accessibility::ActionType action, const std::map<std::string, std::string>& actionArguments,
    RefPtr<NG::AccessibilityProperty> accessibilityProperty)
{
    AccessibilityActionParam param;
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) {
        PrepareActionSetSelection(param, actionArguments, accessibilityProperty);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_TEXT) {
        PrepareActionSetText(param, actionArguments, accessibilityProperty);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT ||
        action == ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT) {
        PrepareActionNextAndPreText(param, actionArguments, accessibilityProperty);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION) {
        PrepareActionSetCursor(param, actionArguments, accessibilityProperty);
    }
    if ((action == ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD) ||
        (action == ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD)) {
        PrepareActionScroll(param, actionArguments, accessibilityProperty);
    }
    auto accessibiltyAction = ACTIONS.find(action);
    if (accessibiltyAction != ACTIONS.end()) {
        param.accessibilityProperty = accessibilityProperty;
        return accessibiltyAction->second(param);
    }
    return false;
}

bool JsAccessibilityManager::ExecuteActionNG(int64_t elementId,
    const std::map<std::string, std::string>& actionArguments, ActionType action, const RefPtr<PipelineBase>& context,
    int64_t uiExtensionOffset)
{
    bool result = false;
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, result);
    ContainerScope instance(ngPipeline->GetInstanceId());
    auto frameNode = GetFramenodeByAccessibilityId(ngPipeline->GetRootElement(), elementId);
    if (!frameNode && elementId == lastElementId_) {
        frameNode = lastFrameNode_.Upgrade();
    }
    CHECK_NULL_RETURN(frameNode, result);
    auto enabled = frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsEnabled() : true;
    if (!enabled && action != ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS &&
        action != ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS) {
        return result;
    }
    result = ConvertActionTypeToBoolen(action, frameNode, elementId, ngPipeline);
    if (!result) {
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        CHECK_NULL_RETURN(accessibilityProperty, false);
        result = ActAccessibilityAction(action, actionArguments, accessibilityProperty);
    }
    return result;
}

bool JsAccessibilityManager::ConvertActionTypeToBoolen(ActionType action, RefPtr<NG::FrameNode>& frameNode,
    int64_t elementId, RefPtr<NG::PipelineContext>& context)
{
    bool result = false;
    switch (action) {
        case ActionType::ACCESSIBILITY_ACTION_FOCUS: {
            result = RequestFocus(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS: {
            result = LostFocus(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLICK: {
            result = ActClick(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_LONG_CLICK: {
            result = ActLongClick(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS: {
            SaveLast(elementId, frameNode);
            result = ActAccessibilityFocus(elementId, frameNode, context, currentFocusNodeId_, false);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
            SaveLast(elementId, frameNode);
            result = ActAccessibilityFocus(elementId, frameNode, context, currentFocusNodeId_, true);
            break;
        }
        default:
            break;
    }
    return result;
}

void JsAccessibilityManager::ExecuteAction(const int64_t elementId, const ActionParam& param, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    auto action = param.action;
    auto actionArguments = param.actionArguments;

    bool actionResult = false;
    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        return;
    }

    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        actionResult = ExecuteActionNG(elementId, actionArguments, action, context, NG::UI_EXTENSION_OFFSET_MAX);
    } else {
        auto node = GetAccessibilityNodeFromPage(elementId);
        if (!node) {
            return;
        }

        actionResult =
            AccessibilityActionEvent(action, actionArguments, node, AceType::DynamicCast<PipelineContext>(context));
    }
    if (actionResult && AceType::InstanceOf<PipelineContext>(context)) {
        SendActionEvent(action, elementId);
    }
}

int JsAccessibilityManager::RegisterInteractionOperation(int windowId)
{
    if (IsRegister()) {
        return 0;
    }

    auto interactionOperation = std::make_shared<JsInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    bool bRet = JsAccessibilityManagerJni::RegisterJsInteractionOperation(windowId, interactionOperation);
    RefPtr<PipelineBase> context;
    for (auto subContext : GetSubPipelineContexts()) {
        context = subContext.Upgrade();
        CHECK_NULL_RETURN(context, -1);
        interactionOperation = std::make_shared<JsInteractionOperation>(context->GetWindowId());
        interactionOperation->SetHandler(WeakClaim(this));
        bRet = JsAccessibilityManagerJni::RegisterJsInteractionOperation(context->GetWindowId(), interactionOperation);
    }
    Register(bRet);
    return bRet;
}

void JsAccessibilityManager::RegisterSubWindowInteractionOperation(int windowId)
{
    if (!AceApplicationInfo::GetInstance().IsAccessibilityEnabled() || !IsRegister()) {
        return;
    }

    auto interactionOperation = std::make_shared<JsInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    JsAccessibilityManagerJni::RegisterJsInteractionOperation(windowId, interactionOperation);
}

void JsAccessibilityManager::DeregisterInteractionOperation()
{
    if (!IsRegister()) {
        return;
    }
    int windowId = GetWindowId();

    Register(false);
    if (currentFocusNodeId_ != -1 && lastElementId_ != -1) {
        auto focusNode = lastFrameNode_.Upgrade();
        if (focusNode != nullptr && focusNode->GetTag() != V2::WEB_CORE_TAG) {
            focusNode->GetRenderContext()->UpdateAccessibilityFocus(false);
        }
    }
    lastFrameNode_.Reset();
    lastElementId_ = -1;
    currentFocusNodeId_ = -1;
    JsAccessibilityManagerJni::UnregisterJsInteractionOperation(windowId);
    RefPtr<PipelineBase> context;
    for (auto subContext : GetSubPipelineContexts()) {
        context = subContext.Upgrade();
        CHECK_NULL_VOID(context);
        JsAccessibilityManagerJni::UnregisterJsInteractionOperation(context->GetWindowId());
    }
}

void JsAccessibilityManager::RegisterAccessibilityChildTreeCallback(
    int64_t elementId, const std::shared_ptr<AccessibilityChildTreeCallback>& callback)
{
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    childTreeCallbackMap_[elementId] = callback;
}

void JsAccessibilityManager::DeregisterAccessibilityChildTreeCallback(int64_t elementId)
{
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    childTreeCallbackMap_.erase(elementId);
}

bool JsAccessibilityManager::CheckIsChildElement(
    int64_t& elementId, const std::vector<std::string>& params, std::vector<std::string>& info)
{
    if (elementId <= 0) {
        return false;
    }

    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;

    GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    if (splitTreeId <= 0 || splitTreeId == treeId_) {
        elementId = splitElementId;
        return false;
    }
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    for (const auto& item : childTreeCallbackMap_) {
        if (item.second == nullptr) {
            continue;
        }
        if (item.second->GetChildTreeId() != splitTreeId) {
            continue;
        }
        item.second->OnDumpChildInfo(params, info);
        for (const auto& childInfo : info) {
            DumpLog::GetInstance().Print(childInfo.c_str());
        }
        return true;
    }
    return false;
}

void JsAccessibilityManager::SetAccessibilityGetParentRectHandler(std::function<void(int32_t&, int32_t&)>&& callback)
{
    getParentRectHandler_ = std::move(callback);
}

void JsAccessibilityManager::UpdateElementInfoTreeId(Accessibility::AccessibilityElementInfo& info)
{
    int32_t treeId = info.GetBelongTreeId();
    if (treeId <= 0) {
        return;
    }

    int64_t elementId = info.GetAccessibilityId();
    SetSplicElementIdTreeId(treeId_, elementId);
    info.SetAccessibilityId(elementId);

    int64_t parentId = info.GetParentNodeId();
    if (parentId != INVALID_PARENT_ID) {
        SetSplicElementIdTreeId(treeId_, parentId);
        info.SetParent(parentId);
    }

    std::vector<int64_t> childIds = info.GetChildIds();
    for (int64_t child : childIds) {
        info.RemoveChild(child);
        SetSplicElementIdTreeId(treeId_, child);
        info.AddChild(child);
    }
}

void JsAccessibilityManager::UpdateElementInfosTreeId(std::list<Accessibility::AccessibilityElementInfo>& infos)
{
    for (auto& item : infos) {
        int32_t treeId = item.GetBelongTreeId();
        if (treeId <= 0) {
            continue;
        }

        int64_t elementId = item.GetAccessibilityId();
        SetSplicElementIdTreeId(treeId_, elementId);
        item.SetAccessibilityId(elementId);

        int64_t parentId = item.GetParentNodeId();
        if (parentId != INVALID_PARENT_ID) {
            SetSplicElementIdTreeId(treeId_, parentId);
            item.SetParent(parentId);
        }

        std::vector<int64_t> childIds = item.GetChildIds();
        for (int64_t child : childIds) {
            item.RemoveChild(child);
            SetSplicElementIdTreeId(treeId_, child);
            item.AddChild(child);
        }
    }
}

void JsAccessibilityManager::SetPipelineContext(const RefPtr<PipelineBase>& context)
{
    context_ = context;
    if (stateObserver_ != nullptr) {
        stateObserver_->SetPipeline(context_);
    }
}

void JsAccessibilityManager::JsAccessibilityStateObserver::OnStateChanged(const bool state)
{
    // Do not upgrade jsAccessibilityManager on async thread, destructor will cause freeze
    auto pipelineRef = pipeline_.Upgrade();
    CHECK_NULL_VOID(pipelineRef);
    auto jsAccessibilityManager = accessibilityManager_.Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    if (state) {
        jsAccessibilityManager->RegisterInteractionOperation(jsAccessibilityManager->GetWindowId());
    } else {
        jsAccessibilityManager->DeregisterInteractionOperation();
    }
    AceApplicationInfo::GetInstance().SetAccessibilityEnabled(state);
}

bool JsAccessibilityManager::RequestAccessibilityFocus(const RefPtr<AccessibilityNode>& node)
{
    auto requestNodeId = node->GetNodeId();
    if (currentFocusNodeId_ == requestNodeId) {
        return false;
    }

    ClearCurrentFocus();
    currentFocusNodeId_ = requestNodeId;
    node->SetAccessibilityFocusedState(true);
    return node->ActionAccessibilityFocus(true);
}

bool JsAccessibilityManager::ClearAccessibilityFocus(const RefPtr<AccessibilityNode>& node)
{
    auto requestNodeId = node->GetNodeId();
    if (currentFocusNodeId_ != requestNodeId) {
        return false;
    }

    currentFocusNodeId_ = -1;
    node->SetAccessibilityFocusedState(false);
    return node->ActionAccessibilityFocus(false);
}

bool JsAccessibilityManager::ClearCurrentFocus()
{
    auto currentFocusNode = GetAccessibilityNodeFromPage(currentFocusNodeId_);
    CHECK_NULL_RETURN(currentFocusNode, false);
    currentFocusNodeId_ = -1;
    currentFocusNode->SetFocusedState(false);
    currentFocusNode->SetAccessibilityFocusedState(false);
    return currentFocusNode->ActionAccessibilityFocus(false);
}

std::string JsAccessibilityManager::GetPagePath()
{
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, "");
    auto frontend = context->GetFrontend();
    CHECK_NULL_RETURN(frontend, "");
    ContainerScope scope(context->GetInstanceId());
    return frontend->GetPagePath();
}

void JsAccessibilityManager::GenerateCommonProperty(const RefPtr<PipelineBase>& context, CommonProperty& output,
    const RefPtr<PipelineBase>& mainContext)
{
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto stageManager = ngPipeline->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    if (!ngPipeline->IsFormRender()) {
        output.windowId = static_cast<int32_t>(ngPipeline->GetFocusWindowId());
    } else {
        output.windowId = static_cast<int32_t>(GetWindowId());
    }
    if (getParentRectHandler_) {
        getParentRectHandler_(output.windowTop, output.windowLeft);
    } else {
        output.windowLeft = GetWindowLeft(ngPipeline->GetWindowId());
        output.windowTop = GetWindowTop(ngPipeline->GetWindowId());
    }
    auto page = stageManager->GetLastPageWithTransition();
    if (page != nullptr) {
        output.pageId = page->GetPageId();
        output.pagePath = GetPagePath();
    }
    if (context->GetWindowId() != mainContext->GetWindowId()) {
        output.pageId = 0;
        output.pagePath = "";
    }
}

static void GetChildFromNode(const RefPtr<NG::UINode>& uiNode, std::vector<int32_t>& children, int32_t pageId,
    OHOS::Ace::Platform::ComponentInfo& parentComponent)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        if (uiNode->GetTag() == "stage") {
        } else if (uiNode->GetTag() == "page") {
            if (uiNode->GetPageId() != pageId) {
                return;
            }
        } else {
            auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
            if (!frameNode->IsInternal()) {
                children.emplace_back(uiNode->GetAccessibilityId());
                parentComponent.children.emplace_back();
                return;
            }
        }
    }

    for (const auto& frameChild : uiNode->GetChildren()) {
        GetChildFromNode(frameChild, children, pageId, parentComponent);
    }
}

static OHOS::Ace::Platform::ComponentInfo SetComponentInfo(const RefPtr<NG::FrameNode>& node)
{
    OHOS::Ace::Platform::ComponentInfo componentInfo;
    NG::RectF rect = node->GetTransformRectRelativeToWindow();
    componentInfo.compid = node->GetInspectorId().value_or("");
    componentInfo.text = node->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetText();
    componentInfo.top = rect.Top();
    componentInfo.width = rect.Width();
    componentInfo.left = rect.Left();
    componentInfo.height = rect.Height();
    auto gestureEventHub = node->GetEventHub<NG::EventHub>()->GetGestureEventHub();
    componentInfo.clickable = gestureEventHub ? gestureEventHub->IsAccessibilityClickable() : false;
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    componentInfo.checked = accessibilityProperty->IsChecked();
    componentInfo.selected = accessibilityProperty->IsSelected();
    componentInfo.checkable = accessibilityProperty->IsCheckable();
    componentInfo.scrollable = accessibilityProperty->IsScrollable();
    componentInfo.enabled = node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true;
    componentInfo.focused = node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false;
    componentInfo.longClickable = gestureEventHub ? gestureEventHub->IsAccessibilityLongClickable() : false;
    componentInfo.type = node->GetTag();
    return componentInfo;
}

RefPtr<NG::FrameNode> GetInspectorById(const RefPtr<NG::FrameNode>& root, int32_t id)
{
    CHECK_NULL_RETURN(root, nullptr);
    std::queue<RefPtr<NG::UINode>> nodes;
    nodes.push(root);
    RefPtr<NG::FrameNode> frameNode;
    while (!nodes.empty()) {
        auto current = nodes.front();
        nodes.pop();
        frameNode = AceType::DynamicCast<NG::FrameNode>(current);
        if (frameNode != nullptr) {
            if (id == frameNode->GetAccessibilityId()) {
                return frameNode;
            }
        }
        const auto& children = current->GetChildren();
        for (const auto& child : children) {
            nodes.push(child);
        }
    }
    return nullptr;
}

void GetComponents(OHOS::Ace::Platform::ComponentInfo& parentComponent, const RefPtr<NG::FrameNode>& parent,
    NodeId nodeID, int32_t pageId)
{
    auto node = GetInspectorById(parent, nodeID);
    if (!node) {
        return;
    }
    if (!node->IsActive()) {
        return;
    }
    parentComponent = SetComponentInfo(node);

    std::vector<int32_t> children;
    for (const auto& item : node->GetChildren()) {
        GetChildFromNode(item, children, pageId, parentComponent);
    }
    for (size_t index = 0; index < children.size(); index++) {
        GetComponents(parentComponent.children[index], node, children[index], pageId);
    }
}

static void GetChildFromNodeId(const RefPtr<NG::UINode>& uiNode, std::vector<int32_t>& children, int32_t pageId,
    std::vector<int32_t>& componentids)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        if (uiNode->GetTag() == "stage") {
        } else if (uiNode->GetTag() == "page") {
            if (uiNode->GetPageId() != pageId) {
                return;
            }
        } else {
            auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
            if (!frameNode->IsInternal()) {
                children.emplace_back(uiNode->GetAccessibilityId());
                return;
            }
        }
    }

    for (const auto& frameChild : uiNode->GetChildren()) {
        GetChildFromNodeId(frameChild, children, pageId, componentids);
    }
}

void GetComponentsId(std::vector<int>& componentids, const RefPtr<NG::FrameNode>& parent,
    NodeId nodeID, int32_t pageId)
{
    auto node = GetInspectorById(parent, nodeID);
    if (!node) {
        return;
    }

    if (!node->IsActive()) {
        return;
    }
    componentids.emplace_back(nodeID);

    std::vector<int32_t> children;
    for (const auto& item : node->GetChildren()) {
        GetChildFromNodeId(item, children, pageId, componentids);
    }
    for (size_t index = 0; index < children.size(); index++) {
        GetComponentsId(componentids, node, children[index], pageId);
    }
}

bool JsAccessibilityManager::GetAllComponentsId(NodeId nodeID, std::vector<int>& componentids, int windowid)
{
    auto pipeline = GetPipelineByWindowId(windowid);
    CHECK_NULL_RETURN(pipeline, false);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(rootNode, false);
    nodeID = rootNode->GetAccessibilityId();
    auto stageManager = ngPipeline->GetStageManager();
    CHECK_NULL_RETURN(stageManager, false);
    auto page = stageManager->GetLastPage();
    int pageId = 0;
    if (page != nullptr) {
        pageId = page->GetPageId();
    } else {
        pageId = -1;
    }
    GetComponentsId(componentids, rootNode, nodeID, pageId);
    return true;
}

bool JsAccessibilityManager::GetAllComponents(NodeId nodeID, OHOS::Ace::Platform::ComponentInfo& rootComponent)
{
    auto pipeline = context_.Upgrade();
    CHECK_NULL_RETURN(pipeline, false);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(rootNode, false);
    nodeID = rootNode->GetAccessibilityId();
    auto stageManager = ngPipeline->GetStageManager();
    CHECK_NULL_RETURN(stageManager, false);
    auto page = stageManager->GetLastPage();
    CHECK_NULL_RETURN(page, false);
    auto pageId = page->GetPageId();
    GetComponents(rootComponent, rootNode, nodeID, pageId);
    return true;
}

void JsAccessibilityManager::OnTouchExplorationStateChange(bool state) {}

void JsAccessibilityManager::JsInteractionOperation::SearchElementInfoByAccessibilityId(
    const int64_t elementId, std::string& retJson)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    auto jsAccessibilityManager = GetHandler().Upgrade();
    jsAccessibilityManager->GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (context == nullptr) {
        return;
    }

    int32_t mode = 0;
    std::list<AccessibilityElementInfo> elementInfos;
    jsAccessibilityManager->SearchElementInfoByAccessibilityId(splitElementId, mode, windowId_, elementInfos);
    jsAccessibilityManager->AccessibilityElementInfo2JsonStr(elementInfos, retJson);
}

void JsAccessibilityManager::JsInteractionOperation::FindFocusedElementInfo(
    const int64_t elementId, const int32_t focusType, std::string& retJson)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    auto jsAccessibilityManager = GetHandler().Upgrade();
    jsAccessibilityManager->GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (context == nullptr) {
        return;
    }

    AccessibilityElementInfo elementInfo;
    jsAccessibilityManager->FindFocusedElementInfo(splitElementId, focusType, windowId_, elementInfo);
    if (!elementInfo.IsValidElement() ||
        elementInfo.GetAccessibilityId() == AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID) {
        return;
    }

    std::list<Accessibility::AccessibilityElementInfo> elementInfos;
    elementInfos.push_back(elementInfo);
    jsAccessibilityManager->AccessibilityElementInfo2JsonStr(elementInfos, retJson);
}

void AddElementInfoJsonImporved(Json& retJson, AccessibilityElementInfo& info)
{
    retJson["SelectedBegin"] = info.GetSelectedBegin();
    retJson["SelectedEnd"] = info.GetSelectedEnd();
    retJson["CurrentIndex"] = info.GetCurrentIndex();
    retJson["BeginIndex"] = info.GetBeginIndex();
    retJson["EndIndex"] = info.GetEndIndex();
    retJson["PageId"] = info.GetPageId();
    retJson["ItemCounts"] = info.GetItemCounts();
    std::vector<std::string> contentList;
    info.GetContentList(contentList);
    Json contentListJson = contentList;
    retJson["ContentList"] = contentListJson.dump();
    retJson["LatestContent"] = info.GetLatestContent();
    retJson["AccessibilityText"] = info.GetAccessibilityText();
    retJson["Offset"] = info.GetOffset();
    retJson["LiveRegion"] = info.GetLiveRegion();
    retJson["ParentNodeId"] = info.GetParentNodeId();
    retJson["GirdInfoRows"] = info.GetGrid().GetRowCount();
    retJson["GirdInfoColumns"] = info.GetGrid().GetColumnCount();
    retJson["TextMoveStep"] = info.GetTextMovementStep();

    std::vector<AccessibleAction> actionList = info.GetActionList();
    std::vector<int> actionListAD = ConvertAceActionToAD(actionList);
    Json actionListJson = actionListAD;
    retJson["ActionList"] = actionListJson.dump();
    GridItemInfo gridItem = info.GetGridItem();
    retJson["GridItemIsHeading"] = gridItem.IsHeading();
    retJson["GridItemColumnIndex"] = gridItem.GetColumnIndex();
    retJson["GridItemRowIndex"] = gridItem.GetRowIndex();
    retJson["GridItemColumnSpan"] = gridItem.GetColumnSpan();
    retJson["GridItemRowSpan"] = gridItem.GetRowSpan();
    retJson["GridItemIsSelected"] = gridItem.IsSelected();

    std::vector<int64_t> childIDs = info.GetChildIds();
    Json childIDsJson = childIDs;
    retJson["childIDs"] = childIDsJson.dump();

    retJson["ImportantForAccessibility"] = info.GetImportantForAccessibility();
    retJson["ZIndex"] = info.GetZIndex();
}

void JsAccessibilityManager::AccessibilityElementInfo2JsonStr(
    const std::list<Accessibility::AccessibilityElementInfo>& elementInfos, std::string& retJsonStr)
{
    if (elementInfos.empty()) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "Element infos is empty. Find element infos failed.");
        return;
    }

    AccessibilityElementInfo info = elementInfos.front();
    Json retJson = {};
    retJson["TextLengthLimit"] = info.GetTextLengthLimit();
    retJson["WindowId"] = info.GetWindowId();
    retJson["ParentNodeId"] = info.GetParentNodeId();
    OHOS::Accessibility::Rect rect = info.GetRectInScreen();
    retJson["RectInScreen_leftTopX"] = rect.GetLeftTopXScreenPostion();
    retJson["RectInScreen_leftTopY"] = rect.GetLeftTopYScreenPostion();
    retJson["RectInScreen_rightBottomX"] = rect.GetRightBottomXScreenPostion();
    retJson["RectInScreen_rightBottomY"] = rect.GetRightBottomYScreenPostion();
    retJson["IsCheckable"] = info.IsCheckable();
    retJson["IsChecked"] = info.IsChecked();
    retJson["IsFocusable"] = info.IsFocusable();
    retJson["IsFocused"] = info.IsFocused();
    retJson["IsVisible"] = info.IsVisible();
    retJson["HasAccessibilityFocus"] = info.HasAccessibilityFocus();
    retJson["IsSelected"] = info.IsSelected();
    retJson["IsClickable"] = info.IsClickable();
    retJson["IsLongClickable"] = info.IsLongClickable();
    retJson["IsEnabled"] = info.IsEnabled();
    retJson["IsPassword"] = info.IsPassword();
    retJson["IsScrollable"] = info.IsScrollable();
    retJson["IsEditable"] = info.IsEditable();
    retJson["IsEssential"] = info.IsEssential();
    retJson["BundleName"] = info.GetBundleName();
    retJson["ComponentType"] = info.GetComponentType();
    retJson["Content"] = info.GetContent();
    retJson["Hint"] = info.GetHint();
    retJson["DescriptionInfo"] = info.GetDescriptionInfo();
    retJson["ComponentResourceId"] = info.GetComponentResourceId();
    retJson["AccessibilityId"] = info.GetAccessibilityId();
    AddElementInfoJsonImporved(retJson, info);
    retJsonStr = retJson.dump();
}

void JsAccessibilityManager::FindFocusedElementInfo(const int64_t elementId, const int32_t focusType,
    const int32_t windowId, Accessibility::AccessibilityElementInfo& nodeInfo)
{
    if (focusType != FOCUS_TYPE_INPUT && focusType != FOCUS_TYPE_ACCESSIBILITY) {
        nodeInfo.SetValidElement(false);
        return;
    }

    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        return;
    }

    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        FindFocusedElementInfoNG(elementId, focusType, nodeInfo, context, NG::UI_EXTENSION_OFFSET_MAX);
        return;
    }

    int64_t nodeId = static_cast<int64_t>(elementId);
    if (elementId == -1) {
        nodeId = 0;
    }

    auto node = GetAccessibilityNodeFromPage(nodeId);
    if (!node) {
        nodeInfo.SetValidElement(false);
        return;
    }

    RefPtr<AccessibilityNode> resultNode = nullptr;
    bool status = false;
    if (focusType == FOCUS_TYPE_ACCESSIBILITY) {
        status = FindAccessibilityFocus(node, resultNode);
    }
    if (focusType == FOCUS_TYPE_INPUT) {
        status = FindInputFocus(node, resultNode);
    }

    if (status && (resultNode != nullptr)) {
        UpdateAccessibilityNodeInfo(resultNode, nodeInfo, Claim(this), windowId);
    }
}

void JsAccessibilityManager::SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t mode,
    const int32_t windowId, std::list<Accessibility::AccessibilityElementInfo>& infos)
{
    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline) {
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        if (ngPipeline) {
            SearchElementInfoByAccessibilityIdNG(elementId, mode, infos, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
            UpdateElementInfosTreeId(infos);
            return;
        }
    }

    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = 0;
    }
    auto weak = WeakClaim(this);
    auto jsAccessibilityManager = weak.Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto node = jsAccessibilityManager->GetAccessibilityNodeFromPage(nodeId);
    if (!node) {
        UpdateElementInfosTreeId(infos);
        return;
    }

    AccessibilityElementInfo nodeInfo;
    UpdateAccessibilityNodeInfo(node, nodeInfo, jsAccessibilityManager, jsAccessibilityManager->windowId_);
    infos.push_back(nodeInfo);
    UpdateCacheInfo(infos, mode, node, jsAccessibilityManager, jsAccessibilityManager->windowId_);

    UpdateElementInfosTreeId(infos);
}

bool JsAccessibilityManager::JsInteractionOperation::GetAllComponentsId(NodeId nodeID, std::vector<int>& componentids)
{
    auto jsAccessibilityManager = GetHandler().Upgrade();

    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (context == nullptr) {
        return false;
    }
    jsAccessibilityManager->GetAllComponentsId(nodeID, componentids, windowId_);

    return true;
}

void JsAccessibilityManager::JsInteractionOperation::OnTouchExplorationStateChange(bool state)
{
    auto jsAccessibilityManager = GetHandler().Upgrade();

    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (context == nullptr) {
        return;
    }
    jsAccessibilityManager->OnTouchExplorationStateChange(state);
}

int32_t JsAccessibilityManager::JsInteractionOperation::GetRootElementId()
{
    auto jsAccessibilityManager = GetHandler().Upgrade();

    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (context == nullptr) {
        return -1;
    }
    return jsAccessibilityManager->GetRootElementId(windowId_);
}

int32_t JsAccessibilityManager::GetRootElementId(int32_t windowId)
{
    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline) {
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        auto rootNode = ngPipeline->GetRootElement();
        CHECK_NULL_RETURN(rootNode, false);
        int32_t nodeId = rootNode->GetAccessibilityId();
        return nodeId;
    }

    return -1;
}
} // namespace OHOS::Ace::Framework