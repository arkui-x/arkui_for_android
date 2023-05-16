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

#include "virtual_rs_window.h"
#include "subwindow_manager_jni.h"

#include <memory>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "flutter/shell/platform/android/vsync_waiter_android.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"
#include "shell/common/vsync_waiter.h"
#include "transaction/rs_interfaces.h"
#include "base/utils/utils.h"

using namespace OHOS::Ace::Platform;

namespace OHOS::Rosen {

std::map<uint32_t, std::vector<std::shared_ptr<Window>>> Window::subWindowMap_;
std::map<std::string, std::pair<uint32_t, std::shared_ptr<Window>>> Window::windowMap_;
std::shared_ptr<Window> Window::mainWindow_ = nullptr;

std::shared_ptr<Window> Window::Create(
    std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, JNIEnv* env, jobject windowView)
{
    auto window = std::make_shared<Window>(context);
    window->SetWindowView(env, windowView);
    mainWindow_ = window;
    return window;
}

std::shared_ptr<Window> Window::CreateSubWindow(
        std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context,
            std::shared_ptr<OHOS::Rosen::WindowOption> option)
{
    LOGI("Window::CreateSubWindow called. windowName=%s", option->GetWindowName().c_str());
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    uint32_t parentId = option->GetParentId();
    auto window = std::make_shared<Window>(context);
    window->SetWindowOption(option);

    SubWindowManagerJni::CreateSubWindow(option);

    jobject view = SubWindowManagerJni::GetContentView(option->GetWindowName());

    uint32_t windowId = SubWindowManagerJni::GetWindowId(option->GetWindowName());

    window->SetWindowId(windowId);
    windowMap_.insert(std::make_pair(window->GetWindowName(), std::pair<uint32_t,
                                        std::shared_ptr<Window>>((uint32_t)windowId, window)));
    if (parentId != INVALID_WINDOW_ID) {
        subWindowMap_[parentId].push_back(window);
    }

    window->SetSubWindowView(env, view);
    window->CreateSurfaceNode(view);
    LOGI("Window::CreateSubWindow: success");

    return window;
}

Window::Window(std::shared_ptr<AbilityRuntime::Platform::Context> context, uint32_t windowId)
    : context_(context), windowId_(windowId)
{}

Window::Window(const flutter::TaskRunners& taskRunners)
    : vsyncWaiter_(std::make_shared<flutter::VsyncWaiterAndroid>(taskRunners))
{}

Window::Window(std::shared_ptr<AbilityRuntime::Platform::Context> context) : context_(context)
{}

Window::~Window()
{
    ReleaseWindowView();
}

std::vector<std::shared_ptr<Window>> Window::GetSubWindow(uint32_t parentId)
{
    LOGI("Window::GetSubWindow called. parentId=%d", parentId);
    if (subWindowMap_.find(parentId) == subWindowMap_.end()) {
        return std::vector<std::shared_ptr<Window>>();
    }
    return std::vector<std::shared_ptr<Window>>(subWindowMap_[parentId].begin(), subWindowMap_[parentId].end());
}

std::shared_ptr<Window> Window::FindWindow(const std::string& name)
{
    LOGI("Window::GetSubWindow called. name=%s", name.c_str());
    auto iter = windowMap_.find(name);
    if (iter == windowMap_.end()) {
        return nullptr;
    }
    return iter->second.second;
}

std::shared_ptr<Window> Window::GetTopWindow(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Context>& context)
{
    LOGI("Window::GetTopWindow");

    if (mainWindow_ != nullptr) {
        return mainWindow_;
    }

    return nullptr;
}

WMError Window::ShowWindow()
{
    LOGI("Window::ShowWindow called.");

    bool result = SubWindowManagerJni::ShowWindow(this->GetWindowName());
    if (result) {
        return WMError::WM_OK;
    } else {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

WMError Window::DestroyWindow()
{
    LOGI("Window::DestroyWindow called.");
    isWindowShow_ = false;

    bool result = SubWindowManagerJni::DestroyWindow(this->GetWindowName());

    if (result) {
        LOGI("Window::DestroyWindow: success");

        if (subWindowMap_.count(GetParentId()) > 0) { // remove from subWindowMap_
            auto& subWindows = subWindowMap_.at(GetParentId());
            for (auto iter = subWindows.begin(); iter < subWindows.end(); ++iter) {
                if ((*iter) == nullptr) {
                    continue;
                }
                if ((*iter)->GetWindowId() == GetWindowId()) {
                    subWindows.erase(iter);
                    break;
                }
            }
        }

        if (subWindowMap_.count(GetWindowId()) > 0) { // remove from subWindowMap_ and windowMap_
            auto& subWindows = subWindowMap_.at(GetWindowId());
            for (auto iter = subWindows.begin(); iter != subWindows.end(); iter = subWindows.begin()) {
                if ((*iter) == nullptr) {
                    subWindows.erase(iter);
                    continue;
                }
                (*iter)->Destroy();
            }
            subWindowMap_[GetWindowId()].clear();
            subWindowMap_.erase(GetWindowId());
        }
        return WMError::WM_OK;
    } else {
        LOGI("Window::DestroyWindow: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}
WMError Window::MoveWindowTo(int32_t x, int32_t y)
{
    LOGI("Window::MoveWindowTo called. x=%d, y=%d", x, y);

    bool result = SubWindowManagerJni::MoveWindowTo(this->GetWindowName(), x, y);

    if (result) {
        return WMError::WM_OK;
    } else {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}
WMError Window::ResizeWindowTo(int32_t width, int32_t height)
{
    LOGI("Window::ResizeWindowTo called. width=%d, height=%d", width, height);

    bool result = SubWindowManagerJni::ResizeWindowTo(this->GetWindowName(), width, height);

    if (result) {
        LOGI("Window::ResizeWindowTo: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::ResizeWindowTo: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

WMError Window::SetBackgroundColor(uint32_t color)
{
    LOGI("Window::SetBackgroundColor called. color=%d", color);
    backgroundColor_ = color;
    if (uiContent_) {
        uiContent_->SetBackgroundColor(color);
    }
    return WMError::WM_OK;
}

WMError Window::SetBrightness(float brightness)
{
    LOGI("Window::SetBrightness called. brightness=%.3f", brightness);
    return WMError::WM_OK;
}
WMError Window::SetKeepScreenOn(bool keepScreenOn)
{
    LOGI("Window::SetKeepScreenOn called. keepScreenOn=%d", keepScreenOn);
    return WMError::WM_OK;
}

bool Window::IsKeepScreenOn()
{
    LOGI("Window::IsKeepScreenOn called.");
    return true;
}

WMError Window::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    LOGI("Window::SetSystemBarProperty called.");
    return WMError::WM_OK;
}

void Window::SetRequestedOrientation(Orientation orientation)
{
    LOGI("Window::SetRequestedOrientation called.");
}

SystemBarProperty Window::GetSystemBarPropertyByType(WindowType type) const
{
    LOGI("Window::GetSystemBarPropertyByType called.");
    return property_;
}

WMError Window::RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    LOGI("Window::RegisterLifeCycleListener called.");
    return WMError::WM_OK;
}
WMError Window::UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    LOGI("Window::UnregisterLifeCycleListener called.");
    return WMError::WM_OK;
}

void Window::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    // stage model
    if (receiver_) {
        auto callback = [vsyncCallback](int64_t timestamp, void*) {
            vsyncCallback->onCallback(timestamp);
        };
        VSyncReceiver::FrameCallback fcb = {
            .userData_ = this,
            .callback_ = callback,
        };
        receiver_->RequestNextVSync(fcb);
        return;
    }

    // fa model
    if (vsyncWaiter_) {
        vsyncWaiter_->AsyncWaitForVsync([vsyncCallback](fml::TimePoint frameStart, fml::TimePoint frameTarget) {
            vsyncCallback->onCallback(frameStart.ToEpochDelta().ToNanoseconds());
        });
    }
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

void Window::NotifySurfaceChanged(int32_t width, int32_t height, float density)
{
    if (!surfaceNode_) {
        LOGE("Window Notify Surface Changed, surfaceNode_ is nullptr!");
        return;
    }
    LOGI("Window Notify Surface Changed wh:[%{public}d, %{public}d]", width, height);
    surfaceWidth_ = width;
    surfaceHeight_ = height;
    density_ = density;
    surfaceNode_->SetBoundsWidth(surfaceWidth_);
    surfaceNode_->SetBoundsHeight(surfaceHeight_);

    if (!uiContent_) {
        LOGW("Window Notify uiContent_ Surface Changed, uiContent_ is nullptr, delay notify.");
        delayNotifySurfaceChanged_ = true;
    } else {
        Ace::ViewportConfig config;
        config.SetDensity(density_);
        config.SetSize(surfaceWidth_, surfaceHeight_);
        LOGI("Window Notify uiContent_ Surface Changed %{public}s", config.ToString().c_str());
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

void Window::WindowFocusChanged(bool hasWindowFocus)
{
    if (!uiContent_) {
        LOGW("Window::Focus uiContent_ is nullptr");
        return;
    }
    if (hasWindowFocus) {
        LOGI("Window: notify uiContent Focus");
        uiContent_->Focus();
        isWindowShow_ = true;
    } else {
        LOGI("Window: notify uiContent UnFocus");
        uiContent_->UnFocus();
        isWindowShow_ = false;
    }
}

void Window::Foreground()
{
    if (!uiContent_) {
        LOGW("Window::Foreground uiContent_ is nullptr");
        return;
    }
    LOGI("Window: notify uiContent Foreground");
    uiContent_->Foreground();
    isWindowShow_ = true;
}

void Window::Background()
{
    if (!uiContent_) {
        LOGW("Window::Background uiContent_ is nullptr");
        return;
    }
    LOGI("Window: notify uiContent Background");
    isWindowShow_ = false;
    uiContent_->Background();
}

void Window::Destroy()
{
    if (!uiContent_) {
        LOGW("Window::Destroy uiContent_ is nullptr");
        return;
    }
    LOGI("Window: notify uiContent Destroy");
    uiContent_->Destroy();
}

bool Window::ProcessBackPressed()
{
    if (!uiContent_) {
        LOGW("Window::ProcessBackPressed uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessBackPressed();
}

bool Window::ProcessPointerEvent(const std::vector<uint8_t>& data)
{
    if (!uiContent_) {
        LOGW("Window::ProcessPointerEvent uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessPointerEvent(data);
}

bool Window::ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp,
    int64_t timeStampStart)
{
    if (!uiContent_) {
        LOGW("Window::ProcessKeyEvent uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessKeyEvent(keyCode, keyAction, repeatTime, timeStamp, timeStampStart);
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
        config.SetDensity(density_);
        config.SetSize(surfaceWidth_, surfaceHeight_);
        config.SetOrientation(surfaceHeight_ >= surfaceWidth_ ? 0 : 1);
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

    uiContent_->Foreground();
    isWindowShow_ = true;

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

void Window::SetSubWindowView(JNIEnv* env, jobject windowView)
{
    if (windowView == nullptr) {
        LOGE("Window::SetWindowView: jobject of WindowView is nullptr!");
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

void Window::UpdateConfiguration(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config)
{
    if (uiContent_ != nullptr) {
        LOGI("Window::UpdateConfiguration called.");
        uiContent_->UpdateConfiguration(config);
    }
}
} // namespace OHOS::Rosen
