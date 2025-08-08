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

#include "interaction_impl.h"

#include "adapter/android/stage/uicontent/ace_container_sg.h"
#include "base/log/log.h"
#include "core/common/container.h"
#include "core/common/interaction/interaction_data.h"

#ifdef ENABLE_DRAG_FRAMEWORK
#include "drag_data.h"
#include "interaction_manager.h"
#endif

#ifdef ENABLE_DRAG_FRAMEWORK
using namespace OHOS::Msdp::DeviceStatus;
#endif

namespace OHOS::Ace {
#ifdef ENABLE_DRAG_FRAMEWORK
Msdp::DeviceStatus::DragCursorStyle TranslateDragCursorStyle(OHOS::Ace::DragCursorStyleCore style);
Msdp::DeviceStatus::DragResult TranslateDragResult(DragRet dragResult);
DragRet TranslateDragResult(Msdp::DeviceStatus::DragResult dragResult);
Msdp::DeviceStatus::DragBehavior TranslateDragBehavior(OHOS::Ace::DragBehavior dragBehavior);
OHOS::Ace::DragBehavior TranslateDragBehavior(Msdp::DeviceStatus::DragBehavior dragBehavior);
#endif

bool g_dragging = false;
#ifdef ENABLE_DRAG_FRAMEWORK
std::function<void(const OHOS::Ace::DragNotifyMsg&)> callback_;
#endif

InteractionInterface* InteractionInterface::GetInstance()
{
    static InteractionImpl instance;
    return &instance;
}

int32_t InteractionImpl::UpdateShadowPic(const OHOS::Ace::ShadowInfoCore& shadowInfo)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    auto pixelMap = shadowInfo.pixelMap;
    if (!pixelMap) {
        Msdp::DeviceStatus::ShadowInfo msdpShadowInfo { nullptr, shadowInfo.x, shadowInfo.y };
        return InteractionManager::GetInstance()->UpdateShadowPic(msdpShadowInfo);
    }
    Msdp::DeviceStatus::ShadowInfo msdpShadowInfo { shadowInfo.pixelMap->GetPixelMapSharedPtr(), shadowInfo.x,
        shadowInfo.y };
    return InteractionManager::GetInstance()->UpdateShadowPic(msdpShadowInfo);
#endif
    return -1;
}

int32_t InteractionImpl::SetDragWindowVisible(bool visible, const std::shared_ptr<Rosen::RSTransaction>& rSTransaction)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    return InteractionManager::GetInstance()->SetDragWindowVisible(visible);
#endif
    return -1;
}

int32_t InteractionImpl::SetMouseDragMonitorState(bool state)
{
    return -1;
}

int32_t InteractionImpl::StartDrag(
    const DragDataCore& dragData, std::function<void(const OHOS::Ace::DragNotifyMsg&)> callback)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    callback_ = callback;
    RegisterDragWindow();
    SetSVGFilePath();
    std::shared_ptr<OHOS::Rosen::Window> window = GetDragWindow();
    surfaceNodeListener_ = new SurfaceNodeListener(window, dragData);
    window->RegisterSurfaceNodeListener(surfaceNodeListener_);
    window->ShowWindow();
    g_dragging = true;
#endif
    return 0;
}

int32_t InteractionImpl::GetDragBundleInfo(DragBundleInfo& dragBundleInfo)
{
    return -1;
}

int32_t InteractionImpl::UpdateDragStyle(OHOS::Ace::DragCursorStyleCore style, const int32_t eventId)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    return InteractionManager::GetInstance()->UpdateDragStyle(TranslateDragCursorStyle(style));
#endif
    return -1;
}

int32_t InteractionImpl::UpdatePreviewStyle(const OHOS::Ace::PreviewStyle& previewStyle)
{
    return -1;
}

int32_t InteractionImpl::UpdatePreviewStyleWithAnimation(
    const OHOS::Ace::PreviewStyle& previewStyle, const OHOS::Ace::PreviewAnimation& animation)
{
    return -1;
}

int32_t InteractionImpl::StopDrag(DragDropRet result)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    Msdp::DeviceStatus::DragDropResult dragDropResult { TranslateDragResult(result.result), result.hasCustomAnimation,
        result.mainWindow, TranslateDragBehavior(result.dragBehavior) };
    int32_t ret = InteractionManager::GetInstance()->StopDrag(dragDropResult);
    g_dragging = false;
    OHOS::Ace::DragNotifyMsg msg { 0, 0, InteractionManager::GetInstance()->GetDragTargetPid(),
            TranslateDragResult(dragDropResult.result), TranslateDragBehavior(dragDropResult.dragBehavior) };
    if (callback_) {
        callback_(msg);
    }
    return ret;
#endif
    return -1;
}

int32_t InteractionImpl::GetUdKey(std::string& udKey)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    return InteractionManager::GetInstance()->GetUdKey(udKey);
#endif
    return -1;
}

int32_t InteractionImpl::GetShadowOffset(ShadowOffsetData& shadowOffsetData)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    return InteractionManager::GetInstance()->GetShadowOffset(
        shadowOffsetData.offsetX, shadowOffsetData.offsetY, shadowOffsetData.width, shadowOffsetData.height);
#endif
    return -1;
}

int32_t InteractionImpl::GetDragSummary(std::map<std::string, int64_t>& summary,
    std::map<std::string, int64_t>& detailedSummary, std::map<std::string, std::vector<int32_t>>& summaryFormat,
    int32_t& version, int64_t& totalSize)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    Msdp::DeviceStatus::DragSummaryInfo dragSummary;
    auto ret = InteractionManager::GetInstance()->GetDragSummaryInfo(dragSummary);
    if (ret != 0) {
        return ret;
    }
    summary = dragSummary.summarys;
    detailedSummary = dragSummary.detailedSummarys;
    summaryFormat = dragSummary.summaryFormat;
    version = dragSummary.version;
    totalSize = dragSummary.totalSize;
    return ret;
#endif
    return -1;
}

int32_t InteractionImpl::GetDragExtraInfo(std::string& extraInfo)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    return InteractionManager::GetInstance()->GetExtraInfo(extraInfo);
#endif
    return -1;
}

int32_t InteractionImpl::EnterTextEditorArea(bool enable)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    return InteractionManager::GetInstance()->EnterTextEditorArea(enable);
#endif
    return -1;
}

int32_t InteractionImpl::GetDragState(DragState& dragState) const
{
#ifdef ENABLE_DRAG_FRAMEWORK
    Msdp::DeviceStatus::DragState state;
    int32_t ret = InteractionManager::GetInstance()->GetDragState(state);
    switch (state) {
        case Msdp::DeviceStatus::DragState::ERROR:
            dragState = DragState::ERROR;
            break;
        case Msdp::DeviceStatus::DragState::START:
            dragState = DragState::START;
            break;
        case Msdp::DeviceStatus::DragState::STOP:
            dragState = DragState::STOP;
            break;
        case Msdp::DeviceStatus::DragState::CANCEL:
            dragState = DragState::CANCEL;
            break;
        case Msdp::DeviceStatus::DragState::MOTION_DRAGGING:
            dragState = DragState::MOTION_DRAGGING;
            break;
        default:
            dragState = DragState::ERROR;
            LOGW("unknow msdp drag state: %d", state);
            break;
    }
    return ret;
#endif
    return -1;
}

int32_t InteractionImpl::AddPrivilege()
{
    return -1;
}

int32_t InteractionImpl::RegisterCoordinationListener(std::function<void()> dragOutCallback)
{
    return -1;
}

int32_t InteractionImpl::UnRegisterCoordinationListener()
{
    return -1;
}

int32_t InteractionImpl::SetDraggableState(bool state)
{
    return -1;
}

int32_t InteractionImpl::GetAppDragSwitchState(bool& state)
{
    return -1;
}

void InteractionImpl::SetDraggableStateAsync(bool state, int64_t downTime) {}

int32_t InteractionImpl::EnableInternalDropAnimation(const std::string& animationInfo)
{
    return -1;
}

bool InteractionImpl::IsDragStart() const
{
#if defined(ENABLE_DRAG_FRAMEWORK)
    return InteractionManager::GetInstance()->IsDragStart();
#endif
    return false;
}

#ifdef ENABLE_DRAG_FRAMEWORK
Msdp::DeviceStatus::DragCursorStyle TranslateDragCursorStyle(OHOS::Ace::DragCursorStyleCore style)
{
    switch (style) {
        case OHOS::Ace::DragCursorStyleCore::DEFAULT:
            return Msdp::DeviceStatus::DragCursorStyle::DEFAULT;
        case OHOS::Ace::DragCursorStyleCore::FORBIDDEN:
            return Msdp::DeviceStatus::DragCursorStyle::FORBIDDEN;
        case OHOS::Ace::DragCursorStyleCore::COPY:
            return Msdp::DeviceStatus::DragCursorStyle::COPY;
        case OHOS::Ace::DragCursorStyleCore::MOVE:
            return Msdp::DeviceStatus::DragCursorStyle::MOVE;
        default:
            return Msdp::DeviceStatus::DragCursorStyle::DEFAULT;
    }
}

Msdp::DeviceStatus::DragResult TranslateDragResult(DragRet dragResult)
{
    switch (dragResult) {
        case DragRet::DRAG_SUCCESS:
            return Msdp::DeviceStatus::DragResult::DRAG_SUCCESS;
        case DragRet::DRAG_FAIL:
            return Msdp::DeviceStatus::DragResult::DRAG_FAIL;
        case DragRet::DRAG_CANCEL:
            return Msdp::DeviceStatus::DragResult::DRAG_CANCEL;
        default:
            return Msdp::DeviceStatus::DragResult::DRAG_SUCCESS;
    }
}

DragRet TranslateDragResult(Msdp::DeviceStatus::DragResult dragResult)
{
    switch (dragResult) {
        case Msdp::DeviceStatus::DragResult::DRAG_SUCCESS:
            return DragRet::DRAG_SUCCESS;
        case Msdp::DeviceStatus::DragResult::DRAG_FAIL:
            return DragRet::DRAG_FAIL;
        case Msdp::DeviceStatus::DragResult::DRAG_CANCEL:
            return DragRet::DRAG_CANCEL;
        default:
            return DragRet::DRAG_SUCCESS;
    }
}

Msdp::DeviceStatus::DragBehavior TranslateDragBehavior(OHOS::Ace::DragBehavior dragBehavior)
{
    switch (dragBehavior) {
        case OHOS::Ace::DragBehavior::COPY:
            return Msdp::DeviceStatus::DragBehavior::COPY;
        case OHOS::Ace::DragBehavior::MOVE:
            return Msdp::DeviceStatus::DragBehavior::MOVE;
        default:
            return Msdp::DeviceStatus::DragBehavior::UNKNOWN;
    }
}
OHOS::Ace::DragBehavior TranslateDragBehavior(Msdp::DeviceStatus::DragBehavior dragBehavior)
{
    switch (dragBehavior) {
        case Msdp::DeviceStatus::DragBehavior::COPY:
            return OHOS::Ace::DragBehavior::COPY;
        case Msdp::DeviceStatus::DragBehavior::MOVE:
            return OHOS::Ace::DragBehavior::MOVE;
        default:
            return OHOS::Ace::DragBehavior::UNKNOWN;
    }
}
#endif

int32_t InteractionImpl::UpdatePointAction(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    return InteractionManager::GetInstance()->UpdatePointerAction(pointerEvent);
#endif
    return -1;
}

void InteractionImpl::RegisterDragWindow()
{
    auto containerId = Container::CurrentId();
    auto Destroycallback = [this, containerId] {
        auto container = Platform::AceContainerSG::GetContainer(containerId);
        CHECK_NULL_VOID(container);
        auto taskExecutor = container->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        ContainerScope scope(containerId);
        auto task = [weak = AceType::WeakClaim(this)] {
            auto interaction = weak.Upgrade();
            auto window = interaction->surfaceNodeListener_->dragWindow_;
            window->UnregisterSurfaceNodeListener(interaction->surfaceNodeListener_);
            window->Destroy();
            interaction->surfaceNodeListener_->dragWindow_ = nullptr;
        };
        taskExecutor->PostTask(task, TaskExecutor::TaskType::UI, "ArkUI-XInteractionImplStopDrag");
    };
#ifdef ENABLE_DRAG_FRAMEWORK
    InteractionManager::GetInstance()->RegisterDragWindow(Destroycallback);
#endif
}

void InteractionImpl::SetSVGFilePath()
{
    auto containerId = Container::CurrentId();
    auto container = Platform::AceContainerSG::GetContainer(containerId);
    std::string packagePath = container->GetPackagePathStr();
    std::string filePath = packagePath + "/systemres" + "/resources";
#ifdef ENABLE_DRAG_FRAMEWORK
    InteractionManager::GetInstance()->SetSVGFilePath(filePath);
#endif
}

std::shared_ptr<OHOS::Rosen::Window> InteractionImpl::GetDragWindow()
{
    auto containerId = Container::CurrentId();
    auto container = Platform::AceContainerSG::GetContainer(containerId);
    sptr<Rosen::Window> window = container->GetUIWindow(containerId);
    auto dragWindow = Rosen::Window::CreateDragWindow(window->GetContext());
    return dragWindow;
}

void SurfaceNodeListener::OnSurfaceNodeChanged(int32_t width, int32_t height, float density)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    if (g_dragging) {
        InteractionManager::GetInstance()->SetDragWindow(dragWindow_);
        Msdp::DeviceStatus::DragData msdpDragData { {}, dragData.buffer, dragData.udKey, dragData.extraInfo,
            dragData.filterInfo, dragData.sourceType, dragData.dragNum, dragData.pointerId,
            dragData.displayX, dragData.displayY, dragData.displayId, dragData.mainWindow,
            dragData.hasCanceledAnimation, dragData.hasCoordinateCorrected, dragData.summarys };
        for (auto& shadowInfo : dragData.shadowInfos) {
            if (shadowInfo.pixelMap) {
                msdpDragData.shadowInfos.push_back(
                    { shadowInfo.pixelMap->GetPixelMapSharedPtr(), shadowInfo.x, shadowInfo.y });
            } else {
                msdpDragData.shadowInfos.push_back({ nullptr, shadowInfo.x, shadowInfo.y });
            }
        }
        InteractionManager::GetInstance()->StartDrag(msdpDragData);
        g_dragging = false;
    }
#endif
}
} // namespace OHOS::Ace