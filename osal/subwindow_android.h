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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_SUBWINDOW_ANDROID_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_SUBWINDOW_ANDROID_H

#include <unordered_map>

#include "adapter/android/entrance/java/jni/virtual_rs_window.h"
#include "adapter/android/stage/uicontent/ace_container_sg.h"
#include "base/subwindow/subwindow.h"
#include "base/subwindow/subwindow_manager.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Rosen {
class Window;
class WindowOption;
} // namespace OHOS::Rosen

namespace OHOS::Ace {
class SubwindowAndroid : public Subwindow {
    DECLARE_ACE_TYPE(SubwindowAndroid, Subwindow)

public:
    explicit SubwindowAndroid(int32_t instanceId);
    ~SubwindowAndroid() = default;

    void InitContainer() override;

    void ResizeWindow() override;

    NG::RectF GetRect() override;

    void ShowMenu(const RefPtr<Component>& newComponent) override {}

    void ShowMenuNG(const RefPtr<NG::FrameNode> menuNode, int32_t targetId, const NG::OffsetF& offset);

    void ShowMenuNG(const RefPtr<NG::FrameNode> menuNode, const NG::MenuParam& menuParam,
        const RefPtr<NG::FrameNode>& targetNode, const NG::OffsetF& offset) override;

    void ShowMenuNG(std::function<void()>&& buildFunc, std::function<void()>&& previewBuildFunc,
        const NG::MenuParam& menuParam, const RefPtr<NG::FrameNode>& targetNode, const NG::OffsetF& offset) override {}

    bool ShowPreviewNG() override
    {
        return false;
    }

    void HidePreviewNG() override {}

    void HideMenuNG(const RefPtr<NG::FrameNode>& menu, int32_t targetId) override;

    void HideMenuNG(bool showPreviewAnimation, bool startDrag) override;

    void UpdateHideMenuOffsetNG(const NG::OffsetF& offset) override;

    void ShowPopup(const RefPtr<Component>& newComponent, bool disableTouchEvent = true) override {}
    
    void ShowPopupNG(int32_t targetId, const NG::PopupInfo& popupInfo,
        const std::function<void(int32_t)>&& onWillDismiss = nullptr, bool interactiveDismiss = true) override;

    void HidePopupNG(int32_t targetId) override;

    void GetPopupInfoNG(int32_t targetId, NG::PopupInfo& popupInfo) override;

    bool CancelPopup(const std::string& id) override
    {
        return false;
    }

    void CloseMenu() override {}

    void ClearMenu() override {}

    void ClearMenuNG(int32_t targetId = -1, bool inWindow = true, bool showAnimation = false) override;

    // void ShowDialogNGPrepare() override {}

    RefPtr<NG::FrameNode> ShowDialogNGWithNode(
        const DialogProperties& dialogProps, const RefPtr<NG::UINode>& customNode) override {}

    void ClearPopupNG() override;

    RefPtr<NG::FrameNode> ShowDialogNG(const DialogProperties& dialogProps, std::function<void()>&& buildFunc) override;

    void CloseDialogNG(const RefPtr<NG::FrameNode>& dialogNode) override;

    void HideSubWindowNG() override;

    bool GetShown() override
    {
        return isShowed_;
    }

    void MarkDirtyDialogSafeArea() override {}

    void SetHotAreas(const std::vector<Rect>& rects, int32_t overlayId) override;

    void DeleteHotAreas(int32_t overlayId) override;

    void ClearToast() override;

    void ShowToast(const std::string& message, int32_t duration, const std::string& bottom,
        const NG::ToastShowMode& showMode, int32_t alignment, std::optional<DimensionOffset> offset) override;

    void ShowDialog(const std::string& title, const std::string& message, const std::vector<ButtonInfo>& buttons,
        bool autoCancel, std::function<void(int32_t, int32_t)>&& callback,
        const std::set<std::string>& callbacks) override {}

    void ShowDialog(const PromptDialogAttr& dialogAttr, const std::vector<ButtonInfo>& buttons,
        std::function<void(int32_t, int32_t)>&& callback, const std::set<std::string>& callbacks) override {}

    void ShowActionMenu(const std::string& title, const std::vector<ButtonInfo>& button,
        std::function<void(int32_t, int32_t)>&& callback) override {}

    void CloseDialog(int32_t instanceId) override {}

    void OpenCustomDialog(const PromptDialogAttr& dialogAttr, std::function<void(int32_t)>&& callback) override {}

    void CloseCustomDialog(const int32_t dialogId) override {}

    void CloseCustomDialog(const WeakPtr<NG::UINode>& node, std::function<void(int32_t)> &&callback) override {}

    void OpenCustomDialogNG(const DialogProperties& dialogProps, std::function<void(int32_t)>&& callback) override;

    void CloseCustomDialogNG(int32_t dialogId) override;

    void CloseCustomDialogNG(const WeakPtr<NG::UINode>& node, std::function<void(int32_t)>&& callback) override {}

    void UpdateCustomDialogNG(const WeakPtr<NG::UINode>& node, const DialogProperties& dialogProps,
        std::function<void(int32_t)>&& callback) override {}

    const RefPtr<NG::OverlayManager> GetOverlayManager() override;

    int32_t GetChildContainerId() const override
    {
        return childContainerId_;
    }

    // Gets parent window's size and offset
    Rect GetParentWindowRect() const override;

    Rect GetUIExtensionHostWindowRect() const override;

    bool CheckHostWindowStatus() const override;

    bool IsFocused() override;

    void RequestFocus() override;

    void ResizeWindowForFoldStatus() override {}

    void ResizeWindowForFoldStatus(int32_t parentContainerId) override {}

    bool IsToastWindow() const
    {
        return isToastWindow_;
    }

    void SetIsToastWindow(bool isToastWindow)
    {
        isToastWindow_ = isToastWindow;
    }

private:
    void InitSubwindow(const RefPtr<Platform::AceContainerSG>& parentContainer);
    bool InitSubContainer(const RefPtr<Platform::AceContainerSG>& parentContainer) const;
    void ShowWindow(bool needFocus = true);
    void HideWindow();
    void clearStatus();

    // Convert Rect to Rosen::Rect
    void RectConverter(const Rect& rect, Rosen::Rect& rosenRect);

    void HideFilter();
    void HidePixelMap(bool startDrag = false, double x = 0, double y = 0, bool showAnimation = true);
    void HideEventColumn();

    static int32_t id_;
    int32_t windowId_ = 0;
    int32_t parentContainerId_ = -1;
    int32_t childContainerId_ = -1;
    int32_t popupTargetId_ = -1;
    bool isShowed_ = false;
    bool haveDialog_ = false;
    bool isToastWindow_ = false;
    bool isMenuWindow_ = false;
    std::shared_ptr<OHOS::Rosen::Window> window_ = nullptr;
    sptr<OHOS::Rosen::Window> parentWindow_ = nullptr;
    std::unordered_map<int32_t, std::vector<Rosen::Rect>> hotAreasMap_;
    std::unordered_map<int32_t, std::vector<Rosen::Rect>> popupHotAreasMap_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_SUBWINDOW_ANDROID_H
