/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_FLUTTER_ACE_VIEW_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_FLUTTER_ACE_VIEW_H

#include <memory>

#ifdef NG_BUILD
#include "ace_shell/shell/platform/android/android_shell_holder.h"
#include "flutter/lib/ui/window/pointer_data_packet.h"
#else
#include "flutter/shell/platform/android/android_shell_holder.h"
#endif
#include "jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/utils/noncopyable.h"
#include "core/common/ace_view.h"
#include "core/common/flutter/flutter_thread_model.h"
#include "core/common/platform_res_register.h"
#include "core/event/key_event_recognizer.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "render_service_client/core/ui/rs_surface_node.h"
#include "render_service_client/core/ui/rs_ui_director.h"

#include "adapter/android/entrance/java/jni/virtual_rs_window.h"
#endif

namespace OHOS::Ace::Platform {

using ReleaseCallback = std::function<void()>;
#ifndef ENABLE_ROSEN_BACKEND
#ifdef NG_BUILD
using flutter::ace::AndroidShellHolder;
#else
using flutter::AndroidShellHolder;
#endif
#endif

class FlutterAceView : public AceView, public Referenced {
public:
    FlutterAceView() : object_(nullptr, nullptr) {};
    explicit FlutterAceView(int32_t id) : instanceId_(id), object_(nullptr, nullptr)
    {
#ifdef ENABLE_ROSEN_BACKEND
        threadModel_ = FlutterThreadModel::CreateThreadModel(false, true, false);
#endif
    }
    ~FlutterAceView() override = default;

    void RegisterTouchEventCallback(TouchEventCallback&& callback) override;
    void RegisterKeyEventCallback(KeyEventCallback&& callback) override;
    void RegisterMouseEventCallback(MouseEventCallback&& callback) override;
    void RegisterRotationEventCallback(RotationEventCallBack&& callback) override;
    void RegisterCardViewPositionCallback(CardViewPositionCallBack&& callback) override {}
    void RegisterAxisEventCallback(AxisEventCallback&& callback) override {}

    void Launch() override;
#ifdef ENABLE_ROSEN_BACKEND
    FlutterThreadModel* GetThreadModel()
    {
        return threadModel_.get();
    }
#else
    void SetShellHolder(std::unique_ptr<AndroidShellHolder> holder);
    AndroidShellHolder* GetShellHolder() const
    {
        return shellHolder_.get();
    }
#endif

    bool ProcessTouchEvent(std::unique_ptr<flutter::PointerDataPacket> packet);
    void ProcessMouseEvent(std::unique_ptr<flutter::PointerDataPacket> packet);
    bool ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp = 0,
        int64_t timeStampStart = 0, int32_t metaKey = 0, int32_t sourceDevice = 0, int32_t deviceId = 0);
    void ProcessIdleEvent(int64_t deadline);
    bool ProcessRotationEvent(float rotationValue);

    int32_t GetInstanceId() const override
    {
        return instanceId_;
    }

    void RegisterCardViewAccessibilityParamsCallback(CardViewAccessibilityParamsCallback&& callback) override {}

    void RegisterViewChangeCallback(ViewChangeCallback&& callback) override
    {
        if (callback) {
            viewChangeCallback_ = std::move(callback);
        }
    }

    void RegisterDensityChangeCallback(DensityChangeCallback&& callback) override
    {
        if (callback) {
            densityChangeCallback_ = std::move(callback);
        }
    }

    void RegisterSystemBarHeightChangeCallback(SystemBarHeightChangeCallback&& callback) override
    {
        if (callback) {
            systemBarHeightChangeCallback_ = std::move(callback);
        }
    }

    void RegisterSurfaceDestroyCallback(SurfaceDestroyCallback&& callback) override
    {
        if (callback) {
            surfaceDestroyCallback_ = std::move(callback);
        }
    }

    void RegisterIdleCallback(IdleCallback&& callback) override
    {
        if (callback) {
            idleCallback_ = std::move(callback);
        }
    }

    void RegisterDragEventCallback(DragEventCallBack&& callback) override {}

    void RegisterViewPositionChangeCallback(ViewPositionChangeCallback&& callback) override {}

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

    std::unique_ptr<DrawDelegate> GetDrawDelegate() override;
    std::unique_ptr<PlatformWindow> GetPlatformWindow() override;
    const void* GetNativeWindowById(uint64_t textureId) override;

    bool Dump(const std::vector<std::string>& params) override;
    void SetViewCallback(JNIEnv* env, jobject jObject);

    void NotifySurfaceChanged(int32_t width, int32_t height)
    {
        if (viewChangeCallback_) {
            viewChangeCallback_(width, height, WindowSizeChangeReason::RESIZE, nullptr);
        }
        width_ = width;
        height_ = height;
    }

    void NotifyDensityChanged(double density)
    {
        if (densityChangeCallback_) {
            densityChangeCallback_(density);
        }
    }

    void NotifySystemBarHeightChanged(double statusBar, double navigationBar) const
    {
        if (systemBarHeightChangeCallback_) {
            systemBarHeightChangeCallback_(statusBar, navigationBar);
        }
    }

    void NotifySurfaceDestroyed() const
    {
        if (surfaceDestroyCallback_) {
            surfaceDestroyCallback_();
        }
    }

    void RegisterSurface(int64_t textureId, void* nativewindow)
    {
        nativeWindowMap_.emplace(textureId, nativewindow);
    }

    void UnregisterSurface(int64_t textureId)
    {
        nativeWindowMap_.erase(textureId);
    }

#ifdef ENABLE_ROSEN_BACKEND
    void SetSurfaceNode(std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode)
    {
        surfaceNode_ = surfaceNode;
        if (uiDirector_) {
            uiDirector_->SetRSSurfaceNode(surfaceNode_);
        }
    }
    void SetUIDirector(std::shared_ptr<Rosen::RSUIDirector> uiDirector)
    {
        uiDirector_ = uiDirector;
        if (uiDirector_ && surfaceNode_) {
            uiDirector_->SetRSSurfaceNode(surfaceNode_);
        }
    }
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
    static bool RegisterCommonNatives(JNIEnv* env, const jclass myClass);

#ifndef ENABLE_ROSEN_BACKEND
    std::unique_ptr<AndroidShellHolder> shellHolder_;
#endif

    TouchEventCallback touchEventCallback_;
    MouseEventCallback mouseEventCallback_;
    RotationEventCallBack rotationEventCallback_;
    ViewChangeCallback viewChangeCallback_;
    DensityChangeCallback densityChangeCallback_;
    SystemBarHeightChangeCallback systemBarHeightChangeCallback_;
    SurfaceDestroyCallback surfaceDestroyCallback_;
    IdleCallback idleCallback_;
    KeyEventCallback keyEventCallback_;
    KeyEventRecognizer keyEventRecognizer_;

    int32_t instanceId_ = 0;
    bool viewLaunched_ = false;
    RefPtr<PlatformResRegister> resRegister_;

    enum class EventState { INITIAL_STATE, HORRIZATIONAL_STATE, VERTICAL_STATE };

    struct TouchPointInfo {
        Offset offset_;
        EventState eventState_ = EventState::INITIAL_STATE;

        TouchPointInfo() = default;
        explicit TouchPointInfo(const Offset& offset) : offset_(offset) {}
        ~TouchPointInfo() = default;
    };

    std::unordered_map<int32_t, TouchPointInfo> touchPointInfoMap_;
    std::unordered_map<int64_t, void*> nativeWindowMap_;
    JniEnvironment::JavaGlobalRef object_;

#ifdef ENABLE_ROSEN_BACKEND
    std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode_;
    std::shared_ptr<Rosen::RSUIDirector> uiDirector_;
    std::unique_ptr<FlutterThreadModel> threadModel_;
    sptr<Rosen::Window> rsWinodw_;
#endif

    ACE_DISALLOW_COPY_AND_MOVE(FlutterAceView);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_FLUTTER_ACE_VIEW_H