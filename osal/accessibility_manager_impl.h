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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_ACCESSIBILITY_MANAGER_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_ACCESSIBILITY_MANAGER_IMPL_H

#include "foundation/appframework/arkui/uicontent/component_info.h"

#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/bridge/common/accessibility/accessibility_node_manager.h"

namespace OHOS::Ace::Platform {
struct ComponentInfo;
}

namespace OHOS::Ace::Framework {

class ACE_EXPORT AccessibilityManagerImpl : public AccessibilityNodeManager {
    DECLARE_ACE_TYPE(AccessibilityManagerImpl, AccessibilityNodeManager);

public:
    AccessibilityManagerImpl() = default;
    ~AccessibilityManagerImpl() override = default;
    bool GetAllComponents(NodeId nodeID, OHOS::Ace::Platform::ComponentInfo& rootComponent);

protected:
    void DumpHandleEvent(const std::vector<std::string>& params) override;
    void DumpProperty(const std::vector<std::string>& params) override;
    void DumpTree(int32_t depth, NodeId nodeID) override;

private:
    RefPtr<NG::PipelineContext> FindPipelineByElementId(const int32_t elementId, RefPtr<NG::FrameNode>& node);
    RefPtr<NG::FrameNode> FindNodeFromPipeline(const WeakPtr<PipelineBase>& context, const int32_t elementId);
    bool ExecuteActionNG(int32_t elementId, AceAction action, const RefPtr<PipelineBase>& context);
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_ADAPTER_PREVIEW_INSPECTOR_JS_INSPECTOR_MANAGER_H
