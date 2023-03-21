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
#include "ability_context.h"
#include "ability.h"
#include "base/log/log.h"
#include "flutter/shell/platform/android/vsync_waiter_android.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"
#include "shell/common/vsync_waiter.h"

namespace OHOS::Rosen {

Window::Window(const flutter::TaskRunners& taskRunners)
    : vsyncWaiter_(std::make_shared<flutter::VsyncWaiterAndroid>(taskRunners))
{}

Window::Window(std::shared_ptr<AbilityRuntime::Platform::Context> context) : context_(context)
{}

void Window::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    vsyncWaiter_->AsyncWaitForVsync([vsyncCallback](fml::TimePoint frameStart, fml::TimePoint frameTarget) {
        vsyncCallback->onCallback(frameStart.ToEpochDelta().ToNanoseconds());
    });
}

void Window::CreateSurfaceNode(void* nativeWindow)
{
    struct Rosen::RSSurfaceNodeConfig rsSurfaceNodeConfig = { .SurfaceNodeName = "arkui-x_surface",
        .additionalData = nativeWindow };
    surfaceNode_ = Rosen::RSSurfaceNode::Create(rsSurfaceNodeConfig);

    if (!uiContent_) {
        LOGE("Window Notify uiContent_ Surface Created, uiContent_ is nullptr!");
        return;
    }
    LOGI("Window Notify uiContent_ Surface Created");
    uiContent_->NotifySurfaceCreated();
}

void Window::NotifySurfaceChanged(int32_t width, int32_t height)
{
    if (!surfaceNode_) {
        LOGE("Window Notify Surface Changed, surfaceNode_ is nullptr!");
        return;
    }
    LOGI("Window Notify Surface Changed");
    surfaceNode_->SetBoundsWidth(width);
    surfaceNode_->SetBoundsHeight(height);
}

void Window::NotifySurfaceDestroyed()
{
    surfaceNode_ = nullptr;

    if (!uiContent_) {
        LOGE("Window Notify Surface Destroyed, uiContent_ is nullptr!");
        return;
    }
    LOGI("Window Notify uiContent_ Surface Destroyed");
    uiContent_->NotifySurfaceDestroyed();
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

    return 0;
}

void Window::SetWindowView(JNIEnv* env, jobject windowView)
{
    windowView_ = windowView;
    Ace::Platform::WindowViewJni::RegisterWindow(env, this, windowView);
}
}; // namespace OHOS::Rosen
