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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H

#include <memory>

#include "flutter/shell/common/vsync_waiter.h"
#include "refbase.h"
#include "render_service_client/core/ui/rs_surface_node.h"

#include "adapter/android/entrance/java/jni/window_view_jni.h"
#include "base/log/log.h"
#include "base/utils/noncopyable.h"
#include "foundation/appframework/window_manager/interfaces/innerkits/wm/window_option.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"

class NativeValue;
class NativeEngine;

namespace OHOS {
namespace AbilityRuntime::Platform {
class Context;
class Configuration;
class Ability;
}

namespace Ace::Platform {
class UIContent;
}

namespace AppExecFwk {
class EventHandler;
}

namespace Rosen {
class IWindowLifeCycle;
class WindowOption;
using OnCallback = std::function<void(int64_t)>;
struct VsyncCallback {
    OnCallback onCallback;
};
class VSyncReceiver;

enum class WindowSizeChangeReason : uint32_t {
    UNDEFINED = 0,
    MAXIMIZE,
    RECOVER,
    ROTATION,
    DRAG,
    DRAG_START,
    DRAG_END,
    RESIZE,
    MOVE,
    HIDE,
    TRANSFORM,
    CUSTOM_ANIMATION_SHOW,
    FULL_TO_SPLIT,
    SPLIT_TO_FULL,
    END,
};

class Window : public RefBase {
public:
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static std::shared_ptr<Window> Create(
        std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, JNIEnv* env, jobject windowView);
    static std::shared_ptr<Window> CreateSubWindow(
        std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context,
        std::shared_ptr<OHOS::Rosen::WindowOption> option);

    explicit Window(const flutter::TaskRunners& taskRunners);
    explicit Window(std::shared_ptr<AbilityRuntime::Platform::Context> context);
    explicit Window(std::shared_ptr<AbilityRuntime::Platform::Context> context, uint32_t windowId);
    ~Window() override;

    static std::vector<std::shared_ptr<Window>> GetSubWindow(uint32_t parentId);
    static std::shared_ptr<Window> FindWindow(const std::string& name);
    static std::shared_ptr<Window> GetTopWindow(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Context>& context);

    WMError ShowWindow();
    WMError DestroyWindow();
    WMError MoveWindowTo(int32_t x, int32_t y);
    WMError ResizeWindowTo(int32_t width, int32_t height);

    bool CreateVSyncReceiver(std::shared_ptr<AppExecFwk::EventHandler> handler);
    void RequestNextVsync(std::function<void(int64_t, void*)> callback);

    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback);

    void CreateSurfaceNode(void* nativeWindow);
    void NotifySurfaceChanged(int32_t width, int32_t height, float density);
    void NotifySurfaceDestroyed();

    void WindowFocusChanged(bool hasWindowFocus);
    void Foreground();
    void Background();
    void Destroy();

    // event process
    bool ProcessBackPressed();
    bool ProcessPointerEvent(const std::vector<uint8_t>& data);
    bool ProcessKeyEvent(
        int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp = 0, int64_t timeStampStart = 0);

    int SetUIContent(const std::string& contentInfo, NativeEngine* engine,
        NativeValue* storage, bool isdistributed, AbilityRuntime::Platform::Ability* ability);

    WMError SetBackgroundColor(uint32_t color);
    uint32_t GetBackgroundColor() const
    {
        return backgroundColor_;
    }
    WMError SetBrightness(float brightness);
    float GetBrightness() const
    {
        return 0;
    }
    WMError SetKeepScreenOn(bool keepScreenOn);
    bool IsKeepScreenOn();
    WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property);
    SystemBarProperty GetSystemBarPropertyByType(WindowType type) const;
    void SetRequestedOrientation(Orientation);
    WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener);
    WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener);

    void SetWindowOption(std::shared_ptr<WindowOption> option) {
        option_ = option;
    }

    std::shared_ptr<WindowOption> GetWindowOption()
    {
        return option_;
    }

    void SetWindowId(uint32_t windowId)
    {
        windowId_ = windowId;
    }

    uint32_t GetWindowId()
    {
        return windowId_;
    }

    bool IsWindowShow()
    {
        return isWindowShow_;
    }

    // @override
    const std::string& GetWindowName()
    {
        return option_->GetWindowName();
    }

    // @override
    WindowType GetType()
    {
        return option_->GetWindowType();
    }

    // @override
    uint32_t GetParentId()
    {
        return  option_->GetParentId();
    }
    // @override
    Rect GetRect()
    {
        return option_->GetWindowRect();
    }
    // @override
    WindowMode GetMode()
    {
        return option_->GetWindowMode();
    }

    WindowState GetWindowState()
    {
        return state_;
    }

    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const
    {
        return surfaceNode_;
    }

    void UpdateConfiguration(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config);

private:
    void SetWindowView(JNIEnv* env, jobject windowView);
    void SetSubWindowView(JNIEnv* env, jobject windowView);
    void ReleaseWindowView();

    void DelayNotifyUIContentIfNeeded();

    bool isWindowShow_ = false;
    std::shared_ptr<jobject> viewController_ = nullptr;
    uint32_t windowId_ = 0;
    std::shared_ptr<WindowOption> option_;

    uint32_t backgroundColor_;
    SystemBarProperty property_;
    WindowState state_ { WindowState::STATE_INITIAL };
    static std::shared_ptr<Window> mainWindow_;
    static std::map<uint32_t, std::vector<std::shared_ptr<Window>>> subWindowMap_;
    static std::map<std::string, std::pair<uint32_t, std::shared_ptr<Window>>> windowMap_;

    int32_t surfaceWidth_ = 0;
    int32_t surfaceHeight_ = 0;
    float density_ = 3.0f;
    std::shared_ptr<RSSurfaceNode> surfaceNode_;
    std::shared_ptr<flutter::VsyncWaiter> vsyncWaiter_;

    jobject windowView_ = nullptr;
    std::shared_ptr<AbilityRuntime::Platform::Context> context_;
    std::unique_ptr<OHOS::Ace::Platform::UIContent> uiContent_;

    std::shared_ptr<VSyncReceiver> receiver_ = nullptr;

    bool delayNotifySurfaceCreated_ = false;
    bool delayNotifySurfaceChanged_ = false;
    bool delayNotifySurfaceDestroyed_ = false;

    DISALLOW_COPY_AND_MOVE(Window);
};

} // namespace Rosen
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H
