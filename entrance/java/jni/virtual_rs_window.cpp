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

#include "adapter/android/entrance/java/jni/virtual_rs_window.h"

#include <memory>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "flutter/shell/platform/android/vsync_waiter_android.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"
#include "shell/common/vsync_waiter.h"
#include "transaction/rs_interfaces.h"

namespace OHOS::Rosen {
std::shared_ptr<Window> Window::Create(
    std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, JNIEnv* env, jobject windowView)
{
    auto window = std::make_shared<Window>(context);
    window->SetWindowView(env, windowView);
    return window;
}

Window::Window(const flutter::TaskRunners& taskRunners)
    : vsyncWaiter_(std::make_shared<flutter::VsyncWaiterAndroid>(taskRunners))
{}

Window::Window(std::shared_ptr<AbilityRuntime::Platform::Context> context) : context_(context)
{}

Window::~Window()
{
    ReleaseWindowView();
}

void Window::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    vsyncWaiter_->AsyncWaitForVsync([vsyncCallback](fml::TimePoint frameStart, fml::TimePoint frameTarget) {
        vsyncCallback->onCallback(frameStart.ToEpochDelta().ToNanoseconds());
    });
}

bool Window::CreateVSyncReceiver(std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    if (receiver_) {
        return true;
    }
    auto& rsClient = Rosen::RSInterfaces::GetInstance();
    receiver_ = rsClient.CreateVSyncReceiver("Window_Andorid", handler);
    VsyncError ret = receiver_->Init();
    if (ret) {
        LOGE("Window_Andorid: vsync receiver init failed: %{public}d", ret);
        return false;
    }
    return true;
}

void Window::RequestNextVsync(std::function<void(int64_t, void*)> callback)
{
    if (!receiver_) {
        return;
    }
    VSyncReceiver::FrameCallback fcb = {
        .userData_ = this,
        .callback_ = callback,
    };
    receiver_->RequestNextVSync(fcb);
}

void Window::CreateSurfaceNode(void* nativeWindow)
{
    struct Rosen::RSSurfaceNodeConfig rsSurfaceNodeConfig = { .SurfaceNodeName = "arkui-x_surface",
        .additionalData = nativeWindow };
    surfaceNode_ = Rosen::RSSurfaceNode::Create(rsSurfaceNodeConfig);

    if (!uiContent_) {
        LOGW("Window Notify uiContent_ Surface Created, uiContent_ is nullptr, delay notify.");
        delayNotifySurfaceCreated_ = true;
    } else {
        LOGI("Window Notify uiContent_ Surface Created");
        uiContent_->NotifySurfaceCreated();
    }
}

void Window::NotifySurfaceChanged(int32_t width, int32_t height)
{
    if (!surfaceNode_) {
        LOGE("Window Notify Surface Changed, surfaceNode_ is nullptr!");
        return;
    }
    LOGI("Window Notify Surface Changed wh:[%{public}d, %{public}d]", width, height);
    surfaceWidth_ = width;
    surfaceHeight_ = height;
    surfaceNode_->SetBoundsWidth(surfaceWidth_);
    surfaceNode_->SetBoundsHeight(surfaceHeight_);

    if (!uiContent_) {
        LOGW("Window Notify uiContent_ Surface Created, uiContent_ is nullptr, delay notify.");
        delayNotifySurfaceChanged_ = true;
    } else {
        LOGI("Window Notify uiContent_ Surface Created");
        Ace::ViewportConfig config;
        config.SetSize(surfaceWidth_, surfaceHeight_);
        uiContent_->UpdateViewportConfig(config, WindowSizeChangeReason::RESIZE);
    }
}

void Window::NotifySurfaceDestroyed()
{
    surfaceNode_ = nullptr;

    if (!uiContent_) {
        LOGW("Window Notify Surface Destroyed, uiContent_ is nullptr, delay notify.");
        delayNotifySurfaceDestroyed_ = true;
    } else {
        LOGI("Window Notify uiContent_ Surface Destroyed");
        uiContent_->NotifySurfaceDestroyed();
    }
}

void Window::DelayNotifyUIContentIfNeeded()
{
    if (!uiContent_) {
        LOGE("Window Delay Notify uiContent_ is nullptr!");
        return;
    }

    if (delayNotifySurfaceCreated_) {
        LOGI("Window Delay Notify uiContent_ Surface Created");
        uiContent_->NotifySurfaceCreated();
        delayNotifySurfaceCreated_ = false;
    }

    if (delayNotifySurfaceChanged_) {
        LOGI("Window Delay Notify uiContent_ Surface Changed wh:[%{public}d, %{public}d]",
            surfaceWidth_, surfaceHeight_);
        Ace::ViewportConfig config;
        config.SetSize(surfaceWidth_, surfaceHeight_);
        uiContent_->UpdateViewportConfig(config, WindowSizeChangeReason::RESIZE);
        delayNotifySurfaceChanged_ = false;
    }

    if (delayNotifySurfaceDestroyed_) {
        LOGI("Window Delay Notify uiContent_ Surface Destroyed");
        uiContent_->NotifySurfaceDestroyed();
        delayNotifySurfaceDestroyed_ = false;
    }
}

int Window::SetUIContent(const std::string& contentInfo,
    NativeEngine* engine, NativeValue* storage, bool isdistributed, AbilityRuntime::Platform::Ability* ability)
{
    using namespace OHOS::Ace::Platform;
    (void)ability;
    std::unique_ptr<UIContent> uiContent;
    uiContent = UIContent::Create(context_.get(), engine);
    if (uiContent == nullptr) {
        return -1;
    }
    uiContent->Initialize(this, contentInfo, storage);
    // make uiContent available after Initialize/Restore
    uiContent_ = std::move(uiContent);

    if (state_ == WindowState::STATE_SHOWN) {
        // UIContent may be nullptr when show window, need to notify again when window is shown
        uiContent_->Foreground();
        OHOS::Ace::ViewportConfig config;
        float virtualPixelRatio = 1.0;
        config.SetDensity(virtualPixelRatio);
        uiContent_->UpdateViewportConfig(config, WindowSizeChangeReason::UNDEFINED);
    }

    DelayNotifyUIContentIfNeeded();
    return 0;
}

void Window::SetWindowView(JNIEnv* env, jobject windowView)
{
    if (windowView == nullptr) {
        LOGE("Window::SetWindowView: jobject of WindowView is nullptr!");
        return;
    }
    if (windowView_ != nullptr) {
        LOGW("Window::SetWindowView: windowView_ has already been set!");
        return;
    }
    windowView_ = env->NewGlobalRef(windowView);
    Ace::Platform::WindowViewJni::RegisterWindow(env, this, windowView);
}

void Window::ReleaseWindowView()
{
    if (windowView_ == nullptr) {
        return;
    }
    auto jniEnv = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    Ace::Platform::WindowViewJni::UnRegisterWindow(jniEnv.get(), windowView_);
    Ace::Platform::JniEnvironment::DeleteJavaGlobalRef(windowView_);
    windowView_ = nullptr;
}
} // namespace OHOS::Rosen