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

#include <cstdint>
#include <memory>

#include "ability_context.h"
#include "flutter/shell/platform/android/vsync_waiter_android.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"
#include "hilog.h"
#include "render_service_client/core/pipeline/rs_render_thread.h"
#include "shell/common/vsync_waiter.h"
#include "subwindow_manager_jni.h"
#include "transaction/rs_interfaces.h"
#include "window_view_adapter.h"

#include "adapter/android/entrance/java/jni/ace_env_jni.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "core/event/touch_event.h"

using namespace OHOS::Ace::Platform;

namespace OHOS::Rosen {
void DummyWindowRelease(Window* window)
{
    window->DecStrongRef(window);
    LOGI("Rosenwindow rsWindow_Window: dummy release");
}
std::map<uint32_t, std::vector<std::shared_ptr<Window>>> Window::subWindowMap_;
std::map<std::string, std::pair<uint32_t, std::shared_ptr<Window>>> Window::windowMap_;
std::map<uint32_t, std::vector<sptr<IWindowLifeCycle>>> Window::lifecycleListeners_;
std::recursive_mutex Window::globalMutex_;

Window::Window(std::shared_ptr<AbilityRuntime::Platform::Context> context, uint32_t windowId)
    : context_(context), windowId_(windowId), brightness_(SubWindowManagerJni::GetAppScreenBrightness())
{}

Window::Window(const flutter::TaskRunners& taskRunners)
    : vsyncWaiter_(std::make_shared<flutter::VsyncWaiterAndroid>(taskRunners)),
      brightness_(SubWindowManagerJni::GetAppScreenBrightness())
{}

Window::Window(std::shared_ptr<AbilityRuntime::Platform::Context> context)
    : context_(context), brightness_(SubWindowManagerJni::GetAppScreenBrightness())
{}

Window::~Window()
{
    LOGI("Rosenwindow rsWindow_Window: release id = %u", windowId_);
    ReleaseWindowView();
}

void Window::AddToWindowMap(std::shared_ptr<Window> window)
{
    DeleteFromWindowMap(window);
    windowMap_.insert(std::make_pair(
        window->GetWindowName(), std::pair<uint32_t, std::shared_ptr<Window>>(window->GetWindowId(), window)));
}

void Window::DeleteFromWindowMap(std::shared_ptr<Window> window)
{
    auto iter = windowMap_.find(window->GetWindowName());
    if (iter != windowMap_.end()) {
        windowMap_.erase(iter);
    }
}

void Window::DeleteFromWindowMap(Window* window)
{
    if (window == nullptr) {
        return;
    }
    auto iter = windowMap_.find(window->GetWindowName());
    if (iter != windowMap_.end()) {
        windowMap_.erase(iter);
    }
}

void Window::AddToSubWindowMap(std::shared_ptr<Window> window)
{
    if (window == nullptr) {
        LOGE("window is null");
        return;
    }
    if (window->GetType() != OHOS::Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW ||
        window->GetParentId() == INVALID_WINDOW_ID) {
        LOGE("window is not subwindow");
        return;
    }
    DeleteFromSubWindowMap(window);
    uint32_t parentId = window->GetParentId();
    subWindowMap_[parentId].push_back(window);
}

void Window::DeleteFromSubWindowMap(std::shared_ptr<Window> window)
{
    if (window == nullptr) {
        return;
    }
    uint32_t parentId = window->GetParentId();
    if (parentId == INVALID_WINDOW_ID) {
        return;
    }
    auto iter1 = subWindowMap_.find(parentId);
    if (iter1 == subWindowMap_.end()) {
        return;
    }
    auto subWindows = iter1->second;
    auto iter2 = subWindows.begin();
    while (iter2 != subWindows.end()) {
        if (*iter2 == window) {
            subWindows.erase(iter2);
            ((*iter2)->Destroy());
            break;
        }
    }
}

std::shared_ptr<Window> Window::Create(
    std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, JNIEnv* env, jobject windowView)
{
    std::string windowName = AbilityRuntime::Platform::WindowViewAdapter::GetInstance()->GetWindowName(windowView);
    if (windowName.empty()) {
        LOGE("Window::Create called failed due to null windowName");
        return nullptr;
    }

    LOGI("Window::Create called. windowName=%s", windowName.c_str());

    auto window = std::shared_ptr<Window>(new Window(context), DummyWindowRelease);
    window->SetWindowView(env, windowView);
    window->SetWindowName(windowName);

    auto abilityContext =
        OHOS::AbilityRuntime::Platform::Context::ConvertTo<OHOS::AbilityRuntime::Platform::AbilityContext>(context);
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> info;
    CHECK_NULL_RETURN(abilityContext, nullptr);
    info = abilityContext->GetAbilityInfo();
    if (info) {
        LOGI("info->name = %s, info->instanceId = %d", info->name.c_str(), info->instanceId);
        window->SetWindowId(info->instanceId);
    }
    window->IncStrongRef(window.get());
    AddToWindowMap(window);
    return window;
}

std::shared_ptr<Window> Window::CreateSubWindow(
    std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, std::shared_ptr<OHOS::Rosen::WindowOption> option)
{
    if (option == nullptr) {
        LOGE("Window::CreateSubWindow called failed due to null option");
        return nullptr;
    }

    LOGI("Window::CreateSubWindow called. windowName=%s", option->GetWindowName().c_str());

    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    bool result = SubWindowManagerJni::CreateSubWindow(option);

    if (result) {
        jobject view = SubWindowManagerJni::GetContentView(option->GetWindowName());
        uint32_t windowId = SubWindowManagerJni::GetWindowId(option->GetWindowName());

        auto window = std::shared_ptr<Window>(new Window(context), DummyWindowRelease);
        window->SetWindowId(windowId);
        window->SetWindowName(option->GetWindowName());
        window->SetWindowType(option->GetWindowType());
        window->SetParentId(option->GetParentId());
        window->SetWindowMode(option->GetWindowMode());
        window->SetRect(option);
        window->IncStrongRef(window.get());
        AddToSubWindowMap(window);
        AddToWindowMap(window);

        window->SetSubWindowView(env, view);
        window->CreateSurfaceNode(view);
        LOGI("Window::CreateSubWindow: success");

        return window;
    }

    LOGI("Window::CreateSubWindow: failed");
    return nullptr;
}

WMError Window::Destroy()
{
    LOGI("Window::Destroy: %s", this->GetWindowName().c_str());
    if (uiContent_ != nullptr) {
        uiContent_->Destroy();
        uiContent_ = nullptr;
    }

    if (GetType() == OHOS::Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW && !GetWindowName().empty()) {
        SubWindowManagerJni::DestroyWindow(GetWindowName());
    }

    NotifyBeforeDestroy(GetWindowName());

    ClearListenersById(GetWindowId());

    // Remove subWindows of current window from subWindowMap_
    if (subWindowMap_.count(GetWindowId()) > 0) {
        auto& subWindows = subWindowMap_.at(GetWindowId());
        for (auto iter = subWindows.begin(); iter != subWindows.end(); iter = subWindows.begin()) {
            if ((*iter) == nullptr) {
                subWindows.erase(iter);
                continue;
            }

            auto windowPtr = (*iter);
            subWindows.erase(iter);
            LOGI("Window::Destroy SubWindow %s", (windowPtr)->GetWindowName().c_str());
            DeleteFromWindowMap(windowPtr);
            (windowPtr)->Destroy();
        }
        subWindowMap_[GetWindowId()].clear();
        subWindowMap_.erase(GetWindowId());
    }

    // Rmove current window from subWindowMap_ of parent window
    if (subWindowMap_.count(GetParentId()) > 0) {
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

    // Remove current window from windowMap_
    if (windowMap_.count(GetWindowName()) > 0) {
        DeleteFromWindowMap(this);
    }

    NotifyAfterBackground();
    return WMError::WM_OK;
}

void Window::RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func)
{
    LOGD("Start register");
    notifyNativefunc_ = std::move(func);
}

const std::vector<std::shared_ptr<Window>>& Window::GetSubWindow(uint32_t parentId)
{
    LOGI("Window::GetSubWindow called. parentId=%d", parentId);
    if (subWindowMap_.find(parentId) == subWindowMap_.end()) {
        return std::vector<std::shared_ptr<Window>>();
    }
    return subWindowMap_[parentId];
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

    auto iter = windowMap_.begin();
    while (iter != windowMap_.end()) {
        std::pair<uint32_t, std::shared_ptr<Window>> pair = iter->second;
        std::shared_ptr<Window> window = pair.second;

        if (window->isForground()) {
            LOGI("Window::GetTopWindow is forground: %s", window->GetWindowName().c_str());
            return window;
        }

        iter++;
    }

    LOGE("Window::GetTopWindow no window on forground");
    return nullptr;
}

WMError Window::ShowWindow()
{
    LOGI("Window::ShowWindow called.");
    if (this->GetWindowName().empty()) {
        LOGI("Window::ShowWindow called failed due to null option");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    bool result = SubWindowManagerJni::ShowWindow(this->GetWindowName());
    if (result) {
        NotifyAfterForeground();
        return WMError::WM_OK;
    } else {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

bool Window::IsWindowShow()
{
    if (this->GetWindowName().empty()) {
        LOGE("Window::IsWindowShow called failed due to null window name");
        return false;
    }
    return SubWindowManagerJni::IsWindowShowing(this->GetWindowName());
}

WMError Window::MoveWindowTo(int32_t x, int32_t y)
{
    LOGI("Window::MoveWindowTo called. x=%d, y=%d", x, y);
    if (this->GetWindowName().empty()) {
        LOGI("Window::MoveWindowTo called failed due to null option");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    rect_.posX_ = x;
    rect_.posY_ = y;

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

    if (this->GetWindowName().empty()) {
        LOGI("Window::ResizeWindowTo called failed due to null option");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    rect_.width_ = width;
    rect_.height_ = height;

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
    LOGI("Window::SetBackgroundColor called. color=%u", color);
    backgroundColor_ = color;

    if (uiContent_) {
        uiContent_->SetBackgroundColor(color);
    }
    return WMError::WM_OK;
}

WMError Window::SetBrightness(float brightness)
{
    LOGI("Window::SetBrightness called. brightness=%.3f", brightness);

    bool result = SubWindowManagerJni::SetAppScreenBrightness(brightness);

    brightness_ = brightness;
    if (result) {
        LOGI("Window::SetBrightness: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetBrightness: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}
WMError Window::SetKeepScreenOn(bool keepScreenOn)
{
    LOGI("Window::SetKeepScreenOn called. keepScreenOn=%d", keepScreenOn);

    bool result = SubWindowManagerJni::SetKeepScreenOn(keepScreenOn);

    if (result) {
        LOGI("Window::SetKeepScreenOn: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetKeepScreenOn: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

bool Window::IsKeepScreenOn()
{
    LOGI("Window::IsKeepScreenOn called.");

    return SubWindowManagerJni::IsKeepScreenOn();
}

WMError Window::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    LOGI("Window::SetSystemBarProperty called.");

    bool hide = !property.enable_;
    bool result = false;

    if (type == WindowType::WINDOW_TYPE_NAVIGATION_BAR) {
        if (hide) {
            result = SubWindowManagerJni::SetActionBarStatus(true);
        } else {
            result = SubWindowManagerJni::SetActionBarStatus(false);
        }

    } else if (type == WindowType::WINDOW_TYPE_STATUS_BAR) {
        if (hide) {
            result = SubWindowManagerJni::SetStatusBarStatus(true);
        } else {
            result = SubWindowManagerJni::SetStatusBarStatus(false);
        }
    }

    sysBarPropMap_[type] = property;

    if (result) {
        LOGI("Window::SetSystemBarProperty: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetSystemBarProperty: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

void Window::SetRequestedOrientation(Orientation orientation)
{
    LOGI("Window::SetRequestedOrientation called.");

    bool result = SubWindowManagerJni::RequestOrientation(orientation);

    if (result) {
        LOGI("Window::SetRequestedOrientation: success");
    } else {
        LOGI("Window::SetRequestedOrientation: failed");
    }
}

SystemBarProperty Window::GetSystemBarPropertyByType(WindowType type) const
{
    LOGI("Window::GetSystemBarPropertyByType called.");
    for (auto& it : sysBarPropMap_) {
        if (it.first == type) {
            return it.second;
        }
    }
}

void Window::ClearListenersById(uint32_t winId)
{
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    ClearUselessListeners(lifecycleListeners_, winId);
}

WMError Window::RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    LOGD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(lifecycleListeners_[GetWindowId()], listener);
}

WMError Window::UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    LOGD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(lifecycleListeners_[GetWindowId()], listener);
}

template<typename T>
WMError Window::RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        LOGE("listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (std::find(holder.begin(), holder.end(), listener) != holder.end()) {
        LOGE("Listener already registered");
        return WMError::WM_OK;
    }
    holder.emplace_back(listener);
    return WMError::WM_OK;
}

template<typename T>
WMError Window::UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        LOGE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    holder.erase(std::remove_if(holder.begin(), holder.end(),
                     [listener](sptr<T> registeredListener) { return registeredListener == listener; }),
        holder.end());
    return WMError::WM_OK;
}

void Window::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    // stage model
    if (receiver_) {
        SetUpThreadInfo();
        auto callback = [vsyncCallback](int64_t timestamp, void*) { vsyncCallback->onCallback(timestamp); };
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
    rect_.width_ = width;
    rect_.height_ = height;
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
        NotifyAfterActive();
        isForground_ = true;
    } else {
        LOGI("Window: notify uiContent UnFocus");
        uiContent_->UnFocus();
        NotifyAfterInactive();
        isForground_ = false;
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
    NotifyAfterForeground();
    isForground_ = true;
}

void Window::Background()
{
    if (!uiContent_) {
        LOGW("Window::Background uiContent_ is nullptr");
        return;
    }
    LOGI("Window: notify uiContent Background");
    uiContent_->Background();
    NotifyAfterBackground();
    isForground_ = false;
}

bool Window::ProcessBackPressed()
{
    if (!uiContent_) {
        LOGW("Window::ProcessBackPressed uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessBackPressed();
}

bool Window::ProcessBasicEvent(const std::vector<Ace::TouchEvent>& touchEvents)
{
    if (!uiContent_) {
        LOGW("Window::ProcessBasicEvent uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessBasicEvent(touchEvents);
}

bool Window::ProcessPointerEvent(const std::vector<uint8_t>& data)
{
    if (!uiContent_) {
        LOGW("Window::ProcessPointerEvent uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessPointerEvent(data);
}

bool Window::ProcessKeyEvent(
    int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp, int64_t timeStampStart)
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
        LOGI("Window Delay Notify uiContent_ Surface Changed wh:[%{public}d, %{public}d]", surfaceWidth_,
            surfaceHeight_);
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

int Window::SetUIContent(const std::string& contentInfo, NativeEngine* engine, NativeValue* storage, bool isdistributed,
    AbilityRuntime::Platform::Ability* ability)
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

void Window::SetUpThreadInfo()
{
    static int32_t renderTid = -1;
    if (renderTid < 0) {
        int32_t tid = -1;
        RSRenderThread::Instance().PostSyncTask([&tid]() { tid = gettid(); });
        renderTid = tid;
        bool ret = AceEnvJni::SetThreadInfo(renderTid);
        LOGI("Window::SetUpThreadInfo tid:%{public}d ret:%{public}d.", renderTid, ret);
    }
}

} // namespace OHOS::Rosen
