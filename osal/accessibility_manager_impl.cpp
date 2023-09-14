/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "adapter/android/osal/accessibility_manager_impl.h"

#include <cstdint>

#include "foundation/appframework/arkui/uicontent/component_info.h"

#include "base/log/dump_log.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/accessibility/accessibility_utils.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::Framework {

constexpr int32_t INVALID_PARENT_ID = -2100000;

RefPtr<AccessibilityNodeManager> AccessibilityNodeManager::Create()
{
    return AceType::MakeRefPtr<AccessibilityManagerImpl>();
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

void GetFrameNodeChildren(const RefPtr<NG::UINode>& uiNode, std::vector<int32_t>& children, int32_t pageId)
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
        GetFrameNodeChildren(frameChild, children, pageId);
    }
}

void DumpTreeNG(const RefPtr<NG::FrameNode>& parent, int32_t depth, NodeId nodeID, int32_t pageId)
{
    auto node = GetInspectorById(parent, nodeID);
    if (!node) {
        DumpLog::GetInstance().Print("Error: failed to get accessibility node with ID " + std::to_string(nodeID));
        return;
    }

    if (!node->IsActive()) {
        return;
    }

    NG::RectF rect = node->GetTransformRectRelativeToWindow();
    DumpLog::GetInstance().AddDesc("ID: " + std::to_string(node->GetAccessibilityId()));
    DumpLog::GetInstance().AddDesc("compid: " + node->GetInspectorId().value_or(""));
    DumpLog::GetInstance().AddDesc("text: " + node->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetText());
    DumpLog::GetInstance().AddDesc("top: " + std::to_string(rect.Top()));
    DumpLog::GetInstance().AddDesc("left: " + std::to_string(rect.Left()));
    DumpLog::GetInstance().AddDesc("width: " + std::to_string(rect.Width()));
    DumpLog::GetInstance().AddDesc("height: " + std::to_string(rect.Height()));
    DumpLog::GetInstance().AddDesc("visible: " + std::to_string(node->IsVisible()));
    auto gestureEventHub = node->GetEventHub<NG::EventHub>()->GetGestureEventHub();
    DumpLog::GetInstance().AddDesc(
        "clickable: " + std::to_string(gestureEventHub ? gestureEventHub->IsAccessibilityClickable() : false));
    DumpLog::GetInstance().AddDesc(
        "checkable: " + std::to_string(node->GetAccessibilityProperty<NG::AccessibilityProperty>()->IsCheckable()));

    std::vector<int32_t> children;
    for (const auto& item : node->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId);
    }
    DumpLog::GetInstance().Print(depth, node->GetTag(), children.size());

    for (auto nodeId : children) {
        DumpTreeNG(node, depth + 1, nodeId, pageId);
    }
}

int32_t GetParentId(const RefPtr<NG::UINode>& uiNode)
{
    auto parent = uiNode->GetParent();
    while (parent) {
        if (AceType::InstanceOf<NG::FrameNode>(parent)) {
            return parent->GetAccessibilityId();
        }
        parent = parent->GetParent();
    }
    return INVALID_PARENT_ID;
}

inline std::string ChildernToString(const RefPtr<NG::FrameNode>& frameNode, int32_t pageId)
{
    std::string ids;
    std::vector<int32_t> children;
    for (const auto& item : frameNode->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId);
    }
    for (auto child : children) {
        if (!ids.empty()) {
            ids.append(",");
        }
        ids.append(std::to_string(child));
    }
    return ids;
}

inline std::string BoolToString(bool tag)
{
    return tag ? "true" : "false";
}

inline bool IsPopupSupported(const RefPtr<NG::FrameNode>& frameNode, const RefPtr<NG::PipelineContext>& pipeline)
{
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    if (overlayManager) {
        return overlayManager->HasPopupInfo(frameNode->GetId());
    }
    return false;
}

inline std::string ConvertInputTypeToString(int32_t inputType)
{
    static std::vector<std::string> sInputTypes { "default", "text", "email", "date", "time", "number", "password" };
    if (inputType < 0 || inputType >= static_cast<int32_t>(sInputTypes.size())) {
        return "none";
    }
    return sInputTypes[inputType];
}

// execute action
bool RequestFocus(RefPtr<NG::FrameNode>& frameNode)
{
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    return focusHub->RequestFocusImmediately();
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

static std::string ConvertActionTypeToString(AceAction action)
{
    switch (action) {
        case AceAction::ACTION_NONE:
            return "none";
        case AceAction::GLOBAL_ACTION_BACK:
            return "back";
        case AceAction::CUSTOM_ACTION:
            return "custom action";
        case AceAction::ACTION_CLICK:
            return "click";
        case AceAction::ACTION_LONG_CLICK:
            return "long click";
        case AceAction::ACTION_SCROLL_FORWARD:
            return "scroll forward";
        case AceAction::ACTION_SCROLL_BACKWARD:
            return "scroll backward";
        case AceAction::ACTION_FOCUS:
            return "focus";
        default:
            return "not support";
    }
}

static void DumpCommonPropertyNG(
    const RefPtr<NG::FrameNode>& frameNode, const RefPtr<PipelineBase>& pipeline, int32_t pageId)
{
    CHECK_NULL_VOID(frameNode);
    auto gestureEventHub = frameNode->GetEventHub<NG::EventHub>()->GetGestureEventHub();
    DumpLog::GetInstance().AddDesc("ID: ", frameNode->GetAccessibilityId());
    DumpLog::GetInstance().AddDesc("parent ID: ", GetParentId(frameNode));
    DumpLog::GetInstance().AddDesc("child IDs: ", ChildernToString(frameNode, pageId));
    DumpLog::GetInstance().AddDesc("component type: ", frameNode->GetTag());
    DumpLog::GetInstance().AddDesc(
        "enabled: ", BoolToString(frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsEnabled() : true));
    DumpLog::GetInstance().AddDesc(
        "focusable: ", BoolToString(frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsFocusable() : false));
    DumpLog::GetInstance().AddDesc(
        "focused: ", BoolToString(frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsCurrentFocus() : false));
    DumpLog::GetInstance().AddDesc("visible: ", BoolToString(frameNode->IsVisible()));
    if (frameNode->IsVisible()) {
        NG::RectF rect;
        if (frameNode->IsActive()) {
            rect = frameNode->GetTransformRectRelativeToWindow();
        }
        DumpLog::GetInstance().AddDesc("rect: ", rect.ToString());
    }
    DumpLog::GetInstance().AddDesc("inspector key: ", frameNode->GetInspectorId().value_or(""));
    DumpLog::GetInstance().AddDesc("bundle name: ", AceApplicationInfo::GetInstance().GetPackageName());
    DumpLog::GetInstance().AddDesc("page id: " + std::to_string(pageId));
    DumpLog::GetInstance().AddDesc(
        "clickable: ", BoolToString(gestureEventHub ? gestureEventHub->IsAccessibilityClickable() : false));
    DumpLog::GetInstance().AddDesc(
        "long clickable: ", BoolToString(gestureEventHub ? gestureEventHub->IsAccessibilityLongClickable() : false));
    DumpLog::GetInstance().AddDesc("popup supported: ",
        BoolToString(IsPopupSupported(frameNode, AceType::DynamicCast<NG::PipelineContext>(pipeline))));
}

static void DumpAccessibilityPropertyNG(const RefPtr<NG::FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    DumpLog::GetInstance().AddDesc("text: ", accessibilityProperty->GetText());
    DumpLog::GetInstance().AddDesc("checked: ", BoolToString(accessibilityProperty->IsChecked()));
    DumpLog::GetInstance().AddDesc("selected: ", BoolToString(accessibilityProperty->IsSelected()));
    DumpLog::GetInstance().AddDesc("checkable: ", BoolToString(accessibilityProperty->IsCheckable()));
    DumpLog::GetInstance().AddDesc("scrollable: ", BoolToString(accessibilityProperty->IsScrollable()));
    DumpLog::GetInstance().AddDesc("accessibility hint: ", BoolToString(accessibilityProperty->IsHint()));
    DumpLog::GetInstance().AddDesc("hint text: ", accessibilityProperty->GetHintText());
    DumpLog::GetInstance().AddDesc("error text: ", accessibilityProperty->GetErrorText());
    DumpLog::GetInstance().AddDesc("max text length: ", accessibilityProperty->GetTextLengthLimit());
    DumpLog::GetInstance().AddDesc("text selection start: ", accessibilityProperty->GetTextSelectionStart());
    DumpLog::GetInstance().AddDesc("text selection end: ", accessibilityProperty->GetTextSelectionEnd());
    DumpLog::GetInstance().AddDesc("is multi line: ", BoolToString(accessibilityProperty->IsMultiLine()));
    DumpLog::GetInstance().AddDesc("is password: ", BoolToString(accessibilityProperty->IsPassword()));
    DumpLog::GetInstance().AddDesc(
        "text input type: ", ConvertInputTypeToString(static_cast<int32_t>(accessibilityProperty->GetTextInputType())));
    DumpLog::GetInstance().AddDesc("min value: ", accessibilityProperty->GetAccessibilityValue().min);
    DumpLog::GetInstance().AddDesc("max value: ", accessibilityProperty->GetAccessibilityValue().max);
    DumpLog::GetInstance().AddDesc("current value: ", accessibilityProperty->GetAccessibilityValue().current);
    DumpLog::GetInstance().AddDesc("gird info rows: ", accessibilityProperty->GetCollectionInfo().rows);
    DumpLog::GetInstance().AddDesc("gird info columns: ", accessibilityProperty->GetCollectionInfo().columns);
    DumpLog::GetInstance().AddDesc("gird info select mode: ", accessibilityProperty->GetCollectionInfo().selectMode);
    DumpLog::GetInstance().AddDesc("gird item info, row: ", accessibilityProperty->GetCollectionItemInfo().row);
    DumpLog::GetInstance().AddDesc("gird item info, column: ", accessibilityProperty->GetCollectionItemInfo().column);
    DumpLog::GetInstance().AddDesc("gird item info, rowSpan: ", accessibilityProperty->GetCollectionItemInfo().rowSpan);
    DumpLog::GetInstance().AddDesc(
        "gird item info, columnSpan: ", accessibilityProperty->GetCollectionItemInfo().columnSpan);
    DumpLog::GetInstance().AddDesc(
        "gird item info, is heading: ", accessibilityProperty->GetCollectionItemInfo().heading);
    DumpLog::GetInstance().AddDesc("gird item info, selected: ", BoolToString(accessibilityProperty->IsSelected()));
    DumpLog::GetInstance().AddDesc("current index: ", accessibilityProperty->GetCurrentIndex());
    DumpLog::GetInstance().AddDesc("begin index: ", accessibilityProperty->GetBeginIndex());
    DumpLog::GetInstance().AddDesc("end index: ", accessibilityProperty->GetEndIndex());
    DumpLog::GetInstance().AddDesc("collection item counts: ", accessibilityProperty->GetCollectionItemCounts());
    DumpLog::GetInstance().AddDesc("editable: ", BoolToString(accessibilityProperty->IsEditable()));
    DumpLog::GetInstance().AddDesc("deletable: ", accessibilityProperty->IsDeletable());
    DumpLog::GetInstance().AddDesc("content invalid: ", BoolToString(accessibilityProperty->GetContentInvalid()));

    std::string actionForDump;
    accessibilityProperty->ResetSupportAction();
    auto gestureEventHub = frameNode->GetEventHub<NG::EventHub>()->GetGestureEventHub();
    if (gestureEventHub) {
        if (gestureEventHub->IsAccessibilityClickable()) {
            accessibilityProperty->AddSupportAction(AceAction::ACTION_CLICK);
        }
        if (gestureEventHub->IsAccessibilityLongClickable()) {
            accessibilityProperty->AddSupportAction(AceAction::ACTION_LONG_CLICK);
        }
    }
    if (frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsFocusable() : false) {
        if (frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsCurrentFocus() : false) {
            accessibilityProperty->AddSupportAction(AceAction::ACTION_CLEAR_FOCUS);
        } else {
            accessibilityProperty->AddSupportAction(AceAction::ACTION_FOCUS);
        }
    }
    auto supportAceActions = accessibilityProperty->GetSupportAction();
    for (auto item : supportAceActions) {
        if (!actionForDump.empty()) {
            actionForDump.append(",");
        }
        actionForDump.append(ConvertActionTypeToString(item));
        actionForDump.append(": ");
        actionForDump.append(std::to_string(static_cast<int32_t>(item)));
    }
    DumpLog::GetInstance().AddDesc("support action: ", actionForDump);
}

void AccessibilityManagerImpl::DumpTree(int32_t depth, NodeId nodeID)
{
    DumpLog::GetInstance().Print("Dump Accessiability Tree:");
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    nodeID = rootNode->GetAccessibilityId();
    auto stageManager = ngPipeline->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto page = stageManager->GetLastPage();
    CHECK_NULL_VOID(page);
    auto pageId = page->GetPageId();
    DumpTreeNG(rootNode, depth, nodeID, pageId);
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
    for (int index = 0; index < children.size(); index++) {
        GetComponents(parentComponent.children[index], node, children[index], pageId);
    }
}

bool AccessibilityManagerImpl::GetAllComponents(NodeId nodeID, OHOS::Ace::Platform::ComponentInfo& rootComponent)
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

void AccessibilityManagerImpl::DumpProperty(const std::vector<std::string>& params)
{
    DumpLog::GetInstance().Print("Dump Accessiability Property:");
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);

    RefPtr<NG::FrameNode> frameNode;
    auto nodeID = StringUtils::StringToInt(params[1]);
    auto ngPipeline = FindPipelineByElementId(nodeID, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);

    int32_t pageId = 0;
    if (ngPipeline->GetWindowId() == pipeline->GetWindowId()) {
        auto stageManager = ngPipeline->GetStageManager();
        CHECK_NULL_VOID(stageManager);
        auto page = stageManager->GetLastPage();
        CHECK_NULL_VOID(page);
        pageId = page->GetPageId();
    }

    DumpCommonPropertyNG(frameNode, pipeline, pageId);

    DumpAccessibilityPropertyNG(frameNode);
    DumpLog::GetInstance().Print(0, frameNode->GetTag(), frameNode->GetChildren().size());
}

void AccessibilityManagerImpl::DumpHandleEvent(const std::vector<std::string>& params)
{
    DumpLog::GetInstance().Print("Dump Accessiability Execute Action");
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    int32_t nodeId = StringUtils::StringToInt(params[EVENT_DUMP_ID_INDEX]);
    auto action = static_cast<AceAction>(StringUtils::StringToInt(params[EVENT_DUMP_ACTION_INDEX]));

    if (AceType::InstanceOf<NG::PipelineContext>(pipeline)) {
        RefPtr<NG::FrameNode> node;
        pipeline = FindPipelineByElementId(nodeId, node);
        CHECK_NULL_VOID(pipeline);
        CHECK_NULL_VOID(node);
        pipeline->GetTaskExecutor()->PostTask(
            [weak = WeakClaim(this), action, nodeId, pipeline]() {
                auto accessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(accessibilityManager);
                accessibilityManager->ExecuteActionNG(nodeId, action, pipeline);
            },
            TaskExecutor::TaskType::UI);
        return;
    }
}

bool AccessibilityManagerImpl::ExecuteActionNG(int32_t elementId, AceAction action, const RefPtr<PipelineBase>& context)
{
    bool result = false;
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, result);
    ContainerScope instance(ngPipeline->GetInstanceId());
    auto frameNode = GetInspectorById(ngPipeline->GetRootElement(), elementId);
    CHECK_NULL_RETURN(frameNode, result);

    switch (action) {
        case AceAction::ACTION_FOCUS: {
            result = RequestFocus(frameNode);
            break;
        }
        case AceAction::ACTION_CLICK: {
            result = ActClick(frameNode);
            break;
        }
        case AceAction::ACTION_LONG_CLICK: {
            result = ActLongClick(frameNode);
            break;
        }
        case AceAction::ACTION_SCROLL_BACKWARD:
        case AceAction::ACTION_SCROLL_FORWARD:
            return true;
        default:
            break;
    }

    return result;
}

RefPtr<NG::FrameNode> AccessibilityManagerImpl::FindNodeFromPipeline(
    const WeakPtr<PipelineBase>& context, const int32_t elementId)
{
    auto pipeline = context.Upgrade();
    CHECK_NULL_RETURN(pipeline, nullptr);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(rootNode, nullptr);

    NodeId nodeId = elementId;
    // accessibility use -1 for first search to get root node
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }

    RefPtr<NG::FrameNode> node = GetInspectorById(rootNode, nodeId);
    if (node) {
        return node;
    }
    return nullptr;
}

RefPtr<NG::PipelineContext> AccessibilityManagerImpl::FindPipelineByElementId(
    const int32_t elementId, RefPtr<NG::FrameNode>& node)
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

} // namespace OHOS::Ace::Framework
