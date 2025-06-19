/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <map>
#include <memory>

#include "foundation/appframework/window_manager/interfaces/innerkits/wm/window_interface.h"
#include "foundation/appframework/window_manager/interfaces/innerkits/wm/window_option.h"
#include "refbase.h"
#include "render_service_client/core/ui/rs_surface_node.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/entrance/java/jni/window_view_jni.h"
#include "base/log/log.h"
#include "base/utils/noncopyable.h"
#include "core/event/touch_event.h"

class NativeEngine;
typedef struct napi_value__* napi_value;

namespace OHOS {
namespace AbilityRuntime::Platform {
class Context;
class Configuration;
class Ability;
} // namespace AbilityRuntime::Platform

namespace Ace::Platform {
class UIContent;
}

namespace AppExecFwk {
class EventHandler;
}

namespace Rosen {
class IWindowLifeCycle;
class WindowOption;
class RSUIContext;
class RSUIDirector;
using NotifyNativeWinDestroyFunc = std::function<void(std::string windowName)>;
using OnCallback = std::function<void(int64_t, int64_t)>;
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

/**
 * @brief Enumerates occupied area type.
 */
enum class OccupiedAreaType : uint32_t {
    TYPE_INPUT, // area of input window
};

/**
 * @class IOccupiedAreaChangeListener
 *
 * @brief IOccupiedAreaChangeListener is used to observe OccupiedArea change.
 */
class IOccupiedAreaChangeListener : public RefBase {
public:
    /**
     * @brief Notify caller when OccupiedArea size change.
     *
     * @param info Occupied area info when occupied changed.
     */
    virtual void OnSizeChange(const Rect& rect, OccupiedAreaType type) {}
};

class Window : public RefBase {
public:
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static std::shared_ptr<Window> Create(
        std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, JNIEnv* env, jobject windowView);
    static std::shared_ptr<Window> CreateSubWindow(std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context,
        std::shared_ptr<OHOS::Rosen::WindowOption> option);
    static std::shared_ptr<Window> CreateDragWindow(std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context);

    explicit Window(std::shared_ptr<AbilityRuntime::Platform::Context> context);
    explicit Window(std::shared_ptr<AbilityRuntime::Platform::Context> context, uint32_t windowId);
    ~Window() override;

    static std::vector<std::shared_ptr<Window>> GetSubWindow(uint32_t parentId);
    static std::shared_ptr<Window> FindWindow(const std::string& name);
    static std::shared_ptr<Window> GetTopWindow(
        const std::shared_ptr<OHOS::AbilityRuntime::Platform::Context>& context);

    WMError ShowWindow();
    WMError MoveWindowTo(int32_t x, int32_t y);
    WMError ResizeWindowTo(int32_t width, int32_t height);
    bool IsWindowShow();

    bool CreateVSyncReceiver(std::shared_ptr<AppExecFwk::EventHandler> handler);
    void RequestNextVsync(std::function<void(int64_t, void*)> callback);

    virtual void FlushFrameRate(int32_t rate, bool isAnimatorStopped, int32_t rateType) {}
    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback);

    virtual void SetUiDvsyncSwitch(bool dvsyncSwitch) {}

    virtual void NotifyExtensionTimeout(int32_t errorCode)  {}

    virtual void NotifySnapshotUpdate() {}

    virtual uint32_t GetStatusBarHeight() const
    {
        return 0;
    }

    virtual void FlushLayoutSize(int32_t width, int32_t height) {}

    void CreateSurfaceNode(void* nativeWindow);
    void NotifySurfaceChanged(int32_t width, int32_t height, float density);
    void NotifyKeyboardHeightChanged(int32_t height);
    void NotifySizeChange(Rect rect);
    void NotifySurfaceDestroyed();
    void NotifyTouchOutside();
    void NotifyAvoidAreaChange(const std::shared_ptr<AvoidArea>& avoidArea, AvoidAreaType type);
    void AvoidAreaChange();
    bool IsTouchingBottom(const Rect& subWindowRect, const Rect& safeAreaRect);
    bool IsTouchingRight(const Rect& subWindowRect, const Rect& safeAreaRect);
    void GetAvoidAreaForSubWindow(const Rect& subWindowRect, const Rect& safeAreaRect, AvoidArea& avoidArea);
    void UpdateAvoidArea(const std::shared_ptr<Rosen::AvoidArea>& avoidArea, AvoidAreaType type);
    void SubWindowHide();

    void WindowFocusChanged(bool hasWindowFocus);
    void Foreground();
    void Background();
    WMError Destroy();
    void RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func);
    WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener);
    WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener);

    bool IsSubWindow() const
    {
        return windowType_  == OHOS::Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    }
    // event process
    bool ProcessBackPressed();
    bool ProcessBasicEvent(const std::vector<Ace::TouchEvent>& touchEvents);
    bool ProcessPointerEvent(const std::vector<uint8_t>& data);
    bool ProcessMouseEvent(const std::vector<uint8_t>& data);
    bool ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp = 0,
        int64_t timeStampStart = 0, int32_t source = 0, int32_t deviceId = 0, int32_t metaKey = 0);

    WMError SetUIContent(const std::string& contentInfo, NativeEngine* engine, napi_value storage, bool isdistributed,
        AbilityRuntime::Platform::Ability* ability, bool loadContentByName);

    Ace::Platform::UIContent* GetUIContent();

    WMError SetBackgroundColor(uint32_t color);
    uint32_t GetBackgroundColor() const
    {
        return backgroundColor_;
    }
    WMError SetBrightness(float brightness);

    WMError SetColorSpace(ColorSpace colorSpace);
    ColorSpace GetColorSpace() const;

    float GetBrightness() const
    {
        return brightness_;
    }
    WMError SetKeepScreenOn(bool keepScreenOn);
    bool IsKeepScreenOn();
    WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property);
    SystemBarProperty GetSystemBarPropertyByType(WindowType type) const;
    void SetRequestedOrientation(Orientation);
    WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener);
    WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener);
    WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener);
    WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener);
    WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener);
    WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener);
    WMError RegisterSurfaceNodeListener(const sptr<IWindowSurfaceNodeListener>& listener);
    WMError UnregisterSurfaceNodeListener(const sptr<IWindowSurfaceNodeListener>& listener);
    WMError RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener);
    WMError UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener);
    WMError RegisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener);
    WMError UnregisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener);

    WMError SetLayoutFullScreen(bool status);
    WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property);
    WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea);
    WMError SetWindowPrivacyMode(bool isPrivacyMode);
    WMError UpdateSystemBarProperties(const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
        const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags);

    void SetRect(std::shared_ptr<WindowOption> option)
    {
        rect_.width_ = option->GetWindowRect().width_;
        rect_.height_ = option->GetWindowRect().height_;
        rect_.posX_ = option->GetWindowRect().posX_;
        rect_.posY_ = option->GetWindowRect().posY_;
    }

    void SetWindowId(uint32_t windowId)
    {
        windowId_ = windowId;
    }

    uint32_t GetWindowId()
    {
        return windowId_;
    }

    void SetWindowName(const std::string& windowName)
    {
        name_ = windowName;
    }

    void SetWindowType(WindowType windowType)
    {
        windowType_ = windowType;
    }

    void SetParentId(uint32_t parentId)
    {
        parentId_ = parentId;
    }

    void SetWindowMode(WindowMode mode)
    {
        windowMode_ = mode;
    }

    // @override
    const std::string& GetWindowName()
    {
        return name_;
    }

    // @override
    WindowType GetType()
    {
        return windowType_;
    }

    // @override
    uint32_t GetParentId()
    {
        return parentId_;
    }
    // @override
    Rect GetRect()
    {
        return rect_;
    }
    // @override
    WindowMode GetMode()
    {
        return windowMode_;
    }

    WindowState GetWindowState()
    {
        return state_;
    }

    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const
    {
        return surfaceNode_;
    }

    bool isForground()
    {
        return isForground_;
    }

    void UpdateConfiguration(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config);

    int64_t GetVSyncPeriod()
    {
        return static_cast<int64_t>(1000000000.0f / 60); // SyncPeriod of 60 fps
    }

    void SetUpThreadInfo();

    const std::shared_ptr<OHOS::AbilityRuntime::Platform::Context>GetContext() const
    {
        return context_;
    }

    float GetDensity()
    {
        return density_;
    }

    WMError Hide();
    WMError SetFocusable(bool isFocusable);
    WMError SetTouchable(bool isTouchable);
    bool RequestFocus();
    bool IsFocused();
    WMError SetTouchHotAreas(const std::vector<Rect>& rects);
    WMError SetFullScreen(bool status);
    WMError SetOnTop(bool status);

    /**
     * @brief Get the associated RSUIDirector instance
     *
     * @return std::shared_ptr<RSUIDirector> Shared pointer to the RSUIDirector instance,
     *         or nullptr if RS client multi-instance is disabled.
     */
    virtual std::shared_ptr<RSUIDirector> GetRSUIDirector() const { return nullptr; }

    /**
     * @brief Get the associated RSUIContext instance
     *
     * @return std::shared_ptr<RSUIContext> Shared pointer to the RSUIContext instance,
     *         or nullptr if RS client multi-instance is disabled.
     */
    virtual std::shared_ptr<RSUIContext> GetRSUIContext() const { return nullptr; }

private:
    void SetWindowView(JNIEnv* env, jobject windowView);
    void SetSubWindowView(JNIEnv* env, jobject windowView);
    void ReleaseWindowView();

    void DelayNotifyUIContentIfNeeded();

    std::string name_ = "";
    std::shared_ptr<jobject> viewController_ = nullptr;
    uint32_t windowId_ = 0;
    uint32_t parentId_ = 0;
    WindowMode windowMode_;
    WindowType windowType_;
    Rect rect_ = { 0, 0, 0, 0 };
    bool isForground_ = false;
    bool isFocused_ = false;
    bool isFullScreen_ = false;

    uint32_t backgroundColor_ = 0xffffffff;
    float brightness_;
    std::unordered_map<WindowType, SystemBarProperty> sysBarPropMap_ {
        { WindowType::WINDOW_TYPE_STATUS_BAR, SystemBarProperty() },
        { WindowType::WINDOW_TYPE_NAVIGATION_BAR, SystemBarProperty() },
        { WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, SystemBarProperty() },
    };
    WindowState state_ { WindowState::STATE_INITIAL };
    std::map<AvoidAreaType, AvoidArea> avoidAreaMap_;
    static void AvoidAreaMapInit(std::shared_ptr<Window> window);
    static std::map<uint32_t, std::vector<std::shared_ptr<Window>>> subWindowMap_;
    static std::map<std::string, std::pair<uint32_t, std::shared_ptr<Window>>> windowMap_;
    static void AddToWindowMap(std::shared_ptr<Window> window);
    static void DeleteFromWindowMap(std::shared_ptr<Window> window);
    static void DeleteFromWindowMap(Window* window);
    static void AddToSubWindowMap(std::shared_ptr<Window> window);
    static void DeleteFromSubWindowMap(std::shared_ptr<Window> window);
    GraphicColorGamut GetSurfaceGamutFromColorSpace(ColorSpace colorSpace) const;
    ColorSpace GetColorSpaceFromSurfaceGamut(GraphicColorGamut colorGamut) const;
    Rect GetTopRect(const Rect& safeAreaRect, const Rect& surfaceRect);
    Rect GetLeftRect(const Rect& safeAreaRect, const Rect& surfaceRect);
    Rect GetRightRect(const Rect& safeAreaRect, const Rect& surfaceRect);
    Rect GetBottomRect(const Rect& safeAreaRect, const Rect& surfaceRect);
    void GetCutoutRect(const Rect& safeAreaRect, const Rect& surfaceRect, AvoidArea& avoidArea);

    int32_t surfaceWidth_ = 0;
    int32_t surfaceHeight_ = 0;
    float density_ = 3.0f;
    std::shared_ptr<RSSurfaceNode> surfaceNode_;

    jobject windowView_ = nullptr;
    std::shared_ptr<AbilityRuntime::Platform::Context> context_;
    std::unique_ptr<OHOS::Ace::Platform::UIContent> uiContent_;

    std::shared_ptr<VSyncReceiver> receiver_ = nullptr;

    bool delayNotifySurfaceCreated_ = false;
    bool delayNotifySurfaceChanged_ = false;
    bool delayNotifySurfaceDestroyed_ = false;
    NotifyNativeWinDestroyFunc notifyNativefunc_;
    static std::map<uint32_t, std::vector<sptr<IOccupiedAreaChangeListener>>> occupiedAreaChangeListeners_;
    //////////WindowLifeCycle Listeners////////////////////

    static std::recursive_mutex globalMutex_;
    static std::map<uint32_t, std::vector<sptr<IWindowLifeCycle>>> lifecycleListeners_;
    static std::map<uint32_t, std::vector<sptr<IWindowChangeListener>>> windowChangeListeners_;
    static std::map<uint32_t, std::vector<sptr<ITouchOutsideListener>>> touchOutsideListeners_;
    static std::map<uint32_t, std::vector<sptr<IWindowSurfaceNodeListener>>> surfaceNodeListeners_;
    static std::map<uint32_t, std::vector<sptr<IAvoidAreaChangedListener>>> avoidAreaChangeListeners_;
    static std::map<uint32_t, std::vector<sptr<IWindowStatusChangeListener>>> windowStatusChangeListeners_;

    template<typename T1, typename T2, typename Ret>
    using EnableIfSame = typename std::enable_if<std::is_same_v<T1, T2>, Ret>::type;
    template<typename T>
    WMError RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T>
    WMError UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T>
    void ClearUselessListeners(std::map<uint32_t, T>& listeners, uint32_t winId)
    {
        listeners.erase(winId);
    }

    template<typename T>
    inline EnableIfSame<T, IOccupiedAreaChangeListener, std::vector<sptr<IOccupiedAreaChangeListener>>> GetListeners()
    {
        std::vector<sptr<IOccupiedAreaChangeListener>> occupiedAreaChangeListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(globalMutex_);
            for (auto& listener : occupiedAreaChangeListeners_[GetWindowId()]) {
                occupiedAreaChangeListeners.push_back(listener);
            }
        }
        return occupiedAreaChangeListeners;
    }

    template<typename T>
    inline EnableIfSame<T, IWindowLifeCycle, std::vector<wptr<IWindowLifeCycle>>> GetListeners()
    {
        std::vector<wptr<IWindowLifeCycle>> lifecycleListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(globalMutex_);
            for (auto& listener : lifecycleListeners_[GetWindowId()]) {
                lifecycleListeners.push_back(listener);
            }
        }
        return lifecycleListeners;
    }

    template<typename T>
    inline EnableIfSame<T, IWindowChangeListener, std::vector<sptr<IWindowChangeListener>>> GetListeners()
    {
        std::vector<sptr<IWindowChangeListener>> windowChangeListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(globalMutex_);
            for (auto& listener : windowChangeListeners_[GetWindowId()]) {
                windowChangeListeners.push_back(listener);
            }
        }
        return windowChangeListeners;
    }

    template<typename T>
    inline EnableIfSame<T, ITouchOutsideListener, std::vector<wptr<ITouchOutsideListener>>> GetListeners()
    {
        std::vector<wptr<ITouchOutsideListener>> touchOutsideListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(globalMutex_);
            for (auto& listener : touchOutsideListeners_[GetWindowId()]) {
                touchOutsideListeners.push_back(listener);
            }
        }
        return touchOutsideListeners;
    }

    template<typename T>
    inline EnableIfSame<T, IWindowSurfaceNodeListener, std::vector<wptr<IWindowSurfaceNodeListener>>> GetListeners()
    {
        std::vector<wptr<IWindowSurfaceNodeListener>> surfaceNodeListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(globalMutex_);
            for (auto& listener : surfaceNodeListeners_[GetWindowId()]) {
                surfaceNodeListeners.push_back(listener);
            }
        }
        return surfaceNodeListeners;
    }

    template<typename T>
    inline EnableIfSame<T, IAvoidAreaChangedListener, std::vector<sptr<IAvoidAreaChangedListener>>> GetListeners()
    {
        std::vector<sptr<IAvoidAreaChangedListener>> avoidAreaChangeListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(globalMutex_);
            for (auto& listener : avoidAreaChangeListeners_[GetWindowId()]) {
                avoidAreaChangeListeners.push_back(listener);
            }
        }
        return avoidAreaChangeListeners;
    }

    template<typename T>
    inline EnableIfSame<T, IWindowStatusChangeListener, std::vector<sptr<IWindowStatusChangeListener>>> GetListeners()
    {
        std::vector<sptr<IWindowStatusChangeListener>> windowStatusChangeListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(globalMutex_);
            for (auto& listener : windowStatusChangeListeners_[GetWindowId()]) {
                windowStatusChangeListeners.push_back(listener);
            }
        }
        return windowStatusChangeListeners;
    }

#define CALL_LIFECYCLE_LISTENER(windowLifecycleCb)                  \
    do {                                                            \
        auto lifecycleListeners = GetListeners<IWindowLifeCycle>(); \
        for (auto& listener : (lifecycleListeners)) {               \
            LOGI("Window: notify listener");                        \
            if (listener.GetRefPtr() != nullptr) {                  \
                listener.GetRefPtr()->windowLifecycleCb();          \
                LOGI("Window: notify listener not nullptr");        \
            }                                                       \
        }                                                           \
    } while (0)

    inline void NotifyAfterForeground(bool needNotifyListeners = true, bool needNotifyUiContent = true)
    {
        if (needNotifyListeners) {
            CALL_LIFECYCLE_LISTENER(AfterForeground);
        }
    }

    inline void NotifyAfterBackground(bool needNotifyListeners = true, bool needNotifyUiContent = true)
    {
        if (needNotifyListeners) {
            CALL_LIFECYCLE_LISTENER(AfterBackground);
        }
    }

    inline void NotifyAfterActive()
    {
        CALL_LIFECYCLE_LISTENER(AfterFocused);
    }

    inline void NotifyAfterInactive()
    {
        CALL_LIFECYCLE_LISTENER(AfterUnfocused);
    }

    inline void NotifyBeforeDestroy(std::string windowName)
    {
        std::lock_guard<std::recursive_mutex> lock(globalMutex_);
        if (notifyNativefunc_) {
            notifyNativefunc_(windowName);
        }
    }
    void ClearListenersById(uint32_t winId);

    DISALLOW_COPY_AND_MOVE(Window);
};

} // namespace Rosen
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H
