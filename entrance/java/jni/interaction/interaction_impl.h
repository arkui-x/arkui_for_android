/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INTERACTION_MANAGER_IMPL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INTERACTION_MANAGER_IMPL_H

#include "virtual_rs_window.h"

#include "core/common/interaction/interaction_interface.h"

namespace OHOS::Ace {

class SurfaceNodeListener : public OHOS::Rosen::IWindowSurfaceNodeListener {
public:
    explicit SurfaceNodeListener(std::shared_ptr<OHOS::Rosen::Window> dragWindow, const DragDataCore& dragData)
        : dragWindow_(dragWindow), dragData(dragData) {};
    ~SurfaceNodeListener() = default;

    void OnSurfaceNodeCreated() {}

    void OnSurfaceNodeChanged(int32_t width, int32_t height, float density);

    void OnSurfaceNodeDestroyed() {}

    std::shared_ptr<OHOS::Rosen::Window> dragWindow_;
    DragDataCore dragData;
};

class InteractionImpl : public InteractionInterface {
    DECLARE_ACE_TYPE(InteractionImpl, InteractionInterface);

public:
    int32_t UpdateShadowPic(const ShadowInfoCore& shadowInfo) override;

    int32_t SetDragWindowVisible(bool visible, const std::shared_ptr<Rosen::RSTransaction>& rSTransaction) override;

    int32_t SetMouseDragMonitorState(bool state) override;

    int32_t StartDrag(const DragDataCore& dragData,
        std::function<void(const OHOS::Ace::DragNotifyMsg&)> callback) override;
        
    int32_t GetDragBundleInfo(DragBundleInfo& dragBundleInfo) override;

    int32_t UpdateDragStyle(DragCursorStyleCore style, const int32_t eventId = -1) override;

    int32_t UpdatePreviewStyle(const PreviewStyle& previewStyle) override;

    int32_t UpdatePreviewStyleWithAnimation(const PreviewStyle& previewStyle,
        const PreviewAnimation& animation) override;

    int32_t StopDrag(DragDropRet result) override;

    int32_t GetUdKey(std::string& udKey) override;

    int32_t GetShadowOffset(ShadowOffsetData& shadowOffsetData) override;

    int32_t GetDragState(DragState& dragState) const override;

    int32_t GetDragSummary(std::map<std::string, int64_t>& summary, std::map<std::string, int64_t>& detailedSummary,
        std::map<std::string, std::vector<int32_t>>& summaryFormat, int32_t& version, int64_t& totalSize) override;

    int32_t GetDragExtraInfo(std::string& extraInfo) override;

    int32_t EnterTextEditorArea(bool enable) override;

    int32_t AddPrivilege() override;

    int32_t RegisterCoordinationListener(std::function<void()> dragOutCallback) override;

    int32_t UnRegisterCoordinationListener() override;

    int32_t SetDraggableState(bool state) override;

    int32_t GetAppDragSwitchState(bool& state) override;

    void SetDraggableStateAsync(bool state, int64_t downTime) override;

    int32_t EnableInternalDropAnimation(const std::string& animationInfo) override;

    bool IsDragStart() const override;

    int32_t UpdatePointAction(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    
    int32_t GetPointerId()
    {
        if (surfaceNodeListener_ == nullptr) {
            return -1;
        }
        return surfaceNodeListener_->dragData.pointerId;
    }
private:
    std::shared_ptr<OHOS::Rosen::Window> GetDragWindow();
    void RegisterDragWindow();
    void SetSVGFilePath();
    OHOS::sptr<SurfaceNodeListener> surfaceNodeListener_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CAPABILITY_INTERACTION_IMPL_H