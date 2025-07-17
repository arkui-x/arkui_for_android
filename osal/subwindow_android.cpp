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

#include "adapter/android/osal/subwindow_android.h"

#include "adapter/android/stage/uicontent/ace_view_sg.h"
#include "adapter/android/entrance/java/jni/display_info.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "display_manager.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "interfaces/inner_api/ace/viewport_config.h"

namespace OHOS::Ace {
namespace {
const Rect MIN_WINDOW_HOT_AREA = Rect(0.0f, 0.0f, 1.0f, 1.0f);
const std::string SUBWINDOW_PREFIX = "ARK_APP_SUBWINDOW_";
} // namespace

int32_t SubwindowAndroid::id_ = 0;
RefPtr<Subwindow> Subwindow::CreateSubwindow(int32_t instanceId)
{
    return AceType::MakeRefPtr<SubwindowAndroid>(instanceId);
}

SubwindowAndroid::SubwindowAndroid(int32_t instanceId) : windowId_(id_), parentContainerId_(instanceId)
{
    SetSubwindowId(windowId_);
    id_++;
}

void SubwindowAndroid::InitContainer()
{
    LOGI("Init container enter");
    auto parentContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
    CHECK_NULL_VOID(parentContainer);
    InitSubwindow(parentContainer);
    CHECK_NULL_VOID(window_);

    std::string url = "";
    window_->SetUIContent(
        url, reinterpret_cast<NativeEngine*>(parentContainer->GetSharedRuntime()), nullptr, false, nullptr, false);
    childContainerId_ = window_->GetWindowId();

    SubwindowManager::GetInstance()->AddParentContainerId(childContainerId_, parentContainerId_);

    if (!InitSubContainer(parentContainer)) {
        SetIsRosenWindowCreate(false);
        return;
    }
    // create ace view
    auto* aceView = Platform::AceViewSG::CreateView(childContainerId_);
    Platform::AceViewSG::SurfaceCreated(aceView, window_.get());

    sptr<Rosen::Display> defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplaySync();
    CHECK_NULL_VOID(defaultDisplay);
    sptr<Rosen::DisplayInfo> defaultDisplayInfo = defaultDisplay->GetDisplayInfo();
    CHECK_NULL_VOID(defaultDisplayInfo);
    int32_t width = defaultDisplayInfo->GetWidth();
    int32_t height = defaultDisplayInfo->GetHeight();
    auto parentPipeline = parentContainer->GetPipelineContext();
    CHECK_NULL_VOID(parentPipeline);
    auto density = parentPipeline->GetDensity();
    LOGI(
        "UIContent Initialize: width: %{public}d, height: %{public}d, density: %{public}lf", width, height, density);

    // set view
    ViewportConfig config;
    SetIsRosenWindowCreate(true);
    Platform::AceContainerSG::SetView(aceView, density, width, height, window_.get());
    Platform::AceViewSG::SurfaceChanged(aceView, width, height, config.Orientation());

    auto subPipelineContextNG = AceType::DynamicCast<NG::PipelineContext>(
        Platform::AceContainerSG::GetContainer(childContainerId_)->GetPipelineContext());
    CHECK_NULL_VOID(subPipelineContextNG);
    subPipelineContextNG->SetParentPipeline(parentContainer->GetPipelineContext());
    subPipelineContextNG->SetupSubRootElement();
    subPipelineContextNG->SetMinPlatformVersion(parentPipeline->GetMinPlatformVersion());
}

void SubwindowAndroid::InitSubwindow(const RefPtr<Platform::AceContainerSG>& parentContainer)
{
    if (window_) {
        LOGI("The window has been created, windowName: %{public}s",
            window_->GetWindowName().c_str());
        return;
    }

    sptr<Rosen::Display> defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplaySync();
    CHECK_NULL_VOID(defaultDisplay);
    sptr<Rosen::DisplayInfo> defaultDisplayInfo = defaultDisplay->GetDisplayInfo();
    CHECK_NULL_VOID(defaultDisplayInfo);

    std::shared_ptr<Rosen::WindowOption> windowOption = std::make_shared<Rosen::WindowOption>();
    auto parentWindowName = parentContainer->GetWindowName();
    auto parentWindowId = parentContainer->GetWindowId();
    sptr<Rosen::Window> parentWindow = parentContainer->GetUIWindow(parentContainerId_);
    CHECK_NULL_VOID(parentWindow);
    parentWindow_ = parentWindow;
    windowOption->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    windowOption->SetParentId(parentWindowId);
    windowOption->SetWindowRect({ 0, 0, defaultDisplayInfo->GetWidth(), defaultDisplayInfo->GetHeight() });
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    if (parentWindow->GetContext() == nullptr) {
        LOGW("Parent window context is null!");
    }

    windowOption->SetWindowName(SUBWINDOW_PREFIX + parentWindowName + std::to_string(windowId_));
    window_ = Rosen::Window::CreateSubWindow(parentWindow->GetContext(), windowOption);
    if (!window_) {
        LOGE("Window create failed.");
    }
}

bool SubwindowAndroid::InitSubContainer(const RefPtr<Platform::AceContainerSG>& parentContainer) const
{
    auto container = Platform::AceContainerSG::GetContainer(childContainerId_);
    if (!container) {
        LOGE("Window get ace container failed.");
        return false;
    }

    if (!parentContainer) {
        LOGE("Init sub container, parentContainer is null.");
        return false;
    }
    CHECK_NULL_RETURN(window_, false);
    container->SetWindowId(window_->GetWindowId());
    container->SetParentId(parentContainerId_);
    container->GetSettings().SetUsingSharedRuntime(true);
    container->SetSharedRuntime(parentContainer->GetSharedRuntime());
    container->Initialize();
    container->SetAssetManagerIfNull(parentContainer->GetAssetManager());
    container->SetResourceConfiguration(parentContainer->GetResourceConfiguration());
    container->SetPackagePathStr(parentContainer->GetPackagePathStr());
    container->SetHapPath(parentContainer->GetHapPath());
    container->SetIsSubContainer(true);
    container->InitializeSubContainer(parentContainerId_);
    return true;
}

void SubwindowAndroid::ShowWindow(bool needFocus)
{
    CHECK_NULL_VOID(window_);
    LOGI("Show the subwindow %{public}s", window_->GetWindowName().c_str());
    if (isToastWindow_) {
        ClearToast();
    }
    if (isShowed_) {
        LOGI("Subwindow id:%{public}u is on display", window_->GetWindowId());
        if (needFocus) {
            window_->SetFocusable(needFocus);
            RequestFocus();
        }
        return;
    }
    // Set min window hot area so that sub window can transparent event.
    std::vector<Rect> rects;
    rects.emplace_back(MIN_WINDOW_HOT_AREA);
    SetHotAreas(rects, -1);
    auto retFocusable = window_->SetFocusable(needFocus);
    if (retFocusable != Rosen::WMError::WM_OK) {
        LOGW(
            "Subwindow id:%{public}u set focusable %{public}d failed with WMError: %{public}d", window_->GetWindowId(),
            needFocus, static_cast<int32_t>(retFocusable));
    }
    auto ret = window_->ShowWindow();
    if (ret != Rosen::WMError::WM_OK) {
        LOGW("Show subwindow id:%{public}u failed with WMError: %{public}d",
            window_->GetWindowId(), static_cast<int32_t>(ret));
        return;
    }
    if (needFocus) {
        RequestFocus();
    }

    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = aceContainer->GetPipelineContext();
    CHECK_NULL_VOID(context);
    AccessibilityEvent event;
    event.type = AccessibilityEventType::PAGE_CHANGE;
    event.windowId = context->GetWindowId();
    event.windowChangeTypes = WINDOW_UPDATE_ADDED;
    context->SendEventToAccessibility(event);
    isShowed_ = true;
    SubwindowManager::GetInstance()->SetCurrentSubwindow(AceType::Claim(this));
}

void SubwindowAndroid::ResizeWindow()
{
    LOGI("Resize window called.");
    CHECK_NULL_VOID(window_);
    sptr<Rosen::Display> defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplaySync();
    CHECK_NULL_VOID(defaultDisplay);
    sptr<Rosen::DisplayInfo> defaultDisplayInfo = defaultDisplay->GetDisplayInfo();
    CHECK_NULL_VOID(defaultDisplayInfo);
    auto ret = window_->ResizeWindowTo(defaultDisplayInfo->GetWidth(), defaultDisplayInfo->GetHeight());
    if (ret != Rosen::WMError::WM_OK) {
        LOGW("Resize window by default display failed with errCode: %{public}d",
            static_cast<int32_t>(ret));
    } else {
        LOGI(
            "Resize window rect to x: %{public}d, y: %{public}d, width: %{public}u, height: %{public}u",
            window_->GetRect().posX_, window_->GetRect().posY_, window_->GetRect().width_, window_->GetRect().height_);
    }
}

void SubwindowAndroid::HideSubWindowNG()
{
    HideWindow();
}

void SubwindowAndroid::clearStatus()
{
    isMenuWindow_ = false;
    SetIsToastWindow(false);
    window_->SetFullScreen(false);
}

void SubwindowAndroid::HideWindow()
{
    CHECK_NULL_VOID(window_);
    LOGI("Hide the subwindow %{public}s", window_->GetWindowName().c_str());

    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);

    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    auto focusHub = rootNode->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetIsDefaultHasFocused(false);

    OHOS::Rosen::WMError ret = window_->Hide();
    auto parentContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
    CHECK_NULL_VOID(parentContainer);
    if (parentContainer->IsSceneBoardWindow()) {
        window_->SetTouchable(true);
    }

    if (ret != OHOS::Rosen::WMError::WM_OK && window_->IsWindowShow()) {
        LOGW("Hide window failed with errCode: %{public}d", static_cast<int32_t>(ret));
        return;
    }
    isShowed_ = false;
    clearStatus();
    LOGI("Hide the subwindow successfully.");

    AccessibilityEvent event;
    event.type = AccessibilityEventType::PAGE_CHANGE;
    event.windowId = context->GetWindowId();
    event.windowChangeTypes = WINDOW_UPDATE_REMOVED;
    context->SendEventToAccessibility(event);
}

bool SubwindowAndroid::IsSameDisplayWithParentWindow(bool useInitializedId)
{
    return false;
}

void SubwindowAndroid::SetHotAreas(const std::vector<Rect>& rects, int32_t overlayId)
{
    LOGI("Set hot areas enter.");
    CHECK_NULL_VOID(window_);

    std::vector<Rosen::Rect> hotAreas;
    Rosen::Rect rosenRect {};
    for (const auto& rect : rects) {
        RectConverter(rect, rosenRect);
        hotAreas.emplace_back(rosenRect);
    }
    if (overlayId >= 0) {
        hotAreasMap_[overlayId] = hotAreas;
    }

    window_->SetTouchHotAreas(hotAreas);
}

void SubwindowAndroid::DeleteHotAreas(int32_t overlayId)
{
    LOGI("Delete hot areas enter.");
    CHECK_NULL_VOID(window_);
    hotAreasMap_.erase(overlayId);
    std::vector<Rosen::Rect> hotAreas;
    for (auto it = hotAreasMap_.begin(); it != hotAreasMap_.end(); it++) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            hotAreas.emplace_back(*it2);
        }
    }
    window_->SetTouchHotAreas(hotAreas);
}

void SubwindowAndroid::RectConverter(const Rect& rect, Rosen::Rect& rosenRect)
{
    rosenRect.posX_ = static_cast<int>(rect.GetOffset().GetX());
    rosenRect.posY_ = static_cast<int>(rect.GetOffset().GetY());
    rosenRect.width_ = static_cast<uint32_t>(rect.GetSize().Width());
    rosenRect.height_ = static_cast<uint32_t>(rect.GetSize().Height());
    LOGI(
        "Convert rect to rosenRect, x is %{public}d, y is %{public}d, width is %{public}d, height is %{public}d",
        rosenRect.posX_, rosenRect.posY_, rosenRect.width_, rosenRect.height_);
}

void SubwindowAndroid::GetPopupInfoNG(int32_t targetId, NG::PopupInfo& popupInfo)
{
    LOGI("Get popup info ng enter.");
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    popupInfo = overlayManager->GetPopupInfo(targetId);
}

void SubwindowAndroid::ShowPopupNG(int32_t targetId, const NG::PopupInfo& popupInfo,
    const std::function<void(int32_t)>&& onWillDismiss, bool interactiveDismiss)
{
    LOGI("Show popup ng enter.");
    CHECK_NULL_VOID(window_);
    popupTargetId_ = targetId;
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);

    ShowWindow(popupInfo.focusable);
    window_->SetFullScreen(true);
    window_->SetTouchable(true);

    ResizeWindow();
    ContainerScope scope(childContainerId_);
    overlayManager->ShowPopup(targetId, popupInfo);
    window_->SetFocusable(true);
}

void SubwindowAndroid::HidePopupNG(int32_t targetId)
{
    LOGI("Hide popup ng enter");
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    auto popupInfo = overlayManager->GetPopupInfo(targetId == -1 ? popupTargetId_ : targetId);
    popupInfo.markNeedUpdate = true;
    ContainerScope scope(childContainerId_);
    overlayManager->HidePopup(targetId == -1 ? popupTargetId_ : targetId, popupInfo);
    HideWindow();
    context->FlushPipelineImmediately();
    HideEventColumn();
    HidePixelMap();
    HideFilter();
}

void SubwindowAndroid::ClearPopupNG()
{
    LOGI("Clear popup ng enter");
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    overlay->CleanPopupInSubWindow();
    HideWindow();
    context->FlushPipelineImmediately();
}

void SubwindowAndroid::ShowMenuNG(const RefPtr<NG::FrameNode> menuNode, int32_t targetId, const NG::OffsetF& offset)
{
    LOGI("Show menu ng enter");
    CHECK_NULL_VOID(window_);
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    ShowWindow();
    ResizeWindow();
    isMenuWindow_ = isShowed_;
    window_->SetTouchable(true);
    ContainerScope scope(childContainerId_);
    overlay->ShowMenuInSubWindow(targetId, offset, menuNode);
}

void SubwindowAndroid::ShowMenuNG(const RefPtr<NG::FrameNode> customNode, const NG::MenuParam& menuParam,
    const RefPtr<NG::FrameNode>& targetNode, const NG::OffsetF& offset)
{
    CHECK_NULL_VOID(customNode);
    CHECK_NULL_VOID(targetNode);
    ShowMenuNG(customNode, targetNode->GetId(), offset);
}

void SubwindowAndroid::ShowMenuNG(std::function<void()>&& buildFunc, std::function<void()>&& previewBuildFunc,
    const NG::MenuParam& menuParam, const RefPtr<NG::FrameNode>& targetNode, const NG::OffsetF& offset)
{
    LOGI("Show menu ng enter");
    CHECK_NULL_VOID(window_);
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    ShowWindow();
    ResizeWindow();
    isMenuWindow_ = isShowed_;
    window_->SetTouchable(true);
    NG::ScopedViewStackProcessor builderViewStackProcessor;
    buildFunc();
    auto customNode = NG::ViewStackProcessor::GetInstance()->Finish();
    RefPtr<NG::UINode> previewCustomNode;
    if (previewBuildFunc && menuParam.previewMode == MenuPreviewMode::CUSTOM) {
        previewBuildFunc();
        previewCustomNode = NG::ViewStackProcessor::GetInstance()->Finish();
    }
    auto menuNode =
        NG::MenuView::Create(customNode, targetNode->GetId(), targetNode->GetTag(), menuParam, true, previewCustomNode);
    auto menuWrapperPattern = menuNode->GetPattern<NG::MenuWrapperPattern>();
    CHECK_NULL_VOID(menuWrapperPattern);
    menuWrapperPattern->RegisterMenuCallback(menuNode, menuParam);
    menuWrapperPattern->SetMenuTransitionEffect(menuNode, menuParam);
    overlay->ShowMenuInSubWindow(targetNode->GetId(), offset, menuNode);
}

void SubwindowAndroid::SetWindowTouchable(bool touchable){}

void SubwindowAndroid::HideMenuNG(bool showPreviewAnimation, bool startDrag)
{
    LOGI("Hide menu ng enter");
    if (!isShowed_) {
        LOGE("Hide menu ng failed.");
        return;
    }
    isShowed_ = false;
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    ContainerScope scope(childContainerId_);
    overlay->HideMenuInSubWindow(showPreviewAnimation, startDrag);
    HideEventColumn();
    HidePixelMap(false, 0, 0, false);
    HideFilter();
}

void SubwindowAndroid::HideMenuNG(const RefPtr<NG::FrameNode>& menu, int32_t targetId)
{
    LOGI("Hide menu ng enter");
    if (!isShowed_) {
        LOGW("Hide menu ng failed.");
        return;
    }
    isShowed_ = false;
    LOGI("Subwindow hide menu for target id %{public}d", targetId);
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    overlay->HideMenuInSubWindow(menu, targetId);
    HideEventColumn();
    HidePixelMap(false, 0, 0, false);
    HideFilter();
}

void SubwindowAndroid::ClearMenuNG(int32_t targetId, bool inWindow, bool showAnimation)
{
    if (isMenuWindow_) {
        LOGI("Clear menu ng enter");
        auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
        CHECK_NULL_VOID(aceContainer);
        auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
        CHECK_NULL_VOID(context);
        auto overlay = context->GetOverlayManager();
        CHECK_NULL_VOID(overlay);
        if (showAnimation) {
            overlay->CleanMenuInSubWindowWithAnimation();
        } else {
            overlay->CleanMenuInSubWindow(targetId);
        }
        HideWindow();
        context->FlushPipelineImmediately();
        if (inWindow) {
            HideEventColumn();
        }
        HidePixelMap(false, 0, 0, false);
        HideFilter();
    }
}

void SubwindowAndroid::UpdateHideMenuOffsetNG(
    const NG::OffsetF& offset, float menuScale, bool isRedragStart, int32_t menuWrapperId)
{
    ContainerScope scope(childContainerId_);
    auto pipelineContext = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto overlay = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    if (overlay->IsContextMenuDragHideFinished()) {
        return;
    }
    overlay->UpdateContextMenuDisappearPosition(offset);
}

RefPtr<NG::FrameNode> SubwindowAndroid::ShowDialogNG(
    const DialogProperties& dialogProps, std::function<void()>&& buildFunc)
{
    LOGI("Show dialog ng enter");
    CHECK_NULL_RETURN(window_, nullptr);
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_RETURN(aceContainer, nullptr);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_RETURN(context, nullptr);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_RETURN(overlay, nullptr);
    std::map<int32_t, RefPtr<NG::FrameNode>> DialogMap(overlay->GetDialogMap().begin(), overlay->GetDialogMap().end());
    if (static_cast<int>(DialogMap.size()) == 0) {
        auto parentAceContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
        CHECK_NULL_RETURN(parentAceContainer, nullptr);
        auto parentcontext = DynamicCast<NG::PipelineContext>(parentAceContainer->GetPipelineContext());
        CHECK_NULL_RETURN(parentcontext, nullptr);
        auto parentOverlay = parentcontext->GetOverlayManager();
        CHECK_NULL_RETURN(parentOverlay, nullptr);
        parentOverlay->SetSubWindowId(SubwindowManager::GetInstance()->GetDialogSubwindowInstanceId(GetSubwindowId()));
    }
    ShowWindow();
    window_->SetFullScreen(true);
    window_->SetTouchable(true);
    ResizeWindow();
    ContainerScope scope(childContainerId_);
    auto dialog = overlay->ShowDialog(dialogProps, std::move(buildFunc));
    CHECK_NULL_RETURN(dialog, nullptr);
    haveDialog_ = true;
    return dialog;
}

void SubwindowAndroid::CloseDialogNG(const RefPtr<NG::FrameNode>& dialogNode)
{
    LOGI("Close dialog ng enter");
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    ContainerScope scope(childContainerId_);
    overlay->CloseDialog(dialogNode);
}

void SubwindowAndroid::OpenCustomDialogNG(const DialogProperties& dialogProps, std::function<void(int32_t)>&& callback)
{
    LOGI("Open customDialog ng subwindow enter.");
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    std::map<int32_t, RefPtr<NG::FrameNode>> DialogMap(overlay->GetDialogMap().begin(), overlay->GetDialogMap().end());
    if (static_cast<int>(DialogMap.size()) == 0) {
        auto parentAceContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
        CHECK_NULL_VOID(parentAceContainer);
        auto parentcontext = DynamicCast<NG::PipelineContext>(parentAceContainer->GetPipelineContext());
        CHECK_NULL_VOID(parentcontext);
        auto parentOverlay = parentcontext->GetOverlayManager();
        CHECK_NULL_VOID(parentOverlay);
        parentOverlay->SetSubWindowId(SubwindowManager::GetInstance()->GetDialogSubwindowInstanceId(GetSubwindowId()));
    }
    ShowWindow();
    window_->SetFullScreen(true);
    window_->SetTouchable(true);
    ResizeWindow();
    ContainerScope scope(childContainerId_);
    overlay->OpenCustomDialog(dialogProps, std::move(callback));
    haveDialog_ = true;
}

void SubwindowAndroid::CloseCustomDialogNG(int32_t dialogId)
{
    LOGI("Close customDialog ng subwindow enter.");
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    ContainerScope scope(childContainerId_);
    return overlay->CloseCustomDialog(dialogId);
}

void SubwindowAndroid::ShowToast(const NG::ToastInfo& toastInfo, std::function<void(int32_t)>&& callback)
{
    CHECK_NULL_VOID(window_);
    SubwindowManager::GetInstance()->SetCurrentSubwindow(AceType::Claim(this));
    SetIsToastWindow(toastInfo.showMode == NG::ToastShowMode::TOP_MOST);

    auto aceContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto engine = EngineHelper::GetEngine(aceContainer->GetInstanceId());
    CHECK_NULL_VOID(engine);
    auto delegate = engine->GetFrontend();
    CHECK_NULL_VOID(delegate);

    ContainerScope scope(childContainerId_);
    auto parentContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
    CHECK_NULL_VOID(parentContainer);
    if (parentContainer->IsSceneBoardWindow() || toastInfo.showMode == NG::ToastShowMode::TOP_MOST) {
        ShowWindow(false);
        ResizeWindow();
        window_->SetTouchable(false);
        window_->SetFullScreen(true);
    }
    delegate->ShowToast(toastInfo, std::move(callback));
}

void SubwindowAndroid::ClearToast()
{
    if (!IsToastWindow()) {
        LOGW("Default toast needs not to be clear");
        return;
    }
    window_->SetFullScreen(false);
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    ContainerScope scope(childContainerId_);
    overlayManager->ClearToast();
    context->FlushPipelineImmediately();
    HideWindow();
}

void SubwindowAndroid::SetRect(const NG::RectF& rect)
{
    windowRect_ = rect;
}

NG::RectF SubwindowAndroid::GetRect()
{
    NG::RectF rect;
    CHECK_NULL_RETURN(window_, rect);
    rect.SetRect(
        window_->GetRect().posX_, window_->GetRect().posY_, window_->GetRect().width_, window_->GetRect().height_);
    return rect;
}

Rect SubwindowAndroid::GetParentWindowRect() const
{
    Rect rect;
    CHECK_NULL_RETURN(parentWindow_, rect);
    auto parentWindowRect = parentWindow_->GetRect();
    return Rect(parentWindowRect.posX_, parentWindowRect.posY_, parentWindowRect.width_, parentWindowRect.height_);
}

Rect SubwindowAndroid::GetUIExtensionHostWindowRect() const
{
    Rect rect;
    return rect;
}

Rect SubwindowAndroid::GetFoldExpandAvailableRect() const
{
    Rect rect;
    return rect;
}

bool SubwindowAndroid::CheckHostWindowStatus() const
{
    auto parentContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
    CHECK_NULL_RETURN(parentContainer, false);
    auto parentWindow = parentContainer->GetUIWindow(parentContainerId_);
    CHECK_NULL_RETURN(parentWindow, false);
    return true;
}

void SubwindowAndroid::RequestFocus()
{
    if (window_->IsFocused()) {
        LOGI("subwindow id:%{public}u already focused", window_->GetWindowId());
        // already focused, no need to focus
        return;
    }
    auto ret = window_->RequestFocus();
    if (!ret) {
        return;
    }
    LOGI("subwindow id:%{public}u request focus successfully.", window_->GetWindowId());
}

bool SubwindowAndroid::IsFocused()
{
    return window_->IsFocused();
}

const RefPtr<NG::OverlayManager> SubwindowAndroid::GetOverlayManager()
{
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_RETURN(aceContainer, nullptr);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_RETURN(context, nullptr);
    return context->GetOverlayManager();
}

void SubwindowAndroid::HideFilter()
{
    auto parentAceContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
    CHECK_NULL_VOID(parentAceContainer);
    auto parentPipeline = DynamicCast<NG::PipelineContext>(parentAceContainer->GetPipelineContext());
    CHECK_NULL_VOID(parentPipeline);
    auto manager = parentPipeline->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    ContainerScope scope(parentContainerId_);
    manager->RemoveFilterAnimation();
}

void SubwindowAndroid::HidePixelMap(bool startDrag, double x, double y, bool showAnimation)
{
    auto parentAceContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
    CHECK_NULL_VOID(parentAceContainer);
    auto parentPipeline = DynamicCast<NG::PipelineContext>(parentAceContainer->GetPipelineContext());
    CHECK_NULL_VOID(parentPipeline);
    auto manager = parentPipeline->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    ContainerScope scope(parentContainerId_);
    if (showAnimation) {
        manager->RemovePixelMapAnimation(startDrag, x, y);
    } else {
        manager->RemovePixelMap();
    }
}

void SubwindowAndroid::HideEventColumn()
{
    auto parentAceContainer = Platform::AceContainerSG::GetContainer(parentContainerId_);
    CHECK_NULL_VOID(parentAceContainer);
    auto parentPipeline = DynamicCast<NG::PipelineContext>(parentAceContainer->GetPipelineContext());
    CHECK_NULL_VOID(parentPipeline);
    auto manager = parentPipeline->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    ContainerScope scope(parentContainerId_);
    manager->RemoveEventColumn();
}

bool SubwindowAndroid::SetFollowParentWindowLayoutEnabled(bool enable)
{
    return false;
}

bool SubwindowAndroid::ShowSelectOverlay(const RefPtr<NG::FrameNode>& overlayNode)
{
    return false;
}

void SubwindowAndroid::ResizeWindowForMenu()
{
}

MenuWindowState SubwindowAndroid::GetAttachState()
{
    return attachState_;
}

MenuWindowState SubwindowAndroid::GetDetachState()
{
    return detachState_;
}

void SubwindowAndroid::ShowBindSheetNG(bool isShow, std::function<void(const std::string&)>&& callback,
    std::function<RefPtr<NG::UINode>()>&& buildNodeFunc, std::function<RefPtr<NG::UINode>()>&& buildtitleNodeFunc,
    NG::SheetStyle& sheetStyle, std::function<void()>&& onAppear, std::function<void()>&& onDisappear,
    std::function<void()>&& shouldDismiss, std::function<void(const int32_t)>&& onWillDismiss,
    std::function<void()>&& onWillAppear, std::function<void()>&& onWillDisappear,
    std::function<void(const float)>&& onHeightDidChange,
    std::function<void(const float)>&& onDetentsDidChange,
    std::function<void(const float)>&& onWidthDidChange,
    std::function<void(const float)>&& onTypeDidChange,
    std::function<void()>&& sheetSpringBack, const RefPtr<NG::FrameNode>& targetNode)
{
    auto aceContainer = Platform::AceContainerSG::GetContainer(childContainerId_);
    CHECK_NULL_VOID(aceContainer);
    auto context = DynamicCast<NG::PipelineContext>(aceContainer->GetPipelineContext());
    CHECK_NULL_VOID(context);
    auto overlay = context->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    ResizeWindow();
    ShowWindow();
    CHECK_NULL_VOID(window_);
    window_->SetFullScreen(true);
    window_->SetTouchable(true);
    ContainerScope scope(childContainerId_);
    overlay->OnBindSheet(isShow, std::move(callback), std::move(buildNodeFunc),
        std::move(buildtitleNodeFunc), sheetStyle, std::move(onAppear), std::move(onDisappear),
        std::move(shouldDismiss), std::move(onWillDismiss),
        std::move(onWillAppear), std::move(onWillDisappear), std::move(onHeightDidChange),
        std::move(onDetentsDidChange), std::move(onWidthDidChange), std::move(onTypeDidChange),
        std::move(sheetSpringBack), targetNode);
}

void SubwindowAndroid::SwitchFollowParentWindowLayout(bool freeMultiWindowEnable) {}

bool SubwindowAndroid::NeedFollowParentWindowLayout()
{
    return false;
}

void SubwindowAndroid::AddFollowParentWindowLayoutNode(int32_t nodeId) {}

void SubwindowAndroid::RemoveFollowParentWindowLayoutNode(int32_t nodeId) {}

void SubwindowAndroid::SetNodeId(int32_t nodeId) {}

int32_t SubwindowAndroid::GetNodeId() const
{
    return -1;
}

void SubwindowAndroid::SetWindowAnchorInfo(const NG::OffsetF& offset, SubwindowType type, int32_t nodeId) {}
} // namespace Ace
