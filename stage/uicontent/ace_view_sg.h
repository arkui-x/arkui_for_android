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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ACE_VIEW_SG_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ACE_VIEW_SG_H

#include <memory>

#include "interfaces/inner_api/ace/viewport_config.h"
#include "wm/window.h"

#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/common/ace_view.h"
#include "core/common/platform_res_register.h"
#include "core/event/key_event_recognizer.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "core/common/flutter/flutter_thread_model.h"
#include "render_service_client/core/ui/rs_surface_node.h"
#include "render_service_client/core/ui/rs_ui_director.h"

#include "adapter/android/entrance/java/jni/virtual_rs_window.h"
#endif

namespace OHOS::Ace::Platform {
class ACE_FORCE_EXPORT AceViewSG : public AceView, public Referenced {
public:
    explicit AceViewSG(int32_t id) : instanceId_(id)
    {
#ifdef ENABLE_ROSEN_BACKEND
        threadModel_ = FlutterThreadModel::CreateThreadModel(false, true, false);
#endif  
    }
    ~AceViewSG() override = default;

    static AceViewSG* CreateView(int32_t instanceId);
    static void SurfaceCreated(AceViewSG* view, OHOS::Rosen::Window* window);
    static void SurfaceChanged(AceViewSG* view, int32_t width, int32_t height, int32_t orientation,
        WindowSizeChangeReason type = WindowSizeChangeReason::UNDEFINED);
    static void SurfacePositionChanged(AceViewSG* view, int32_t posX, int32_t posY);
    static void SetViewportMetrics(AceViewSG* view, const ViewportConfig& config);

    struct KeyEventInfo {
        int32_t keyCode {};
        int32_t keyAction {};
        int32_t repeatTime {};
        int64_t timeStamp {};
        int64_t timeStampStart {};
        int32_t metaKey {};
        int32_t sourceDevice {};
        int32_t deviceId {};
    };

    int32_t GetInstanceId() const override
    {
        return instanceId_;
    }

    void RegisterTouchEventCallback(TouchEventCallback&& callback) override;
    void RegisterKeyEventCallback(KeyEventCallback&& callback) override;
    void RegisterMouseEventCallback(MouseEventCallback&& callback) override;
    void RegisterRotationEventCallback(RotationEventCallBack&& callback) override;
    void RegisterViewChangeCallback(ViewChangeCallback&& callback) override;
    void RegisterDensityChangeCallback(DensityChangeCallback&& callback) override;
    void RegisterSurfaceDestroyCallback(SurfaceDestroyCallback&& callback) override;
    void RegisterDragEventCallback(DragEventCallBack&& callback) override;
    void RegisterAxisEventCallback(AxisEventCallback&& callback) override;
    void RegisterCardViewPositionCallback(CardViewPositionCallBack&& callback) override {}
    void RegisterCardViewAccessibilityParamsCallback(CardViewAccessibilityParamsCallback&& callback) override {}
    void RegisterViewPositionChangeCallback(ViewPositionChangeCallback&& callback) override;
    void RegisterSystemBarHeightChangeCallback(SystemBarHeightChangeCallback&& callback) override;
    void RegisterIdleCallback(IdleCallback&& callback) override {}

    bool Dump(const std::vector<std::string>& params) override;
    const void* GetNativeWindowById(uint64_t textureId) override;
    std::unique_ptr<DrawDelegate> GetDrawDelegate() override;
    std::unique_ptr<PlatformWindow> GetPlatformWindow() override;

    void Launch() override;

#ifdef ENABLE_ROSEN_BACKEND
    FlutterThreadModel* GetThreadModel()
    {
        return threadModel_.get();
    }
#endif

    bool DispatchTouchEvent(const std::vector<uint8_t>& data);
    bool DispatchKeyEvent(const KeyEventInfo& eventInfo);

    void NotifySurfaceDestroyed() const;
    void NotifySurfaceChanged(int32_t width, int32_t height, WindowSizeChangeReason type);
    void NotifyDensityChanged(double density);

    void SetPlatformResRegister(const RefPtr<PlatformResRegister>& resRegister)
    {
        resRegister_ = resRegister;
    }

    const RefPtr<PlatformResRegister>& GetPlatformResRegister() const override
    {
        return resRegister_;
    }

    ViewType GetViewType() const override
    {
        return AceView::ViewType::SURFACE_VIEW;
    }

#ifdef ENABLE_ROSEN_BACKEND
    // void SetSurfaceNode(std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode)
    // {
    //     surfaceNode_ = surfaceNode;
    //     if (uiDirector_) {
    //         uiDirector_->SetRSSurfaceNode(surfaceNode_);
    //     }
    // }
    // void SetUIDirector(std::shared_ptr<Rosen::RSUIDirector> uiDirector)
    // {
    //     uiDirector_ = uiDirector;
    //     if (uiDirector_ && surfaceNode_) {
    //         uiDirector_->SetRSSurfaceNode(surfaceNode_);
    //     }
    // }
    void SetRSWinodw(sptr<Rosen::Window> window)
    {
        rsWinodw_ = std::move(window);
    }
    sptr<Rosen::Window> GetRsWindow()
    {
        return rsWinodw_;
    }
#endif

private:
    bool IsLastPage() const;
    void NotifySurfacePositionChanged(int32_t posX, int32_t posY);

    int32_t instanceId_ = -1;

    TouchEventCallback touchEventCallback_;
    MouseEventCallback mouseEventCallback_;
    AxisEventCallback axisEventCallback_;
    RotationEventCallBack rotationEventCallback_;
    ViewChangeCallback viewChangeCallback_;
    ViewPositionChangeCallback viewPositionChangeCallback_;
    DensityChangeCallback densityChangeCallback_;
    SystemBarHeightChangeCallback systemBarHeightChangeCallback_;
    SurfaceDestroyCallback surfaceDestroyCallback_;
    DragEventCallBack dragEventCallback_;
    KeyEventCallback keyEventCallback_;
    KeyEventRecognizer keyEventRecognizer_;

    RefPtr<PlatformResRegister> resRegister_;

#ifdef ENABLE_ROSEN_BACKEND
    // std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode_;
    // std::shared_ptr<Rosen::RSUIDirector> uiDirector_;
    std::unique_ptr<FlutterThreadModel> threadModel_;
    sptr<Rosen::Window> rsWinodw_;
#endif

    ACE_DISALLOW_COPY_AND_MOVE(AceViewSG);
};
} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ACE_VIEW_SG_H
