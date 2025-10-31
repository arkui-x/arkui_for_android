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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_JS_ACCESSIBILITY_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_JS_ACCESSIBILITY_MANAGER_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "accessibility_definitions.h"
#include "accessibility_element_operator.h"
#include "accessibility_event_info.h"
#include "accessibility_state_event.h"
#include "core/accessibility/accessibility_manager.h"
#include "core/accessibility/accessibility_utils.h"
#include "foundation/appframework/arkui/uicontent/component_info.h"
#include "frameworks/bridge/common/accessibility/accessibility_node_manager.h"
#include "nlohmann/json.hpp"

namespace OHOS::Ace::Framework {
struct SearchParameter {
    int64_t nodeId;
    std::string text;
    int32_t mode;
    int64_t uiExtensionOffset;
};

struct CommonProperty {
    int32_t windowId = 0;
    int32_t windowLeft = 0;
    int32_t windowTop = 0;
    int32_t pageId = 0;
    std::string pagePath;
};

struct CacheParam {
    CommonProperty commonProperty;
    RefPtr<NG::PipelineContext> ngPipeline;
    SearchParameter searchParam;
};

constexpr int32_t ELEMENT_MOVE_BIT = 40;
constexpr int32_t CONT_SPLIT_ID = -1;
constexpr int64_t MAX_ELEMENT_ID = 0xFFFFFFFFFF;

class JsAccessibilityManager : public AccessibilityNodeManager {
    DECLARE_ACE_TYPE(JsAccessibilityManager, AccessibilityNodeManager);
public:
    struct ActionParam {
        Accessibility::ActionType action;
        std::map<std::string, std::string> actionArguments;
    };

    JsAccessibilityManager() = default;
    ~JsAccessibilityManager() override;
    bool GetAllComponents(NodeId nodeID, OHOS::Ace::Platform::ComponentInfo& rootComponent);
    bool GetAllComponentsId(NodeId nodeID, std::vector<int>& componentids, int windowid);
    void SendAccessibilityEvent(const AccessibilityEvent& accessibilityEvent);
    void InitializeCallback() override;
    void SendAccessibilityAsyncEvent(const AccessibilityEvent& accessibilityEvent) override;
    void UpdateVirtualNodeFocus() override;
    void HandleComponentPostBinding() override;
    void RegisterSubWindowInteractionOperation(int windowId) override;
    void SetPipelineContext(const RefPtr<PipelineBase>& context) override;

    float GetScaleX() const
    {
        return scaleX_;
    }

    float GetScaleY() const
    {
        return scaleY_;
    }

    uint32_t GetWindowId() const
    {
        return windowId_;
    }

    void SetWindowId(uint32_t windowId)
    {
        windowId_ = windowId;
    }

    void SaveLast(const int64_t elementId, const RefPtr<NG::FrameNode>& node)
    {
        lastElementId_ = elementId;
        lastFrameNode_ = node;
    }

    bool SubscribeStateObserver();
    bool UnsubscribeStateObserver();
    int RegisterInteractionOperation(int windowId);
    void DeregisterInteractionOperation();
    bool SendAccessibilitySyncEvent(
        const AccessibilityEvent& accessibilityEvent, Accessibility::AccessibilityEventInfo eventInfo);
    void SendExtensionAccessibilityEvent(
        const Accessibility::AccessibilityEventInfo& eventInfo, int64_t uiExtensionOffset) {}
    void SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode, const int32_t windowId);
    void ExecuteAction(const int64_t accessibilityId, const ActionParam& param, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId);
    bool ClearCurrentFocus();
    void UpdateNodeChildIds(const RefPtr<AccessibilityNode>& node);
    void SendActionEvent(const Accessibility::ActionType& action, int64_t nodeId);
    void SearchElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        const int64_t uiExtensionOffset = 0);
    void FindFocusedElementInfoNG(int64_t elementId, int32_t focusType, Accessibility::AccessibilityElementInfo& info,
        const RefPtr<PipelineBase>& context, const int64_t uiExtensionOffset = 0);
    std::string GetPagePath();
    void RegisterAccessibilityChildTreeCallback(
        int64_t elementId, const std::shared_ptr<AccessibilityChildTreeCallback>& callback) override;
    void DeregisterAccessibilityChildTreeCallback(int64_t elementId) override;
    void SetAccessibilityGetParentRectHandler(std::function<void(int32_t&, int32_t&)>&& callback) override;
    void SendEventToAccessibilityWithNode(const AccessibilityEvent& accessibilityEvent, const RefPtr<AceType>& node,
        const RefPtr<PipelineBase>& context) override;
    void SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t mode, const int32_t windowId,
        std::list<Accessibility::AccessibilityElementInfo>& infos);
    void FindFocusedElementInfo(const int64_t elementId, const int32_t focusType, const int32_t windowId,
        Accessibility::AccessibilityElementInfo& nodeInfo);
    int32_t GetRootElementId(int32_t windowId);
    void OnTouchExplorationStateChange(bool state);

    static void SetSplicElementIdTreeId(const int32_t treeId, int64_t& elementId)
    {
        if (treeId == CONT_SPLIT_ID || elementId == CONT_SPLIT_ID) {
            elementId = CONT_SPLIT_ID;
            return;
        }
        if ((elementId & MAX_ELEMENT_ID) != elementId) {
            return;
        }
        uint64_t itemp = 0;
        itemp = treeId;
        itemp = (itemp << ELEMENT_MOVE_BIT);
        itemp |= static_cast<uint64_t>(elementId);
        elementId = static_cast<int64_t>(itemp);
    }

    static void GetTreeIdAndElementIdBySplitElementId(
        const int64_t elementId, int64_t& splitElementId, int32_t& splitTreeId)
    {
        if (elementId <= CONT_SPLIT_ID) {
            splitTreeId = CONT_SPLIT_ID;
            splitElementId = CONT_SPLIT_ID;
            return;
        }
        splitTreeId = (static_cast<uint64_t>(elementId) >> ELEMENT_MOVE_BIT);
        splitElementId = MAX_ELEMENT_ID & elementId;
    }

    class JsInteractionOperation : public Accessibility::AccessibilityElementOperator {
    public:
        explicit JsInteractionOperation(int32_t windowId) : windowId_(windowId) {}
        virtual ~JsInteractionOperation() = default;
        void SearchElementInfoByAccessibilityId(const int64_t elementId, std::string& retJson);
        void FindFocusedElementInfo(const int64_t elementId, const int32_t focusType, std::string& retJson);
        void SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode) override {}
        void SearchElementInfosByText(const int64_t elementId, const std::string& text, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override {}
        void FindFocusedElementInfo(const int64_t elementId, const int32_t focusType, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override {}
        void FocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override {}
        void ExecuteAction(const int64_t elementId, const int32_t action,
            const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void ClearFocus() override {}
        void OutsideTouch() override {}
        void GetCursorPosition(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override {}
        bool GetAllComponentsId(NodeId nodeID, std::vector<int>& componentids);
        int32_t GetRootElementId();
        void OnTouchExplorationStateChange(bool state);

        void SetHandler(const WeakPtr<JsAccessibilityManager>& js)
        {
            js_ = js;
        }

        const WeakPtr<JsAccessibilityManager>& GetHandler() const
        {
            return js_;
        }

    private:
        WeakPtr<JsAccessibilityManager> js_;
        uint32_t windowId_ = 0;
    };

    class JsAccessibilityStateObserver : public Accessibility::AccessibilityStateObserver {
    public:
        void OnStateChanged(const bool state) override;
        void SetAccessibilityManager(const WeakPtr<JsAccessibilityManager>& accessibilityManager)
        {
            accessibilityManager_ = accessibilityManager;
        }

        void SetPipeline(const WeakPtr<PipelineBase>& pipeline)
        {
            pipeline_ = pipeline;
        }

    private:
        // Do not upgrade accessibilityManager_ on async thread, destructor might cause freeze
        WeakPtr<JsAccessibilityManager> accessibilityManager_;
        WeakPtr<PipelineBase> pipeline_;
    };

private:
    static constexpr int32_t INVALID_PARENT_ID = -2100000;

    bool AccessibilityActionEvent(const Accessibility::ActionType& action,
        const std::map<std::string, std::string>& actionArguments, const RefPtr<AccessibilityNode>& node,
        const RefPtr<PipelineContext>& context);
    bool RequestAccessibilityFocus(const RefPtr<AccessibilityNode>& node);
    bool ClearAccessibilityFocus(const RefPtr<AccessibilityNode>& node);
    bool ExecuteActionNG(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        Accessibility::ActionType action, const RefPtr<PipelineBase>& context, int64_t uiExtensionOffset);
    bool ConvertActionTypeToBoolen(Accessibility::ActionType action, RefPtr<NG::FrameNode>& frameNode,
        int64_t elementId, RefPtr<NG::PipelineContext>& context);
    RefPtr<NG::PipelineContext> FindPipelineByElementId(const int64_t elementId, RefPtr<NG::FrameNode>& node);
    RefPtr<NG::FrameNode> FindNodeFromPipeline(const WeakPtr<PipelineBase>& context, const int64_t elementId);
    RefPtr<PipelineBase> GetPipelineByWindowId(const int32_t windowId);
    void ProcessParameters(Accessibility::ActionType op, const std::vector<std::string>& params,
        std::map<std::string, std::string>& paramsMap);
    RefPtr<NG::PipelineContext> GetPipelineByWindowId(uint32_t windowId);
    void GenerateCommonProperty(
        const RefPtr<PipelineBase>& context, CommonProperty& output, const RefPtr<PipelineBase>& mainContext);
    void UpdateAccessibilityElementInfo(
        const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityElementInfo& nodeInfo);
    void UpdateAccessibilityElementInfoBase(const RefPtr<NG::FrameNode>& node,
        Accessibility::AccessibilityElementInfo& nodeInfo,
        const RefPtr<NG::AccessibilityProperty>& accessibilityProperty);
    void UpdateAccessibilityElementInfoImproved(const RefPtr<NG::FrameNode>& node,
        Accessibility::AccessibilityElementInfo& nodeInfo,
        const RefPtr<NG::AccessibilityProperty>& accessibilityProperty);
    void UpdateAccessibilityElementInfoImportant(const RefPtr<NG::FrameNode>& node,
        Accessibility::AccessibilityElementInfo& nodeInfo,
        const RefPtr<NG::AccessibilityProperty>& accessibilityProperty);
    void UpdateVirtualNodeInfo(std::list<Accessibility::AccessibilityElementInfo>& infos,
        Accessibility::AccessibilityElementInfo& nodeInfo, const RefPtr<NG::UINode>& uiVirtualNode,
        const CommonProperty& commonProperty, const RefPtr<NG::PipelineContext>& ngPipeline);
    void UpdateVirtualNodeChildAccessibilityElementInfo(const RefPtr<NG::FrameNode>& node,
        const CommonProperty& commonProperty, Accessibility::AccessibilityElementInfo& nodeParentInfo,
        Accessibility::AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline);
    void UpdateVirtualNodeAccessibilityElementInfo(const RefPtr<NG::FrameNode>& parent,
        const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
        Accessibility::AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline);
    void UpdateAccessibilityElementInfo(const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
        Accessibility::AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline);
    void UpdateCacheInfoNG(std::list<Accessibility::AccessibilityElementInfo>& infos,
        const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
        const RefPtr<NG::PipelineContext>& ngPipeline, const SearchParameter& searchParam);
    void UpdateCacheInfoNG4FrameNode(std::list<Accessibility::AccessibilityElementInfo>& infos,
        const std::variant<RefPtr<NG::FrameNode>, int64_t>& parent,
        const CacheParam &cacheParam,
        std::list<std::variant<RefPtr<NG::FrameNode>, int64_t>>& children);
    void NotifyChildTreeOnRegister(int32_t treeId);
    bool CheckIsChildElement(
        int64_t& elementId, const std::vector<std::string>& params, std::vector<std::string>& info);
    void UpdateElementInfoTreeId(Accessibility::AccessibilityElementInfo& info);
    void UpdateElementInfosTreeId(std::list<Accessibility::AccessibilityElementInfo>& infos);
    void FillEventInfoWithNode(const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityEventInfo& eventInfo,
        const RefPtr<NG::PipelineContext>& context, int64_t elementId);
    void AccessibilityElementInfo2JsonStr(
        const std::list<Accessibility::AccessibilityElementInfo>& infos, std::string& retJsonStr);

    bool ActionClick(const std::map<std::string, std::string>& actionArguments, const RefPtr<AccessibilityNode>& node,
        const RefPtr<PipelineContext>& context);
    bool ActionLongClick(const std::map<std::string, std::string>& actionArguments,
        const RefPtr<AccessibilityNode>& node, const RefPtr<PipelineContext>& context);
    bool ActionSetText(const std::map<std::string, std::string>& actionArguments, const RefPtr<AccessibilityNode>& node,
        const RefPtr<PipelineContext>& context);
    bool ActionFocus(const std::map<std::string, std::string>& actionArguments, const RefPtr<AccessibilityNode>& node,
        const RefPtr<PipelineContext>& context);

    void UpdateSpecialChildren(const RefPtr<AccessibilityNode>& node,
        const std::list<RefPtr<AccessibilityNode>>& children, std::vector<int32_t>& childrenVec, int32_t cardId,
        int32_t rootNodeId);
    void UpdateRegularChildren(const RefPtr<AccessibilityNode>& node,
        const std::list<RefPtr<AccessibilityNode>>& children, std::vector<int32_t>& childrenVec, int32_t cardId,
        int32_t rootNodeId);

    std::string callbackKey_;
    uint32_t windowId_ = 0;
    std::shared_ptr<JsAccessibilityStateObserver> stateObserver_ = nullptr;
    float scaleX_ = 1.0f;
    float scaleY_ = 1.0f;
    int64_t currentFocusNodeId_ = -1;
    int64_t lastElementId_ = -1;
    WeakPtr<NG::FrameNode> lastFrameNode_;
    mutable std::mutex childTreeCallbackMapMutex_;
    std::unordered_map<int64_t, std::shared_ptr<AccessibilityChildTreeCallback>> childTreeCallbackMap_;
    int64_t parentElementId_ = INVALID_PARENT_ID;
    uint32_t parentWindowId_ = 0;
    int32_t parentTreeId_ = 0;
    std::function<void(int32_t&, int32_t&)> getParentRectHandler_;
    bool isUseJson_ = false;
};
} // namespace OHOS::Ace::Framework
#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_JS_ACCESSIBILITY_MANAGER_H
